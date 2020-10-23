/****************************************************************************
 **
 **  Name        gki_linux.c
 **
 **  Function    pthreads version of Linux GKI. This version is used for
 **              settop projects that already use pthreads and not pth.
 **
 **  Copyright (c) 2000-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <pthread.h>  /* must be 1st header defined */
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/times.h>
#include <unistd.h>
#include "gki_int.h"
#include "bt_trace.h"
#include "uipc.h"
#include "uipc_fifo.h"
#if defined (NSA) && (NSA == TRUE)
#include "nsa_task.h"
#else
#include "bsa_task.h"
#endif
#include "uipc_sv_socket.h"
#include "uipc_rb.h"

#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
#include "upio.h"
#ifndef HCILP_BT_POWER_GPIO
#define HCILP_BT_POWER_GPIO UPIO_GENERAL3
#endif
#endif

#if defined(__FreeBSD__)
extern int pthread_mutexattr_settype(pthread_mutexattr_t *__attr, int __kind)
__attribute((nonnull (1)));
extern char *strsignal (int __sig);
#else
extern int pthread_mutexattr_settype(pthread_mutexattr_t *__attr, int __kind)
__THROW __attribute((nonnull (1)));
extern char *strsignal (int __sig) __THROW;
#endif

/* Define the structure that holds the GKI variables
 */
#if GKI_DYNAMIC_MEMORY == FALSE
tGKI_CB gki_cb;
#endif

#if (defined(GKI_SYSTEM_TIMESTAMP) && (GKI_SYSTEM_TIMESTAMP == TRUE))
#else
static UINT32 GKI_InitialTime;
#endif

#define LOCK(m) pthread_mutex_lock(&(m))
#define UNLOCK(m) pthread_mutex_unlock(&(m))
#define INIT(m) pthread_mutex_init(&(m), NULL)

typedef void *(*PTHREAD_START_ROUTINE)(void*);

#if (GKI_DEBUG == TRUE)
static pthread_mutex_t GKI_trace_mutex;
#endif

#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
#if defined(__FreeBSD__)
#define pthread_setname_np(t, n)  pthread_set_name_np((t), (n))
#endif
int __attribute__((weak)) pthread_setname_np(pthread_t thread, const char *name);
#define BSA_GKI_THREAD_NAME_LEN_MAX     16      /* Pthread Limitation */
#ifndef BSA_GKI_THREAD_NAME_PREFIX
#define BSA_GKI_THREAD_NAME_PREFIX  "BSA_Srv:"  /* Pthread Name Prefix */
#endif
#endif

/*
 * Variable used to enable/disable system tick generation.
 * See GKI_run and timer_control_cback for comment.
 * Note that the GKI_timer_mutex is not used to protect GKI_timer_enable access
 */
static BOOLEAN GKI_timer_enable = TRUE;
static pthread_mutex_t GKI_timer_mutex;

volatile sig_atomic_t termination_in_progress = 0;

void timer_control_cback(BOOLEAN enable);
void gki_linux_signal_handler(int sig);
static void gki_timer_update_thread(void);

/*******************************************************************************
 **
 ** Function        GKI_TIMESPEC_ADD
 **
 ** Description     Add 2 timespec structures together
 **
 ** Returns         The summed timespec structure in the first structure
 **
 *******************************************************************************/
#define GKI_TIMESPEC_ADD(__t1, __t2)                                            \
    do {                                                                        \
        __t1.tv_sec += __t2.tv_sec;                                             \
        __t1.tv_nsec += __t2.tv_nsec;                                           \
        if (BCM_UNLIKELY(__t1.tv_nsec >= 1000000000L)) { /* Carry ? */          \
            __t1.tv_sec++;                                                      \
            __t1.tv_nsec -= 1000000000L;                                        \
        }                                                                       \
    } while (0)

/*******************************************************************************
 **
 ** Function        GKI_TRACE
 **
 ** Description .
 **
 ** Returns     void
 **
 *******************************************************************************/
void GKI_TRACE(char *fmt, ...)
{
#if (GKI_DEBUG == TRUE)
    LOCK(GKI_trace_mutex);
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);
    UNLOCK(GKI_trace_mutex);
#endif
}

/*******************************************************************************
 **
 ** Function        GKI_ERROR_TRACE
 **
 ** Description .
 **
 ** Returns     void
 **
 *******************************************************************************/
void GKI_ERROR_TRACE(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}


/*******************************************************************************
 **
 ** Function         gki_task_entry
 **
 ** Description      Function that stops started thread until GKI_create_task
 **                  completion
 **
 ** Returns          void
 **
 *******************************************************************************/
void *gki_task_entry(void *arg)
{
    tOS_THREAD *p_thread = arg;
    
    /* wait for the thread creation process to complete */
    pthread_mutex_lock(&p_thread->evt_mutex);
    pthread_mutex_unlock(&p_thread->evt_mutex);

    p_thread->start(0);
    
    return NULL;
}

/*******************************************************************************
 **
 ** Function         GKI_init
 **
 ** Description      This function is called once at startup to initialize
 **                  all the timer structures.
 **
 ** Returns          void
 **
 *******************************************************************************/
void GKI_init(void)
{
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;
    int task_id;
    tOS_THREAD *p_thread;


#if !(defined(GKI_SIGNAL_HANDLER_EXCLUDED) && (GKI_SIGNAL_HANDLER_EXCLUDED == TRUE))
    int index;
#endif

    memset((void *) &gki_cb, 0, sizeof(gki_cb));

    gki_buffer_init();
    gki_timers_init();

#if (defined(GKI_SYSTEM_TIMESTAMP) && (GKI_SYSTEM_TIMESTAMP == TRUE))
#else
    {
        struct tms tms;
        clock_t current_time;
        current_time = times(&tms);
        if (current_time < 0)
        {
            GKI_ERROR_TRACE("error in getting ticks");
            current_time = 0;
        }
        GKI_InitialTime = (UINT32)current_time;
    }
#endif

    pthread_mutexattr_init(&mutexattr);

#if defined(__CYGWIN__) || defined(__FreeBSD__)
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
#else
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
    GKI_TRACE("pthread_mutexattr_settype");

    pthread_mutex_init(&gki_cb.os.mutex, &mutexattr);

#if (GKI_DEBUG == TRUE)
    pthread_mutex_init(&GKI_trace_mutex, NULL);
#endif

#ifdef  __CYGWIN__
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&GKI_timer_mutex, &mutexattr);
#else
    pthread_mutex_init(&GKI_timer_mutex, NULL);
#endif
    GKI_TRACE("pthread_mutex_init GKI_timer_mutex");

    /* Initialize mutex and condition variable objects for events and timeouts */
    GKI_TRACE("initializing mutex and cond variables");
    pthread_condattr_init(&condattr);
#if ((!defined(__CYGWIN__)) && (!defined(PTHREAD_CONDATTR_SETCLOCK_UNDEFINED)))
    /* Cygwin does not implement pthread_condattr_setclock;
     * some toolchains may not implement it as well (add -DPTHREAD_CONDATTR_SETCLOCK_UNDEFINED for them) */
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
#endif
    for (task_id = 0; task_id < GKI_MAX_TASKS; task_id++)
    {
        p_thread = &gki_cb.os.thread[task_id];
        pthread_mutex_init(&p_thread->evt_mutex, NULL);
        pthread_cond_init(&p_thread->evt_cond, &condattr);
    }

   /* Register function used to start/stop timer management */
    GKI_timer_queue_register_callback(timer_control_cback);

    termination_in_progress = 0;

#if !(defined(GKI_SIGNAL_HANDLER_EXCLUDED) && (GKI_SIGNAL_HANDLER_EXCLUDED == TRUE))
    for(index = SIGHUP; index <= SIGTTOU ; index++)
    {
        signal(index, gki_linux_signal_handler);
    }
#endif

    GKI_TRACE("exiting GKI_Init");
}

/*******************************************************************************
 **
 ** Function         gki_timer_update_thread
 **
 ** Description      This function is called to update the timers
 **
 ** Returns          void
 **
 *******************************************************************************/
static void gki_timer_update_thread(void)
{
    struct timespec timeout;
    struct timespec current;
    int err;
    int restart;

#if BSA_SET_SCHED_PRIORITY
    gki_set_sched_priority(BSA_TASK_SCHED_PRIORITY, SCHED_RR);
    gki_check_sched_priority();
#endif

    /* Indicate that tick is just starting */
    restart = 1;

    /* Configure the tick timeout */
    timeout.tv_sec = 0;
    timeout.tv_nsec = GKI_TICKS_TO_MS(1) * 1000000;
    for (;;)
    {
        /* If the timer has been stopped (no SW timer running) */
        if (GKI_timer_enable == FALSE)
        {
            /*
             * We will lock/wait on GKI_timer_mutex.
             * This mutex will be unlocked when timer is re-started
             */
            GKI_TRACE("GKI_run lock mutex");
            pthread_mutex_lock(&GKI_timer_mutex);

            /* We are here because the mutex has been released by timer cback */
            /* Let's release it for future use */
            GKI_TRACE("GKI_run unlock mutex");
            pthread_mutex_unlock(&GKI_timer_mutex);

            /* Indicate that tick is just starting */
            restart = 1;
        }

        /* Check if tick was just restarted, indicating to the compiler that this is
         * unlikely to happen (to help branch prediction) */
        if (BCM_UNLIKELY(restart))
        {
            /* Clear the restart indication */
            restart = 0;

            /* Get time */
            clock_gettime(CLOCK_MONOTONIC, &current);
        }

        /* Add the tick to the current time to compute next wakeup time */
        GKI_TIMESPEC_ADD(current, timeout);
        do
        {
            err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);
        } while (err < 0 && errno == EINTR);

        /* Increment the GKI time value by one tick and update internal timers */
        GKI_timer_update(1);

    }
    GKI_TRACE("exit  gki_timer_update_thread");
    return;

}
/*******************************************************************************
 **
 ** Function         GKI_linux_signal_handler
 **
 ** Description      This function is called whenever a system signal has been
 **                  raised by the OS.
 **                  The signals causing the call of this function by the OS must
 **                  be registered through system function the signal()
 **
 ** Returns          void
 **
 *******************************************************************************/
void gki_linux_signal_handler(int sig)
{
    switch(sig)
    {
        case SIGPIPE:
            BT_TRACE_0(TRACE_LAYER_GKI | TRACE_CTRL_GENERAL | TRACE_ORG_GKI, TRACE_TYPE_ERROR,
                       "Signal SIGPIPE has been raised by OS");
            BT_TRACE_0(TRACE_LAYER_GKI | TRACE_CTRL_GENERAL | TRACE_ORG_GKI, TRACE_TYPE_ERROR,
                       "UIPC channel not opened at client side");
            break;

        /* SIGINT corresponds to CTRL-C user keypress */
        default:
            /* restore default handler */
            signal(sig, SIG_DFL);

            /* check if termination is already in progress */
            if (termination_in_progress)
            {
                BT_TRACE_2(TRACE_LAYER_GKI | TRACE_CTRL_GENERAL | TRACE_ORG_GKI, TRACE_TYPE_ERROR,
                           "Default handler - Signal %s [%d] has been raised", strsignal(sig), sig);
                raise(sig);
                break;
            }
            termination_in_progress = 1;

            BT_TRACE_2(TRACE_LAYER_GKI | TRACE_CTRL_GENERAL | TRACE_ORG_GKI, TRACE_TYPE_ERROR,
                       "Signal %s [%d] has been raised", strsignal(sig), sig);

            uipc_fifo_terminate();
            uipc_rb_terminate();
            uipc_sv_socket_terminate();
#if defined( RFKILL_BT_POWER ) && (RFKILL_BT_POWER == TRUE)
            UPIO_Set(UPIO_GENERAL, HCILP_BT_POWER_GPIO, UPIO_OFF);
#endif

            raise(sig);

            break;

    }
}

/*******************************************************************************
 **
 ** Function         GKI_get_os_tick_count
 **
 ** Description      This function is called to retrieve the native OS system tick.
 **
 ** Returns          Tick count of native OS.
 **
 *******************************************************************************/
UINT32 GKI_get_os_tick_count(void)
{
    struct timespec current;
    UINT32 tc;

    /* Get time -> do not use the OS tick count because it is stopped when there
     * are no active timers */
    clock_gettime(CLOCK_MONOTONIC, &current);

    /* Convert the time in a number of OS ticks */
    tc  = GKI_MS_TO_TICKS(current.tv_sec * 1000);
    tc += GKI_MS_TO_TICKS(current.tv_nsec/1000000);

    return tc;
}

/*******************************************************************************
 **
 ** Function         GKI_create_task
 **
 ** Description      This function is called to create a new OSS task.
 **
 ** Parameters:      task_entry  - (input) pointer to the entry function of the task
 **                  task_id     - (input) Task id is mapped to priority
 **                  taskname    - (input) name given to the task
 **                  stack       - (input) pointer to the top of the stack (highest memory location)
 **                  stacksize   - (input) size of the stack allocated for the task
 **
 ** Returns          GKI_SUCCESS if all OK, GKI_FAILURE if any problem
 **
 ** NOTE             This function take some parameters that may not be needed
 **                  by your particular OS. They are here for compatibility
 **                  of the function prototype.
 **
 *******************************************************************************/
UINT8 GKI_create_task(TASKPTR task_entry, UINT8 task_id, INT8 *taskname, UINT16 *stack, UINT16 stacksize)
{
    tOS_THREAD *p_thread;
    pthread_attr_t thread_attr;
    int ret;
#if BSA_SET_SCHED_PRIORITY
    struct sched_param param;
    int policy;
#endif
#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
    char thread_name[BSA_GKI_THREAD_NAME_LEN_MAX];
    int thread_name_len;
#endif

    GKI_TRACE("GKI_create_task %x %d %s %x %d", task_entry, task_id, taskname,
            stack, stacksize);

    if (task_id >= GKI_MAX_TASKS)
    {
        GKI_ERROR_TRACE("Error! task ID > max task allowed");
        return (GKI_FAILURE);
    }
    if (gki_cb.com.OSRdyTbl[task_id] != TASK_DEAD)
    {
        GKI_ERROR_TRACE("Error! task ID %d already started", gki_cb.com.OSRdyTbl[task_id]);
        return GKI_FAILURE;
    }

    gki_cb.com.OSRdyTbl[task_id] = TASK_READY;
    gki_cb.com.OSTName[task_id] = taskname;
    /* The current implementation does not use the wait timer */
    /* gki_cb.com.OSWaitTmr[task_id] = 0; */
    /* Do not clear the Pending event mask in case the task already had an event pending */
    /* gki_cb.com.OSWaitEvt[task_id] = 0; */
    gki_cb.com.OSStackSize[task_id] = stacksize;

    p_thread = &gki_cb.os.thread[task_id];
    p_thread->start = task_entry;
    

    pthread_attr_init(&thread_attr);

    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    /* lock to make sure that initialization has completed before starting thread */
    pthread_mutex_lock(&p_thread->evt_mutex);

    ret = pthread_create(&p_thread->id, &thread_attr, gki_task_entry, p_thread);
    if (ret != 0)
    {
        pthread_mutex_unlock(&p_thread->evt_mutex);
        return GKI_FAILURE;
    }

#if BSA_SET_SCHED_PRIORITY
    if (pthread_getschedparam(p_thread->id, &policy, &param) == 0)
    {
        policy = SCHED_RR;
        param.sched_priority = BSA_TASK_SCHED_PRIORITY;
        pthread_setschedparam(p_thread->id, policy, &param);
    }
    else
    {
        GKI_ERROR_TRACE("Error! Unable to set scheduler priorty");
    }
#endif

    /* Change the pthread's name (for debug purpose) */
#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
    BCM_STRNCPY_S(thread_name, sizeof(thread_name),
            BSA_GKI_THREAD_NAME_PREFIX, strlen(BSA_GKI_THREAD_NAME_PREFIX));
    thread_name_len = strlen(BSA_GKI_THREAD_NAME_PREFIX);
    thread_name[thread_name_len] = '\0';
    if (thread_name_len < (BSA_GKI_THREAD_NAME_LEN_MAX - 1))
    {
        BCM_STRNCAT_S(thread_name, sizeof(thread_name),
                (char *)taskname, BSA_GKI_THREAD_NAME_LEN_MAX - 1 - thread_name_len);
    }
    if (pthread_setname_np(p_thread->id, thread_name) < 0)
    {
        GKI_ERROR_TRACE("GKI_create_task cannot change thread's name");
    }
#endif

    /* really start thread */
    pthread_mutex_unlock(&p_thread->evt_mutex);

    GKI_TRACE("Created thread with thread_id= %x", p_thread->id);
    GKI_TRACE("Leaving GKI_create_task %x %d %s %x %d", task_entry, task_id,
            taskname, stack, stacksize);
    return (GKI_SUCCESS);
}

/*******************************************************************************
 **
 ** Function         GKI_shutdown
 **
 ** Description      shutdowns the GKI tasks/threads in from max task id to 0 and frees
 **                  pthread resources!
 **                  IMPORTANT: in case of join method, GKI_shutdown must be called outside
 **                  a GKI thread context!
 **
 ** Returns          void
 **
 *******************************************************************************/
void GKI_shutdown(void)
{
    GKI_ERROR_TRACE("GKI_shutdown not implemented");
}

#if BSA_SET_SCHED_PRIORITY
/*******************************************************************************
 **
 ** Function         gki_check_sched_priority
 **
 ** Description      This function is called to check the scheduling priority
 **                  for a task
 **
 ** Returns          void
 **
 *******************************************************************************/
void gki_check_sched_priority (void)
{
    int    ret;
    int    policy;
    struct sched_param param_chk;
    pthread_t thrd_id;

    thrd_id = pthread_self();
    ret = pthread_getschedparam(thrd_id, &policy, &param_chk);
    if (ret)
    {
        GKI_ERROR_TRACE("ERROR: getschedparam failed!!!");
    }
    else
    {
        GKI_TRACE("For thread_id = %x: Priority = %d Policy = %d", thrd_id, param_chk.sched_priority,policy);
    }
}

/*******************************************************************************
 **
 ** Function         gki_set_sched_priority
 **
 ** Description      This function is called to set the scheduling priority
 **                  for a task
 **
 ** Returns          void
 **
 *******************************************************************************/
void gki_set_sched_priority(int prio, int policy )
{
    struct sched_param  param;
    int    ret;
    int    prio_max;
    int    prio_min;
    char   errorstring[80];

    /* check priority max for the specified policy */
    if((prio_max = sched_get_priority_max(policy)) < 0)
    {
        GKI_ERROR_TRACE("ERROR: Unable to check max priority (max = %d)",prio_max);
        return;
    }

    /* check priority min for the specified policy */
    if((prio_min = sched_get_priority_min(policy)) < 0)
    {
        GKI_ERROR_TRACE("ERROR: Unable to check min priority (min = %d)",prio_min);
        return;
    }

    /* check if desired priority is in range*/
    if((prio < prio_min || prio > prio_max))
    {
        GKI_ERROR_TRACE("ERROR: priority setting out of range");
        return;
    }

    param.sched_priority = prio;
    ret = sched_setscheduler(0, policy, &param);
    if(ret)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        GKI_ERROR_TRACE("ERROR: set_sched_priority failed!!! errno:%d => %s",errno,errorstring);
    }
    else
    {
        GKI_TRACE("set_sched_priority success");
    }

}
#endif


/*******************************************************************************
 **
 ** Function         GKI_run
 **
 ** Description      This function runs a task
 **
 ** Parameters:      p_task_id  - (input) pointer to task id
 **
 ** Returns          void
 **
 ** NOTE             This function is only needed for operating systems where
 **                  starting a task is a 2-step process. Most OS's do it in
 **                  one step, If your OS does it in one step, this function
 **                  should be empty.
 *******************************************************************************/
void GKI_run(void *p_task_id)
{
    pthread_attr_t thread_attr;
#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
    char thread_name[BSA_GKI_THREAD_NAME_LEN_MAX];
    int thread_name_len;
#endif

    /* Start the quick timer thread here
     * Quick timers have been moved in a thread to fix an issue related to SLIP
     */
    pthread_attr_init(&thread_attr);

    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    gki_cb.os.timer_thread_id = TASK_DEAD;

    if (pthread_create(&gki_cb.os.timer_thread_id, &thread_attr,
                        (PTHREAD_START_ROUTINE)gki_timer_update_thread, NULL) < 0)
    {
        GKI_ERROR_TRACE("ERROR: GKI_run: Cannot create Timer thread");
    }

    /* Change the pthread's name (for debug purpose) */
#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
    BCM_STRNCPY_S(thread_name, sizeof(thread_name),
            BSA_GKI_THREAD_NAME_PREFIX, strlen(BSA_GKI_THREAD_NAME_PREFIX));
    thread_name_len = strlen(BSA_GKI_THREAD_NAME_PREFIX);
    thread_name[thread_name_len] = '\0';
    if (thread_name_len < (BSA_GKI_THREAD_NAME_LEN_MAX - 1))
    {
        BCM_STRNCAT_S(thread_name, sizeof(thread_name),
                "GKI_TI", BSA_GKI_THREAD_NAME_LEN_MAX - 1 - thread_name_len);
    }
    if (pthread_setname_np(gki_cb.os.timer_thread_id, thread_name) < 0)
    {
        GKI_ERROR_TRACE("GKI_create_task cannot change thread's name");
    }
#endif

    return;
}

/*******************************************************************************
 **
 ** Function         GKI_stop
 **
 ** Description      This function is called to stop
 **                  the tasks and timers when the system is being stopped
 **
 ** Returns          void
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If you want to use it in your own implementation,
 **                  put specific code here.
 **
 *******************************************************************************/
void GKI_stop(void)
{
    GKI_ERROR_TRACE("GKI_stop not needed/implemented");
}

/*******************************************************************************
 **
 ** Function         GKI_wait
 **
 ** Description      This function is called by tasks to wait for a specific
 **                  event or set of events. The task may specify the duration
 **                  that it wants to wait for, or 0 if infinite.
 **
 ** Parameters:      flag -    (input) the event or set of events to wait for
 **                  timeout - (input) the duration that the task wants to wait
 **                                    for the specific events (in system ticks)
 **
 **
 ** Returns          the event mask of received events or zero if timeout
 **
 *******************************************************************************/
UINT16 GKI_wait(UINT16 flag, UINT32 timeout)
{
    UINT16 evt;
    UINT8 rtask;
    struct timespec abstime;
    int retcode;
    tOS_THREAD *p_thread;

    rtask = GKI_get_taskid();
    if(rtask == GKI_MAX_TASKS)
    {
        GKI_ERROR_TRACE("INVALID Task ID from GKI_get_taskid !!!");
        return 0; /* zero means timeout */
    }

    GKI_TRACE("GKI_wait %d %x %d", rtask, flag, timeout);

    gki_cb.com.OSWaitForEvt[rtask] = flag;

    p_thread = &gki_cb.os.thread[rtask];

    /* protect OSWaitEvt[rtask] from modification from an other thread */
    pthread_mutex_lock(&p_thread->evt_mutex);
    /* Check if anything in any of the mailboxes. Possible race condition. */
    if (gki_cb.com.OSTaskQFirst[rtask][0])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_0_EVT_MASK;
    if (gki_cb.com.OSTaskQFirst[rtask][1])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_1_EVT_MASK;
    if (gki_cb.com.OSTaskQFirst[rtask][2])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_2_EVT_MASK;
    if (gki_cb.com.OSTaskQFirst[rtask][3])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_3_EVT_MASK;


    if (!(gki_cb.com.OSWaitEvt[rtask] & flag))
    {
        if (timeout)
        {
            /* Convert to milliseconds */
            timeout = GKI_TICKS_TO_MS(timeout);

            /* Get current time */
#if ((!defined(__CYGWIN__)) && (!defined(PTHREAD_CONDATTR_SETCLOCK_UNDEFINED)))
            clock_gettime(CLOCK_MONOTONIC, &abstime);
#else
            /* Cygwin does not implement pthread_condattr_setclock so use REALTIME clock;
             * some toolchain may not implement it as well */
            clock_gettime(CLOCK_REALTIME, &abstime);
#endif

            /* Add timeout to current time */
            abstime.tv_sec += timeout / 1000;
            timeout = timeout % 1000;
            abstime.tv_nsec += timeout * 1000000;
            if (abstime.tv_nsec >= 1000000000)
            {
                abstime.tv_sec++;
                abstime.tv_nsec -= 1000000000;
            }

            /* To prevent spurious event, which may append on SMP systems, we
             * call pthread_cond_timedwait while there is not bit set in
             * gki_cb.com.OSWaitEvt[rtask] */
            do {
                retcode = pthread_cond_timedwait(&p_thread->evt_cond, &p_thread->evt_mutex, &abstime);
            } while (((gki_cb.com.OSWaitEvt[rtask] & flag) == 0) && (retcode != ETIMEDOUT));
        }
        else
        {
            /* To prevent spurious event, which may append on SMP systems, we
             * call pthread_cond_wait while there is not bit set in
             * gki_cb.com.OSWaitEvt[rtask] */
            do {
                pthread_cond_wait(&p_thread->evt_cond, &p_thread->evt_mutex);
            } while ((gki_cb.com.OSWaitEvt[rtask] & flag) == 0);
        }
        /* we are waking up after waiting for some events, so refresh variables
         * no need to call GKI_disable() here as we know that we will have some
         * events as we've been waking up after condition pending or timeout
         */
        if (gki_cb.com.OSTaskQFirst[rtask][0])
           gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_0_EVT_MASK;
        if (gki_cb.com.OSTaskQFirst[rtask][1])
           gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_1_EVT_MASK;
        if (gki_cb.com.OSTaskQFirst[rtask][2])
           gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_2_EVT_MASK;
        if (gki_cb.com.OSTaskQFirst[rtask][3])
           gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_3_EVT_MASK;
    }
    /* Clear the wait for event mask */
    gki_cb.com.OSWaitForEvt[rtask] = 0;

    /* Return only those bits which user wants... */
    evt = gki_cb.com.OSWaitEvt[rtask] & flag;

    /* Clear only those bits which user wants... */
    gki_cb.com.OSWaitEvt[rtask] &= ~flag;

    /* unlock thread_evt_mutex as pthread_cond_wait() does auto lock mutex when cond is met */
    pthread_mutex_unlock(&p_thread->evt_mutex);

    GKI_TRACE("GKI_wait %d %x %d %x done", rtask, flag, timeout, evt);
    return (evt);
}

/*******************************************************************************
 **
 ** Function         GKI_delay
 **
 ** Description      This function is called by tasks to sleep unconditionally
 **                  for a specified amount of time. The duration is in milliseconds
 **
 ** Parameters:      timeout -    (input) the duration in milliseconds
 **
 ** Returns          void
 **
 *******************************************************************************/

void GKI_delay(UINT32 timeout)
{
    UINT8 rtask = GKI_get_taskid();
    struct timespec delay;
    int err;

    GKI_TRACE("GKI_delay %d %d", rtask, timeout);

    delay.tv_sec = timeout / 1000;
    delay.tv_nsec = 1000 * 1000 * (timeout % 1000);

    /* [u]sleep can't be used because it uses SIGALRM */

    do {
        err = nanosleep(&delay, &delay);
    } while (err < 0 && errno ==EINTR);

    /* Check if task was killed while sleeping */
    /* NOTE
     **      if you do not implement task killing, you do not
     **      need this check.
     */
    if (rtask && gki_cb.com.OSRdyTbl[rtask] == TASK_DEAD)
    {
    }

    GKI_TRACE("GKI_delay %d %d done", rtask, timeout);
    return;
}

/*******************************************************************************
 **
 ** Function         GKI_send_event
 **
 ** Description      This function is called by tasks to send events to other
 **                  tasks. Tasks can also send events to themselves.
 **
 ** Parameters:      task_id -  (input) The id of the task to which the event has to
 **                  be sent
 **                  event   -  (input) The event that has to be sent
 **
 **
 ** Returns          GKI_SUCCESS if all OK, else GKI_FAILURE
 **
 *******************************************************************************/
UINT8 GKI_send_event(UINT8 task_id, UINT16 event)
{
    tOS_THREAD *p_thread;

#if (GKI_DEBUG == TRUE)
    GKI_TRACE("GKI_send_event %d %x (%d)", task_id, event, GKI_get_taskid());
#endif
    
    if (task_id < GKI_MAX_TASKS)
    {
        p_thread = &gki_cb.os.thread[task_id];

        /* protect OSWaitEvt[task_id] from manipulation in GKI_wait() */
        pthread_mutex_lock(&p_thread->evt_mutex);

        /* Set the event bit */
        gki_cb.com.OSWaitEvt[task_id] |= event;

        pthread_cond_signal(&p_thread->evt_cond);

        pthread_mutex_unlock(&p_thread->evt_mutex);

#if (GKI_DEBUG == TRUE)
        GKI_TRACE("GKI_send_event %d %x done", task_id, event);
#endif
        return GKI_SUCCESS;
    }
    GKI_ERROR_TRACE("GKI_send_event bad task_id:%d %x", task_id);
    return GKI_FAILURE;

}

/*******************************************************************************
 **
 ** Function         GKI_isend_event
 **
 ** Description      This function is called from ISRs to send events to other
 **                  tasks. The only difference between this function and GKI_send_event
 **                  is that this function assumes interrupts are already disabled.
 **
 ** Parameters:      task_id -  (input) The destination task Id for the event.
 **                  event   -  (input) The event flag
 **
 ** Returns          GKI_SUCCESS if all OK, else GKI_FAILURE
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If you want to use it in your own implementation,
 **                  put your code here, otherwise you can delete the entire
 **                  body of the function.
 **
 *******************************************************************************/
UINT8 GKI_isend_event(UINT8 task_id, UINT16 event)
{

    GKI_TRACE("GKI_isend_event %d %x", task_id, event);
    GKI_TRACE("GKI_isend_event %d %x done", task_id, event);
    return GKI_send_event(task_id, event);
}

/*******************************************************************************
 **
 ** Function         GKI_get_taskid
 **
 ** Description      This function gets the currently running task ID.
 **
 ** Returns          task ID
 **
 ** NOTE             The Widcomm upper stack and profiles may run as a single task.
 **                  If you only have one GKI task, then you can hard-code this
 **                  function to return a '1'. Otherwise, you should have some
 **                  OS-specific method to determine the current task.
 **
 *******************************************************************************/
UINT8 GKI_get_taskid(void)
{
    int i;
#if defined UCLIBC_PTHREADS && UCLIBC_PTHREADS == TRUE
    int j;
#endif

    pthread_t thread_id = pthread_self();
#if (GKI_DEBUG == TRUE)
    GKI_TRACE("GKI_get_taskid %x", thread_id);
#endif
#if defined UCLIBC_PTHREADS && UCLIBC_PTHREADS == TRUE
    for(j=0; j<2;j++) /* give two tries to retrieve task_id 2820 linux 2.0 specific */

    {
#endif

    for (i = 0; i < GKI_MAX_TASKS; i++) {
        if (gki_cb.os.thread[i].id == thread_id)
        {
#if (GKI_DEBUG == TRUE)
            GKI_TRACE("GKI_get_taskid %x %d done", thread_id, i);
#endif
            return(i);
        }
    }
#if defined UCLIBC_PTHREADS && UCLIBC_PTHREADS == TRUE
    /* specific to 2820 Linux 2.0 uclib pthreads */
    sleep(1);
}
#endif
#if (GKI_DEBUG == TRUE)
    GKI_TRACE("GKI_get_taskid: task id = %d", GKI_MAX_TASKS);
#endif

    return (GKI_MAX_TASKS);
}

/*******************************************************************************
 **
 ** Function         GKI_map_taskname
 **
 ** Description      This function gets the task name of the taskid passed as arg.
 **                  If GKI_MAX_TASKS is passed as arg the currently running task
 **                  name is returned
 **
 ** Parameters:      task_id -  (input) The id of the task whose name is being
 **                  sought. GKI_MAX_TASKS is passed to get the name of the
 **                  currently running task.
 **
 ** Returns          pointer to task name
 **
 ** NOTE             this function needs no customization
 **
 *******************************************************************************/
INT8 *GKI_map_taskname(UINT8 task_id)
{
    GKI_TRACE("GKI_map_taskname %d", task_id);

    if (task_id < GKI_MAX_TASKS)
    {
        GKI_TRACE("GKI_map_taskname %d %s done", task_id, gki_cb.com.OSTName[task_id]);
        return (gki_cb.com.OSTName[task_id]);
    }
    else if (task_id == GKI_MAX_TASKS)
    {
        return (gki_cb.com.OSTName[GKI_get_taskid()]);
    }
    else
    {
        return (INT8*) "BAD";
    }
}

/*******************************************************************************
 **
 ** Function         GKI_enable
 **
 ** Description      This function enables interrupts.
 **
 ** Returns          void
 **
 *******************************************************************************/
void GKI_enable(void)
{
    GKI_TRACE("GKI_enable");
    pthread_mutex_unlock(&gki_cb.os.mutex);
    GKI_TRACE("Leaving GKI_enable");
    return;
}

/*******************************************************************************
 **
 ** Function         GKI_disable
 **
 ** Description      This function disables interrupts.
 **
 ** Returns          void
 **
 *******************************************************************************/
void GKI_disable(void)
{
    GKI_TRACE("GKI_disable");
    pthread_mutex_lock(&gki_cb.os.mutex);
    GKI_TRACE("Leaving GKI_disable");
    return;
}

/*******************************************************************************
 **
 ** Function         GKI_exception
 **
 ** Description      This function throws an exception.
 **                  This is normally only called for a nonrecoverable error.
 **
 ** Parameters:      code    -  (input) The code for the error
 **                  msg     -  (input) The message that has to be logged
 **
 ** Returns          void
 **
 *******************************************************************************/

void GKI_exception(UINT16 code, char *msg)
{
    GKI_ERROR_TRACE("GKI_exception %d %s", code, msg);
}

/*******************************************************************************
 **
 ** Function         GKI_get_time_stamp
 **
 ** Description      This function formats the time into a user area
 **
 ** Parameters:      tbuf -  (output) the address to the memory containing the
 **                  formatted time
 **
 ** Returns          the address of the user area containing the formatted time
 **                  The format of the time is ????
 **
 **
 *******************************************************************************/
INT8 *GKI_get_time_stamp(INT8 *tbuf)
{
#if (defined(GKI_SYSTEM_TIMESTAMP) && (GKI_SYSTEM_TIMESTAMP == TRUE))
    struct timespec ts;
    struct tm lt;

    clock_gettime(GKI_TIMESTAMP_CLOCK, &ts);

    localtime_r(&ts.tv_sec, &lt);

    snprintf((char *)tbuf, GKI_MAX_TIMESTAMP_BUF_SIZE, " %02d/%02d %02dh:%02dm:%02ds:%03ldms",
           (lt.tm_mon + 1), lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, (ts.tv_nsec/1000000));

#else
    UINT32 ms_time;
    UINT32 s_time;
    UINT32 m_time;
    UINT32 h_time;
    struct tms tms;
    INT8 *p_out = tbuf;
    UINT32 system_time;

    /* Get number of system ticks from OS directly */
    /* The main thread which updates ticks may be stopped */
    system_time = times(&tms) - GKI_InitialTime;
    system_time = GKI_TICKS_TO_MS(system_time); /* convert to ms */
    ms_time = system_time % 1000; /* 1000 ms per sec */
    s_time = (system_time / 1000) % 60 ; /* 60 second per minutes*/
    m_time = (system_time / (1000 * 60)) % 60; /* 60 minutes per hours */
    h_time = (system_time / (1000 * 60 * 60)) % 24; /* 24 hours per day */

    *p_out++ = (INT8)((h_time / 10) + '0');
    *p_out++ = (INT8)((h_time % 10) + '0');
    *p_out++ = 'h';
    *p_out++ = (INT8)((m_time / 10) + '0');
    *p_out++ = (INT8)((m_time % 10) + '0');
    *p_out++ = 'm';
    *p_out++ = (INT8)((s_time / 10) + '0');
    *p_out++ = (INT8)((s_time % 10) + '0');
    *p_out++ = 's';
    *p_out++ = (INT8)((ms_time / 100) + '0');
    ms_time %= 100; /* remove hundred ms */
    *p_out++ = (INT8)((ms_time / 10) + '0');
    *p_out++ = (INT8)((ms_time % 10) + '0');
    *p_out++ = 'm';
    *p_out++ = 's';
    *p_out = 0;
#endif
    return (tbuf);
}

/*******************************************************************************
 **
 ** Function         GKI_register_mempool
 **
 ** Description      This function registers a specific memory pool.
 **
 ** Parameters:      p_mem -  (input) pointer to the memory pool
 **
 ** Returns          void
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If your OS has different memory pools, you
 **                  can tell GKI the pool to use by calling this function.
 **
 *******************************************************************************/
void GKI_register_mempool(void *p_mem)
{
    gki_cb.com.p_user_mempool = p_mem;

    return;
}

/*******************************************************************************
 **
 ** Function         GKI_os_malloc
 **
 ** Description      This function allocates memory
 **
 ** Parameters:      size -  (input) The size of the memory that has to be
 **                  allocated
 **
 ** Returns          the address of the memory allocated, or NULL if failed
 **
 ** NOTE             This function is called by the Widcomm stack when
 **                  dynamic memory allocation is used. (see dyn_mem.h)
 **
 *******************************************************************************/
void *GKI_os_malloc(UINT32 size)
{
    /* for BSA we do not use dynamic allocation */
    return (0);
}

/*******************************************************************************
 **
 ** Function         GKI_os_free
 **
 ** Description      This function frees memory
 **
 ** Parameters:      size -  (input) The address of the memory that has to be
 **                  freed
 **
 ** Returns          void
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. It is only called from within GKI if dynamic
 **
 *******************************************************************************/
void GKI_os_free(void *p_mem)
{
    /* for BSA we do not use dynamic allocation */
    return;
}

/*******************************************************************************
 **
 ** Function         GKI_suspend_task()
 **
 ** Description      This function suspends the task specified in the argument.
 **
 ** Parameters:      task_id  - (input) the id of the task that has to suspended
 **
 ** Returns          GKI_SUCCESS if all OK, else GKI_FAILURE
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If you want to implement task suspension capability,
 **                  put specific code here.
 **
 *******************************************************************************/
UINT8 GKI_suspend_task(UINT8 task_id)
{
    GKI_TRACE("GKI_suspend_task %d - NOT implemented", task_id);


    GKI_TRACE("GKI_suspend_task %d done", task_id);

    return (GKI_SUCCESS);
}


/*******************************************************************************
 **
 ** Function         GKI_resume_task()
 **
 ** Description      This function resumes the task specified in the argument.
 **
 ** Parameters:      task_id  - (input) the id of the task that has to resumed
 **
 ** Returns          GKI_SUCCESS if all OK
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If you want to implement task suspension capability,
 **                  put specific code here.
 **
 *******************************************************************************/
UINT8 GKI_resume_task(UINT8 task_id)
{
    GKI_TRACE("GKI_resume_task %d - NOT implemented", task_id);


    GKI_TRACE("GKI_resume_task %d done", task_id);

    return (GKI_SUCCESS);
}


/*******************************************************************************
 **
 ** Function         GKI_exit_task
 **
 ** Description      This function is called to stop a GKI task.
 **
 ** Parameters:      task_id  - (input) the id of the task that has to be stopped
 **
 ** Returns          void
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If you want to use it in your own implementation,
 **                  put specific code here to kill a task.
 **
 *******************************************************************************/
void GKI_exit_task(UINT8 task_id)
{
    GKI_TRACE("GKI_exit_task %d", task_id);

    /* for BSA implementation, we use this function to kill the BSA server */
    raise(SIGTERM);

    return;
}

/*******************************************************************************
 **
 ** Function         GKI_sched_lock
 **
 ** Description      This function is called by tasks to disable scheduler
 **                  task context switching.
 **
 ** Returns          void
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If you want to use it in your own implementation,
 **                  put code here to tell the OS to disable context switching.
 **
 *******************************************************************************/
void GKI_sched_lock(void)
{
    GKI_TRACE("GKI_sched_lock");
    pthread_mutex_lock(&gki_cb.os.mutex);
}

/*******************************************************************************
 **
 ** Function         GKI_sched_unlock
 **
 ** Description      This function is called by tasks to enable scheduler switching.
 **
 ** Returns          void
 **
 ** NOTE             This function is NOT called by the Widcomm stack and
 **                  profiles. If you want to use it in your own implementation,
 **                  put code here to tell the OS to re-enable context switching.
 **
 *******************************************************************************/
void GKI_sched_unlock(void)
{
    GKI_TRACE("GKI_sched_unlock");
    pthread_mutex_unlock(&gki_cb.os.mutex);
}

/*******************************************************************************
 **
 ** Function         timer_control_cback
 **
 ** Description      This function is called by GKI timer to enable/disable timer.
 **
 ** Returns          void
 **
 *******************************************************************************/
void timer_control_cback(BOOLEAN enable)
{
    if (enable)
    {
        /*
         * The timer is enabled => GKI_run function will loop forever on
         * nanosleep and call GKI_timer_update every 10 ms
         */
        APPL_TRACE_DEBUG0("GKI Timer Enabled");
        /*
         * Firstly set GKI_timer_enable to TRUE => when the thread in charge
         * of timer (i.e. GKI_run) will be unlocked, it will not be relocked
         */
        GKI_timer_enable = TRUE;
        /*
         * Secondly we can unlock the GKI_timer_mutex.
         * The thread in charge of timer (i.e. GKI_run) will be unlocked
         * and will generate our system tick
         */
        pthread_mutex_unlock(&GKI_timer_mutex);
    }
    else
    {
        /* The GKI timer management can call this callback even if GKI timer
         * are already disabled => check GKI_timer_enable to prevent mutex
         * deadlock */
        if (GKI_timer_enable == TRUE)
        {
            /*
             * The timer is enabled => GKI_run function will loop forever on
             * nanosleep and call GKI_timer_update every 10 ms
             * When GKI_timer_enable is FALSE, the GKI_run's loop will lock on
             * GKI_timer_mutex
             */
            APPL_TRACE_DEBUG0("GKI Timer Disabled");
            /*
             * To stop the thread in charge of timer (i.e. GKI_run) we first
             * lock the GKI_timer_mutex (which is free)
             */
            pthread_mutex_lock(&GKI_timer_mutex);
            /*
             * Secondly, we set GKI_timer_enable to FALSE => the thread in
             * charge of timer (i.e. GKI_run) will lock on the mutex (which is
             * in use because we just locked it) => the thread is locked
             */
            GKI_timer_enable = FALSE;
        }
    }
}

/*******************************************************************************
**
** Function         GKI_get_stack_used
**
** Description      This function, specific for BSA returns the stack used for
**                  the specified task Id.
**
** Returns          Stack size.
**
*******************************************************************************/
UINT16 GKI_get_stack_used(UINT8 task_id)
{
    /* Not implemented on Linux */
    return 0;
}

/*******************************************************************************
**
** Function         GKI_shiftdown
**
** Description      shift memory down (to make space to insert a record)
**
*******************************************************************************/
void GKI_shiftdown (UINT8 *p_mem, UINT32 len, UINT32 shift_amount)
{
    register UINT8 *ps = p_mem + len - 1;
    register UINT8 *pd = ps + shift_amount;
    register UINT32 xx;

    for (xx = 0; xx < len; xx++)
        *pd-- = *ps--;
}

/*******************************************************************************
**
** Function         GKI_shiftup
**
** Description      shift memory up (to delete a record)
**
*******************************************************************************/
void GKI_shiftup (UINT8 *p_dest, UINT8 *p_src, UINT32 len)
{
    register UINT8 *ps = p_src;
    register UINT8 *pd = p_dest;
    register UINT32 xx;

    for (xx = 0; xx < len; xx++)
        *pd++ = *ps++;
}


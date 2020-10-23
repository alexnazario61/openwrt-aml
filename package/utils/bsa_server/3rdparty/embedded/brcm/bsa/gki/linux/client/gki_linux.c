/****************************************************************************
 **
 **  Name        gki_linux.c
 **
 **  Function    pthreads version of Linux GKI. This version is used for
 **              settop projects that already use pthreads and not pth.
 **
 **  Copyright (c) 2000-2012, Broadcom Corp., All Rights Reserved.
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
#include "gki_int.h"

#if defined(__FreeBSD__)
extern int pthread_mutexattr_settype(pthread_mutexattr_t *__attr, int __kind)
__attribute((nonnull (1)));
#else
extern int pthread_mutexattr_settype(pthread_mutexattr_t *__attr, int __kind)
__THROW __attribute((nonnull (1)));
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

#ifndef GKI_DEBUG
#define GKI_DEBUG TRUE
#endif

#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
#if defined(__FreeBSD__)
#define pthread_setname_np(t, n)  pthread_set_name_np((t), (n))
#endif
int __attribute__((weak)) pthread_setname_np(pthread_t thread, const char *name);
#define BSA_GKI_THREAD_NAME_LEN_MAX     16      /* Pthread Limitation */
#ifndef BSA_GKI_THREAD_NAME_PREFIX
#define BSA_GKI_THREAD_NAME_PREFIX  "BSA_Clt:"  /* Pthread Name Prefix */
#endif
#endif

#define LOCK(m)  pthread_mutex_lock(&m)
#define UNLOCK(m) pthread_mutex_unlock(&m)
#define INIT(m) pthread_mutex_init(&m, NULL)

typedef void *(*PTHREAD_START_ROUTINE)(void*);

#if (GKI_DEBUG == TRUE)
static pthread_mutex_t GKI_trace_mutex;
#endif

/*need to add this variable for client compiling here under NSA_STANDALONE compile switch
 * so that it will be declared only when compiling the libipcnsa; remove it from nsa_mgt_int.c
 * so that when both libnsa and libbsa are linked there won't be a mutiple declaration*/
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
/* Added appl_trace_level here to be able compile client side */
UINT8 appl_trace_level = APPL_INITIAL_TRACE_LEVEL;
#endif

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
    tOS_THREAD *p_thread;
    int task_id;
#if !(defined(GKI_SIGNAL_HANDLER_EXCLUDED) && (GKI_SIGNAL_HANDLER_EXCLUDED == TRUE))
    sigset_t sig;
#endif

    memset((void *) &gki_cb, 0, sizeof(gki_cb));

    gki_buffer_init();

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

#if !(defined(GKI_SIGNAL_HANDLER_EXCLUDED) && (GKI_SIGNAL_HANDLER_EXCLUDED == TRUE))
    /* block SIGPIPE raised when server is killed while writing in the UIPC */
    GKI_TRACE("GKI_Init is blocking SIGPIPE");

    sigemptyset(&sig);
    sigaddset(&sig, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &sig, (sigset_t *) NULL);
#endif

    GKI_TRACE("exiting GKI_Init");
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
    pthread_attr_t thread_attr;
    UINT8 task_index;
    tOS_THREAD *p_thread;
#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
    char thread_name[BSA_GKI_THREAD_NAME_LEN_MAX];
    int thread_name_len;
#endif

    GKI_TRACE("GKI_create_task %x %d %s %x %d", task_entry, task_id, taskname,
            stack, stacksize);

    /* specific case used to create a temporary pseudo-task */
    if (task_id == GKI_MAX_TASKS)
    {
        /* Look for a dead task which can be used */
        for (task_index = APP_TASK + 1 ; task_index < GKI_MAX_TASKS ; task_index++)
        {
            if (gki_cb.com.OSRdyTbl[task_index] == TASK_DEAD)
            {
                task_id = task_index;
                break;
            }
        }
    }

    if (task_id >= GKI_MAX_TASKS)
    {
        GKI_ERROR_TRACE("Error! task ID > max task allowed");
        return (GKI_FAILURE);
    }

    p_thread = &gki_cb.os.thread[task_id];

    /* These variables MUST be initialized before thread creation */
    gki_cb.com.OSRdyTbl[task_id] = TASK_READY;
    gki_cb.com.OSTName[task_id] = taskname;
    gki_cb.com.OSWaitTmr[task_id] = 0;
    gki_cb.com.OSWaitEvt[task_id] = 0;
    gki_cb.com.OSStackSize[task_id] = stacksize;

    p_thread->start = task_entry;

    /* Only the Callback thread should be created */
    if ((task_id == CBACK_TASK)
#if defined (NSA_CBACK_TASK)
         || (task_id == NSA_CBACK_TASK)
#endif
        )
    {
        pthread_attr_init(&thread_attr);

        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

        /* lock to make sure that initialization has completed before starting thread */
        pthread_mutex_lock(&p_thread->evt_mutex);

        /* Create the pthread */
        if (pthread_create(&p_thread->id, &thread_attr, gki_task_entry, p_thread) != 0)
        {
            pthread_mutex_unlock(&p_thread->evt_mutex);
            GKI_ERROR_TRACE("GKI_create_task cannot create thread");
            p_thread->id = 0;
            gki_cb.com.OSRdyTbl[task_id] = TASK_DEAD;
            return GKI_FAILURE;
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
                    (char *)taskname, BSA_GKI_THREAD_NAME_LEN_MAX - 1 - thread_name_len);
        }
        if (pthread_setname_np(p_thread->id, thread_name) < 0)
        {
            GKI_ERROR_TRACE("GKI_create_task cannot change thread's name");
        }
#endif

        pthread_mutex_unlock(&p_thread->evt_mutex);
        GKI_ERROR_TRACE("Created thread with thread_id=  %x", p_thread->id);
    }
    else
    {
        p_thread->id = pthread_self();
        GKI_TRACE("Use user's thread with thread_id=  %x", p_thread->id);
    }

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
    GKI_TRACE("GKI_run not Implemented");
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
    p_thread = &gki_cb.os.thread[rtask];

    GKI_TRACE("GKI_wait %d %x %d", rtask, flag, timeout);

    gki_cb.com.OSWaitForEvt[rtask] = flag;

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
            do
            {
                /* Wait a signal on the condition with a timeout */
                retcode = pthread_cond_timedwait(&p_thread->evt_cond, &p_thread->evt_mutex, &abstime);
            } while (((gki_cb.com.OSWaitEvt[rtask] & flag) == 0) && (retcode != ETIMEDOUT));
        }
        else
        {
            do
            {
                /* Wait a signal on the condition without timeout */
                pthread_cond_wait(&p_thread->evt_cond, &p_thread->evt_mutex);
            } while ((gki_cb.com.OSWaitEvt[rtask] & flag) == 0);
        }
        /*
         * we are waking up after waiting for some events, so refresh variables
         * no need to call GKI_disable() here as we know that we will have some
         * events as we've been waking up after condition pending or timeout
         * */
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
    struct timespec abstime;
    int err;

    GKI_TRACE("GKI_delay %d %d", rtask, timeout);

    /* Get current time */
    clock_gettime(CLOCK_MONOTONIC, &abstime);

    /* Add timeout to current time */
    abstime.tv_sec += timeout / 1000;
    timeout = timeout % 1000;
    abstime.tv_nsec += timeout * 1000000;
    if (BCM_UNLIKELY(abstime.tv_nsec >= 1000000000L))
    {
        abstime.tv_sec++;
        abstime.tv_nsec -= 1000000000L;
    }

    do
    {
        err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &abstime, NULL);
    } while ((err < 0) && (errno == EINTR));

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

    GKI_TRACE("GKI_send_event %d %x (%d)", task_id, event, GKI_get_taskid());

    if (task_id < GKI_MAX_TASKS)
    {
        p_thread = &gki_cb.os.thread[task_id];

        /* protect OSWaitEvt[task_id] from manipulation in GKI_wait() */
        pthread_mutex_lock(&p_thread->evt_mutex);

        /* Set the event bit */
        gki_cb.com.OSWaitEvt[task_id] |= event;

        pthread_cond_signal(&p_thread->evt_cond);

        pthread_mutex_unlock(&p_thread->evt_mutex);

        GKI_TRACE("GKI_send_event %d %x done", task_id, event);
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

    pthread_t thread_id = pthread_self();

    GKI_TRACE("GKI_get_taskid %x", thread_id);
    for (i = 0; i < GKI_MAX_TASKS; i++) {
        if ((gki_cb.com.OSRdyTbl[i] != TASK_DEAD) &&
            (gki_cb.os.thread[i].id == thread_id)) {
            GKI_TRACE("GKI_get_taskid %x %d done", thread_id, i);
            return(i);
        }
    }
    GKI_TRACE("GKI_get_taskid: task id = GKI_MAX_TASKS %d", GKI_MAX_TASKS);

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
    pthread_mutex_unlock(&gki_cb.os.mutex);
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
    pthread_mutex_lock(&gki_cb.os.mutex);
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
    INT8 *p_out = tbuf;
    UINT32 system_time;
    struct tms tms;

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

    if(task_id < GKI_MAX_TASKS)
    {
        gki_cb.com.OSRdyTbl[task_id] = TASK_DEAD;
    }
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
    return;
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



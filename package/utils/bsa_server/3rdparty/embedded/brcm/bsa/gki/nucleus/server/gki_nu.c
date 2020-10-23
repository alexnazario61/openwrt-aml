/****************************************************************************
**
**  Name        gki_nu.c
**
**  Function    this file Gki functions specific to WIN_32
**
**
**  Copyright (c) 1999-2004, WIDCOMM Inc., All Rights Reserved.
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Proprietary and confidential.
**
*****************************************************************************/
#include <string.h>
#include <stdio.h>
#include "gki.h"
#include "gki_int.h"
#include "nucleus.h"

#ifndef _BT_NUCLEUS
#error Nucleus Os specific file ( gki_nu.c ) included but _BT_NUCLEUS is not defined!
#endif

#ifndef GKI_NU_PS_MALLOC
#define GKI_NU_MALLOC(x, y)\
    if(gki_cb.com.p_user_mempool == NULL)\
    {\
        APPL_TRACE_ERROR0("Error!  GKI mempool not set by GKI_register_mempool()");\
        y = NULL;\
    }\
    else\
        NU_Allocate_Memory ((NU_MEMORY_POOL *)gki_cb.com.p_user_mempool, &y, x, NU_NO_SUSPEND);

#else
#define GKI_NU_MALLOC(x, y) \
    y = (UINT8 *)GKI_NU_PS_MALLOC(x);

#endif

#ifndef GKI_NU_PS_DEALLOC
#define GKI_NU_DEALLOC NU_Deallocate_Memory
#else
#define GKI_NU_DEALLOC GKI_NU_PS_DEALLOC
#endif

#if (GKI_MAX_TASKS > 16)
#error Number of gki tasks out of range (16 Max)!
#endif

#ifndef NUCLEUS_BASE_PRIORITY
unsigned char gBasePriority = NUCLEUS_BASE_PRIORITY;
#else
unsigned char gBasePriority = 28;
#endif

/**********************************************************************
** Nucleus specific definitions
*/

#define OS_ENTER_CRITICAL()     NU_Control_Interrupts (NU_DISABLE_INTERRUPTS)
#define OS_EXIT_CRITICAL()      NU_Control_Interrupts (NU_ENABLE_INTERRUPTS)

typedef void (*NU_TASK_ENTRY)(UNSIGNED, VOID *);

/* Define the structure that holds the GKI variables
*/
#if GKI_DYNAMIC_MEMORY == FALSE
tGKI_CB   gki_cb;
#endif

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
    memset (&gki_cb, 0, sizeof (tGKI_CB));

    gki_cb.com.OSDisableNesting = 1;

    gki_buffer_init();
    gki_timers_init();

    gki_cb.com.OSTicks = 0;

#ifndef NUCLEUS_BASE_PRIORITY
    gki_cb.os.OSBasePriority = NUCLEUS_BASE_PRIORITY;
#else
    gki_cb.os.OSBasePriority = 28;
#endif

    APPL_TRACE_DEBUG0("GKI_init()")
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
    /* TODO - add any OS specific code here
    **/
    return (0); //(UINT32) GetTickCount());
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
**                  by your particular OS. They are here for compatability
**                  of the function prototype.
**
*******************************************************************************/
UINT8 GKI_create_task (TASKPTR task_entry, UINT8 task_id, INT8 *taskname,
                       UINT16 *stack, UINT16 stacksize)
{
    UINT16  i;
    UINT32  *frame;
    UINT8   *p;
    STATUS  status;
    OPTION          priority=0;              /* Task Priority      */
    CHAR            name[9];                 /* Task Name          */
    DATA_ELEMENT    task_status;             /* Current Task Status*/
    UNSIGNED        scheduled_count;         /* Scheduled count    */
    OPTION          preempt=0;               /* Task Preempt       */
    UNSIGNED        time_slice;              /* Task Time/slice    */
    VOID            *stack_base;             /* Task stack base    */
    UNSIGNED        stack_size;              /* Task stack size    */
    UNSIGNED        minimum_stack;           /* Minimum stack      */
    NU_TASK         tmp_nutask;

    /* Purely to get rid of compiler warning */

    frame = NULL;

    if (task_id >= GKI_MAX_TASKS)
    {
        return (GKI_FAILURE);
    }

#if 1
    APPL_TRACE_DEBUG3("GKI_create_task() taskname:%s task_id:%u stacksize:%u",
        taskname, task_id, stacksize);
#endif

    if (stack)
    {

        gki_cb.com.OSStack[task_id]     = (UINT8 *)stack - stacksize;
        gki_cb.com.OSStackSize[task_id] = stacksize;

    }
    else
    {
        /* Allocate memory for the main task */
        gki_cb.com.OSStack[task_id] = GKI_os_malloc(stacksize);
        gki_cb.com.OSStackSize[task_id] = stacksize;
    }

    gki_cb.com.OSRdyTbl[task_id]    = TASK_READY;
    gki_cb.com.OSTName[task_id]     = taskname;
    gki_cb.com.OSWaitTmr[task_id]   = 0;
    gki_cb.com.OSWaitEvt[task_id]   = 0;


    /* Create one Event Group for this task */
    status = NU_Create_Event_Group (&gki_cb.os.OSEvtGrp[task_id], taskname);

    if(status != NU_SUCCESS)
    {
        APPL_TRACE_ERROR1("GKI_create_task() NU_Create_Event_Group failed status %d", status);
        return (GKI_FAILURE);
    }

    if(task_id == APP_TASK)
    {
        /* For APP_TASK, map the calling task's param with GKI APP_TASK.
        No need to create a separate task */
        /* Get information */
        gki_cb.os.OSTCB[task_id] = *(NU_TASK *) NU_Current_Task_Pointer();
        NU_Task_Information (NU_Current_Task_Pointer(),
                             name, &task_status,
                             &scheduled_count, &priority,
                             &preempt, &time_slice, &stack_base,
                             &stack_size, &minimum_stack);
        gki_cb.com.OSStack[task_id] = (UINT8 *)stack_base;
        gki_cb.os.priority[task_id] = priority;
        APPL_TRACE_ERROR4("GKI_create_task() Mapping task_id:%u(name:%s) for APP_TASK(%u), prio %d",
            task_id, name, task_id, priority);
    }
    else
    {
    /* Create Task */
        status = NU_Create_Task (&gki_cb.os.OSTCB[task_id], /* Task Control Block */
                                taskname,                   /* Task Name */
                                (NU_TASK_ENTRY )task_entry, /* Task Entry Function */
                                task_id,                    /* ARGC */
                                NULL,                       /* ARGV */
                                gki_cb.com.OSStack[task_id],/* Begining of Stack */
                                stacksize,                  /* Stack size */
                                task_id  + gki_cb.os.OSBasePriority, /* Priority */
                                20,                         /* No Time Slicing */
                                NU_PREEMPT,                 /* Preemption allowed */
                                NU_START);                  /* Start the task */

        gki_cb.os.priority[task_id] = task_id  + gki_cb.os.OSBasePriority;
        if (status != NU_SUCCESS)
            return (GKI_FAILURE);
    }

    return (GKI_SUCCESS);
}


/*******************************************************************************
**
** Function         GKI_run
**
** Description      This function is called at startup if there is no OS
**                  to start the GKI dispatcher. It is also used on Windows
**                  to launch the application tasks.
**
** Returns          void
**
*******************************************************************************/
void GKI_run (void *p_task_id)
{

    gki_cb.com.OSDisableNesting = 0;

}


/*******************************************************************************
**
** Function         GKI_suspend_task()
**
** Description      This function suspends the task specified in the argument.
**
** Returns          0 if all OK, else 1
**                  It also tries to do context switch.
**                  If task tries to suspend itself, then some other task must
**                  resume it, because it is no longer running.
**
*******************************************************************************/
UINT8 GKI_suspend_task(UINT8 task_id)
{

    return (GKI_SUCCESS);
}

/*******************************************************************************
**
** Function         GKI_resume_task()
**
** Description      This function resumes the task specified in the argument.
**
** Returns          0 if all OK, else 1
**                  It also tries to do context switch.
**                  If task tries to suspend itself, then some other task must
**                  resume it, because it is no longer running.
**
*******************************************************************************/
UINT8 GKI_resume_task(UINT8 task_id)
{
    return (GKI_SUCCESS);
}


/*******************************************************************************
**
** Function         GKI_exit_task
**
** Description      This function is called to stop a GKI task.
**
** Returns          void
**
*******************************************************************************/
void GKI_exit_task (UINT8 task_id)
{
    APPL_TRACE_DEBUG1("GKI_exit_task() for task_id:%d", task_id);

    if(task_id>=GKI_MAX_TASKS)
    {
        return;
    }

    gki_cb.os.priority[task_id]= 0;

    if(task_id == APP_TASK)
    {
        return;
    }
    /*NU_Terminate_Task (&gki_cb.os.OSTCB[task_id]);*/
    NU_Delete_Task (&gki_cb.os.OSTCB[task_id]);
    if(gki_cb.com.OSStack[task_id] != NULL)
    {
        NU_Deallocate_Memory(gki_cb.com.OSStack[task_id]);
    }
    gki_cb.com.OSStack[task_id] = NULL;
    gki_cb.com.OSRdyTbl[task_id] = TASK_DEAD;

}

/*******************************************************************************
**
** Function         GKI_sched_lock
**
** Description      This function is called by tasks to disable scheduler switching.
**
** Returns          void
**
*******************************************************************************/
void GKI_sched_lock(void)
{
}

/*******************************************************************************
**
** Function         GKI_sched_unlock
**
** Description      This function is called by tasks to disable scheduler switching.
**
** Returns          void
**
*******************************************************************************/
void GKI_sched_unlock(void)
{

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
UINT16 GKI_wait (UINT16 flag, UINT32 timeout)
{

    UNSIGNED    evt   = 0;
    UINT8       rtask = GKI_get_taskid();
    STATUS      status;

    /* Check if anything in any of the mailboxes. Possible race condition. */
    if (gki_cb.com.OSTaskQFirst[rtask][0])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_0_EVT_MASK;
    if (gki_cb.com.OSTaskQFirst[rtask][1])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_1_EVT_MASK;
    if (gki_cb.com.OSTaskQFirst[rtask][2])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_2_EVT_MASK;
    if (gki_cb.com.OSTaskQFirst[rtask][3])
        gki_cb.com.OSWaitEvt[rtask] |= TASK_MBOX_3_EVT_MASK;

    /* If any valid event if pending, return immediately */
    if (gki_cb.com.OSWaitEvt[rtask] & flag)
    {
/*        APPL_TRACE_ERROR3("GKI_wait() got the shortcut, flag:0x%x evt:0x%x rtask:%u",
        flag, evt, rtask);*/
       /* Return only those bits which user wants... */
       evt = (UINT16) (gki_cb.com.OSWaitEvt[rtask] & flag);

       /* Clear only those bits which user wants... */
       gki_cb.com.OSWaitEvt[rtask] &= ~flag;

        return ((UINT16) evt);
    }

    if (!timeout)
        timeout = 0xFFFFFFFFL;

    status = NU_Retrieve_Events (&gki_cb.os.OSEvtGrp[rtask], (UNSIGNED) flag, NU_OR_CONSUME,
                        (UNSIGNED *)&evt, GKI_TICKS_TO_MS(timeout));
    if(status != NU_SUCCESS)
    {
        APPL_TRACE_ERROR4(
            "GKI_wait() status for NU_Retrieve_Events():%d, flag:0x%x evt:0x%x rtask:%u",
            status, flag, evt, rtask);
    }
    /* Return only those bits which user wants... */
    evt = gki_cb.com.OSWaitEvt[rtask] & flag;

    /* Clear only those bits which user wants... */
    gki_cb.com.OSWaitEvt[rtask] &= ~evt;

    return ((UINT16) evt);

}


/*******************************************************************************
**
** Function         GKI_delay
**
** Description      This function is called by tasks to sleep unconditionally
**                  for a specified amount of time.
**
** Returns          void
**
*******************************************************************************/
void GKI_delay (UINT32 timeout)
{
    timeout = GKI_MS_TO_TICKS(timeout);     /* convert from milliseconds to ticks */

    if (timeout == 0)
    {
        timeout = 1;
    }

    NU_Sleep(timeout);

}


/*******************************************************************************
**
** Function         GKI_send_event
**
** Description      This function is called by tasks to send events to other
**                  tasks. Tasks can also send events to themselves.
**
** Returns          GKI_SUCCESS if all OK, else GKI_FAILURE
**
*******************************************************************************/
UINT8 GKI_send_event (UINT8 task_id, UINT16 event)
{
    STATUS status;
    /*APPL_TRACE_DEBUG2("GKI_send_event() task_id:%d event:%d", task_id, event);*/
    if (task_id >= GKI_MAX_TASKS)
    {
        return (GKI_FAILURE);
    }


    GKI_disable();

    gki_cb.com.OSWaitEvt[task_id] |= event;
    gki_cb.com.OSRdyTbl[task_id]   = TASK_READY;

    GKI_enable();


    status = NU_Set_Events (&gki_cb.os.OSEvtGrp[task_id], (UNSIGNED)event, NU_OR);
    if(status != NU_SUCCESS)
    {
        APPL_TRACE_ERROR1("GKI_send_event() fails with status:%d", status);
    }


    return (GKI_SUCCESS);
}

/*******************************************************************************
**
** Function         GKI_isend_event
**
** Description      This function is called by ISRs to send events to other
**                  tasks. Does not cause reschedule.
**                  Assume interrupts are disabled...
** Returns          GKI_SUCCESS if all OK, else GKI_FAILURE
**
*******************************************************************************/
UINT8 GKI_isend_event (UINT8 task_id, UINT16 event)
{
    if (task_id >= GKI_MAX_TASKS)
    {
        return (GKI_FAILURE);
    }

    gki_cb.com.OSWaitEvt[task_id] |= event;
    if(gki_cb.com.OSRdyTbl[task_id] != TASK_SUSPEND)
    {
        gki_cb.com.OSRdyTbl[task_id]   = TASK_READY;
    }

    NU_Set_Events (&gki_cb.os.OSEvtGrp[task_id], (UNSIGNED)event, NU_OR);

    return (GKI_SUCCESS);
}


/*******************************************************************************
**
** Function         GKI_get_taskid
**
** Description      This function gets the currently running task ID.
**
** Returns          task ID
**
*******************************************************************************/
UINT8 GKI_get_taskid(void)
{
    OPTION          priority=0;              /* Task Priority      */
    CHAR            name[20];                 /* Task Name          */
    DATA_ELEMENT    task_status;             /* Current Task Status*/
    UNSIGNED        scheduled_count;         /* Scheduled count    */
    OPTION          preempt=0;               /* Task Preempt       */
    UNSIGNED        time_slice;              /* Task Time/slice    */
    VOID            *stack_base;             /* Task stack base    */
    UNSIGNED        stack_size;              /* Task stack size    */
    UNSIGNED        minimum_stack;           /* Minimum stack      */
    UINT8 TaskId;

    /* Get information */
    NU_Task_Information (NU_Current_Task_Pointer(),
                         name, &task_status,
                         &scheduled_count, &priority,
                         &preempt, &time_slice, &stack_base,
                         &stack_size, &minimum_stack);


    for(TaskId = 0; TaskId<GKI_MAX_TASKS; TaskId++)
    {
        if(priority == gki_cb.os.priority[TaskId])
        {
            return TaskId;
        }
    }

    APPL_TRACE_DEBUG1("GKI_get_taskid() priority:%d",
            priority);

    return (GKI_MAX_TASKS);  /* Task ID is mapped to priority */
}


/*******************************************************************************
**
** Function         GKI_map_taskname
**
** Description      This function gets the task name of the taskid passed as arg.
**                  If GKI_MAX_TASKS is passed as arg the currently running task
**                  name is returned
**
** Returns          pointer to task name
**
*******************************************************************************/
INT8 *GKI_map_taskname(UINT8 task_id)
{
    if (task_id < GKI_MAX_TASKS)
        return (gki_cb.com.OSTName[task_id]);
    else if (task_id == GKI_MAX_TASKS )
        return (gki_cb.com.OSTName[GKI_get_taskid()]);
    else
        return "BAD";
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
    if (gki_cb.com.OSIntNesting)
        return;

    if (gki_cb.com.OSDisableNesting > 0)
    {
        gki_cb.com.OSDisableNesting--;
    }
    if (gki_cb.com.OSDisableNesting == 0)
    {
        NU_Control_Interrupts (gki_cb.os.OSLastIntLevel);
    }
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
void GKI_disable (void)
{
    if (gki_cb.com.OSIntNesting)
        return;

    /* For NUCLEUS, control interrupt nesting ourselves */
    if (!gki_cb.com.OSDisableNesting)
        gki_cb.os.OSLastIntLevel = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    gki_cb.com.OSDisableNesting++;
}



/****************************************************************************/
/* For Big Endian Processors swap the bytes                                 */
#if BIG_ENDIAN == FALSE
UINT16 ntohs(UINT16 n)
{
    register UINT8  tmp;
    register UINT8  *p=(UINT8 *)&n;

    tmp  = p[0];
    p[0] = p[1];
    p[1] = tmp;

    return n;
}

UINT32 ntohl(UINT32 n)
{
    register UINT8 tmp;
    register UINT8 *p=(UINT8 *)&n;

    tmp  = p[0];
    p[0] = p[3];
    p[3] = tmp;

    tmp  = p[1];
    p[1] = p[2];
    p[2] = tmp;

    return n;
}

#endif


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
void GKI_exception (UINT16 code, char *msg)
{

#if (GKI_DEBUG == TRUE)
    GKI_disable();

    if (gki_cb.com.ExceptionCnt < GKI_MAX_EXCEPTION)
    {
        EXCEPTION_T *pExp;

        pExp =  &gki_cb.com.Exception[gki_cb.com.ExceptionCnt++];
        pExp->type = code;
        pExp->taskid = GKI_get_taskid();
        strncpy((char *)pExp->msg, msg, GKI_MAX_EXCEPTION_MSGLEN - 1);
    }

    GKI_enable();
    {
        UINT16 i;
        EXCEPTION_T *pExp;

        APPL_TRACE_DEBUG0("GKI Exceptions:");
        for (i = 0; i < gki_cb.com.ExceptionCnt; i++)
        {
            pExp =     &gki_cb.com.Exception[i];
            APPL_TRACE_DEBUG3("%d: Type=%d, Task=%d: %s\n", i,
                (INT32)pExp->type, (INT32)pExp->taskid, (INT8 *)pExp->msg);
        }
    }

#else
    APPL_TRACE_DEBUG2("GKI_exception() [err:%u] : %s", code, msg);
#endif

    return;
}



/*******************************************************************************
**
** Function         GKI_get_time_stamp
**
** Description      This function formats the time into a user area
**
** Returns          the address of the user area containing the formatted time
**
*******************************************************************************/
INT8 *GKI_get_time_stamp (INT8 *tbuf)
{
    UINT32 ms_time;
    UINT32 s_time;
    UINT32 m_time;
    UINT32 h_time;
    INT8   *p_out = tbuf;

    ms_time = GET_MSEC;
    s_time  = GET_SEC;
    m_time  = GET_MSEC;
    h_time  = GET_HOUR;

    *p_out++ = (INT8)((h_time / 10) + '0');
    *p_out++ = (INT8)((h_time % 10) + '0');
    *p_out++ = ':';
    *p_out++ = (INT8)((m_time / 10) + '0');
    *p_out++ = (INT8)((m_time % 10) + '0');
    *p_out++ = ':';
    *p_out++ = (INT8)((s_time / 10) + '0');
    *p_out++ = (INT8)((s_time % 10) + '0');
    *p_out++ = ':';
    *p_out++ = (INT8)((ms_time / 10) + '0');
    *p_out++ = (INT8)((ms_time % 10) + '0');
    *p_out++ = ':';
    *p_out   = 0;

    return (tbuf);
}


/*******************************************************************************
**
** Function         GKI_register_mempool
**
** Description      This function registers a specific memory partition to be
**                  used with dynamic memory pool allocation.
**
** Returns          void
**
*******************************************************************************/
void GKI_register_mempool (void *p_mem)
{
    gki_cb.com.p_user_mempool = p_mem;
}

/*******************************************************************************
**
** Function         GKI_os_malloc
**
** Description      This function allocates memory
**
** Returns          the address of the memory allocated, or NULL if failed
**
*******************************************************************************/
void *GKI_os_malloc (UINT32 size)
{
    void    *p_mem = NULL;
    GKI_NU_MALLOC(size, p_mem);
    return p_mem;
}

/*******************************************************************************
**
** Function         GKI_os_free
**
** Description      This function frees memory
**
** Returns          void
**
*******************************************************************************/
void GKI_os_free (void *p_mem)
{
    GKI_NU_DEALLOC(p_mem);
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    tv->tv_sec = GET_SEC;
    tv->tv_usec = GET_MSEC * 1000;
    return 0;
}

/*******************************************************************************
**
** Function         GKI_PrintBufferUsage
**
** Description      Displays Current Buffer Pool summary
**
** Returns          void
**
*******************************************************************************/
void GKI_PrintBufferUsage(UINT8 *p_num_pools, UINT16 *p_cur_used)
{
    int i;
    FREE_QUEUE_T    *p;
    UINT8   num = gki_cb.com.curr_total_no_of_pools;
    UINT16   cur[GKI_NUM_TOTAL_BUF_POOLS];

    APPL_TRACE_DEBUG0("");
    APPL_TRACE_DEBUG0("--- GKI Buffer Pool Summary (R - restricted, P - public) ---");

    APPL_TRACE_DEBUG0("POOL     SIZE  USED  MAXU  TOTAL");
    APPL_TRACE_DEBUG0("------------------------------");
    for (i = 0; i < gki_cb.com.curr_total_no_of_pools; i++)
    {
        p = &gki_cb.com.freeq[i];
        if ((1 << i) & gki_cb.com.pool_access_mask)
        {
            APPL_TRACE_DEBUG5("%02d: (R), %4d, %3d, %3d, %3d",
                        i, p->size, p->cur_cnt, p->max_cnt, p->total);
        }
        else
        {
            APPL_TRACE_DEBUG5("%02d: (P), %4d, %3d, %3d, %3d",
                        i, p->size, p->cur_cnt, p->max_cnt, p->total);
        }
        cur[i] = p->cur_cnt;
    }
    if (p_num_pools)
        *p_num_pools = num;
    if (p_cur_used)
        memcpy(p_cur_used, cur, num*2);
}


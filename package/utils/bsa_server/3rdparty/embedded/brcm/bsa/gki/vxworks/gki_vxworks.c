/****************************************************************************
**
**  Name        gki_vx.c
**
**  Function    this file contains GKI functions ported to vxWorks
**
**  Copyright (c) 1999-2004, WIDCOMM Inc., All Rights Reserved.
**  Proprietary and confidential.
**
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "gki_int.h"

#define _BT_VXWORKS

#ifndef _BT_VXWORKS
#error  vxWorks specific file (gki_vx.c) included but BT_VXWORKS not defined!
#endif

void GKI_yield(int task_entry);

/* arbitrary max number of 2-byte event messages to allow */
#define MAX_EVENT_MSGS 40

/* Define the structure that holds the GKI variables */
#if GKI_DYNAMIC_MEMORY == FALSE
tGKI_CB   gki_cb;
#endif

BOOLEAN GKI_InitDone = FALSE;

extern void LogMsg(UINT32 trace_set_mask, const char *fmt_str, ...);

/*******************************************************************************
 **
 ** Function    GKI_TRACE
 **
 ** Description .
 **
 ** Returns     void
 **
 *******************************************************************************/
void GKI_TRACE(char *fmt, ...)
{
//#if 0
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
//    LogMsg(0,fmt, ap);
    va_end(ap);
//#endif
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
    GKI_TRACE("GKI_init");

    if (GKI_InitDone == TRUE)
    {
        GKI_TRACE("GKI_init was already done (hopefully by server) don't do it again");
        return;
    }
    GKI_InitDone = TRUE;
    memset (&gki_cb, 0, sizeof (tGKI_CB));

    gki_cb.com.OSDisableNesting = 1;

    gki_buffer_init();
    gki_timers_init();

    gki_cb.com.OSTicks = 0;
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
    return (0); /* (UINT32) GetTickCount()); */
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
**                  stack       - (input) pointer to the start of stack
**                  stacksize   - (input) size of the stack allocated for the task
**
** Returns          GKI_SUCCESS if all OK, GKI_FAILURE if any problem
*******************************************************************************/
UINT8 GKI_create_task (TASKPTR task_entry, UINT8 task_id, INT8 *taskname, UINT16 *stack, UINT16 stacksize)
{
    INT32 taskID;
    INT32 task_index;

    GKI_TRACE("GKI_create_task %x %d %s %x %d", task_entry, task_id, taskname,
            stack, stacksize);

    /* specific case used to create a temporary pseudo-task */
    if (task_id == GKI_MAX_TASKS)
    {
        GKI_TRACE("GKI_create_task dynamic/temporary task creation required");
        /* Look for a dead task which can be used */
        for (task_index = APP_TASK + 1 ; task_index < GKI_MAX_TASKS ; task_index++)
        {
            if (gki_cb.com.OSRdyTbl[task_index] == TASK_DEAD)
            {
                GKI_TRACE("GKI_create_task dynamic/temporary task % d created", task_index);
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

    /* Create a message queue for each task, event messages are 16 bits (2 bytes) */
    gki_cb.os.gki_msg_q_id_list[task_id] = msgQCreate(MAX_EVENT_MSGS,2,MSG_Q_FIFO);  /*  vxWorks call */

    /* Only the server Stack task and the client callback task must be created */
    if (task_id <= CBACK_TASK)
    {
        /* Call the vxWorks "task create" routine, save the assigned taskID, create msg Q for event handling */
        /* note: 128 is passed as the vxWorks task priority */
        taskID = taskSpawn((char *) taskname, 128, 0, stacksize, (FUNCPTR)GKI_yield, (int)task_entry,0,0,0,0,0,0,0,0,0);
        GKI_TRACE("GKI_create_task(): %s task_id=%d taskID=%08x msgQId=%08x",taskname,task_id,taskID,gki_cb.os.gki_msg_q_id_list[task_id]);
        if (taskID == ERROR)
        {
            GKI_ERROR_TRACE("taskSpawn fail");
            return (GKI_FAILURE);
        }
        else
        {
            gki_cb.os.gki_map_to_vx_taskid_lut[task_id] = taskID;
        }
        GKI_TRACE("Created thread with thread_id=  %x", taskID);
    }
    else
    {
        /* Task with Id > CBACK_TASK are temporary task created to send/receive req to server */
        taskID = taskIdSelf();  /*  vxWorks call */;
        gki_cb.os.gki_map_to_vx_taskid_lut[task_id] = taskID;
        GKI_TRACE("Use user's thread with thread_id=  %x", taskID);
    }

    gki_cb.com.OSRdyTbl[task_id]    = TASK_READY;
    gki_cb.com.OSTName[task_id]     = taskname;
    gki_cb.com.OSWaitTmr[task_id]   = 0;
    gki_cb.com.OSWaitEvt[task_id]   = 0;
    gki_cb.com.OSStackSize[task_id] = stacksize;

    return (GKI_SUCCESS);
}


/*******************************************************************************
**
** Function         GKI_yield
**
** Description      This function provides an initialization entry point for
**                  newly-spawned GKI tasks. It causes the new GKI task to temporarily
**                  block as soon as it is started.  This allows the vxWorks taskSpawn
**                  call to return with a vxWorks taskID that is used to initialize
**                  the GKI data structure. Otherwise the new task could start making GKI calls
**                  that require referencing the structure, before it was initialized.
**
** Returns          void
**
*******************************************************************************/
void GKI_yield(int task_entry)
{

/* temporarily block the new task while the GKI data structure is being initialized */
taskDelay(10);

/* OK, now run the task starting at the actual entry point requested by the user */
(*(FUNCPTR)task_entry)();

}


/*******************************************************************************
**
** Function         GKI_suspend_task()
**
** Description      This function suspends the task specified in the argument.
**
** Returns          GKI_SUCCESS if all OK, else GKI_FAILURE
**                  It also tries to do context switch.
**                  If task tries to suspend itself, then some other task must
**                  resume it, because it is no longer running.
**
*******************************************************************************/
UINT8 GKI_suspend_task(UINT8 task_id)
{
    if (taskSuspend(gki_cb.os.gki_map_to_vx_taskid_lut[task_id]))  /*  vxWorks call */
        return GKI_FAILURE;
    else
        return GKI_SUCCESS;
}


/*******************************************************************************
**
** Function         GKI_resume_task()
**
** Description      This function resumes the task specified in the argument.
**
** Returns          GKI_SUCCESS if all OK, else GKI_FAILURE
**                  It also tries to do context switch.
**                  If task tries to suspend itself, then some other task must
**                  resume it, because it is no longer running.
**
*******************************************************************************/
UINT8 GKI_resume_task(UINT8 task_id)
{
    if (taskResume(gki_cb.os.gki_map_to_vx_taskid_lut[task_id]))  /*  vxWorks call */
        return GKI_FAILURE;
    else
        return GKI_SUCCESS;
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
    /* Only the server Stack task, the client callback tasks must be killed */
    if (task_id <= CBACK_TASK)
    {
        taskDelete(gki_cb.os.gki_map_to_vx_taskid_lut[task_id]);  /*  vxWorks call */
    }
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
    taskLock();  /*  vxWorks call */
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
    taskUnlock();  /*  vxWorks call */
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
**                  Calling routine must check the returned event bits versus
**                  what it was waiting for, and re-call this routine if
**                  desired event bit(s) not returned.
**
*******************************************************************************/
UINT16 GKI_wait (UINT16 flag, UINT32 timeout)
{
    UINT16  evt = 0;
    UINT8   rtask;
    UINT8   msg_buf[2];
    STATUS  rtn = 5;

    rtask = GKI_get_taskid();

//    GKI_TRACE("GKI_wait enter taskId:%d timeout:%d", rtask, timeout);

    gki_cb.com.OSWaitTmr[rtask] = (UINT16) timeout;
    gki_cb.com.OSWaitForEvt[rtask] = flag;

    evt = gki_cb.com.OSWaitEvt[rtask]; /*  start by picking up any events sent from interrupts or leftover */

/*printf("GKI_wait: rtask=%x evt=%x flag=%x\n",rtask,evt,flag);*/
    if ((evt & flag) != flag) /* If requested events not already received then check for new ones */
    {
        if (timeout)
        {
            if ((rtn=msgQReceive(gki_cb.os.gki_msg_q_id_list[rtask], (char *) msg_buf, sizeof(msg_buf), timeout)) > 0)  /* vxWorks call */
                evt |= (msg_buf[0]<<8 | msg_buf[1]);  /*  gather all the events that have been sent so far */
        }
        else
        {
            msgQReceive(gki_cb.os.gki_msg_q_id_list[rtask], (char *) msg_buf, sizeof(msg_buf), WAIT_FOREVER);  /* gki 0 means forever, vxWorks means immediate */
            evt |= (msg_buf[0]<<8 | msg_buf[1]);  /*  gather all the events that have been sent so far */
        }
    }

    /* Clear only those bits which user wants...save the rest as leftovers */
    gki_cb.com.OSWaitEvt[rtask] = ~flag & evt;

    /* Return only those bits which user wants... */
    evt &= flag;

//    GKI_TRACE("GKI_wait exit taskId:%d", rtask);

/*printf("GKI_wait: returning evt %x, rtn status was %d rtask=%d\n",evt,rtn,rtask);*/
    return evt;
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
//    GKI_TRACE("GKI_delay timeout:%d", timeout);
    timeout = GKI_MS_TO_TICKS(timeout);     /* convert from milliseconds to ticks */

    if (timeout == 0)
        timeout = 1;

    taskDelay((int)timeout);  /*  vxWorks call */
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
    UINT8 msg_buf[2];

//    GKI_TRACE("GKI_send_event task_id:%d event:%d", task_id, event);

    if (task_id >= GKI_MAX_TASKS)
    {
        return (GKI_FAILURE);
    }

    msg_buf[0] = (UINT8) (event >> 8); /* MSB */
    msg_buf[1] = (UINT8) (event & 0xFF); /* LSB */

    taskLock();
    if (msgQSend(gki_cb.os.gki_msg_q_id_list[task_id], (char *) msg_buf, sizeof(msg_buf), NO_WAIT, MSG_PRI_NORMAL))  /* vxWorks call */
    {
        return (GKI_FAILURE);
    }
    taskUnlock();

    return (GKI_SUCCESS);
}


/*******************************************************************************
**
** Function         GKI_get_taskid
**
** Description      This function gets the currently running GKI task ID.
**
**
** Returns          GKI task ID, or 0 if current task is not a GKI task
**
*******************************************************************************/
UINT8 GKI_get_taskid(void)
{
    INT32 vx_taskID;
    UINT8 task_id = 0xFF;
    UINT8 i;

    vx_taskID = taskIdSelf();  /*  vxWorks call */

    for (i=0; i<GKI_MAX_TASKS; i++)
    {
        if ((gki_cb.com.OSRdyTbl[i] == TASK_READY) &&
                (gki_cb.os.gki_map_to_vx_taskid_lut[i] == vx_taskID))
        {
            task_id = i;
            break;
        }
    }
    return task_id;
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
        return ((INT8 *) "BAD");
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
        intUnlock(gki_cb.os.gki_int_lockout_key);  /* vxWorks call */
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

    gki_cb.os.gki_int_lockout_key = intLock();  /* vxWorks call */

    gki_cb.com.OSDisableNesting++;
}


/*******************************************************************************
**
** Function         GKI_exception
**
** Description      This function throws an exception
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
#endif

    GKI_ERROR_TRACE("GKI_exception code:%d msg:%s", code, msg);

    /* TODO:  Put the specific handler or logger here */

    return;
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
    UINT32 ms_time;
    UINT32 s_time;
    UINT32 m_time;
    UINT32 h_time;
    INT8 *p_out = tbuf;
    UINT32 system_time;

    /* Get the current system time: to be done */
    system_time = tickGet();

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

    return (tbuf);
}

/*******************************************************************************
 **
 ** Function         GKI_TimerTask
 **
 ** Description      This function runs the task in charge of updating timer tick
 **
 ** Parameters:
 **
 ** Returns          void
 **
 **
 *******************************************************************************/
static void GKI_TimerTask (void)
{
    while(1)
    {
        /* Sleep 10 ms */
        taskDelay(1);

        /* Update GKI tick (one tick per 10 ms) */
        GKI_timer_update(1);
    }
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
 **
 *******************************************************************************/
void GKI_run(void *p_task_id)
{
    INT32 taskID;

    p_task_id = p_task_id;  /* to prevent compilatiomn warning */

    taskID = taskSpawn((char *) "GKI_Timer", 128, 0, 16*1024, (FUNCPTR)GKI_TimerTask, 0,0,0,0,0,0,0,0,0,0);
    if (taskID == ERROR)
    {
        APPL_TRACE_DEBUG0("GKI_run taskSpawn fail");
    }
    else
    {
        APPL_TRACE_DEBUG0("GKI_run timer task created");
    }
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
    void *mem_ptr = malloc(size);

    if (mem_ptr == NULL)
        printf("GKI_os_malloc(): Not enough memory to fit %d bytes\n", size);

    return (mem_ptr);
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
    free(p_mem);
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
    /* Not implemented on VxWorks */
    return 0;
}


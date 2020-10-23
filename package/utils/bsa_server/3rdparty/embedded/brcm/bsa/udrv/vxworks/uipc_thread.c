/*****************************************************************************
**
**  Name:           pthread.c
**
**  Description:    PTHREAD management
**
**  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "uipc_thread.h"

typedef void *(*PTHREAD_ENTRY)(void*);

static UINT8 task_number = 0;

#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
#include "gki_int.h"
#define UIPC_THREAD_TM_MAX  15
typedef struct
{
    BOOLEAN in_use;
    UINT16 stacksize;
    tTHREAD *p_thread;
}tUIPC_THREAD_TM;

tUIPC_THREAD_TM uipc_thread_tm_cb[UIPC_THREAD_TM_MAX];
#endif /* BSA_TM_INCLUDED */


/*******************************************************************************
 **
 ** Function         create_thread
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int uipc_create_thread(THREAD_ENTRY task_entry, UINT16 *stack, UINT16 stacksize, tTHREAD *p_thread, void *arg)
{
    INT32 taskID;
    UINT8 taskname[30];
#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    int index;
#endif

    sprintf(taskname, "task%d", task_number++);

    taskID = taskSpawn((char *) taskname, 128, 0, stacksize,
        (FUNCPTR)task_entry, (int)arg,0,0,0,0,0,0,0,0,0);
    APPL_TRACE_DEBUG3("create_thread(): %s task_id=%d taskID=%08x", taskname, task_number + 20, taskID);
    if (taskID == ERROR)
    {
        APPL_TRACE_DEBUG0("taskSpawn fail");
        return (-1);
    }
    else
    {
        *p_thread = taskID;
    }
#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    /* Save the task information for Test module (TM) */
    for (index = 0; index < UIPC_THREAD_TM_MAX; index++)
    {
        if (uipc_thread_tm_cb[index].in_use == FALSE)
        {
            uipc_thread_tm_cb[index].in_use = TRUE;
            uipc_thread_tm_cb[index].stacksize = stacksize;
            uipc_thread_tm_cb[index].p_thread = p_thread;
            break;
        }
    }
    if (index >= UIPC_THREAD_TM_MAX)
    {
        APPL_TRACE_WARNING0("create_thread: no memory to save Thread info for TM !!!");
    }
#endif
    return 0;

}


/*******************************************************************************
 **
 ** Function         stop_thread
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int uipc_stop_thread(tTHREAD thread)
{
#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    int index;
#endif

    taskDelete(thread);

    #if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    /* Remove the task information for Test module (TM) */
    for (index = 0; index < UIPC_THREAD_TM_MAX; index++)
    {
        if ((uipc_thread_tm_cb[index].in_use != FALSE) &&
            (*uipc_thread_tm_cb[index].p_thread == thread))
        {
            uipc_thread_tm_cb[index].in_use = FALSE;
            break;
        }
    }
    if (index >= UIPC_THREAD_TM_MAX)
    {
        APPL_TRACE_WARNING0("stop_thread: Thread info not retreived from TM !!!");
    }
#endif

    return 0;
}

#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
/*******************************************************************************
 **
 ** Function         uipc_thread_get_usage
 **
 ** Description      Get thread usage (numbers, stack size, etc.).
 **
 ** Parameters
 **
 ** Returns          Number of thread when ok, -1 if missed tasks
 **
 *******************************************************************************/
int uipc_thread_get_usage(tBSA_TM_TASK_STATUS *p_task_status, UINT16 nb_element)
{
    int index;
    int nb_task = 0;

    /* Look for task information for Test module (TM) */
    for (index = 0; index < UIPC_THREAD_TM_MAX; index++)
    {
        /* If this thread is active */
        if (uipc_thread_tm_cb[index].in_use != FALSE)
        {
            if (nb_element >= 0)
            {
                p_task_status->task_used = TRUE;
                /* Add 50 to do not confuse with GKI tasks */
                p_task_status->task_id = 50 + index;
                p_task_status->task_state = TASK_READY;
                p_task_status->stack_size = uipc_thread_tm_cb[index].stacksize;
                p_task_status->stack_used = 0; /* todo */
                strncpy((char *)p_task_status->task_name, "UIPC", BSA_TM_TASK_NAME_MAX);
                p_task_status++;
                nb_element--;
                nb_task++;
            }
            else
            {
                APPL_TRACE_WARNING0("uipc_thread_get_usage: task missed");
                nb_task = -1;
            }
        }
    }
    return nb_task;
}
#endif



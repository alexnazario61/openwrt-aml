/*****************************************************************************
 **
 **  Name:           uipc_thread.c
 **
 **  Description:    Thread management API
 **
 **  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "buildcfg.h"
#include "bsa_api.h"
#include "bsa_int.h"

#include "uipc_thread.h"
#include "gki_target.h"
#include "uipc_thread.h"

typedef void *(*PTHREAD_ENTRY)(void*);

#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
#include "gki_int.h"
#ifndef UIPC_THREAD_TM_MAX
#define UIPC_THREAD_TM_MAX  15
#endif

//#define UIPC_THREAD_DEBUG

tUIPC_THREAD_TM uipc_thread_tm_cb[UIPC_THREAD_TM_MAX];
#endif /* BSA_TM_INCLUDED */

extern unsigned char gBasePriority;

#ifdef NUCLEUS_BASE_PRIORITY
static UINT8 thread_id = NUCLEUS_BASE_PRIORITY + GKI_MAX_TASKS + 1;
#else
static UINT8 thread_id = 28 + GKI_MAX_TASKS + 1;
#endif

/*******************************************************************************
 **
 ** Function         uipc_thread_create
 **
 ** Description
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int uipc_thread_create(THREAD_ENTRY task_entry, UINT8 *task_name,
        UINT16 *stack, UINT16 stacksize, NU_TASK *p_thread, void *arg)
{
    STATUS status = 0;
    VOID *         pointer;
    NU_MEMORY_POOL *sys_pool;
    UINT8 priority_id;

    /* Thread ID correction */
#ifdef NUCLEUS_BASE_PRIORITY
    thread_id = thread_id - NUCLEUS_BASE_PRIORITY + gBasePriority;
#else
    thread_id = thread_id - 28 + gBasePriority;
#endif
    priority_id = thread_id - gBasePriority;
#ifdef UIPC_THREAD_DEBUG
    APPL_TRACE_DEBUG2("uipc_thread_create: task_name:%s thread_id:%d",
            task_name, thread_id);
#endif
    /* Allocate memory for the UIPC Thread task */
    pointer = GKI_os_malloc(stacksize);

    status = NU_Create_Task(p_thread, "UIPC TASK", task_entry, priority_id, arg,
            pointer, stacksize, thread_id, 20, NU_PREEMPT, NU_START);

    if (status != NU_SUCCESS)
    {
        APPL_TRACE_ERROR1("uipc_thread_create NU_Create_Task failed:%d", status);
        return -1;
    }
    APPL_TRACE_DEBUG1("uipc_thread_create NU_Create_Task(task_id:%d) Success!", priority_id);
    thread_id++;
    return status;
}

/*******************************************************************************
 **
 ** Function         uipc_thread_stop
 **
 ** Description      Function that must be called when finishing a thread.  This
 **                  function does not end the thread but clears its entry from
 **                  the internal tables.
 **
 ** Parameters
 **
 ** Returns          The status of the execution.
 **
 *******************************************************************************/
int uipc_thread_stop(NU_TASK thread)
{
    STATUS status = 0;

    APPL_TRACE_DEBUG0("uipc_thread_stop");

    status = NU_Delete_Task(&thread);
    APPL_TRACE_DEBUG1("uipc_thread_stop() NU_Delete_Task status:%d", status);
    GKI_os_free(thread.tc_stack_pointer);
    return status;
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
            if (nb_element > 0)
            {
                p_task_status->task_used = TRUE;
                /* Add 50 to do not confuse with GKI tasks */
                p_task_status->task_id = 50 + index;
                p_task_status->task_state = TASK_READY;
                p_task_status->stack_size = uipc_thread_tm_cb[index].stacksize;
                p_task_status->stack_used = 0; /* todo */
                strncpy((char *)p_task_status->task_name,
                        (char *)uipc_thread_tm_cb[index].name,
                        BSA_TM_TASK_NAME_MAX);
                /* make sure the string is terminated */
                p_task_status->task_name[BSA_TM_TASK_NAME_MAX-1] = 0;
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



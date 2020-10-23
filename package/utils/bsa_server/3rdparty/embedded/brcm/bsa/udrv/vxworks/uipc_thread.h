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

#ifndef THREAD_H
#define THREAD_H


typedef void (THREAD_ENTRY)(void *arg);
typedef  INT32		tTHREAD;


/*******************************************************************************
 **
 ** Function         create_thread
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int uipc_create_thread(THREAD_ENTRY task_entry, UINT16 *stack, UINT16 stacksize, tTHREAD *p_thread, void *arg);

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
int uipc_stop_thread(tTHREAD thread);

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
#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
int uipc_thread_get_usage(tBSA_TM_TASK_STATUS *p_task_status, UINT16 nb_element);
#endif

#endif


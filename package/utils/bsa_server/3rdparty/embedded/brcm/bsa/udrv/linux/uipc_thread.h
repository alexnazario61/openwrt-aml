/*****************************************************************************
 **
 **  Name:           uipc_thread.h
 **
 **  Description:    Thread management API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_THREAD_H
#define UIPC_THREAD_H

#include <pthread.h>
#include "bsa_int.h"

/*
 * defines
 */

typedef void (THREAD_ENTRY)(void *arg);
typedef pthread_t tTHREAD;


/*******************************************************************************
 **
 ** Function         uipc_thread_create
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int uipc_thread_create(THREAD_ENTRY task_entry, UINT8 *task_name,
        UINT16 *stack, UINT16 stacksize, tTHREAD *p_thread, void *arg);

/*******************************************************************************
 **
 ** Function         uipc_thread_stop
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int uipc_thread_stop(tTHREAD thread);

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


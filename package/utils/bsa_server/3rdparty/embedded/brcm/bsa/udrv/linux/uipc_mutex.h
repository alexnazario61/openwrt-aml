/*****************************************************************************
**
**  Name:           uipc_mutex.h
**
**  Description:    Mutex management API
**
**  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef UIPC_MUTEX_H
#define UIPC_MUTEX_H

#include <pthread.h>

typedef pthread_mutex_t tMUTEX;

/*******************************************************************************
 **
 ** Function         init_mutex
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int init_mutex(tMUTEX *mutex);

/*******************************************************************************
 **
 ** Function         delete_sema
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int delete_mutex(tMUTEX *mutex);

/*******************************************************************************
 **
 ** Function         lock_mutex
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int lock_mutex(tMUTEX *mutex);

/*******************************************************************************
 **
 ** Function         unlock_mutex
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int unlock_mutex(tMUTEX *mutex);

#endif


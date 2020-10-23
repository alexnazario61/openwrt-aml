/*****************************************************************************
**
**  Name:           uipc_mutex.h
**
**  Description:    Mutex management API for Nucleus Platform
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef UIPC_MUTEX_H
#define UIPC_MUTEX_H

#include "uipc_bsa.h"
#include "nucleus.h"

typedef NU_SEMAPHORE tMUTEX;

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


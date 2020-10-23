/*****************************************************************************
**
**  Name:           bsa_mutex.h
**
**  Description:    Functions in charge of Linux mutex management
**
**  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef BSA_MUTEX_H
#define BSA_MUTEX_H

#include <vxWorks.h>
#include <semLib.h>

typedef SEM_ID		tMUTEX;

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


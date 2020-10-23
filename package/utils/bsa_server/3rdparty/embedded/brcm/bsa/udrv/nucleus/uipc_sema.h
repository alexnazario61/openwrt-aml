/*****************************************************************************
**
**  Name:           uipc_sema.h
**
**  Description:    Functions in charge of Linux semaphores management
**
**  Copyright (c) 2010-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#ifndef UIPC_SEMA_H
#define UIPC_SEMA_H

#include "nucleus.h"

typedef NU_SEMAPHORE *tUIPC_SEMA;

/*******************************************************************************
 **
 ** Function         uipc_sema_get_val
 **
 ** Description      Get semaphore value
 **
 ** Parameters       semid: semaphore identifier returned by create operation
 **
 ** Returns          The current semaphore value, negative value if unsuccessful
 **
 *******************************************************************************/
int uipc_sema_get_val(tUIPC_SEMA semid);

/*******************************************************************************
 **
 ** Function         uipc_sema_set_val
 **
 ** Description      Set semaphore value
 **
 ** Parameters       semid: semaphore identifier returned by create operation
 **                  value: value to set in the semaphore
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int uipc_sema_set_val(tUIPC_SEMA semid, int value);

/*******************************************************************************
 **
 ** Function         uipc_sema_lock
 **
 ** Description      Lock a semaphore
 **
 ** Parameters       semid: semaphore identifier returned by create operation
 **                  num: number of elements to wait for
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int uipc_sema_lock(tUIPC_SEMA semid, short num);

/*******************************************************************************
 **
 ** Function         uipc_sema_unlock
 **
 ** Description      Unlock a semaphore
 **
 ** Parameters       semid: semaphore identifier returned by create operation
 **                  num: number of elements to release
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int uipc_sema_unlock(tUIPC_SEMA semid, short num);

/*******************************************************************************
 **
 ** Function         uipc_sema_create
 **
 ** Description      Create a semaphore and make sure there is no
 **                  conflict in the initialization
 **
 ** Parameters       key: unique key identifier, can be same as shared memory
 **                       but not as another semaphore
 **                  initval: initialization value of the semaphore
 **
 ** Returns          semaphore identifier, -1 in case of error
 **
 *******************************************************************************/
tUIPC_SEMA uipc_sema_create(key_t key, short initval);

/*******************************************************************************
 **
 ** Function         uipc_sema_get
 **
 ** Description      Get an already created semaphore
 **
 ** Parameters       key: unique key identifier, can be same as shared memory
 **                       but not as another semaphore
 **
 ** Returns          semaphore identifier, -1 in case of error
 **
 *******************************************************************************/
tUIPC_SEMA uipc_sema_get(key_t key);

/*******************************************************************************
 **
 ** Function         uipc_sema_delete
 **
 ** Description      Destroy a semaphore
 **
 ** Parameters       semid: semaphore identifier returned by create operation
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int uipc_sema_delete(tUIPC_SEMA semid);

#endif


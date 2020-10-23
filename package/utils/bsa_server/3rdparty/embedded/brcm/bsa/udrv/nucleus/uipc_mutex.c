/*****************************************************************************
**
**  Name:           uipc_mutex.c
**
**  Description:    Mutex management API for Nucleus Platform
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "uipc_mutex.h"

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
int init_mutex(tMUTEX *mutex)
{

    STATUS status; /* Semaphore creation status */
    /* Create a semaphore with an initial count of 1 and priority
    order task suspension. */
    status = NU_Create_Semaphore(mutex, "UIPC-Sema", 1,
            NU_PRIORITY);
    return (status == NU_SUCCESS)?0:1;
}

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
int delete_mutex(tMUTEX *mutex)
{
    STATUS status; /* Semaphore deletion status */
    status = NU_Delete_Semaphore(mutex);
    return (status == NU_SUCCESS)?0:1;
}

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
int lock_mutex(tMUTEX *mutex)
{
    STATUS status; /* Semaphore lock-obtaining status */
    NU_Obtain_Semaphore(mutex, NU_SUSPEND);
    return (status == NU_SUCCESS)?0:1;
}

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
int unlock_mutex(tMUTEX *mutex)
{
    STATUS status; /* Semaphore lock-releasing status */
    status = NU_Release_Semaphore(mutex);
    return (status == NU_SUCCESS)?0:1;
}

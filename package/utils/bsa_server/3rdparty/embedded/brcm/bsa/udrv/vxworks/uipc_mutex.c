/*****************************************************************************
**
**  Name:           linux_mutex.c
**
**  Description:    Functions in charge of Linux mutex management
**
**  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <vxWorks.h>
#include <semLib.h>
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
    *mutex = semBCreate (SEM_Q_PRIORITY, SEM_FULL);
    return 0;
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
    semDelete(*mutex);
	return 0;
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
    semTake (*mutex, WAIT_FOREVER);
	return 0;
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
    semGive (*mutex);
	return 0;
}

/*****************************************************************************
**
**  Name:           uipc_mutex.c
**
**  Description:    Mutex management API
**
**  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>



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
int init_mutex(pthread_mutex_t *mutex)
{
    int status;

    status = pthread_mutex_init (mutex, NULL);
    if(status != 0)
    {
        perror("init_mutex pthread_mutex_init failed Reason:");
    }
    return status;

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
int delete_mutex(pthread_mutex_t *mutex)
{
    int status;

    status = pthread_mutex_destroy(mutex);
    if(status != 0)
    {
        perror("delete_mutex pthread_mutex_destroy failure while destroying Reason:");
        return (-1);
    }
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
int lock_mutex(pthread_mutex_t *mutex)
{
    int status;

    /* locking the mutex */
    status = pthread_mutex_lock(mutex);

    if(status != 0)
    {
        perror("lock_mutex pthread_mutex_lock failure Reason:");
    }
    return status;
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
int unlock_mutex(pthread_mutex_t *mutex)
{
    int status;

    /* unlocking the mutex */
    status = pthread_mutex_unlock(mutex);

    if(status != 0)
    {
        perror("unlock_mutex pthread_mutex_unlock failure Reason:");
    }
    return status;
}

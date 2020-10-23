/*****************************************************************************
**
**  Name:           uipc_sema.c
**
**  Description:    Functions in charge of Linux semaphores management
**
**  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "buildcfg.h"
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"
#else
#include "bsa_api.h"
#include "bsa_int.h"
#endif

#include "uipc_bsa.h"
#include "uipc_sema.h"

#if defined(__FreeBSD__)
typedef union semun semun_t;
#else
typedef union semun
{
    int val;
    struct semid_ds *buf;
    ushort * array;
} semun_t;
#endif

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
int uipc_sema_get_val(tUIPC_SEMA semid)
{
    semun_t arg;
    int rv;

    /* Get the Semaphore's value */
    rv = semctl(semid, 0, GETVAL, arg);
    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sema_get_val: semctl fail:%d", rv);
    }
    return rv;
}

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
int uipc_sema_set_val(tUIPC_SEMA semid, int value)
{
    semun_t arg;
    int rv;
    arg.val = value;

    /* Set the Semaphoreś value */
    rv = semctl(semid, 0, SETVAL, arg);
    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sema_set_val: semctl fail:%d", rv);
    }
    return rv;
}

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
int uipc_sema_lock(tUIPC_SEMA semid, short num)
{
    struct sembuf sb;
    int rv;
    sb.sem_num = 0;
    sb.sem_op = 0 - num;
    sb.sem_flg = 0;

    /* Lock the Semaphore */
    rv = semop(semid, &sb, 1);
    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sema_lock: semop fail:%d", rv);
    }
    return rv;
}

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
int uipc_sema_unlock(tUIPC_SEMA semid, short num)
{
    struct sembuf sb;
    int rv;
    sb.sem_num = 0;
    sb.sem_op = num;
    sb.sem_flg = 0;

    /* Unlock the Semaphore */
    rv = semop(semid, &sb, 1);
    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sema_unlock: semop fail:%d", rv);
    }
    return rv;
}

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
tUIPC_SEMA uipc_sema_create(key_t key, short initval)
{
    tUIPC_SEMA semid;

    /* Create the semaphore (fails if already exists) */
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid >= 0)
    {
        /* First creation: initialize the semaphore (set to value) */
        uipc_sema_unlock(semid, initval);
    }
    else
    {
        /* Only supported error is that semaphore already exists */
        if (errno != EEXIST)
        {
            APPL_TRACE_ERROR1("uipc_sema_create: semget fail errno:%d", errno);
            return -1;
        }
        APPL_TRACE_WARNING0("uipc_sema_create Semaphore already exists");
        /* Already exists: get the id of the semaphore */
        semid = semget(key, 1, 0);
        if (semid < 0)
        {
            APPL_TRACE_ERROR1("uipc_sema_create: semget fail:%d", semid);
            return -1;
        }
        /* Delete it */
        if (uipc_sema_delete(semid) < 0)
        {
            APPL_TRACE_ERROR0("uipc_sema_create: uipc_sema_delete fail");
            return -1;
        }
        /* Re-Create the semaphore (must not fail now) */
        semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
        if (semid < 0)
        {
            APPL_TRACE_ERROR1("uipc_sema_create: semget(2) fail errno:%d", errno);
            return -1;
        }
        /* First creation: initialize the semaphore (set to value) */
        uipc_sema_unlock(semid, initval);
    }
    return semid;
}

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
tUIPC_SEMA uipc_sema_get(key_t key)
{
    tUIPC_SEMA semid;

    /* Get the semaphore */
    semid = semget(key, 1, 0);
    if (semid < 0)
    {
        APPL_TRACE_ERROR1("uipc_sema_get: semget fail:%d", semid);
        return -1;
    }

    return semid;
}

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
int uipc_sema_delete(tUIPC_SEMA semid)
{
    semun_t arg;
    int rv;

    /* Remove the Semaphore */
    rv = semctl(semid, 0, IPC_RMID, arg);
    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sema_delete: semctl fail:%d", rv);
    }
    return rv;
}


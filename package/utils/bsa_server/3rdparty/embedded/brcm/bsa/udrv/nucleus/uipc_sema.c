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
#include "buildcfg.h"
#include "bsa_api.h"
#include "bsa_int.h"

#include "uipc_bsa.h"
#include "uipc_sema.h"

typedef union semun
{
    int val;
    UINT8 *array;
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
    /* TODO */
    return 0;
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
    /* TODO */
    return 0;
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
    /* Obtain the Semaphore */
    status = NU_Obtain_Semaphore(semid, NU_SUSPEND);
    if(STATUS != NU_SUCCESS)
    {
        APPL_TRACE_ERROR1("uipc_sema_delete: NU_Obtain_Semaphore fail status:%d",
            status);
        return -1;
    }
    return 0;
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
    /* Release the Semaphore */
    status = NU_Release_Semaphore(semid);
    if(STATUS != NU_SUCCESS)
    {
        APPL_TRACE_ERROR1("uipc_sema_delete: NU_Release_Semaphore fail status:%d",
            status);
        return -1;
    }
    return 0;
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
    STATUS status;

    status = NU_Create_Semaphore(semid, "SEMA", initval, NU_FIFO);

    if(STATUS != NU_SUCCESS)
    {
        APPL_TRACE_ERROR1("uipc_sema_create: NU_Create_Semaphore fail status:%d",
            status);
        return NULL;
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
    STATUS status;
    /* Delete the Semaphore */
    status = NU_Delete_Semaphore(semid);
    if(STATUS != NU_SUCCESS)
    {
        APPL_TRACE_ERROR1("uipc_sema_delete: NU_Delete_Semaphore fail status:%d",
            status);
        return -1;
    }
    return 0;
}


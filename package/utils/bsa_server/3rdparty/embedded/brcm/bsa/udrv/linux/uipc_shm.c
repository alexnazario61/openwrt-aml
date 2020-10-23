/*****************************************************************************
**
**  Name:           uipc_shm.c
**
**  Description:    Functions in charge of Linux Shared Memory management
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include <errno.h>
#include <sys/shm.h>

#include "bt_target.h"
#include "uipc_shm.h"
#include "bt_trace.h"



/*******************************************************************************
 **
 ** Function         uipc_shm_create
 **
 ** Description      Create a Shared Memory
 **
 ** Parameters       key: unique key identifier, can be same as shared memory
 **                       but not as another semaphore
 **                  length: length of the shared memory
 **                  pp_shmem: pointer on the shared memory
 **
 ** Returns          Shared Memory Id, -1 in case of error
 **
 *******************************************************************************/
tUIPC_SHMID uipc_shm_create(key_t key, UINT32 length, void **pp_shmem)
{
    tUIPC_SHMID shmid;
    int rv;

    if (pp_shmem == NULL)
    {
        APPL_TRACE_ERROR0("uipc_shm_create: *pp_shmem is NULL");
        return -1;
    }

    /* Create the Shared Memory (fails if already exists) */
    shmid = shmget(key, (size_t)length, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0)
    {
        /* Only supported error is that Shared Memory already exists */
        if (errno != EEXIST)
        {
            APPL_TRACE_ERROR1("uipc_shm_create: shmget fail errno=%d", errno);
            return -1;
        }
        APPL_TRACE_WARNING0("uipc_shm_create Shared Memory already exists");
        /* Already exists: get the id of the Shared Memory */
        shmid = shmget(key, (size_t)length, 0666);
        if (shmid < 0)
        {
            APPL_TRACE_ERROR1("uipc_shm_create: semget(2) fail errno=%d", errno);
            return -1;
        }
        /* Delete it */
        rv = shmctl(shmid, IPC_RMID, 0);
        if (rv < 0)
        {
            APPL_TRACE_ERROR1("uipc_shm_create: shmctl fail errno=%d", errno);
            return -1;
        }
        /* Re-Create the Shared Memory */
        shmid = shmget(key, (size_t)length, IPC_CREAT | IPC_EXCL | 0666);
        if (shmid < 0)
        {
            APPL_TRACE_ERROR1("uipc_shm_create: semget(3) fail errno=%d", errno);
            return -1;
        }
    }

    /* Attach the segment */
    *pp_shmem = shmat(shmid, NULL, 0);
    if (*pp_shmem == (void *)-1)
    {
        APPL_TRACE_ERROR1("uipc_shm_create: shmat fail errno=%d", errno);
        rv = shmctl(shmid, IPC_RMID, 0);
        if (rv < 0)
        {
            APPL_TRACE_ERROR1("uipc_shm_create: shmctl(2) fail errno=%d", errno);
        }
        return -1;
    }

    return shmid;
}

/*******************************************************************************
 **
 ** Function         uipc_shm_get
 **
 ** Description      Get an already created Shared Memory
 **
 ** Parameters       key: unique key identifier, can be same as shared memory
 **                       but not as another semaphore
 **                  length: length of the shared memory
 **                  pp_shmem: pointer on the shared memory
 **
 ** Returns          Shared Memory Id, -1 in case of error
 **
 *******************************************************************************/
tUIPC_SHMID uipc_shm_get(key_t key, UINT32 length, void **pp_shmem)
{
    tUIPC_SHMID shmid;

    if (pp_shmem == NULL)
    {
        APPL_TRACE_ERROR0("uipc_shm_create: *pp_shmem is NULL");
        return -1;
    }

    /* Create the Shared Memory (fails if already exists) */
    shmid = shmget(key, (size_t)length, 0666);
    if (shmid < 0)
    {
        APPL_TRACE_ERROR1("uipc_shm_get: shmget fail errno=%d", errno);
        return -1;
    }

    /* Attach the segment */
    *pp_shmem = shmat(shmid, NULL, 0);
    if (*pp_shmem == (void *)-1)
    {
        APPL_TRACE_ERROR1("uipc_shm_create: shmat fail errno=%d", errno);
        return -1;
    }

    return shmid;
}

/*******************************************************************************
 **
 ** Function        uipc_shm_detach
 **
 ** Description     Detach a Shared Memory
 **
 ** Parameters      shmid: Shared Memory Id
 **                 p_shmem: pointer on the shared memory
 **
 ** Returns         status
 **
 *******************************************************************************/
int uipc_shm_detach(tUIPC_SHMID shmid, void *p_shmem)
{
    int rv;
    struct shmid_ds shmds;

    /* Detach the Shared Memory */
    rv = shmdt(p_shmem);
    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_shm_detach shmdt fail errno=%d", errno);
        return -1;
    }

    /* Retrieve the information about the segment */
    rv = shmctl(shmid, IPC_STAT, &shmds);
    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_shm_detach shmctl fail errno=%d", errno);
        return -1;
    }

    /* If no more process attached to this Shared Memory */
    if (shmds.shm_nattch == 0)
    {
        APPL_TRACE_DEBUG0("uipc_shm_detach delete Shared Memory");

        /* Destroy the SHM */
        rv = shmctl(shmid, IPC_RMID, 0);
        if (rv < 0)
        {
            APPL_TRACE_ERROR1("uipc_shm_detach shmctl(2) fail errno=%d", errno);
            return -1;
        }
    }
    else
    {
        APPL_TRACE_DEBUG1("uipc_shm_detach do not delete Shared Memory shm_nattch=%d",
                shmds.shm_nattch);
    }
    return rv;
}



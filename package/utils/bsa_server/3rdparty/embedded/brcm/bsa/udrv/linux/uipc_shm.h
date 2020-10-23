/*****************************************************************************
**
**  Name:           uipc_shm.h
**
**  Description:    Functions in charge of Linux Shared Memory management
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef UIPC_SHM_H
#define UIPC_SHM_H

#include <sys/types.h>
#include "data_types.h"

/*
 * Definitions
 */
typedef int tUIPC_SHMID;

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
tUIPC_SHMID uipc_shm_create(key_t key, UINT32 length, void **pp_shmem);

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
tUIPC_SHMID uipc_shm_get(key_t key, UINT32 length, void **pp_shmem);

/*******************************************************************************
 **
 ** Function         uipc_shm_detach
 **
 ** Description      Detach a Shared Memory
 **
  ** Parameters      shmid: Shared Memory Id
 **                 p_shmem: pointer on the shared memory
 **
 ** Returns         status
 **
 *******************************************************************************/
int uipc_shm_detach(tUIPC_SHMID shmid, void *p_shmem);

#endif


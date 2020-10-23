/****************************************************************************
**
**  Name        gki_int.h
**
**  Function    This file contains GKI private definitions
**
**
**  Copyright (c) 1999-2004, WIDCOMM Inc., All Rights Reserved.
**  Proprietary and confidential.
**
*****************************************************************************/
#ifndef GKI_INT_H
#define GKI_INT_H

#include "gki_common.h"
#include "gki_target.h"

#include "nucleus.h"

/**********************************************************************
** Nucleus specific definitions
*/
typedef struct
{
    NU_TASK         OSTCB[GKI_MAX_TASKS];
    NU_EVENT_GROUP  OSEvtGrp[GKI_MAX_TASKS];
    NU_PROTECT      OSProtect;
    int             OSLastIntLevel;
    UINT8           priority[GKI_MAX_TASKS];
    unsigned char   OSBasePriority;

} tGKI_OS;


/* Contains common control block as well as OS specific variables */
typedef struct
{
    tGKI_OS     os;
    tGKI_COM_CB com;
} tGKI_CB;

#ifdef __cplusplus
extern "C" {
#endif

#if GKI_DYNAMIC_MEMORY == FALSE
GKI_API extern tGKI_CB  gki_cb;
#else
GKI_API extern tGKI_CB *gki_cb_ptr;
#define gki_cb (*gki_cb_ptr)
#endif

#define GKI_SET_BASE_PRIORITY(i) { gki_cb.os.OSBasePriority = i; }

#ifndef GKI_SYSTEM_TIMESTAMP
#define GKI_SYSTEM_TIMESTAMP TRUE
#endif
#ifndef GKI_TIMESTAMP_CLOCK
#define GKI_TIMESTAMP_CLOCK CLOCK_MONOTONIC
#endif
#define GKI_MAX_TIMESTAMP_BUF_SIZE 40

#ifdef __cplusplus
}
#endif

#endif

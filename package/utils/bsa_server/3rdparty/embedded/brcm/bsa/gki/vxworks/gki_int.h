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
//#include "ioLib.h"
//#include "vxWorks.h"

/**********************************************************************
** VxWorks specific definitions
*/
typedef struct
{
    /* interrupt lockout key returned from vxWorks disable interrupts routine,
       required by re-enable interrupts routine */
    int      gki_int_lockout_key;

    /* for vxWorks only, define LookUpTable for
       (8-bit GKI task_id) -> (32-bit vxWorks taskID) */
    INT32    gki_map_to_vx_taskid_lut[GKI_MAX_TASKS];

    /* there will be one message queue for each task used to receive events */
    MSG_Q_ID gki_msg_q_id_list[GKI_MAX_TASKS];
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

#ifdef __cplusplus
}
#endif

#endif

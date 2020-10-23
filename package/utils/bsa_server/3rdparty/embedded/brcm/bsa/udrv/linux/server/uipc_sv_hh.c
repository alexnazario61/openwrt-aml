/*****************************************************************************
 **
 **  Name:           uipc_sv_hh.c
 **
 **  Description:    Server API for BSA UIPC HH
 **
 **  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bt_target.h"
#include "bsa_api.h"
#include "bsa_int.h"
#include "uipc.h"
#include "bsa_task.h"
#include "uipc_sv_hh.h"
#include "uipc_fifo.h"

//#define DEBUG_BSA_SV_HH

#define BSA_HH_FIFO_NAME "./bt-hh-fifo"

#ifndef UIPC_SV_TX_HH_MODE
/* Create it, Write direction, Write blocks */
#define UIPC_SV_TX_HH_MODE (UIPC_FIFO_MODE_CREATE | UIPC_FIFO_MODE_WRITE  | UIPC_FIFO_MODE_TX_DATA_BLOCK)
#endif

typedef struct
{
    tUIPC_FIFO_DESC desc;
} tUIPC_SV_HH_CB;

static tUIPC_SV_HH_CB uipc_sv_hh_cb;

/*
 * Local static function to UIPC callback
 */

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_init
 **
 ** Description      Initialize the HH UIPC channel.
 **
 ** Parameters       None.
 **
 ** Returns          TRUE if successful, FALSE otherwise.
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_init(void)
{
    uipc_sv_hh_cb.desc = UIPC_FIFO_BAD_DESC;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_open
 **
 ** Description      Open an HH UIPC channel.
 **
 ** Parameters
 **
 ** Returns          TRUE if successful, FALSE otherwise.
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    /* Open a fifo in Tx/write direction */
    uipc_sv_hh_cb.desc = uipc_fifo_open((UINT8*)BSA_HH_FIFO_NAME, channel_id, NULL, UIPC_SV_TX_HH_MODE);
    if (uipc_sv_hh_cb.desc == UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sv_hh_open fails to open Tx hh channel");
        return FALSE;
    }
    
    APPL_TRACE_DEBUG0("uipc_sv_hh_open hh channel opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_close
 **
 ** Description      Close an HH UIPC channel.
 **
 ** Parameters
 **
 ** Returns          None
 **
 *******************************************************************************/
void uipc_sv_hh_close(tUIPC_CH_ID channel_id)
{
    uipc_fifo_close(uipc_sv_hh_cb.desc, TRUE);
}

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen)
{
    BOOLEAN uipc_status;

#ifdef DEBUG_BSA_SV_HH
    APPL_TRACE_DEBUG1("uipc_sv_hh_send length:%d", msglen);
#endif

    uipc_status = uipc_fifo_send(uipc_sv_hh_cb.desc, p_buf, msglen);

    return uipc_status;
}


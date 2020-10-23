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
#include "uipc_channel.h"

/*
#define DEBUG_UIPC_SV_HH
*/

#ifndef UIPC_SV_TX_HH_MODE
/* Create it, Write direction, Write blocks */
#define UIPC_SV_TX_HH_MODE (UIPC_CHANNEL_MODE_CREATE | UIPC_CHANNEL_MODE_WRITE  | UIPC_CHANNEL_MODE_TX_DATA_BLOCK)
#endif

static tUIPC_CHANNEL_DESC   uipc_sv_hh_desc = UIPC_CHANNEL_BAD_DESC;

/*
 * Local static function to UIPC callback
 */

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_init(void)
{
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    if (uipc_sv_hh_desc != UIPC_CHANNEL_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sv_hh_open UIPC HH was already open.");
        APPL_TRACE_ERROR0("uipc_sv_hh_open Reuse this connection.");
        return TRUE; /* not an error */
    }

    /* Open a channel in Tx/write direction */
    uipc_sv_hh_desc = uipc_channel_open(channel_id, NULL, UIPC_SV_TX_HH_MODE);
    if (uipc_sv_hh_desc == UIPC_CHANNEL_BAD_DESC)
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
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_hh_close(tUIPC_CH_ID channel_id)
{
    uipc_channel_close(uipc_sv_hh_desc);
    uipc_sv_hh_desc = UIPC_CHANNEL_BAD_DESC;
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

#ifdef DEBUG_UIPC_SV_HH
    APPL_TRACE_DEBUG1("uipc_sv_hh_send length:%d", msglen);
/*    scru_dump_hex(p_buf, NULL, msglen, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);*/
#endif

    uipc_status = uipc_channel_send(uipc_sv_hh_desc, p_buf, msglen);

    return uipc_status;
}


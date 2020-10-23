/*****************************************************************************
 **
 **  Name:           uipc_cl_hh.c
 **
 **  Description:    Server API for BSA UIPC HH
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bt_target.h"
#include "bsa_api.h"
#include "bsa_int.h"
#include "uipc.h"
#include "bsa_client.h"
#include "uipc_cl_hh.h"
#include "uipc_channel.h"

/*
#define DEBUG_UIPC_CL_HH
*/

#ifndef UIPC_CL_TX_HH_MODE
/* Read direction, Data event */
#define UIPC_CL_TX_HH_MODE (UIPC_CHANNEL_MODE_READ | UIPC_CHANNEL_MODE_RX_DATA_EVT)
#endif

static tUIPC_CHANNEL_DESC   uipc_cl_hh_desc = UIPC_CHANNEL_BAD_DESC;

static tUIPC_RCV_CBACK *p_uipc_cl_hh_cback = NULL;

/*
 * Local static function to UIPC callback
 */
static tUIPC_RCV_CBACK uipc_cl_hh_cback;

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_init()
{
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    if (uipc_cl_hh_desc != UIPC_CHANNEL_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_cl_hh_open UIPC HH was already open.");
        APPL_TRACE_ERROR0("uipc_cl_hh_open Reuse this connection.");
        return TRUE; /* not an error */
    }

    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_hh_open Null callback.");
        return FALSE;
    }

    p_uipc_cl_hh_cback = p_cback;  /* Save callback function */

    /* Open a channel in Rx/Read direction */
    uipc_cl_hh_desc = uipc_channel_open(channel_id, uipc_cl_hh_cback, UIPC_CL_TX_HH_MODE);
    if (uipc_cl_hh_desc == UIPC_CHANNEL_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_cl_hh_open fails to open Rx control channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_cl_hh_open hh channel opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_hh_close(tUIPC_CH_ID channel_id)
{
    uipc_channel_close(uipc_cl_hh_desc);
    uipc_cl_hh_desc = UIPC_CHANNEL_BAD_DESC;
    p_uipc_cl_hh_cback = NULL;
}


/*******************************************************************************
 **
 ** Function         uipc_cl_hh_cback
 **
 ** Description      This function Handle UIPC callback
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
static void uipc_cl_hh_cback(BT_HDR *p_msg)
{
    BOOLEAN free_buf = TRUE;
#ifdef DEBUG_UIPC_CL_HH
    APPL_TRACE_DEBUG0("uipc_cl_hh_cback p_msg called");
#endif

    /*  If this is a connection event, call the BSA server cback */
    if (p_msg->event == UIPC_CHANNEL_OPEN)
    {
#ifdef DEBUG_UIPC_CL_HH
        APPL_TRACE_DEBUG0("uipc_cl_hh_cback Connect event");
#endif
    }
    /*  If this is a disconnection event, call the BSA server cback */
    else if (p_msg->event == UIPC_CHANNEL_CLOSE)
    {
#ifdef DEBUG_UIPC_CL_HH
        APPL_TRACE_DEBUG0("uipc_cl_hh_cback disconnect event");
#endif
    }
    /*  If this is an Rx data indication  */
    else if (p_msg->event == UIPC_CHANNEL_DATA_RX)
    {
        p_msg->event = UIPC_RX_DATA_EVT;
#ifdef DEBUG_UIPC_CL_HH
        APPL_TRACE_DEBUG0("uipc_cl_hh_cback Rx Data event");
#endif
        if (p_uipc_cl_hh_cback != NULL)
        {
            p_uipc_cl_hh_cback(p_msg);
            free_buf = FALSE;       /* Callback will free the buffer */
        }
        else
        {
            APPL_TRACE_ERROR0("ERROR uipc_cl_hh_cback null callback");
        }
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_hh_cback bad event:%d received from UIPC",
                p_msg->event);
    }

    /* User callback is supposed to free the buffer */
    if (free_buf != FALSE)
    {
        /* Free it if it's not an UIPC_CHANNEL_DATA_RX event */
        GKI_freebuf(p_msg);
    }
}


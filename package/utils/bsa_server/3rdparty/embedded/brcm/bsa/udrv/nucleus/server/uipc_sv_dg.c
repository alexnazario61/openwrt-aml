/*****************************************************************************
 **
 **  Name:           uipc_sv_dg.c
 **
 **  Description:    Server API for BSA UIPC Data Gateway
 **
 **  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bt_target.h"
#include "bsa_api.h"
#include "bsa_int.h"
#include "uipc.h"
/* #include "bsa_client.h" */
#include "uipc_sv_dg.h"
#include "uipc_channel.h"

/* #define DEBUG_BSA_CL_DG */

#ifndef UIPC_SV_TX_DG_MODE
/* Write direction, Write blocks */
#define UIPC_SV_TX_DG_MODE (UIPC_CHANNEL_MODE_CREATE | UIPC_CHANNEL_MODE_WRITE | UIPC_CHANNEL_MODE_TX_DATA_BLOCK)
#endif

#ifndef UIPC_SV_RX_DG_MODE
/* Read direction, Data event */
#define UIPC_SV_RX_DG_MODE (UIPC_CHANNEL_MODE_CREATE | UIPC_CHANNEL_MODE_READ | UIPC_CHANNEL_MODE_RX_DATA_EVT)
#endif

static tUIPC_CHANNEL_DESC   uipc_sv_dg_tx_desc = UIPC_CHANNEL_BAD_DESC;
static tUIPC_CHANNEL_DESC   uipc_sv_dg_rx_desc = UIPC_CHANNEL_BAD_DESC;

static tUIPC_RCV_CBACK *p_uipc_sv_dg_cback = NULL;

static tUIPC_RCV_CBACK uipc_sv_dg_cback;

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_init(void)
{
    uipc_sv_dg_tx_desc = UIPC_CHANNEL_BAD_DESC;
    uipc_sv_dg_rx_desc = UIPC_CHANNEL_BAD_DESC;
    p_uipc_sv_dg_cback = NULL;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    APPL_TRACE_DEBUG1("uipc_sv_dg_open ChId:%d", channel_id);

    if (uipc_sv_dg_tx_desc != UIPC_CHANNEL_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sv_dg_open UIPC DG was already open.");
        APPL_TRACE_ERROR0("uipc_sv_dg_open Reuse this connection.");
        return TRUE; /* not an error */
    }

    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_sv_dg_open Null callback.");
        return FALSE;
    }

    p_uipc_sv_dg_cback = p_cback;  /* Save callback function */

    APPL_TRACE_DEBUG0("uipc_sv_dg_open DG channels");

    /* Open a channel in Tx/write direction */
    uipc_sv_dg_tx_desc = uipc_channel_open(channel_id, NULL, UIPC_SV_TX_DG_MODE);
    if (uipc_sv_dg_tx_desc == UIPC_CHANNEL_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sv_dg_open fails to open Tx control channel");
        return FALSE;
    }

    /* Open a channel in Rx/Read direction */
    uipc_sv_dg_rx_desc = uipc_channel_open(channel_id, uipc_sv_dg_cback, UIPC_SV_RX_DG_MODE);
    if (uipc_sv_dg_rx_desc == UIPC_CHANNEL_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sv_dg_open fails to open Rx control channel");
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_dg_close(tUIPC_CH_ID channel_id)
{
    uipc_channel_close(uipc_sv_dg_tx_desc);
    uipc_channel_close(uipc_sv_dg_rx_desc);
    uipc_sv_dg_tx_desc = UIPC_CHANNEL_BAD_DESC;
    uipc_sv_dg_rx_desc = UIPC_CHANNEL_BAD_DESC;
    p_uipc_sv_dg_cback = NULL;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen)
{
    BOOLEAN uipc_status = FALSE;

#ifdef DEBUG_BSA_CL_DG
    APPL_TRACE_DEBUG1("uipc_sv_dg_send length:%d", msglen);
/*    scru_dump_hex(p_buf, NULL, msglen, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);*/
#endif

    uipc_status =  uipc_channel_send(uipc_sv_dg_tx_desc, p_buf, msglen);
    if (FALSE == uipc_status)
    {
        APPL_TRACE_ERROR0("uipc_sv_dg_send cannot send data");
        return uipc_status;
    }

    return uipc_status;
}

/*******************************************************************************
**
** Function         uipc_sv_dg_send_buf
**
** Description      This function is used to send a data buffer to a svient
**
** Parameters       channel_id: UIPC channel id
**                  p_msg: data to send
**
** Returns          BOOLEAN
**
*******************************************************************************/
BOOLEAN uipc_sv_dg_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
    BOOLEAN uipc_sv_status = TRUE;
    UINT8 *p_buffer = NULL;
    /* Get to the data buffer */
    p_buffer = ((UINT8 *)(p_msg + 1)) + p_msg->offset;
    uipc_sv_status = uipc_channel_send(uipc_sv_dg_tx_desc, p_buffer, p_msg->len);

    GKI_freebuf(p_msg);

    if (FALSE == uipc_sv_status)
    {
        APPL_TRACE_ERROR0("uipc_sv_dg_send_buf cannot send data");
    }

    return uipc_sv_status;

}

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_cback
 **
 ** Description      This function Handle UIPC callback
 **
 ** Parameters
 **
 ** Returns          none
 **
 *******************************************************************************/
static void uipc_sv_dg_cback(BT_HDR *p_msg)
{
    BOOLEAN free_buffer = TRUE;

#ifdef DEBUG_BSA_CL_DG
    APPL_TRACE_DEBUG0("uipc_sv_dg_cback called");
#endif

    /*  If this is a connection event, call the BSA Client cback */
    if (p_msg->event == UIPC_CHANNEL_OPEN)
    {
#ifdef DEBUG_BSA_CL_DG
        APPL_TRACE_DEBUG0("uipc_sv_dg_cback Connect event");
#endif
        p_msg->event = UIPC_OPEN_EVT;
        p_uipc_sv_dg_cback(p_msg);
        /* free_buffer = FALSE; */
    }
    /*  If this is a disconnection event, call the BSA client cback */
    else if (p_msg->event == UIPC_CHANNEL_CLOSE)
    {
#ifdef DEBUG_BSA_CL_DG
        APPL_TRACE_DEBUG0("uipc_sv_dg_cback disconnect event");
#endif
        p_msg->event = UIPC_CLOSE_EVT;
        p_uipc_sv_dg_cback(p_msg);
        /* free_buffer = FALSE; */
    }
    /*  If this is an Rx data indication, pass it to application */
    else if (p_msg->event == UIPC_CHANNEL_DATA_RX)
    {
        p_msg->event = UIPC_RX_DATA_EVT;
#ifdef DEBUG_BSA_CL_DG
        APPL_TRACE_DEBUG0("uipc_sv_dg_cback Rx Data event");
#endif
        p_uipc_sv_dg_cback(p_msg);
        /* free_buffer = FALSE; */
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_sv_dg_cback bad event:%d received from UIPC",
                p_msg->event);
    }

    /* Free the buffer if it was not passed to the application */
    if (free_buffer)
    {
        GKI_freebuf(p_msg);
    }
}



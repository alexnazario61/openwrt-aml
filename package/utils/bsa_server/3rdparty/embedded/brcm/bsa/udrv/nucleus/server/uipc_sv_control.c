/*****************************************************************************
 **
 **  Name:           uipc_sv_control.c
 **
 **  Description:    Server API for BSA UIPC control
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
#include "bsa_task.h"
#include "uipc_sv_control.h"
#include "uipc_channel.h"
#include "bsa_strm_dec.h"

/*
#define DEBUG_UIPC_SV_CONTROL
*/

#define UIPC_SV_CONTROL_MAX_CLIENT 1

#ifndef UIPC_SV_TX_CONTROL_MODE
/* Create it, Write direction, Write blocks */
#define UIPC_SV_TX_CONTROL_MODE (UIPC_CHANNEL_MODE_CREATE | UIPC_CHANNEL_MODE_WRITE  | UIPC_CHANNEL_MODE_TX_DATA_BLOCK)
#endif

#ifndef UIPC_SV_RX_CONTROL_MODE
/* Create it, Read direction, Data event */
#define UIPC_SV_RX_CONTROL_MODE (UIPC_CHANNEL_MODE_CREATE | UIPC_CHANNEL_MODE_READ | UIPC_CHANNEL_MODE_RX_DATA_EVT)
#endif

static tUIPC_CHANNEL_DESC uipc_sv_ctl_tx_desc = UIPC_CHANNEL_BAD_DESC;
static tUIPC_CHANNEL_DESC uipc_sv_ctl_rx_desc = UIPC_CHANNEL_BAD_DESC;

/*
 * Local static function to UIPC callback
 */
static tUIPC_RCV_CBACK *p_uipc_sv_control_cback = NULL;

static tBSA_RX_STREAM uipc_sv_rx_control_stream[UIPC_SV_CONTROL_MAX_CLIENT];

static void uipc_sv_control_rx_data(BT_HDR *p_msg);

static tUIPC_RCV_CBACK uipc_sv_control_cback;

/*******************************************************************************
 **
 ** Function         uipc_sv_control_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_control_init(void) {
    int index;

    APPL_TRACE_DEBUG0("uipc_sv_control_init");

    if (p_uipc_sv_control_cback != NULL) {
        APPL_TRACE_ERROR0("UIPC Already initialized. Skipping..");
        return TRUE;
    }
    uipc_sv_ctl_tx_desc = UIPC_CHANNEL_BAD_DESC;
    uipc_sv_ctl_rx_desc = UIPC_CHANNEL_BAD_DESC;
    p_uipc_sv_control_cback = NULL;
    /* Init all Rx stream */
    for (index = 0; index < UIPC_SV_CONTROL_MAX_CLIENT; index++) {
        bsa_strm_dec_init(&uipc_sv_rx_control_stream[index]);
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_control_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback) {
    int index;

#ifdef DEBUG_UIPC_SV_CONTROL
    APPL_TRACE_DEBUG0("uipc_sv_control_open enter.");
#endif

    p_uipc_sv_control_cback = p_cback; /* Save callback function */

    /* Open a channel in Rx/Read direction */
    uipc_sv_ctl_rx_desc = uipc_channel_open(channel_id, uipc_sv_control_cback,
            UIPC_SV_RX_CONTROL_MODE);
    if (uipc_sv_ctl_rx_desc == UIPC_CHANNEL_BAD_DESC) {
        APPL_TRACE_ERROR0(
                "uipc_sv_control_open fails to open Rx control channel");
        return FALSE;
    }
    /* Open a channel in Tx/write direction */
    uipc_sv_ctl_tx_desc = uipc_channel_open(channel_id, NULL,
            UIPC_SV_TX_CONTROL_MODE);
    if (uipc_sv_ctl_tx_desc == UIPC_CHANNEL_BAD_DESC) {
        APPL_TRACE_ERROR0(
                "uipc_sv_control_open fails to open Tx control channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_sv_control_open control channels opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_control_close(tUIPC_CH_ID channel_id) {
    uipc_channel_close(uipc_sv_ctl_tx_desc);
    uipc_channel_close(uipc_sv_ctl_rx_desc);

    uipc_sv_ctl_tx_desc = UIPC_CHANNEL_BAD_DESC;
    uipc_sv_ctl_rx_desc = UIPC_CHANNEL_BAD_DESC;
    p_uipc_sv_control_cback = NULL;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_control_send(tUIPC_CH_ID channel_id, UINT8 *p_buf,
        UINT16 msglen) {
    BOOLEAN uipc_status;

#ifdef DEBUG_UIPC_SV_CONTROL
    APPL_TRACE_DEBUG1("uipc_sv_control_send length:%d", msglen);
    scru_dump_hex(p_buf, "Data", msglen, 0, 0);
#endif

    uipc_status = uipc_channel_send(uipc_sv_ctl_tx_desc, p_buf, msglen);

    return uipc_status;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_cback
 **
 ** Description      This function Handle UIPC callback
 **
 ** Parameters
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
static void uipc_sv_control_cback(BT_HDR *p_msg) {
    BOOLEAN free_buffer = TRUE;
#ifdef DEBUG_UIPC_SV_CONTROL
    APPL_TRACE_DEBUG0("uipc_sv_control_cback p_msg called");
#endif

    /*  If this is a connection event, call the BSA server cback */
    if (p_msg->event == UIPC_CHANNEL_OPEN) {
        p_msg->event = UIPC_OPEN_EVT;
#ifdef DEBUG_UIPC_SV_CONTROL
        APPL_TRACE_DEBUG0("uipc_sv_control_cback Connect event");
#endif
        if (p_uipc_sv_control_cback != NULL) {
            p_uipc_sv_control_cback(p_msg);
            free_buffer = FALSE;
        } else {
            APPL_TRACE_ERROR0(
                    "ERROR uipc_sv_control_cback NULL Callback !!!!!");
        }
    }
    /*  If this is a disconnection event, call the BSA server cback */
    else if (p_msg->event == UIPC_CHANNEL_CLOSE) {
        p_msg->event = UIPC_CLOSE_EVT;
#ifdef DEBUG_UIPC_SV_CONTROL
        APPL_TRACE_DEBUG0("uipc_sv_control_cback disconnect event");
#endif
        if (p_uipc_sv_control_cback != NULL) {
            p_uipc_sv_control_cback(p_msg);
            free_buffer = FALSE;
        } else {
            APPL_TRACE_ERROR0(
                    "ERROR uipc_sv_control_cback NULL Callback !!!!!");
        }
    }
    /*  If Rx data indication, let's decode the msg first */
    else if (p_msg->event == UIPC_CHANNEL_DATA_RX) {
        p_msg->event = UIPC_RX_DATA_EVT;
#ifdef DEBUG_UIPC_SV_CONTROL
        APPL_TRACE_DEBUG0("uipc_sv_control_cback Rx Data event");
#endif
        uipc_sv_control_rx_data(p_msg);
        free_buffer = FALSE;
    } else {
        APPL_TRACE_ERROR1(
                "uipc_sv_control_cback bad event:%d received from UIPC",
                p_msg->event);
    }

    /* Free buffer if not passed (error) */
    if (free_buffer == TRUE) {
        GKI_freebuf(p_msg);
    }
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_rx_data
 **
 ** Description      This function Handle message from a server application
 **
 ** Parameters       buffer: Pointer to received buffer
 **                  Length: length of buffer
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
static void uipc_sv_control_rx_data(BT_HDR *p_msg) {
    int status;
    UINT8 *p_buffer = (UINT8 *) (p_msg + 1);
    int length = p_msg->len;
    tBSA_SV_CONTROL_CLIENT_MSG *p_rx_data_msg;
    UINT16 client_num = p_msg->layer_specific;

#ifdef DEBUG_UIPC_SV_CONTROL
    APPL_TRACE_DEBUG1("uipc_sv_control_rx_data length:%d", length);
    //scru_dump_hex(p_buffer, "Data", length, 0, 0);
#endif

    do {
        /* Call the function in charge of decoding control stream */
        status = bsa_strm_dec(&uipc_sv_rx_control_stream[client_num], &p_buffer,
                &length);
#ifdef DEBUG_UIPC_SV_CONTROL
        APPL_TRACE_DEBUG1("uipc_sv_control_rx_data bsa_strm_dec():%d", status);
#endif

        /* If a complete message have been received */
        if (status) {
            /* Allocate buffer for Rx data Msg */
            if ((p_rx_data_msg = (tBSA_SV_CONTROL_CLIENT_MSG *) GKI_getbuf(
                    sizeof(tBSA_SV_CONTROL_CLIENT_MSG))) != NULL) {
                memcpy(p_rx_data_msg->data,
                        &uipc_sv_rx_control_stream[client_num].data[0],
                        uipc_sv_rx_control_stream[client_num].decoded_length);
                p_rx_data_msg->hdr.event = UIPC_RX_DATA_EVT;
                p_rx_data_msg->hdr.len =
                        uipc_sv_rx_control_stream[client_num].decoded_length;
                p_rx_data_msg->hdr.offset = 0;
                p_rx_data_msg->message_id =
                        uipc_sv_rx_control_stream[client_num].decoded_msg_id;
                p_rx_data_msg->hdr.layer_specific = client_num;
#ifdef DEBUG_UIPC_SV_CONTROL
                APPL_TRACE_DEBUG0("uipc_sv_control_rx_data calling server_rx_cback");
                APPL_TRACE_DEBUG2(
                        "uipc_sv_control_rx_data decoded_len:%d decoded_msgid:%d",
                        p_rx_data_msg->hdr.len,
                        p_rx_data_msg->message_id);
#endif
                if (p_uipc_sv_control_cback != NULL) {
                    p_uipc_sv_control_cback((BT_HDR *) p_rx_data_msg);
                } else {
                    APPL_TRACE_ERROR0(
                            "ERROR uipc_sv_control_rx_data NULL Callback !!!!!");
                }
            } else {
                APPL_TRACE_ERROR0("uipc_sv_control_rx_data: GKI_getbuf failed");
            }
            /* uipc_sv_rx_control_stream must be re-initialized for every new message */
            bsa_strm_dec_init(&uipc_sv_rx_control_stream[client_num]);

        } else {
#ifdef DEBUG_UIPC_SV_CONTROL
            APPL_TRACE_DEBUG0(
                    "uipc_sv_control_rx_data bsa_strm_dec waiting for remaining data");
#endif
        }
    } while ((status) && (length != 0));

    GKI_freebuf(p_msg);
}


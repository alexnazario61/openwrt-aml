/*****************************************************************************
 **
 **  Name:           uipc_cl_control.c
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
#include "bsa_client.h"
#include "uipc_cl_control.h"
#include "uipc_channel.h"
#include "uipc_mutex.h"
#include "bsa_strm_dec.h"

/*#define DEBUG_BSA_CL_CONTROL*/

#ifndef UIPC_CL_TX_CONTROL_MODE
/* Write direction, Write blocks */
#define UIPC_CL_TX_CONTROL_MODE (UIPC_CHANNEL_MODE_WRITE | UIPC_CHANNEL_MODE_TX_DATA_BLOCK)
#endif

#ifndef UIPC_CL_RX_CONTROL_MODE
/* Read direction, Data event */
#define UIPC_CL_RX_CONTROL_MODE (UIPC_CHANNEL_MODE_READ | UIPC_CHANNEL_MODE_RX_DATA_EVT)
#endif

static tUIPC_CHANNEL_DESC   uipc_cl_ctl_tx_desc = UIPC_CHANNEL_BAD_DESC;
static tUIPC_CHANNEL_DESC   uipc_cl_ctl_rx_desc = UIPC_CHANNEL_BAD_DESC;
static tMUTEX uipc_cl_control_mutex;

/*
 * Local static function to UIPC callback
 */
static tUIPC_RCV_CBACK *p_uipc_cl_control_cback = NULL;
static int OPEN_COUNT = 0;

static tBSA_RX_STREAM uipc_cl_rx_control_stream;

static void uipc_cl_control_rx_data(BT_HDR *p_msg);

static tUIPC_RCV_CBACK uipc_cl_control_cback;

/*******************************************************************************
 **
 ** Function         uipc_cl_control_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_control_init(void)
{
#ifdef DEBUG_BSA_CL_CONTROL
    APPL_TRACE_DEBUG0("uipc_cl_control_init");
#endif

    if (p_uipc_cl_control_cback != NULL) {
        APPL_TRACE_ERROR0("UIPC-Client Already initialized. Skipping..");
        return TRUE;
    }
    uipc_cl_ctl_tx_desc = UIPC_CHANNEL_BAD_DESC;
    uipc_cl_ctl_rx_desc = UIPC_CHANNEL_BAD_DESC;
    p_uipc_cl_control_cback = NULL;
    /* Init Rx stream decoder */
    bsa_strm_dec_init(&uipc_cl_rx_control_stream);
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_control_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_control_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    APPL_TRACE_ERROR1("uipc_cl_control_open() count - %d", OPEN_COUNT++);
    p_uipc_cl_control_cback = p_cback;  /* Save callback function */

    if (init_mutex(&uipc_cl_control_mutex) < 0)
    {
        APPL_TRACE_ERROR0("uipc_cl_control_open cannot init mutex");
        return FALSE;
    }

    /* Open a channel in Tx/write direction */
    uipc_cl_ctl_tx_desc = uipc_channel_open(channel_id, NULL, UIPC_CL_TX_CONTROL_MODE);
    if (uipc_cl_ctl_tx_desc == UIPC_CHANNEL_BAD_DESC)
    {
        delete_mutex(&uipc_cl_control_mutex);
        APPL_TRACE_ERROR0("uipc_cl_control_open fails to open Tx control channel");
        return FALSE;
    }

    /* Open a channel in Rx/Read direction */
    uipc_cl_ctl_rx_desc = uipc_channel_open(channel_id, uipc_cl_control_cback,
        UIPC_CL_RX_CONTROL_MODE);
    APPL_TRACE_DEBUG1("uipc_cl_control_open() uipc_cl_ctl_rx_desc:%d", uipc_cl_ctl_rx_desc);
    if (uipc_cl_ctl_rx_desc == UIPC_CHANNEL_BAD_DESC)
    {
        delete_mutex(&uipc_cl_control_mutex);
        APPL_TRACE_ERROR0("uipc_cl_control_open fails to open Rx control channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_cl_control_open control channels opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_control_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_control_close(tUIPC_CH_ID channel_id)
{
    APPL_TRACE_DEBUG1("uipc_cl_control_close for channel:%d", channel_id);
    uipc_channel_close(uipc_cl_ctl_tx_desc);
    uipc_channel_close(uipc_cl_ctl_rx_desc);
    uipc_cl_ctl_tx_desc = UIPC_CHANNEL_BAD_DESC;
    uipc_cl_ctl_rx_desc = UIPC_CHANNEL_BAD_DESC;
    p_uipc_cl_control_cback = NULL;
    delete_mutex(&uipc_cl_control_mutex);
}

/*******************************************************************************
 **
 ** Function         uipc_cl_control_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_control_send(tUIPC_CH_ID channel_id, UINT16 msg_id, UINT8 *p_buf, UINT16 msglen)
{
    BOOLEAN uipc_status = FALSE;
    unsigned char    header[4];

#ifdef DEBUG_BSA_CL_CONTROL
    APPL_TRACE_DEBUG1("uipc_cl_control_send length:%d", msglen);
    scru_dump_hex(p_buf, NULL, msglen, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);

#endif

    /* Add Message_id to length */
    header[0] = (unsigned char)((msglen + 2) & 0xff);
    header[1] = (unsigned char)(((msglen + 2) >> 8) & 0xff);
    header[2] = (unsigned char) (msg_id & 0xff);
    header[3] = (unsigned char)((msg_id >> 8) & 0xff);

    if (lock_mutex(&uipc_cl_control_mutex) < 0)
    {
        APPL_TRACE_ERROR0("uipc_cl_control_send cannot lock mutex");
        return uipc_status;
    }

    uipc_status =  uipc_channel_send(uipc_cl_ctl_tx_desc, header, sizeof(header));
    if (FALSE == uipc_status)
    {
        APPL_TRACE_ERROR0("uipc_cl_control_send cannot send header");
        return uipc_status;
    }

    uipc_status =  uipc_channel_send(uipc_cl_ctl_tx_desc, p_buf, msglen);
    if (FALSE == uipc_status)
    {
        APPL_TRACE_ERROR0("uipc_cl_control_send cannot send body");
        return uipc_status;
    }

    if (unlock_mutex(&uipc_cl_control_mutex) < 0)
    {
        APPL_TRACE_ERROR0("uipc_cl_control_send cannot unlock mutex");
        return uipc_status;
    }

    return uipc_status;
}


/*******************************************************************************
 **
 ** Function         uipc_cl_control_cback
 **
 ** Description      This function Handle UIPC callback
 **
 ** Parameters
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
static void uipc_cl_control_cback(BT_HDR *p_msg)
{
#ifdef DEBUG_BSA_CL_CONTROL
    APPL_TRACE_DEBUG1("uipc_cl_control_cback p_msg() event:%d", p_msg->event);
#endif
    /*  If this is a connection event, call the BSA Client cback */
    if (p_msg->event == UIPC_CHANNEL_OPEN)
    {
        p_msg->event = UIPC_OPEN_EVT;
#ifdef DEBUG_BSA_CL_CONTROL
        APPL_TRACE_DEBUG0("uipc_cl_control_cback Connect event");
#endif
        p_uipc_cl_control_cback(p_msg);
    }
    /*  If this is a disconnection event, call the BSA client cback */
    else if (p_msg->event == UIPC_CHANNEL_CLOSE)
    {
        p_msg->event = UIPC_CLOSE_EVT;
#ifdef DEBUG_BSA_CL_CONTROL
        APPL_TRACE_DEBUG0("uipc_cl_control_cback disconnect event");
#endif
        p_uipc_cl_control_cback(p_msg);
    }
    /*  If this is an Rx data indication, let's decode (reassemble) the msg first */
    else if (p_msg->event == UIPC_CHANNEL_DATA_RX)
    {
        p_msg->event = UIPC_RX_DATA_EVT;
#ifdef DEBUG_BSA_CL_CONTROL
        APPL_TRACE_DEBUG0("uipc_cl_control_cback Rx Data event");
#endif
        uipc_cl_control_rx_data(p_msg);
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_control_cback bad event:%d received from UIPC",
                p_msg->event);
        GKI_freebuf(p_msg);

    }
}

/*******************************************************************************
 **
 ** Function         uipc_cl_control_rx_data
 **
 ** Description      This function Handle message from a server application
 **
 ** Parameters       buffer: Pointer to received buffer
 **                  Length: length of buffer
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
static void uipc_cl_control_rx_data(BT_HDR *p_msg)
{
    int status;
    UINT8 *p_buffer = (UINT8 *)(p_msg + 1);
    int length = p_msg->len;
    BT_HDR *p_rx_data_msg;

#ifdef DEBUG_BSA_CL_CONTROL
    APPL_TRACE_DEBUG1("uipc_cl_control_rx_data length:%d", length);
    scru_dump_hex(p_buffer, "Data", length, 0, 0);
#endif

    do
    {
        /* Call the function in charge of decoding control stream */
        status = bsa_strm_dec(&uipc_cl_rx_control_stream,
                &p_buffer, &length);

        /* If a complete message have been received */
        if (status)
        {
            /* Allocate buffer for Rx data Msg */
            if ((p_rx_data_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR) +
                    uipc_cl_rx_control_stream.decoded_length)) != NULL)
            {
                memcpy((UINT8 *)(p_rx_data_msg + 1),
                        &uipc_cl_rx_control_stream.data[0],
                        uipc_cl_rx_control_stream.decoded_length);
                p_rx_data_msg->event = UIPC_RX_DATA_EVT;
                p_rx_data_msg->len
                        = uipc_cl_rx_control_stream.decoded_length;
                p_rx_data_msg->offset = 0;
                p_rx_data_msg->layer_specific
                        = uipc_cl_rx_control_stream.decoded_msg_id;
#ifdef DEBUG_BSA_CL_CONTROL
                APPL_TRACE_DEBUG0("uipc_cl_control_rx_data calling server_rx_cback");
                APPL_TRACE_DEBUG2(
                        "uipc_cl_control_rx_data decoded_len:%d decoded_msgid:%x",
                        p_rx_data_msg->len,
                        p_rx_data_msg->layer_specific);
#endif
                p_uipc_cl_control_cback((BT_HDR *)p_rx_data_msg);
            }
            else
            {
                APPL_TRACE_ERROR0("uipc_sv_control_rx_data: GKI_getbuf failed");
            }
            /* uipc_cl_rx_control_stream must be re-initialized for every new message */
            bsa_strm_dec_init(&uipc_cl_rx_control_stream);

        }
        else
        {
#ifdef DEBUG_BSA_CL_CONTROL
            APPL_TRACE_DEBUG0(
                    "uipc_cl_control_rx_data bsa_strm_dec waiting for remaining data");
#endif
        }
    } while ((status) && (length != 0));

    GKI_freebuf(p_msg);
}


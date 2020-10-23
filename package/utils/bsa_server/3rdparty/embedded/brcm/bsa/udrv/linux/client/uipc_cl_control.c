/*****************************************************************************
 **
 **  Name:           uipc_cl_control.c
 **
 **  Description:    Client API for BSA UIPC control
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bt_target.h"
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"
#include "nsa_client.h"
#else
#include "bsa_api.h"
#include "bsa_int.h"
#include "bsa_client.h"
#endif

#include "uipc.h"
#include "uipc_cl_control.h"
#include "uipc_cl_socket.h"
#include "uipc_mutex.h"
#include "bsa_strm_dec.h"

/*
#define UIPC_CL_CONTROL_DEBUG
*/

#ifndef UIPC_CONTROL_SOCKET_NAME
#define UIPC_CONTROL_SOCKET_NAME "./bt-daemon-socket"
#endif

#ifndef UIPC_NFC_CONTROL_SOCKET_NAME
#define UIPC_NFC_CONTROL_SOCKET_NAME "./nfc-daemon-socket"
#endif

#ifndef UIPC_CL_CONTROL_SOCKET_MODE
/* By default. Send Rx event (with data) and Write is blocking */
#define UIPC_CL_CONTROL_SOCKET_MODE (UIPC_CL_SOCKET_MODE_BLOCK | \
                                     UIPC_CL_SOCKET_MODE_RX_DATA_EVT)
#endif


typedef struct
{
    tMUTEX tx_mutex;
    tUIPC_RCV_CBACK *p_cback;
    tBSA_RX_STREAM rx_ctrl_stream;
    tUIPC_CL_SOCKET_DESC socket_desc;
} tUIPC_CL_CTRL_CB;

/*
 * Global variables
 */
static tUIPC_CL_CTRL_CB uipc_cl_ctrl_cb;
static tUIPC_CL_CTRL_CB uipc_cl_nfc_ctrl_cb;

/*
 * Local functions
 */
static void uipc_cl_control_rx_data(BT_HDR *p_msg);
static void uipc_cl_control_cback(BT_HDR *p_msg);
static void uipc_cl_nfc_control_rx_data(BT_HDR *p_msg);
static void uipc_cl_nfc_control_cback(BT_HDR *p_msg);


/*******************************************************************************
 **
 ** Function         uipc_cl_control_init
 **
 ** Description      UIPC Control Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_control_init(void)
{
    uipc_cl_ctrl_cb.p_cback = NULL;
    uipc_cl_ctrl_cb.socket_desc = UIPC_CL_SOCKET_DESC_MAX;
    uipc_cl_nfc_ctrl_cb.p_cback = NULL;
    uipc_cl_nfc_ctrl_cb.socket_desc = UIPC_CL_SOCKET_DESC_MAX;
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
    tUIPC_CL_CTRL_CB* p_uipc_cl_ctrl_cb;

#ifdef UIPC_CL_CONTROL_DEBUG
    APPL_TRACE_DEBUG0("uipc_cl_control_open enter");
#endif
    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_control_open fail p_cback NULL");
        return FALSE;
    }

    if (channel_id == UIPC_CH_ID_CTL)
    {
        p_uipc_cl_ctrl_cb = &uipc_cl_ctrl_cb;
    }
    else if (channel_id == UIPC_NSA_CH_ID_CTL)
    {
        p_uipc_cl_ctrl_cb = &uipc_cl_nfc_ctrl_cb;
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_control_open: unexpected ctrl ch_id = %d", channel_id);
        return FALSE;
    }

    if (init_mutex(&(p_uipc_cl_ctrl_cb->tx_mutex)) < 0)
    {
         APPL_TRACE_ERROR0("uipc_cl_control_open fail init mutex");
         return FALSE;
    }
    /* Init Rx stream */
    bsa_strm_dec_init(&(p_uipc_cl_ctrl_cb->rx_ctrl_stream));

    p_uipc_cl_ctrl_cb->p_cback = p_cback;  /* Save callback function */

    /* Starts server with local callback function */
    if (channel_id == UIPC_CH_ID_CTL)
    {
        p_uipc_cl_ctrl_cb->socket_desc = uipc_cl_socket_connect((UINT8 *)UIPC_CONTROL_SOCKET_NAME,
                uipc_cl_control_cback,  UIPC_CL_CONTROL_SOCKET_MODE);
    }
    else
    {
        /*this is nfc control channel, check on ch_id done before*/
        p_uipc_cl_ctrl_cb->socket_desc = uipc_cl_socket_connect((UINT8 *)UIPC_NFC_CONTROL_SOCKET_NAME,
                uipc_cl_nfc_control_cback,  UIPC_CL_CONTROL_SOCKET_MODE);
    }

    if (p_uipc_cl_ctrl_cb->socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        p_uipc_cl_ctrl_cb->p_cback = NULL;
        APPL_TRACE_ERROR0("uipc_cl_control_open fails to connect control socket");
        return FALSE;
    }
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
    tUIPC_CL_CTRL_CB* p_uipc_cl_ctrl_cb;

    if (channel_id == UIPC_CH_ID_CTL)
    {
        p_uipc_cl_ctrl_cb = &uipc_cl_ctrl_cb;
    }
    else if (channel_id == UIPC_NSA_CH_ID_CTL)
    {
        p_uipc_cl_ctrl_cb = &uipc_cl_nfc_ctrl_cb;
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_control_close: unexpected ctrl ch_id = %d", channel_id);
        return;
    }

    delete_mutex(&(p_uipc_cl_ctrl_cb->tx_mutex));
    uipc_cl_socket_disconnect(p_uipc_cl_ctrl_cb->socket_desc);
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
BOOLEAN uipc_cl_control_send(tUIPC_CH_ID channel_id,
    UINT16 msg_id, UINT8 *p_buf, UINT16 msglen)
{
    BOOLEAN uipc_status;
    unsigned char header[4];
    tUIPC_CL_CTRL_CB* p_uipc_cl_ctrl_cb;

#ifdef UIPC_CL_CONTROL_DEBUG
    APPL_TRACE_DEBUG2("uipc_cl_control_send msg_id:%x length:%d",
            msg_id, msglen);
/*    scru_dump_hex(p_buf, NULL, msglen, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);*/
#endif

    /* Add Message_id to length */
    header[0] = (unsigned char)((msglen + 2) & 0xff);
    header[1] = (unsigned char)(((msglen + 2) >> 8) & 0xff);
    header[2] = (unsigned char) (msg_id & 0xff);
    header[3] = (unsigned char)((msg_id >> 8) & 0xff);

    if (channel_id == UIPC_CH_ID_CTL)
    {
        p_uipc_cl_ctrl_cb = &uipc_cl_ctrl_cb;
    }
    else if (channel_id == UIPC_NSA_CH_ID_CTL)
    {
        p_uipc_cl_ctrl_cb = &uipc_cl_nfc_ctrl_cb;
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_control_send: unexpected ctrl ch_id = %d", channel_id);
        return FALSE;
    }

    /* Lock the Tx Mutex to prevent collision with another thread */
    lock_mutex(&(p_uipc_cl_ctrl_cb->tx_mutex));

    /* Send the Packet Header */
    uipc_status = uipc_cl_socket_send(p_uipc_cl_ctrl_cb->socket_desc, header, sizeof(header));
    if(uipc_status)
    {
        /* Send the Packet Payload */
        uipc_status = uipc_cl_socket_send(p_uipc_cl_ctrl_cb->socket_desc, p_buf, msglen);
    }

    /* Lock the Tx Mutex */
    unlock_mutex(&(p_uipc_cl_ctrl_cb->tx_mutex));

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
#ifdef UIPC_CL_CONTROL_DEBUG
    APPL_TRACE_DEBUG0("uipc_cl_control_cback");
#endif

    /*  If this is a disconnection event, call the BSA server cback */
    if (p_msg->event == UIPC_CLOSE_EVT)
    {
#ifdef UIPC_CL_CONTROL_DEBUG
        APPL_TRACE_DEBUG0("uipc_cl_control_cback Disconnect event");
#endif
        uipc_cl_ctrl_cb.p_cback(p_msg);
    }
    /* If it's a Rx data indication, let's decode the msg first */
    else if (p_msg->event == UIPC_RX_DATA_EVT)
    {
#ifdef UIPC_CL_CONTROL_DEBUG
        APPL_TRACE_DEBUG0("uipc_cl_control_cback Rx Data event");
#endif
        uipc_cl_control_rx_data(p_msg);
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_control_cback bad event received from UIPC:%d",
                p_msg->event);
    }
}

/*******************************************************************************
 **
 ** Function         uipc_cl_nfc_control_cback
 **
 ** Description      This function Handle UIPC callback
 **
 ** Parameters
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
static void uipc_cl_nfc_control_cback(BT_HDR *p_msg)
{
#ifdef UIPC_CL_CONTROL_DEBUG
    APPL_TRACE_DEBUG0("uipc_cl_nfc_control_cback");
#endif

    /*  If this is a disconnection event, call the BSA server cback */
    if (p_msg->event == UIPC_CLOSE_EVT)
    {
#ifdef UIPC_CL_CONTROL_DEBUG
        APPL_TRACE_DEBUG0("uipc_cl_nfc_control_cback Disconnect event");
#endif
        uipc_cl_nfc_ctrl_cb.p_cback(p_msg);
    }
    /* If it's a Rx data indication, let's decode the msg first */
    else if (p_msg->event == UIPC_RX_DATA_EVT)
    {
#ifdef UIPC_CL_CONTROL_DEBUG
        APPL_TRACE_DEBUG0("uipc_cl_nfc_control_cback Rx Data event");
#endif
        uipc_cl_nfc_control_rx_data(p_msg);
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_nfc_control_cback bad event received from UIPC:%d",
                p_msg->event);
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

#ifdef UIPC_CL_CONTROL_DEBUG
    APPL_TRACE_DEBUG1("uipc_cl_control_rx_data length:%d", length);
/*    scru_dump_hex(p_buffer, "Data", length, 0, 0); */
#endif

    do
    {
        /* Call the function in charge of decoding control stream */
        status = bsa_strm_dec(&uipc_cl_ctrl_cb.rx_ctrl_stream,
                &p_buffer, &length);

        /* If a complete message have been received */
        if (status)
        {
            /* Allocate buffer for Rx data Msg */
            if ((p_rx_data_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR) +
                    uipc_cl_ctrl_cb.rx_ctrl_stream.decoded_length)) != NULL)
            {
                memcpy((UINT8 *)(p_rx_data_msg + 1),
                        &uipc_cl_ctrl_cb.rx_ctrl_stream.data[0],
                        uipc_cl_ctrl_cb.rx_ctrl_stream.decoded_length);
                p_rx_data_msg->event = UIPC_RX_DATA_EVT;
                p_rx_data_msg->len = uipc_cl_ctrl_cb.rx_ctrl_stream.decoded_length;
                p_rx_data_msg->offset = 0;
                p_rx_data_msg->layer_specific = uipc_cl_ctrl_cb.rx_ctrl_stream.decoded_msg_id;
#ifdef UIPC_CL_CONTROL_DEBUG
                APPL_TRACE_DEBUG0("uipc_cl_control_rx_data calling client_rx_cback");
                APPL_TRACE_DEBUG2("uipc_cl_control_rx_data decoded_len:%d decoded_msgid:%x",
                        p_rx_data_msg->len,
                        p_rx_data_msg->layer_specific);
#endif
                uipc_cl_ctrl_cb.p_cback((BT_HDR *)p_rx_data_msg);
            }
            else
            {
                APPL_TRACE_ERROR0("uipc_sv_control_rx_data: GKI_getbuf failed");
            }
            /* uipc_cl_ctrl_cb.rx_ctrl_stream must be re-initialized for every new message */
            bsa_strm_dec_init(&uipc_cl_ctrl_cb.rx_ctrl_stream);
        }
        else
        {
#ifdef UIPC_CL_CONTROL_DEBUG
            APPL_TRACE_DEBUG0("uipc_cl_control_rx_data bsa_strm_dec waiting for remaining data");
#endif
        }
    } while ((status) && (length != 0));

    GKI_freebuf(p_msg);
}

/*******************************************************************************
 **
 ** Function         uipc_cl_nfc_control_rx_data
 **
 ** Description      This function Handle message from a server application
 **
 ** Parameters       buffer: Pointer to received buffer
 **                  Length: length of buffer
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
static void uipc_cl_nfc_control_rx_data(BT_HDR *p_msg)
{
    int status;
    UINT8 *p_buffer = (UINT8 *)(p_msg + 1);
    int length = p_msg->len;
    BT_HDR *p_rx_data_msg;

#ifdef UIPC_CL_CONTROL_DEBUG
    APPL_TRACE_DEBUG1("uipc_cl_nfc_control_rx_data length:%d", length);
/*    scru_dump_hex(p_buffer, "Data", length, 0, 0); */
#endif

    do
    {
        /* Call the function in charge of decoding control stream */
        status = bsa_strm_dec(&uipc_cl_nfc_ctrl_cb.rx_ctrl_stream,
                &p_buffer, &length);

        /* If a complete message have been received */
        if (status)
        {
            /* Allocate buffer for Rx data Msg */
            if ((p_rx_data_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR) +
                    uipc_cl_nfc_ctrl_cb.rx_ctrl_stream.decoded_length)) != NULL)
            {
                memcpy((UINT8 *)(p_rx_data_msg + 1),
                        &uipc_cl_nfc_ctrl_cb.rx_ctrl_stream.data[0],
                        uipc_cl_nfc_ctrl_cb.rx_ctrl_stream.decoded_length);
                p_rx_data_msg->event = UIPC_RX_DATA_EVT;
                p_rx_data_msg->len = uipc_cl_nfc_ctrl_cb.rx_ctrl_stream.decoded_length;
                p_rx_data_msg->offset = 0;
                p_rx_data_msg->layer_specific = uipc_cl_nfc_ctrl_cb.rx_ctrl_stream.decoded_msg_id;
#ifdef UIPC_CL_CONTROL_DEBUG
                APPL_TRACE_DEBUG0("uipc_cl_nfc_control_rx_data calling client_rx_cback");
                APPL_TRACE_DEBUG2("uipc_cl_nfc_control_rx_data decoded_len:%d decoded_msgid:%x",
                        p_rx_data_msg->len,
                        p_rx_data_msg->layer_specific);
#endif
                uipc_cl_nfc_ctrl_cb.p_cback((BT_HDR *)p_rx_data_msg);
            }
            else
            {
                APPL_TRACE_ERROR0("uipc_cl_nfc_control_rx_data: GKI_getbuf failed");
            }
            /* uipc_cl_nfc_ctrl_cb.rx_ctrl_stream must be re-initialized for every new message */
            bsa_strm_dec_init(&uipc_cl_nfc_ctrl_cb.rx_ctrl_stream);
        }
        else
        {
#ifdef UIPC_CL_CONTROL_DEBUG
            APPL_TRACE_DEBUG0("uipc_cl_nfc_control_rx_data bsa_strm_dec waiting for remaining data");
#endif
        }
    } while ((status) && (length != 0));

    GKI_freebuf(p_msg);
}


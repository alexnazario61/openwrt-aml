/*****************************************************************************
 **
 **  Name:           uipc_sv_control.c
 **
 **  Description:    Server API for BSA UIPC control
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bt_target.h"

#if defined (NSA) && (NSA == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"
#include "nsa_task.h"
#else
#include "bsa_api.h"
#include "bsa_int.h"
#include "bsa_task.h"
#endif

#include "uipc.h"
#include "uipc_sv_control.h"
#include "uipc_sv_socket.h"
#include "bsa_strm_dec.h"

//#define DEBUG_BSA_SV_CONTROL

#ifndef UIPC_SV_CONTROL_MAX_CLIENT
#define UIPC_SV_CONTROL_MAX_CLIENT 5
#endif

/*TODO: current design is for either NSA or BSA server, not both;
 * code is differently compiled based on NSA = TRUE compile option
 * for an NSA or BSA server
 * to design a 'combo' server need to:
 * - define both control socket name;
 * - define 2 tUIPC_SV_CTRL_CB for 2 logic control ch or
 *   define control socket and a separate nfc socket channel*/
#ifndef UIPC_CONTROL_SOCKET_NAME
#if defined (NSA) && (NSA == TRUE)
#define UIPC_CONTROL_SOCKET_NAME "./nfc-daemon-socket"
#else
#define UIPC_CONTROL_SOCKET_NAME "./bt-daemon-socket"
#endif
#endif

#ifndef UIPC_CL_CONTROL_SOCKET_MODE
/* By default. Send Rx event (with data) and Write is blocking */
#define UIPC_CL_CONTROL_SOCKET_MODE (UIPC_SV_SOCKET_MODE_RX_DATA_EVT | UIPC_SV_SOCKET_MODE_BLOCK)
#endif

typedef struct
{
    tUIPC_RCV_CBACK *p_cback;
    tBSA_RX_STREAM rx_ctrl_stream[UIPC_SV_CONTROL_MAX_CLIENT];
    tUIPC_SV_SOCKET_SERVER_DESC socket_desc;
} tUIPC_SV_CTRL_CB;

/*
 * Global variables
 */
tUIPC_SV_CTRL_CB uipc_sv_ctrl_cb;

/*
 * Local static functions
 */
static void uipc_sv_control_rx_data(BT_HDR *p_msg);
static void uipc_sv_control_cback(BT_HDR *p_msg);


/*******************************************************************************
 **
 ** Function         uipc_sv_control_init
 **
 ** Description      UIPC Control Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_control_init(void)
{
    uipc_sv_ctrl_cb.p_cback = NULL;
    uipc_sv_ctrl_cb.socket_desc = UIPC_SV_SOCKET_SERVER_MAX;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_open
 **
 ** Description      Open UIPC control channel
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_control_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    int index;

    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_sv_control_open fail p_cback NULL");
        return FALSE;
    }

    if (uipc_sv_ctrl_cb.socket_desc < UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR0("uipc_sv_control_open already opened");
        return FALSE;
    }

    /* Init all Rx stream */
    for (index = 0; index < UIPC_SV_CONTROL_MAX_CLIENT; index++)
    {
        bsa_strm_dec_init(&uipc_sv_ctrl_cb.rx_ctrl_stream[index]);
    }

    /* Save callback function */
    uipc_sv_ctrl_cb.p_cback = p_cback;

    /* Starts server with local callback function */
    uipc_sv_ctrl_cb.socket_desc = uipc_sv_socket_start_server(
            (UINT8 *) UIPC_CONTROL_SOCKET_NAME, uipc_sv_control_cback,
            UIPC_SV_CONTROL_MAX_CLIENT, UIPC_CL_CONTROL_SOCKET_MODE);

    if (uipc_sv_ctrl_cb.socket_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR0("uipc_sv_control_open fails to start control server");
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_close
 **
 ** Description      Close UIPC control channel
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_control_close(tUIPC_CH_ID channel_id)
{
    uipc_sv_socket_stop_server(uipc_sv_ctrl_cb.socket_desc, (UINT8 *)UIPC_CONTROL_SOCKET_NAME);
    uipc_sv_ctrl_cb.socket_desc = UIPC_SV_SOCKET_SERVER_MAX;
    uipc_sv_ctrl_cb.p_cback = NULL;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sv_control_send(tUIPC_CH_ID channel_id,
        tBSA_CLIENT_NB client_num, UINT8 *p_buf, UINT16 msglen)
{
    BOOLEAN uipc_status;

#ifdef DEBUG_BSA_SV_CONTROL
    APPL_TRACE_DEBUG2("uipc_sv_control_send Client:%d length:%d",
            client_num, msglen);
/*    scru_dump_hex(p_buf, NULL, msglen, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);*/
#endif

    uipc_status = uipc_sv_socket_send(uipc_sv_ctrl_cb.socket_desc, client_num, p_buf, msglen);

    return uipc_status;
}


/*******************************************************************************
 **
 ** Function         uipc_sv_control_cback
 **
 ** Description      This function Handle UIPC callback
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_control_cback(BT_HDR *p_msg)
{
#ifdef DEBUG_BSA_SV_CONTROL
    APPL_TRACE_DEBUG1("uipc_sv_control_cback p_msg:%x", p_msg);
#endif

    /*  If this is a connection or disconnection event, call the BSA server cback */
    if ((p_msg->event == UIPC_OPEN_EVT) ||
        (p_msg->event == UIPC_CLOSE_EVT))
    {
#ifdef DEBUG_BSA_SV_CONTROL
        APPL_TRACE_DEBUG0("uipc_sv_control_cback Connect/Disconnect event");
#endif
        if (p_msg->layer_specific < UIPC_SV_CONTROL_MAX_CLIENT)
        {
            bsa_strm_dec_init(&uipc_sv_ctrl_cb.rx_ctrl_stream[p_msg->layer_specific]);
        }
        else
        {
            APPL_TRACE_ERROR1("uipc_sv_control_cback bad client number received from UIPC:%d",
                            p_msg->layer_specific);
        }

        uipc_sv_ctrl_cb.p_cback(p_msg);
    }
    /* If it's a Rx data indication, let's decode the msg first */
    else if (p_msg->event == UIPC_RX_DATA_EVT)
    {
#ifdef DEBUG_BSA_SV_CONTROL
        APPL_TRACE_DEBUG0("uipc_sv_control_cback Rx Data event");
#endif
        uipc_sv_control_rx_data(p_msg);
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_sv_control_cback bad event received from UIPC:%d",
                p_msg->event);
    }
}

/*******************************************************************************
 **
 ** Function         uipc_sv_control_rx_data
 **
 ** Description      This function Handle message from an application
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_control_rx_data(BT_HDR *p_msg)
{
    int status;
    UINT8 *p_buffer = (UINT8 *)(p_msg + 1);
    int length = p_msg->len;
    tBSA_SV_CONTROL_CLIENT_MSG *p_rx_data_msg;
    UINT16 client_num = p_msg->layer_specific;

#ifdef DEBUG_BSA_SV_CONTROL
    APPL_TRACE_DEBUG1("uipc_sv_control_rx_data length:%d", length);
//    scru_dump_hex(p_buffer, "Data", length, 0, 0);
#endif

    do
    {
        /* Call the function in charge of decoding control stream */
        status = bsa_strm_dec(&uipc_sv_ctrl_cb.rx_ctrl_stream[client_num],
                &p_buffer, &length);

        /* If a complete message have been received */
        if (status)
        {
            /* Allocate buffer for Rx data Msg */
            if ((p_rx_data_msg = (tBSA_SV_CONTROL_CLIENT_MSG *) GKI_getbuf(
                    sizeof(tBSA_SV_CONTROL_CLIENT_MSG))) != NULL)
            {
                memcpy(p_rx_data_msg->data,
                        &uipc_sv_ctrl_cb.rx_ctrl_stream[client_num].data[0],
                        uipc_sv_ctrl_cb.rx_ctrl_stream[client_num].decoded_length);
                p_rx_data_msg->hdr.event = UIPC_RX_DATA_EVT;
                p_rx_data_msg->hdr.len
                        = uipc_sv_ctrl_cb.rx_ctrl_stream[client_num].decoded_length;
                p_rx_data_msg->hdr.offset = 0;
                p_rx_data_msg->message_id
                        = uipc_sv_ctrl_cb.rx_ctrl_stream[client_num].decoded_msg_id;
                p_rx_data_msg->hdr.layer_specific = client_num;
#ifdef DEBUG_BSA_SV_CONTROL
                APPL_TRACE_DEBUG0("uipc_sv_control_rx_data calling server_rx_cback");
                APPL_TRACE_DEBUG2("uipc_sv_control_rx_data decoded_len:%d decoded_msgid:%d",
                        p_rx_data_msg->hdr.len,
                        p_rx_data_msg->hdr.layer_specific);
#endif
                uipc_sv_ctrl_cb.p_cback((BT_HDR *)p_rx_data_msg);
            }
            else
            {
                APPL_TRACE_ERROR0("uipc_sv_control_rx_data: GKI_getbuf failed");
            }
            /* rx_ctrl_stream must be re-initialized for every new message */
            bsa_strm_dec_init(&uipc_sv_ctrl_cb.rx_ctrl_stream[client_num]);
        }
        else
        {
#ifdef DEBUG_BSA_SV_CONTROL
            APPL_TRACE_DEBUG0("uipc_sv_control_rx_data bsa_strm_dec waiting for remaining data");
#endif
        }
    } while ((status) && (length != 0));

    GKI_freebuf(p_msg);
}


/*****************************************************************************
 **
 **  Name:           uipc_sv_pan.c
 **
 **  Description:    Server API for BSA UIPC PAN
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
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
#include "uipc_sv_pan.h"
#include "uipc_sv_socket.h"

/*
 * Local definitions
 */
/*
#define UIPC_SV_PAN_DEBUG
*/

#define UIPC_SV_PAN_MAX_CLIENT 1

#ifndef UIPC_PAN_SOCKET_NAME
#define UIPC_PAN_SOCKET_NAME "bt-pan-socket"
#endif

#ifndef UIPC_SV_PAN_SOCKET_MODE
/* By default:
 *  Non blocking mode
 *  Indication when data are ready to be read
 *  Indication when data can be written
 */
#define UIPC_SV_PAN_SOCKET_MODE (UIPC_SV_SOCKET_MODE_NON_BLOCK |         \
                                UIPC_SV_SOCKET_MODE_RX_DATA_READY_EVT | \
                                UIPC_SV_SOCKET_MODE_TX_DATA_READY_EVT)
#endif

#define UIPC_SV_PAN_CH_STATE_IDLE           0x00    /* Idle */
#define UIPC_SV_PAN_CH_STATE_LOCAL_OPEN     0x01    /* Locally Open */
#define UIPC_SV_PAN_CH_STATE_PEER_OPEN      0x02    /* Opened by peer (app) */
typedef UINT8 UIPC_SV_PAN_CH_STATE;

typedef struct
{
    BOOLEAN in_use;
    UIPC_SV_PAN_CH_STATE state;
    tUIPC_RCV_CBACK *p_callback;
    tUIPC_SV_SOCKET_SERVER_DESC server_desc;
} tUIPC_SV_PAN_CHANNEL;

typedef struct
{
    tUIPC_SV_PAN_CHANNEL channels[UIPC_CH_ID_PAN_NB];
} tUIPC_SV_PAN_CB;
static tUIPC_SV_PAN_CB uipc_sv_pan_cb;

/*
 * Local static function to UIPC callback
 */
static void uipc_sv_pan_cback(UINT8 channel_offset, BT_HDR *p_msg);

/* We use one dedicated callback per channel to distinguish which channel calls
 * the callback. These callback will call the generic callback above */
static void uipc_sv_pan_cback0(BT_HDR *p_msg);
#if (UIPC_CH_ID_PAN_NB > 1)
static void uipc_sv_pan_cback1(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_PAN_NB > 2)
static void uipc_sv_pan_cback2(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_PAN_NB > 3)
static void uipc_sv_pan_cback3(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_PAN_NB > 4)
static void uipc_sv_pan_cback4(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_PAN_NB > 5)
static void uipc_sv_pan_cback5(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_PAN_NB > 6)
static void uipc_sv_pan_cback6(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_PAN_NB > 7)
#error "Only 7 PAN connection can be supported"
#endif

tUIPC_RCV_CBACK * const uipc_sv_pan_cback_tab[UIPC_CH_ID_PAN_NB] =
{
    uipc_sv_pan_cback0,
#if (UIPC_CH_ID_PAN_NB > 1)
    uipc_sv_pan_cback1,
#endif
#if (UIPC_CH_ID_PAN_NB > 2)
    uipc_sv_pan_cback2,
#endif
#if (UIPC_CH_ID_PAN_NB > 3)
    uipc_sv_pan_cback3,
#endif
#if (UIPC_CH_ID_PAN_NB > 4)
    uipc_sv_pan_cback4,
#endif
#if (UIPC_CH_ID_PAN_NB > 5)
    uipc_sv_pan_cback5,
#endif
#if (UIPC_CH_ID_PAN_NB > 6)
    uipc_sv_pan_cback6,
#endif
};



/******************************************************************************
 **
 ** Function         uipc_sv_pan_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 ******************************************************************************/
void uipc_sv_pan_init(void)
{
    memset(&uipc_sv_pan_cb, 0, sizeof(uipc_sv_pan_cb));
}

/******************************************************************************
 **
 ** Function         uipc_sv_pan_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 ******************************************************************************/
BOOLEAN uipc_sv_pan_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    tUIPC_SV_PAN_CHANNEL *p_channel;
    int channel_offset;
    char socket_name[sizeof(UIPC_PAN_SOCKET_NAME) + 1];

    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_sv_pan_open fail p_cback NULL");

        return FALSE;
    }

    if ((channel_id < UIPC_CH_ID_PAN_FIRST) ||
            (channel_id > UIPC_CH_ID_PAN_LAST))
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_open wrong channelId:%d", channel_id);

        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_PAN_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_sv_pan_cb.channels[channel_offset];

    if (p_channel->in_use != FALSE)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_open channelId:%d already open",
                channel_id);

        return FALSE;
    }

    p_channel->in_use = TRUE;
    p_channel->state = UIPC_SV_PAN_CH_STATE_LOCAL_OPEN;
    p_channel->p_callback = p_cback;  /* Save callback function */

    /* Build a socket name (default socket name + offset) */
    snprintf(socket_name, sizeof(socket_name), "%s%d",
            UIPC_PAN_SOCKET_NAME, channel_offset);

    /* Starts server with local callback function */
    p_channel->server_desc = uipc_sv_socket_start_server(
            (UINT8 *)socket_name,
            uipc_sv_pan_cback_tab[channel_offset],
            UIPC_SV_PAN_MAX_CLIENT,
            UIPC_SV_PAN_SOCKET_MODE);
    if (p_channel->server_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_open fails to start pan server ChId:%d",
                channel_id);
        p_channel->state = UIPC_SV_PAN_CH_STATE_IDLE;

        return FALSE;
    }

    return TRUE;
}

/******************************************************************************
 **
 ** Function         bsa_sv_stop_pan_server
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 ******************************************************************************/
void uipc_sv_pan_close(tUIPC_CH_ID channel_id)
{
    tUIPC_SV_PAN_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_PAN_FIRST) ||
            (channel_id > UIPC_CH_ID_PAN_LAST))
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_close wrong channelId:%d", channel_id);

        return;
    }
    channel_offset = channel_id - UIPC_CH_ID_PAN_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_sv_pan_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_close channelId:%d was not open",
                channel_id);

        return;
    }

    p_channel->in_use = FALSE;

    uipc_sv_socket_stop_server(p_channel->server_desc,
            (UINT8 *)UIPC_PAN_SOCKET_NAME);
}

/******************************************************************************
 **
 ** Function         uipc_sv_pan_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Returns          BOOLEAN
 **
 ******************************************************************************/
BOOLEAN uipc_sv_pan_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
    BOOLEAN uipc_status;
    tUIPC_SV_PAN_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_PAN_FIRST) ||
            (channel_id > UIPC_CH_ID_PAN_LAST))
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_send_buf wrong channelId:%d",
                channel_id);
        p_msg->layer_specific = UIPC_LS_TX_FAIL;

        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_PAN_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_sv_pan_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_send_buf channelId:%d was not open",
                channel_id);
        p_msg->layer_specific = UIPC_LS_TX_FAIL;

        return FALSE;
    }

    /* Check if peer opened the socket: to prevent Pipe_Broken signal */
    if ((p_channel->state & UIPC_SV_PAN_CH_STATE_PEER_OPEN) == 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_send_buf channelId:%d was not open",
                channel_id);
        /* The application is supposed to open the channel soon */
        p_msg->layer_specific = UIPC_LS_TX_FLOW_OFF;  /* simulate a flow off */

        return FALSE;
    }

#ifdef UIPC_SV_PAN_DEBUG
    APPL_TRACE_DEBUG2("uipc_sv_pan_send_buf ChId:%d len:%d",
            channel_id, p_msg->len);
#endif

    uipc_status = uipc_sv_socket_send_buf(p_channel->server_desc, 0, p_msg);

    return uipc_status;
}

/******************************************************************************
 **
 ** Function           uipc_sv_pan_ioctl
 **
 ** Description        Control the PAN channel
 **
 ** Parameters         desc: socket server descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            TRUE if successful, FALSE otherwise
 **
 ******************************************************************************/
BOOLEAN uipc_sv_pan_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param)
{
    tUIPC_SV_PAN_CHANNEL *p_channel;
    int channel_offset;
    BOOLEAN ret_code = FALSE;

    if ((channel_id < UIPC_CH_ID_PAN_FIRST) ||
            (channel_id > UIPC_CH_ID_PAN_LAST))
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_ioctl wrong channelId:%d", channel_id);

        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_PAN_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_sv_pan_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_ioctl channelId:%d not in use",
                channel_id);

        return FALSE;
    }

    switch (request)
    {
    case UIPC_REQ_TX_READY:
    case UIPC_REQ_RX_READY:
        return uipc_sv_socket_ioctl(p_channel->server_desc, request, NULL);

        break;

    default:
        APPL_TRACE_ERROR1("uipc_sv_pan_ioctl bad request:%d", request);

        break;
    }

    return ret_code;
}


/******************************************************************************
 **
 ** Function         uipc_sv_pan_read
 **
 ** Description      This function is used to read data from a client
 **
 ** Returns          voidUINT32
 **
 ******************************************************************************/
UINT32 uipc_sv_pan_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 buflen)
{
    UINT32 nb_bytes;
    tUIPC_SV_PAN_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_PAN_FIRST) ||
            (channel_id > UIPC_CH_ID_PAN_LAST))
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_read wrong channelId:%d", channel_id);

        return 0;
    }
    channel_offset = channel_id - UIPC_CH_ID_PAN_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_sv_pan_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_read channelId:%d was not open",
                channel_id);

        return 0;
    }

    /* Check if peer opened the socket: to prevent Pipe_Broken signal */
    if ((p_channel->state & UIPC_SV_PAN_CH_STATE_PEER_OPEN) == 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_read channelId:%d was not open",
                channel_id);

        /* The application is supposed to open the channel soon */
        return 0;
    }

    nb_bytes = uipc_sv_socket_read(p_channel->server_desc, 0, p_buf, buflen);

#ifdef UIPC_SV_PAN_DEBUG
    APPL_TRACE_DEBUG3("uipc_sv_pan_read ChId:%d asked:%d read:%d",
            channel_id, buflen, nb_bytes);
#endif

    return nb_bytes;
}


/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback
 **
 ** Description      This function Handle UIPC callback for PAN
 **
 ** Parameters
 **
 ** Returns          void
 **
 ******************************************************************************/
static void uipc_sv_pan_cback(UINT8 channel_offset, BT_HDR *p_msg)
{
    tUIPC_CH_ID channel_id;
    tUIPC_SV_PAN_CHANNEL *p_channel;

#ifdef UIPC_SV_PAN_DEBUG
    APPL_TRACE_DEBUG0("uipc_sv_pan_cback");
#endif

    /* Sanity check */
    if (channel_offset >= UIPC_CH_ID_PAN_NB)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_cback bad channel_offset:%d",
                channel_offset);
        GKI_freebuf(p_msg);

        return;
    }
    if (uipc_sv_pan_cb.channels[channel_offset].in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_sv_pan_cback channel:%d not in use",
                channel_offset);
        GKI_freebuf(p_msg);

        return;
    }

    /* Get reference on server desc */
    p_channel = &uipc_sv_pan_cb.channels[channel_offset];

    /* Calculate Channel_Id from channel_offset */
    channel_id = channel_offset + UIPC_CH_ID_PAN_FIRST;

    /* Update layer_specific field with Channel_id (num_client useless for PAN)     */
    p_msg->layer_specific = channel_id;

    /*  If this is a connection  event */
    if (p_msg->event == UIPC_OPEN_EVT)
    {
        /* The application connects */
        p_channel->state |= UIPC_SV_PAN_CH_STATE_PEER_OPEN;
        p_channel->p_callback(p_msg);
    }
    /*  If this is a disconnection event */
    else if (p_msg->event == UIPC_CLOSE_EVT)
    {
        /* The application disconnects */
        p_channel->state &= ~UIPC_SV_PAN_CH_STATE_PEER_OPEN;
        p_channel->p_callback(p_msg);
    }
    /* If it's a Rx Data Ready indication */
    else if (p_msg->event == UIPC_RX_DATA_READY_EVT)
    {
        p_channel->p_callback(p_msg);
    }
    /* If it's a Tx Data Ready indication */
    else if (p_msg->event == UIPC_TX_DATA_READY_EVT)
    {
        p_channel->p_callback(p_msg);
    }
    /* If it's a Rx data indication */
    else if (p_msg->event == UIPC_RX_DATA_EVT)
    {
        /* The new PAN implementation should not use this event */
        APPL_TRACE_ERROR1("uipc_sv_pan_cback bad channel_offset:%d",
                channel_offset);
        GKI_freebuf(p_msg);

        return;
    }
    else
    {
        APPL_TRACE_ERROR1(
                "uipc_sv_pan_cback bad event received from UIPC:%d",
                p_msg->event);
        GKI_freebuf(p_msg);
    }
}

/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback0
 **
 ** Description      This function Handle UIPC callback of 1st channel for PAN
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
static void uipc_sv_pan_cback0(BT_HDR *p_msg)
{
    uipc_sv_pan_cback(0, p_msg);
}

/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback1
 **
 ** Description      This function Handle UIPC callback of 2nd channel for PAN
 **
 ** Returns          void
 **
 ******************************************************************************/
#if (UIPC_CH_ID_PAN_NB > 1)
static void uipc_sv_pan_cback1(BT_HDR *p_msg)
{
    uipc_sv_pan_cback(1, p_msg);
}
#endif

/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback2
 **
 ** Description      This function Handle UIPC callback of 3rd channel for PAN
 **
 ** Returns          void
 **
 ******************************************************************************/
#if (UIPC_CH_ID_PAN_NB > 2)
static void uipc_sv_pan_cback2(BT_HDR *p_msg)
{
    uipc_sv_pan_cback(2, p_msg);
}
#endif

/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback3
 **
 ** Description      This function Handle UIPC callback of 4th channel for PAN
 **
 ** Returns          void
 **
 ******************************************************************************/
#if (UIPC_CH_ID_PAN_NB > 3)
static void uipc_sv_pan_cback3(BT_HDR *p_msg)
{
    uipc_sv_pan_cback(3, p_msg);
}
#endif

/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback4
 **
 ** Description      This function Handle UIPC callback of 5th channel for PAN
 **
 ** Returns          void
 **
 ******************************************************************************/
#if (UIPC_CH_ID_PAN_NB > 4)
static void uipc_sv_pan_cback4(BT_HDR *p_msg)
{
    uipc_sv_pan_cback(4, p_msg);
}
#endif

/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback5
 **
 ** Description      This function Handle UIPC callback of 6th channel for PAN
 **
 ** Returns          void
 **
 ******************************************************************************/
#if (UIPC_CH_ID_PAN_NB > 5)
static void uipc_sv_pan_cback5(BT_HDR *p_msg)
{
    uipc_sv_pan_cback(5, p_msg);
}
#endif

/******************************************************************************
 **
 ** Function         uipc_sv_pan_cback6
 **
 ** Description      This function Handle UIPC callback of 7th channel for PAN
 **
 ** Returns          void
 **
 ******************************************************************************/
#if (UIPC_CH_ID_PAN_NB > 6)
static void uipc_sv_pan_cback6(BT_HDR *p_msg)
{
    uipc_sv_pan_cback(6, p_msg);
}
#endif


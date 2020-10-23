/*****************************************************************************
 **
 **  Name:           uipc_cl_hl.c
 **
 **  Description:    client API for BSA UIPC HL
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bt_target.h"
#include "bsa_api.h"
#include "bsa_int.h"
#include "uipc.h"
#include "uipc_cl_hl.h"
#include "uipc_cl_socket.h"

/* #define UIPC_CL_HL_DEBUG */

#ifndef UIPC_HL_SOCKET_NAME
#define UIPC_HL_SOCKET_NAME "bt-hl-socket"
#endif

#ifndef UIPC_CL_HL_SOCKET_MODE
/* By default. Read and Write are not blocking and Rx/Tx Ready events needed*/
#define UIPC_CL_HL_SOCKET_MODE (UIPC_CL_SOCKET_MODE_NON_BLOCK |         \
                                UIPC_CL_SOCKET_MODE_RX_DATA_READY_EVT | \
                                UIPC_CL_SOCKET_MODE_TX_DATA_READY_EVT)
#endif

/* Size of the Header (one U16 for the moment)*/
#define tBSA_CL_HL_UIPC_TX_HEADER_SIZE  2

typedef enum
{
    tUIPC_CL_HL_TX_STATE_IDLE = 0,
    tUIPC_CL_HL_TX_STATE_HEADER, /* Only part of header sent */
    tUIPC_CL_HL_TX_STATE_DATA,  /* Only part of data sent */
} tUIPC_CL_HL_TX_STATE;

typedef struct
{
    tUIPC_CL_HL_TX_STATE state;
    BT_HDR *p_msg;
    UINT8 header[tBSA_CL_HL_UIPC_TX_HEADER_SIZE];
    UINT8 header_length_sent;
} tUIPC_CL_HL_TX_FLOW;

typedef struct
{
    BOOLEAN in_use;
    tUIPC_RCV_CBACK *p_callback;
    tUIPC_CL_SOCKET_DESC socket_desc;
    tUIPC_CL_HL_TX_FLOW tx_flow;
} tUIPC_CL_HL_CHANNEL;

typedef struct
{
    tUIPC_CL_HL_CHANNEL channels[UIPC_CH_ID_HL_NB];
} tUIPC_CL_HL_CB;

/*
 * Global variable
 */
static tUIPC_CL_HL_CB uipc_cl_hl_cb;

/*
 * Local functions
 */
static void uipc_cl_hl_cback(UINT8 channel_offset, BT_HDR *p_msg);

/* We use one dedicated callback per channel to distinguish which channel calls
 * the callback. These callback will call the generic callback above */
void uipc_cl_hl_cback0(BT_HDR *p_msg);
#if (UIPC_CH_ID_HL_NB > 1)
void uipc_cl_hl_cback1(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_HL_NB > 2)
void uipc_cl_hl_cback2(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_HL_NB > 3)
void uipc_cl_hl_cback3(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_HL_NB > 4)
void uipc_cl_hl_cback4(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_HL_NB > 5)
void uipc_cl_hl_cback5(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_HL_NB > 6)
void uipc_cl_hl_cback6(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_HL_NB > 7)
#error "Only 7 SPP connection can be supported"
#endif

tUIPC_RCV_CBACK * const uipc_cl_hl_cback_tab[UIPC_CH_ID_HL_NB] =
{
        uipc_cl_hl_cback0,
        #if (UIPC_CH_ID_HL_NB > 1)
        uipc_cl_hl_cback1,
        #endif
        #if (UIPC_CH_ID_HL_NB > 2)
        uipc_cl_hl_cback2,
        #endif
        #if (UIPC_CH_ID_HL_NB > 3)
        uipc_cl_hl_cback3,
        #endif
        #if (UIPC_CH_ID_HL_NB > 4)
        uipc_cl_hl_cback4,
        #endif
        #if (UIPC_CH_ID_HL_NB > 5)
        uipc_cl_hl_cback5,
        #endif
        #if (UIPC_CH_ID_HL_NB > 6)
        uipc_cl_hl_cback6,
        #endif
};


/*******************************************************************************
 **
 ** Function         uipc_cl_hl_init
 **
 ** Description      UIPC Client HL Init
 **
 ** Parameters       none
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_hl_init(void)
{
    memset(&uipc_cl_hl_cb, 0, sizeof(uipc_cl_hl_cb));
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_open
 **
 ** Description      UIPC Client HL open.
 **
 ** Parameters       channel_id: channel id
 **                  p_cback: pointer of callback function
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hl_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    char socket_name[sizeof(UIPC_HL_SOCKET_NAME) + 1];
    tUIPC_CL_HL_CHANNEL *p_channel;
    int channel_offset;

    APPL_TRACE_DEBUG0("uipc_cl_hl_open");

    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_hl_open p_cback NULL");
        return FALSE;
    }

    if ((channel_id < UIPC_CH_ID_HL_FIRST) || (channel_id > UIPC_CH_ID_HL_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_open wrong channelId:%d", channel_id);
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_HL_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_hl_cb.channels[channel_offset];

    if (p_channel->in_use != FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_open channelId:%d already open", channel_id);
        return FALSE;
    }

    p_channel->in_use = TRUE;
    p_channel->p_callback = p_cback;  /* Save callback function */
    p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_IDLE;
    p_channel->tx_flow.p_msg = NULL;

    /* Build a socket name (default socket name + offset) */
    snprintf(socket_name, sizeof(socket_name), "%s%d", UIPC_HL_SOCKET_NAME,
            channel_offset);

    /* connect to server with local callback function */
    p_channel->socket_desc = uipc_cl_socket_connect((UINT8 *)socket_name,
            uipc_cl_hl_cback_tab[channel_offset],
            UIPC_CL_HL_SOCKET_MODE);

    if (p_channel->socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR0("uipc_cl_hl_open fails to connect to HL server");
        p_channel->p_callback = NULL;
        p_channel->in_use = FALSE;
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_close
 **
 ** Description      UIPC Client HL close.
 **
 ** Parameters       channel_id: uipc channel id
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_hl_close(tUIPC_CH_ID channel_id)
{
    tUIPC_CL_HL_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_HL_FIRST) || (channel_id > UIPC_CH_ID_HL_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_close wrong channelId:%d", channel_id);
        return;
    }
    channel_offset = channel_id - UIPC_CH_ID_HL_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_hl_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_close channelId:%d was not open", channel_id);
        return;
    }

    uipc_cl_socket_disconnect(p_channel->socket_desc);
    p_channel->in_use = FALSE;
    p_channel->p_callback = NULL;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** parameters       channel_id: uipc channel id
 **                  p_msg: pointer of data to send
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hl_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
    BOOLEAN uipc_status;
    tUIPC_CL_HL_CHANNEL *p_channel;
    int channel_offset;
    UINT8 *p_header;
    int nb_wrote;

    if ((channel_id < UIPC_CH_ID_HL_FIRST) || (channel_id > UIPC_CH_ID_HL_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_send_buf wrong channelId:%d", channel_id);
        p_msg->layer_specific = UIPC_LS_TX_FAIL;
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_HL_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_hl_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_send_buf channelId:%d was not open", channel_id);
        p_msg->layer_specific = UIPC_LS_TX_FAIL;
        return FALSE;
    }

    /* Accept to try to send a new packet only if no pending packet */
    if (p_channel->tx_flow.state == tUIPC_CL_HL_TX_STATE_IDLE)
    {
        /* Build the header */
        p_header = &p_channel->tx_flow.header[0];
        p_channel->tx_flow.header_length_sent = 0;
        UINT16_TO_STREAM(p_header, p_msg->len);
        nb_wrote = uipc_cl_socket_write(p_channel->socket_desc,
                &p_channel->tx_flow.header[0],
                tBSA_CL_HL_UIPC_TX_HEADER_SIZE);
        if (nb_wrote < 0)
        {
            /* This is an error */
            p_msg->layer_specific = UIPC_LS_TX_FAIL;
            return FALSE;
        }
        else if (nb_wrote == 0)
        {
            /* This is not an error (Tx Flow Off) */
            p_msg->layer_specific = UIPC_LS_TX_FLOW_OFF;
            return FALSE;
        }
        else if (nb_wrote < tBSA_CL_HL_UIPC_TX_HEADER_SIZE)
        {
            /* Only part of Header sent */
            APPL_TRACE_DEBUG0("uipc_cl_hl_send_buf partial header");
            p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_HEADER;
            p_channel->tx_flow.header_length_sent += nb_wrote;
            p_channel->tx_flow.p_msg = p_msg;
            /* Ask to receive Tx_Ready event (to continue later) */
            uipc_cl_socket_ioctl(p_channel->socket_desc, UIPC_REQ_TX_READY, NULL);
            /* Send ok to application */
            return TRUE;
        }
        else
        {
            /* Length sent */
#ifdef UIPC_CL_HL_DEBUG
            APPL_TRACE_DEBUG0("uipc_cl_hl_send_buf Header sent");
            APPL_TRACE_DEBUG2("uipc_cl_hl_send_buf try to send buffer ChId:%d len:%d", channel_id, p_msg->len);
#endif
            /* Try to send the Data buffer */
            uipc_status = uipc_cl_socket_send_buf(p_channel->socket_desc, p_msg);
            if (uipc_status == FALSE)
            {
                /* Partial Write */
                if (p_msg->layer_specific == UIPC_LS_TX_FLOW_OFF)
                {
                    /* Save state and msg for future automatic retransmission */
                    p_channel->tx_flow.p_msg = p_msg;
                    p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_DATA;
                    /* Ask to receive Tx_Ready event (to continue later) */
                    uipc_cl_socket_ioctl(p_channel->socket_desc, UIPC_REQ_TX_READY, NULL);
                    return TRUE;
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_cl_hl_send_buf unable to send buffer");
                    return FALSE;
                }
            }
            else
            {
                /* Buffer has been written to the socket */
                p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_IDLE;
                return TRUE;
            }
        }
    }
    else
    {
        /* This is not an error (Tx Flow Off) */
        APPL_TRACE_DEBUG1("uipc_cl_hl_send_buf a Tx packet was already pending ChId:%d", channel_id);
        p_msg->layer_specific = UIPC_LS_TX_FLOW_OFF;
        return FALSE;
    }
}

/*******************************************************************************
 **
 ** Function           uipc_cl_hl_ioctl
 **
 ** Description        Control the HL channel
 **
 ** Parameters         desc: socket server descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hl_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param)
{
    tUIPC_CL_HL_CHANNEL *p_channel;
    int channel_offset;
    BOOLEAN ret_code = FALSE;

    if ((channel_id < UIPC_CH_ID_HL_FIRST) || (channel_id > UIPC_CH_ID_HL_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_ioctl wrong channelId:%d", channel_id);
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_HL_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_hl_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_ioctl channelId:%d not in use", channel_id);
        return FALSE;
    }

    switch (request)
    {
    case UIPC_REQ_TX_READY:
    case UIPC_REQ_RX_READY:
        return uipc_cl_socket_ioctl(p_channel->socket_desc, request, NULL);
        break;

    default:
        APPL_TRACE_ERROR1("uipc_cl_hl_ioctl bad request:%d", request);
        break;
    }
    return ret_code;
}


/*******************************************************************************
 **
 ** Function         uipc_cl_hl_read
 **
 ** Description      This function is used to read data from the socket
 **
 ** Parameters       channel_id: uipc channel id
 **                  p_buf: pointer of buffer
 **                  buflen: length of buffer
 **
 ** Returns          UINT32
 **
 *******************************************************************************/
UINT32 uipc_cl_hl_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 buflen)
{
    UINT32 nb_bytes;
    tUIPC_CL_HL_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_HL_FIRST) || (channel_id > UIPC_CH_ID_HL_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_read wrong channelId:%d", channel_id);
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_HL_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_hl_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_read channelId:%d was not open", channel_id);
        return FALSE;
    }

    nb_bytes = uipc_cl_socket_read(p_channel->socket_desc, p_buf, buflen);

#ifdef UIPC_CL_HL_DEBUG
    APPL_TRACE_DEBUG3("uipc_cl_hl_read ChId:%d asked:%d read:%d", channel_id, buflen, nb_bytes);
#endif

    return nb_bytes;
}

/*******************************************************************************
 **uipc_cl_hl_tx_ready_ind
 ** Function         uipc_cl_hl_cback
 **
 ** Description      This function handles UIPC TxReady event  HL
 **
 ** Parameters       channel_offset: index of uipc channel number
 **
 ** Returns          void
 **
 *******************************************************************************/
static BOOLEAN uipc_cl_hl_tx_ready_ind(UINT8 channel_offset)
{
    tUIPC_CL_HL_CHANNEL *p_channel;
    int nb_wrote;
    BOOLEAN uipc_status;
    UINT8 *p_header;
    UINT8 header_len;

    if (channel_offset >= UIPC_CH_ID_HL_NB)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_tx_ready_ind wrong Channel Offset:%d", channel_offset);
        return FALSE;
    }
    /* Get reference on the Channel */
    p_channel = &uipc_cl_hl_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_tx_ready_ind ChannelId:%d was not open", channel_offset);
        return FALSE;
    }

    if (p_channel->tx_flow.state == tUIPC_CL_HL_TX_STATE_IDLE)
    {
        /* Nothing pending */
        return TRUE;
    }
    else
    {
        /* If only a part of the header was sent */
        if (p_channel->tx_flow.state == tUIPC_CL_HL_TX_STATE_HEADER)
        {
            /* Point to the Header part to send */
            p_header = &p_channel->tx_flow.header[0] + p_channel->tx_flow.header_length_sent;
            header_len = tBSA_CL_HL_UIPC_TX_HEADER_SIZE - p_channel->tx_flow.header_length_sent;

            /* Try to write the remaining part of the Header */
            nb_wrote = uipc_cl_socket_write(p_channel->socket_desc, p_header, header_len);
            if (nb_wrote < 0)
            {
                APPL_TRACE_ERROR1("uipc_cl_hl_tx_ready_ind unable to write Length ChannelOffset:%d", channel_offset);
                p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_IDLE;
                GKI_freebuf(p_channel->tx_flow.p_msg);
                p_channel->tx_flow.p_msg = NULL;
                return FALSE;
            }
            else if (nb_wrote == 0)
            {
                APPL_TRACE_ERROR1("uipc_cl_hl_tx_ready_ind Flow still Off ??? ChannelOffset:%d", channel_offset);
                /* Ask to receive Tx_Ready event (to continue later) */
                uipc_cl_socket_ioctl(p_channel->socket_desc, UIPC_REQ_TX_READY, NULL);
               return FALSE;
            }
            /* Partial write */
            else if (nb_wrote != header_len)
            {
                APPL_TRACE_ERROR1("uipc_cl_hl_tx_ready_ind Partial write:%d", nb_wrote);
                p_channel->tx_flow.header_length_sent += nb_wrote;
                /* Ask to receive Tx_Ready event (to continue later) */
                uipc_cl_socket_ioctl(p_channel->socket_desc, UIPC_REQ_TX_READY, NULL);
                return FALSE;
            }
            else
            {
                /* Header sent. We can try to send the data */
                p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_DATA;
            }
        }
        /* If Length was sent */
        if (p_channel->tx_flow.state == tUIPC_CL_HL_TX_STATE_DATA)
        {
            uipc_status = uipc_cl_socket_send_buf(p_channel->socket_desc,
                    p_channel->tx_flow.p_msg);
            if (uipc_status == FALSE)
            {
                /* Partial Write */
                if (p_channel->tx_flow.p_msg->layer_specific == UIPC_LS_TX_FLOW_OFF)
                {
                    /* State and msg kept for future automatic retransmission */
                    /* Ask to receive Tx_Ready event (to continue later) */
                    uipc_cl_socket_ioctl(p_channel->socket_desc, UIPC_REQ_TX_READY, NULL);
                    return FALSE;
                }
                else
                {
                    APPL_TRACE_ERROR1("uipc_cl_hl_tx_ready_ind unable to write data ChannelOffset:%d", channel_offset);
                    p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_IDLE;
                    GKI_freebuf(p_channel->tx_flow.p_msg);
                    p_channel->tx_flow.p_msg = NULL;
                    return FALSE;
                }
            }
            else
            {
                /* Buffer has been written to the socket */
                p_channel->tx_flow.state = tUIPC_CL_HL_TX_STATE_IDLE;
                return TRUE;
            }
        }
    }
    /* Just in case */
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback
 **
 ** Description      This function handles UIPC callback for HL
 **
 ** Parameters       channel_offset: index of uipc channel number
 **                  p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_cl_hl_cback(UINT8 channel_offset, BT_HDR *p_msg)
{
    tUIPC_CH_ID channel_id;
    tUIPC_CL_HL_CHANNEL *p_channel;

#ifdef UIPC_CL_HL_DEBUG
    APPL_TRACE_DEBUG0("uipc_cl_hl_cback");
#endif

    /* Sanity check */
    if (channel_offset >= UIPC_CH_ID_HL_NB)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_cback bad channel_offset:%d", channel_offset);
        GKI_freebuf(p_msg);
        return;
    }
    if (uipc_cl_hl_cb.channels[channel_offset].in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_hl_cback channel:%d not in use", channel_offset);
        GKI_freebuf(p_msg);
        return;
    }

    /* Get reference on server desc */
    p_channel = &uipc_cl_hl_cb.channels[channel_offset];

    /* Calculate Channel_Id from channel_offset */
    channel_id = channel_offset + UIPC_CH_ID_HL_FIRST;

    /* Update layer_specific field with Channel_id (num_client useless for HL) */
    p_msg->layer_specific = channel_id;

    /*  If this is a disconnection event */
    if (p_msg->event == UIPC_CLOSE_EVT)
    {
        p_channel->in_use = FALSE;
        p_channel->p_callback(p_msg);
        p_channel->p_callback = NULL;
    }
    /* If it's a Rx Data Ready indication */
    else if (p_msg->event == UIPC_RX_DATA_READY_EVT)
    {
        p_channel->p_callback(p_msg);
    }
    /* If it's a Tx Data Ready indication */
    else if (p_msg->event == UIPC_TX_DATA_READY_EVT)
    {
        /* Handle internally TxReady event (to send pending packet if any) */
        if (uipc_cl_hl_tx_ready_ind(channel_offset))
        {
            /* If Tx is still ready, then pass the info to the application */
            p_channel->p_callback(p_msg);
        }
        else
        {
            /* Ask to receive Tx_Ready event (to continue later) */
            uipc_cl_socket_ioctl(p_channel->socket_desc, UIPC_REQ_TX_READY, NULL);
        }
    }
    /* If it's a Rx data indication */
    else if (p_msg->event == UIPC_RX_DATA_EVT)
    {
        /* The new HL implementation should not use this event */
        APPL_TRACE_ERROR1("uipc_cl_hl_cback UIPC_RX_DATA_EVT received channel_offset:%d", channel_offset);
        GKI_freebuf(p_msg);
        return;
    }
    else
    {
        APPL_TRACE_ERROR2("uipc_cl_hl_cback bad event:%d received from UIPC:%d",
                p_msg->event, channel_id);
        GKI_freebuf(p_msg);
    }
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback0
 **
 ** Description      This function Handle UIPC callback of first channel for HL
 **
 ** Parameters       p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_hl_cback0(BT_HDR *p_msg)
{
    uipc_cl_hl_cback(0, p_msg);
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback1
 **
 ** Description      This function Handle UIPC callback of second channel for HL
 **
 ** Parameters       p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_HL_NB > 1)
void uipc_cl_hl_cback1(BT_HDR *p_msg)
{
    uipc_cl_hl_cback(1, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback2
 **
 ** Description      This function Handle UIPC callback of first channel for HL
 **
 ** Parameters       p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_HL_NB > 2)
void uipc_cl_hl_cback2(BT_HDR *p_msg)
{
    uipc_cl_hl_cback(2, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback3
 **
 ** Description      This function Handle UIPC callback of third channel for HL
 **
 ** Parameters       p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_HL_NB > 3)
void uipc_cl_hl_cback3(BT_HDR *p_msg)
{
    uipc_cl_hl_cback(3, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback4
 **
 ** Description      This function Handle UIPC callback of fourth channel for HL
 **
 ** Parameters       p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_HL_NB > 4)
void uipc_cl_hl_cback4(BT_HDR *p_msg)
{
    uipc_cl_hl_cback(4, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback5
 **
 ** Description      This function Handle UIPC callback of fifth channel for HL
 **
 ** Parameters       p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_HL_NB > 5)
void uipc_cl_hl_cback5(BT_HDR *p_msg)
{
    uipc_cl_hl_cback(5, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_cback6
 **
 ** Description      This function Handle UIPC callback of sixth channel for HL
 **
 ** Parameters       p_msg: pointer of message
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_HL_NB > 6)
void uipc_cl_hl_cback6(BT_HDR *p_msg)
{
    uipc_cl_hl_cback(6, p_msg);
}
#endif


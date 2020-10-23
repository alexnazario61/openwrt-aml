/*****************************************************************************
 **
 **  Name:           uipc_cl_dg.c
 **
 **  Description:    client API for BSA UIPC DG
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
#include "uipc_cl_dg.h"
#include "uipc_cl_socket.h"

#define UIPC_CL_DG_DEBUG

#ifndef UIPC_DG_SOCKET_NAME
#define UIPC_DG_SOCKET_NAME "bt-dg-socket"
#endif

#ifndef UIPC_CL_DG_SOCKET_MODE
/* By default. Read and Write are not blocking and Rx/Tx Ready events needed*/
#define UIPC_CL_DG_SOCKET_MODE (UIPC_CL_SOCKET_MODE_NON_BLOCK |         \
                                UIPC_CL_SOCKET_MODE_RX_DATA_READY_EVT | \
                                UIPC_CL_SOCKET_MODE_TX_DATA_READY_EVT)
#endif

typedef struct
{
    BOOLEAN in_use;
    tUIPC_RCV_CBACK *p_callback;
    tUIPC_CL_SOCKET_DESC socket_desc;
} tUIPC_CL_DG_CHANNEL;

typedef struct
{
    tUIPC_CL_DG_CHANNEL channels[UIPC_CH_ID_DG_NB];
} tUIPC_CL_DG_CB;

/*
 * Global variable
 */
static tUIPC_CL_DG_CB uipc_cl_dg_cb;

/*
 * Local functions
 */
static void uipc_cl_dg_cback(UINT8 channel_offset, BT_HDR *p_msg);

/* We use one dedicated callback per channel to distinguish which channel calls
 * the callback. These callback will call the generic callback above */
void uipc_cl_dg_cback0(BT_HDR *p_msg);
#if (UIPC_CH_ID_DG_NB > 1)
void uipc_cl_dg_cback1(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 2)
void uipc_cl_dg_cback2(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 3)
void uipc_cl_dg_cback3(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 4)
void uipc_cl_dg_cback4(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 5)
void uipc_cl_dg_cback5(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 6)
void uipc_cl_dg_cback6(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 7)
void uipc_cl_dg_cback7(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 8)
void uipc_cl_dg_cback8(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 9)
void uipc_cl_dg_cback9(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 10)
void uipc_cl_dg_cback10(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 11)
void uipc_cl_dg_cback11(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 12)
void uipc_cl_dg_cback12(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 13)
void uipc_cl_dg_cback13(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 14)
void uipc_cl_dg_cback14(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 15)
void uipc_cl_dg_cback15(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 16)
void uipc_cl_dg_cback16(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 17)
void uipc_cl_dg_cback17(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 18)
void uipc_cl_dg_cback18(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 19)
void uipc_cl_dg_cback19(BT_HDR *p_msg);
#endif
#if (UIPC_CH_ID_DG_NB > 20)
#error "Only 20 SPP connection can be supported"
#endif

tUIPC_RCV_CBACK * const uipc_cl_dg_cback_tab[UIPC_CH_ID_DG_NB] =
{
        uipc_cl_dg_cback0,
        #if (UIPC_CH_ID_DG_NB > 1)
        uipc_cl_dg_cback1,
        #endif
        #if (UIPC_CH_ID_DG_NB > 2)
        uipc_cl_dg_cback2,
        #endif
        #if (UIPC_CH_ID_DG_NB > 3)
        uipc_cl_dg_cback3,
        #endif
        #if (UIPC_CH_ID_DG_NB > 4)
        uipc_cl_dg_cback4,
        #endif
        #if (UIPC_CH_ID_DG_NB > 5)
        uipc_cl_dg_cback5,
        #endif
        #if (UIPC_CH_ID_DG_NB > 6)
        uipc_cl_dg_cback6,
        #endif
        #if (UIPC_CH_ID_DG_NB > 7)
        uipc_cl_dg_cback7,
        #endif
        #if (UIPC_CH_ID_DG_NB > 8)
        uipc_cl_dg_cback8,
        #endif
        #if (UIPC_CH_ID_DG_NB > 9)
        uipc_cl_dg_cback9,
        #endif
        #if (UIPC_CH_ID_DG_NB > 10)
        uipc_cl_dg_cback10,
        #endif
        #if (UIPC_CH_ID_DG_NB > 11)
        uipc_cl_dg_cback11,
        #endif
        #if (UIPC_CH_ID_DG_NB > 12)
        uipc_cl_dg_cback12,
        #endif
        #if (UIPC_CH_ID_DG_NB > 13)
        uipc_cl_dg_cback13,
        #endif
        #if (UIPC_CH_ID_DG_NB > 14)
        uipc_cl_dg_cback14,
        #endif
        #if (UIPC_CH_ID_DG_NB > 15)
        uipc_cl_dg_cback15,
        #endif
        #if (UIPC_CH_ID_DG_NB > 16)
        uipc_cl_dg_cback16,
        #endif
        #if (UIPC_CH_ID_DG_NB > 17)
        uipc_cl_dg_cback17,
        #endif
        #if (UIPC_CH_ID_DG_NB > 18)
        uipc_cl_dg_cback18,
        #endif
        #if (UIPC_CH_ID_DG_NB > 19)
        uipc_cl_dg_cback19,
        #endif
};


/*******************************************************************************
 **
 ** Function         uipc_cl_dg_init
 **
 ** Description      UIPC Client DG Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_dg_init(void)
{
    memset(&uipc_cl_dg_cb, 0, sizeof(uipc_cl_dg_cb));
}

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_open
 **
 ** Description      UIPC Client DG connection
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_cl_dg_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    char socket_name[sizeof(UIPC_DG_SOCKET_NAME) + 3];
    tUIPC_CL_DG_CHANNEL *p_channel;
    int channel_offset;

    APPL_TRACE_DEBUG0("uipc_cl_dg_open");

    memset(socket_name, 0, sizeof(socket_name));

    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_dg_open p_cback NULL");
        return FALSE;
    }

    if ((channel_id < UIPC_CH_ID_DG_FIRST) || (channel_id > UIPC_CH_ID_DG_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_open wrong channelId:%d", channel_id);
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_DG_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_dg_cb.channels[channel_offset];

    if (p_channel->in_use != FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_open channelId:%d already open", channel_id);
        return FALSE;
    }

    p_channel->in_use = TRUE;
    p_channel->p_callback = p_cback;  /* Save callback function */

    /* Build a socket name (default socket name + offset) */
    snprintf(socket_name, sizeof(socket_name), "%s%d", UIPC_DG_SOCKET_NAME,
            channel_offset);

    /* connect to server with local callback function */
    p_channel->socket_desc = uipc_cl_socket_connect((UINT8 *)socket_name,
            uipc_cl_dg_cback_tab[channel_offset],
            UIPC_CL_DG_SOCKET_MODE);

    if (p_channel->socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR0("uipc_cl_dg_open fails to connect to DG server");
        p_channel->p_callback = NULL;
        p_channel->in_use = FALSE;
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_dg_close(tUIPC_CH_ID channel_id)
{
    tUIPC_CL_DG_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_DG_FIRST) || (channel_id > UIPC_CH_ID_DG_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_close wrong channelId:%d", channel_id);
        return;
    }
    channel_offset = channel_id - UIPC_CH_ID_DG_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_dg_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_close channelId:%d was not open", channel_id);
        return;
    }

    uipc_cl_socket_disconnect(p_channel->socket_desc);
    p_channel->in_use = FALSE;
    p_channel->p_callback = NULL;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_cl_dg_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
    BOOLEAN uipc_status;
    tUIPC_CL_DG_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_DG_FIRST) || (channel_id > UIPC_CH_ID_DG_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_send_buf wrong channelId:%d", channel_id);
        p_msg->layer_specific = UIPC_LS_TX_FAIL;
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_DG_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_dg_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_send_buf channelId:%d was not open", channel_id);
        p_msg->layer_specific = UIPC_LS_TX_FAIL;
        return FALSE;
    }


#ifdef UIPC_CL_DG_DEBUG
    APPL_TRACE_DEBUG2("uipc_cl_dg_send_buf ChId:%d len:%d", channel_id, p_msg->len);
#endif

    uipc_status = uipc_cl_socket_send_buf(p_channel->socket_desc, p_msg);

    return uipc_status;
}

/*******************************************************************************
 **
 ** Function           uipc_cl_dg_ioctl
 **
 ** Description        Control the DG channel
 **
 ** Parameters         desc: socket server descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_dg_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param)
{
    tUIPC_CL_DG_CHANNEL *p_channel;
    int channel_offset;
    BOOLEAN ret_code = FALSE;

    if ((channel_id < UIPC_CH_ID_DG_FIRST) || (channel_id > UIPC_CH_ID_DG_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_ioctl wrong channelId:%d", channel_id);
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_DG_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_dg_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_ioctl channelId:%d not in use", channel_id);
        return FALSE;
    }

    switch (request)
    {
    case UIPC_REQ_TX_READY:
    case UIPC_REQ_RX_READY:
        return uipc_cl_socket_ioctl(p_channel->socket_desc, request, NULL);
        break;

    default:
        APPL_TRACE_ERROR1("uipc_cl_dg_ioctl bad request:%d", request);
        break;
    }
    return ret_code;
}


/*******************************************************************************
 **
 ** Function         uipc_cl_dg_read
 **
 ** Description      This function is used to read data from the socket
 **
 ** Returns          voidUINT32
 **
 *******************************************************************************/
UINT32 uipc_cl_dg_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 buflen)
{
    UINT32 nb_bytes;
    tUIPC_CL_DG_CHANNEL *p_channel;
    int channel_offset;

    if ((channel_id < UIPC_CH_ID_DG_FIRST) || (channel_id > UIPC_CH_ID_DG_LAST))
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_read wrong channelId:%d", channel_id);
        return FALSE;
    }
    channel_offset = channel_id - UIPC_CH_ID_DG_FIRST;

    /* Get reference on the Channel */
    p_channel = &uipc_cl_dg_cb.channels[channel_offset];

    if (p_channel->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_read channelId:%d was not open", channel_id);
        return FALSE;
    }

    nb_bytes = uipc_cl_socket_read(p_channel->socket_desc, p_buf, buflen);

#ifdef UIPC_CL_DG_DEBUG
    APPL_TRACE_DEBUG3("uipc_cl_dg_read ChId:%d asked:%d read:%d", channel_id, buflen, nb_bytes);
#endif

    return nb_bytes;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback
 **
 ** Description      This function Handle UIPC callback for DG
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_cl_dg_cback(UINT8 channel_offset, BT_HDR *p_msg)
{
    tUIPC_CH_ID channel_id;
    tUIPC_CL_DG_CHANNEL *p_channel;

#ifdef UIPC_CL_DG_DEBUG
    APPL_TRACE_DEBUG0("uipc_cl_dg_cback");
#endif

    /* Sanity check */
    if (channel_offset >= UIPC_CH_ID_DG_NB)
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_cback bad channel_offset:%d", channel_offset);
        GKI_freebuf(p_msg);
        return;
    }
    if (uipc_cl_dg_cb.channels[channel_offset].in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_dg_cback channel:%d not in use", channel_offset);
        GKI_freebuf(p_msg);
        return;
    }

    /* Get reference on server desc */
    p_channel = &uipc_cl_dg_cb.channels[channel_offset];

    /* Calculate Channel_Id from channel_offset */
    channel_id = channel_offset + UIPC_CH_ID_DG_FIRST;

    /* Update layer_specific field with Channel_id (num_client useless for DG) */
    p_msg->layer_specific = channel_id;

    /*  If this is a disconnection event */
    if (p_msg->event == UIPC_CLOSE_EVT)
    {
        p_channel->p_callback(p_msg);
        uipc_cl_dg_close(channel_id);
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
        /* The new DG implementation should not use this event */
        APPL_TRACE_ERROR1("uipc_cl_dg_cback UIPC_RX_DATA_EVT received channel_offset:%d", channel_offset);
        GKI_freebuf(p_msg);
        return;
    }
    else
    {
        APPL_TRACE_ERROR2("uipc_cl_dg_cback bad event:%d received from UIPC:%d",
                p_msg->event, channel_id);
        GKI_freebuf(p_msg);
    }
}

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback0
 **
 ** Description      This function Handle UIPC callback of first channel for DG
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_dg_cback0(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(0, p_msg);
}

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback1
 **
 ** Description      This function Handle UIPC callback of second channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 1)
void uipc_cl_dg_cback1(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(1, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback2
 **
 ** Description      This function Handle UIPC callback of first channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 2)
void uipc_cl_dg_cback2(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(2, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback3
 **
 ** Description      This function Handle UIPC callback of third channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 3)
void uipc_cl_dg_cback3(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(3, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback4
 **
 ** Description      This function Handle UIPC callback of fourth channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 4)
void uipc_cl_dg_cback4(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(4, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback5
 **
 ** Description      This function Handle UIPC callback of fifth channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 5)
void uipc_cl_dg_cback5(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(5, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback6
 **
 ** Description      This function Handle UIPC callback of sixth channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 6)
void uipc_cl_dg_cback6(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(6, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback7
 **
 ** Description      This function Handle UIPC callback of 7th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 7)
void uipc_cl_dg_cback7(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(7, p_msg);
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback8
 **
 ** Description      This function Handle UIPC callback of 8th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 8)
void uipc_cl_dg_cback8(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(8, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback9
 **
 ** Description      This function Handle UIPC callback of 9th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 9)
void uipc_cl_dg_cback9(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(9, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback10
 **
 ** Description      This function Handle UIPC callback of 10th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 10)
void uipc_cl_dg_cback10(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(10, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback11
 **
 ** Description      This function Handle UIPC callback of 11th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 11)
void uipc_cl_dg_cback11(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(11, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback12
 **
 ** Description      This function Handle UIPC callback of 12th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 12)
void uipc_cl_dg_cback12(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(12, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback13
 **
 ** Description      This function Handle UIPC callback of 13th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 13)
void uipc_cl_dg_cback13(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(13, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback14
 **
 ** Description      This function Handle UIPC callback of 14th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 14)
void uipc_cl_dg_cback14(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(14, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback15
 **
 ** Description      This function Handle UIPC callback of 15th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 15)
void uipc_cl_dg_cback15(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(15, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback16
 **
 ** Description      This function Handle UIPC callback of 16th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 16)
void uipc_cl_dg_cback16(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(16, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback17
 **
 ** Description      This function Handle UIPC callback of 17th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 17)
void uipc_cl_dg_cback17(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(17, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback18
 **
 ** Description      This function Handle UIPC callback of 18th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 18)
void uipc_cl_dg_cback18(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(18, p_msg);
}
#endif
/*******************************************************************************
 **
 ** Function         uipc_cl_dg_cback19
 **
 ** Description      This function Handle UIPC callback of 19th channel for DG
 **
 ** Returns          void
 **
 *******************************************************************************/
#if (UIPC_CH_ID_DG_NB > 19)
void uipc_cl_dg_cback19(BT_HDR *p_msg)
{
    uipc_cl_dg_cback(19, p_msg);
}
#endif


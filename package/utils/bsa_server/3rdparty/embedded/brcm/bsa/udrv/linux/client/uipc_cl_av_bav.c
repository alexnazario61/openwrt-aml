/*****************************************************************************
 **
 **  Name:           uipc_cl_av_bav.c
 **
 **  Description:    Client API for BSA UIPC AV Broadcast
 **
 **  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
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
#include "uipc_cl_av.h"
#include "uipc_rb.h"

#define BSA_AV_BAV_RB_NAME "rb_bav"

#ifndef BTA_BAV_STREAM_MAX
#define BTA_BAV_STREAM_MAX 2
#endif

typedef struct
{
    tUIPC_RB_DESC desc; /* RingBuffer/SharedMemory descriptor */
} tUIPC_CL_AV_BAV_RB_CB;

typedef struct
{
    tUIPC_CL_AV_BAV_RB_CB rb[BTA_BAV_STREAM_MAX];
} tUIPC_CL_AV_BAV_CB;


/*
 * Global variables
 */
static tUIPC_CL_AV_BAV_CB uipc_cl_av_bav_cb;

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_init
 **
 ** Description     Initialize the client AV Broadcast UIPC
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_init(void)
{
    int rb;
    /* init control block default value */
    memset(&uipc_cl_av_bav_cb, 0, sizeof(uipc_cl_av_bav_cb));
    for (rb = 0 ; rb < BTA_BAV_STREAM_MAX ; rb++)
    {
        uipc_cl_av_bav_cb.rb[rb].desc = UIPC_RB_BAD_DESC;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_open
 **
 ** Description     Open a client AV Broadcast UIPC channel
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_open(tUIPC_CH_ID channel_id)
{
    tUIPC_CL_AV_BAV_RB_CB *p_rb;

    if (channel_id == UIPC_CH_ID_BAV_1)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[0];
    }
    else if (channel_id == UIPC_CH_ID_BAV_2)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[1];
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_av_bav_open Bad ChannelId:%d", channel_id);
        return FALSE;
    }

    if (p_rb->desc != UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_cl_av_bav_open AV Bcst channel is already opened. Do nothing.");
        return TRUE;
    }

    /* Open a FIFO in TX/write direction */
    p_rb->desc = uipc_rb_open((UINT8*)BSA_AV_BAV_RB_NAME, channel_id,
            BSA_AV_UIPC_BUFFER_SIZE, UIPC_RB_MODE_WR);
    if (p_rb->desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_cl_av_bav_open fails to open Tx AV channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_cl_av_bav_open AV channel opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_close
 **
 ** Description     Open the client AV Broadcast UIPC channel
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_cl_av_bav_close(tUIPC_CH_ID channel_id)
{
    tUIPC_CL_AV_BAV_RB_CB *p_rb;

    if (channel_id == UIPC_CH_ID_BAV_1)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[0];
    }
    else if (channel_id == UIPC_CH_ID_BAV_2)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[1];
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_av_bav_close Bad ChannelId:%d", channel_id);
        return;
    }

    if (p_rb->desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_cl_av_bav_close AV Bcst channel is already closed. Do nothing.");
        return;
    }

    uipc_rb_close(p_rb->desc);
    p_rb->desc = UIPC_RB_BAD_DESC;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_send
 **
 ** Description     Send data to the server through the AV Broadcast UIPC channel
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen)
{
    INT32 rv;
    tUIPC_CL_AV_BAV_RB_CB *p_rb;

    if (channel_id == UIPC_CH_ID_BAV_1)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[0];
    }
    else if (channel_id == UIPC_CH_ID_BAV_2)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[1];
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_av_send Bad ChannelId:%d", channel_id);
        return FALSE;
    }

    if (p_rb->desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_cl_av_bav_close AV Bcst channel is already closed. Do nothing.");
        return FALSE;
    }

    rv = uipc_rb_write(p_rb->desc, p_buf, msglen);
    if (rv < 0)
    {
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_ioctl
 **
 ** Description     Control the client AV Broadcast UIPC channel
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param)
{
    BOOLEAN uipc_status = TRUE;
    tUIPC_CL_AV_BAV_RB_CB *p_rb;

    if (channel_id == UIPC_CH_ID_BAV_1)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[0];
    }
    else if (channel_id == UIPC_CH_ID_BAV_2)
    {
        p_rb = &uipc_cl_av_bav_cb.rb[1];
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_av_send Bad ChannelId:%d", channel_id);
        return FALSE;
    }

    if (p_rb->desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_cl_av_bav_close AV Bcst channel is already closed. Do nothing.");
        return FALSE;
    }

    /* Call the ioctl */
    uipc_status = uipc_rb_ioctl(p_rb->desc, request, param);

    return uipc_status;
}


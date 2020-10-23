/*****************************************************************************
 **
 **  Name:           uipc_sv_av_bav.c
 **
 **  Description:    Server API for BSA UIPC AV Broadcast
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
#include "uipc_sv_av_bav.h"
#include "uipc_fifo.h"
#include "uipc_rb.h"
#include "uipc_thread.h"
#include "bta_bav_api.h"

#define UIPC_DEBUG_SV_AV

#define UIPC_SV_AV_BAV_RB_NAME "rb_bav"

#define UIPC_AV_RX_WM 1000

typedef struct
{
    tUIPC_RB_DESC desc; /* shared mem int descriptor */
} tUIPC_SV_AV_BAV_RB_CB;

typedef struct
{
    tUIPC_SV_AV_BAV_RB_CB rb[BTA_BAV_STREAM_MAX];
} tUIPC_SV_AV_BAV_CB;


/*
 * Global variable
 */
static tUIPC_SV_AV_BAV_CB uipc_sv_av_bav_cb;

/* local prototypes */

/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_init
 **
 ** Description     Initialize the Server AV Broadcast UIPC
 **
 ** Parameters      None
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_bav_init(void)
{
    int rb;
    /* init control block default value */
    memset(&uipc_sv_av_bav_cb, 0, sizeof(uipc_sv_av_bav_cb));
    for (rb = 0 ; rb < BTA_BAV_STREAM_MAX ; rb++)
    {
        uipc_sv_av_bav_cb.rb[rb].desc = UIPC_RB_BAD_DESC;
    }
    return TRUE;
}


/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_open
 **
 ** Description     Open a server AV Broadcast UIPC channel
 **
 ** Parameters      channel_id: identifier of the channel to open
 **                 p_cback: call back to register, may be null.
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_bav_open(tUIPC_CH_ID channel_id)
{
    tUIPC_SV_AV_BAV_RB_CB *p_rb;

    if (channel_id == UIPC_CH_ID_BAV_1)
    {
        p_rb = &uipc_sv_av_bav_cb.rb[0];
    }
    else if (channel_id == UIPC_CH_ID_BAV_2)
    {
        p_rb = &uipc_sv_av_bav_cb.rb[1];
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_sv_av_bav_open Bad ChannelId:%d", channel_id);
        return FALSE;
    }

    if (p_rb->desc != UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_sv_av_bav_open AV Bcst channel is already opened. Do nothing.");
        return TRUE;
    }

    /* Open a RingBuffer FIFO in Rx/Read direction */
    p_rb->desc = uipc_rb_open((UINT8*)UIPC_SV_AV_BAV_RB_NAME, channel_id,
            BSA_AV_UIPC_BUFFER_SIZE, UIPC_RB_MODE_RD);
    if (p_rb->desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sv_av_bav_open fails to open Rx AV Bcst");
        return FALSE;
    }

    APPL_TRACE_DEBUG1("uipc_sv_av_bav_open AV channel:%d opened", channel_id);

    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_close
 **
 ** Description     Open the server AV UIPC channel
 **
 ** Parameters      None
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_sv_av_bav_close(tUIPC_CH_ID channel_id)
{
    tUIPC_SV_AV_BAV_RB_CB *p_rb;

    if (channel_id == UIPC_CH_ID_BAV_1)
    {
        p_rb = &uipc_sv_av_bav_cb.rb[0];
    }
    else if (channel_id == UIPC_CH_ID_BAV_2)
    {
        p_rb = &uipc_sv_av_bav_cb.rb[1];
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_sv_av_bav_close Bad ChannelId:%d", channel_id);
        return;
    }

    if (p_rb->desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_sv_av_bav_close AV Bcst channel is already closed. Do nothing.");
        return;
    }

    uipc_rb_close(p_rb->desc);
    p_rb->desc = UIPC_RB_BAD_DESC;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_read
 **
 ** Description     Read data from client through AV UIPC channel
 **
 ** Parameters      p_buf: pointer to the buffer to send
 **                 len: length of the buffer to send
 **
 ** Returns         length read
 **
 *******************************************************************************/
UINT32 uipc_sv_av_bav_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT32 len)
{
    INT32 rv;
    UINT32 length;
    tUIPC_SV_AV_BAV_RB_CB *p_rb;

    if (channel_id == UIPC_CH_ID_BAV_1)
    {
        p_rb = &uipc_sv_av_bav_cb.rb[0];
    }
    else if (channel_id == UIPC_CH_ID_BAV_2)
    {
        p_rb = &uipc_sv_av_bav_cb.rb[1];
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_sv_av_bav_read Bad ChannelId:%d", channel_id);
        return 0;
    }

    if (p_rb->desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_sv_av_bav_read AV Bcst channel is closed. Cannot read.");
        return 0;
    }

    rv = uipc_rb_read(p_rb->desc, p_buf, len);

    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_av_bav_read read fail ret:%d", rv);
        return 0;
    }
    else
    {
        /* This could cause an error if read size > 2GB, but wont be the case */
        length = (UINT32)rv;
        if (length != len)
        {
            APPL_TRACE_WARNING2("uipc_sv_av_bav_read read fail ret:%d expect:%d", length, len);
        }
    }
    return length;
}


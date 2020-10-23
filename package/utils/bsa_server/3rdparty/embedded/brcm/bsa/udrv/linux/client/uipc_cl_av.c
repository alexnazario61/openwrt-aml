/*****************************************************************************
 **
 **  Name:           uipc_cl_av.c
 **
 **  Description:    Client API for BSA UIPC AV
 **
 **  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
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

/* #define DEBUG_BSA_CL_AV */
#define BSA_AV_RB_NAME "rb_av"

typedef struct
{
    tUIPC_RB_DESC desc;
} tUIPC_CL_AV_CB;

/*
 * Global variables
 */
static tUIPC_CL_AV_CB uipc_cl_av_cb;

/*******************************************************************************
 **
 ** Function        uipc_cl_av_init
 **
 ** Description     Initialize the client AV UIPC
 **
 ** Parameters      None
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_init(void)
{
    uipc_cl_av_cb.desc = UIPC_RB_BAD_DESC;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_open
 **
 ** Description     Open a client AV UIPC channel
 **
 ** Parameters      channel_id: identifier of the channel to open
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_open(tUIPC_CH_ID channel_id)
{
    if (uipc_cl_av_cb.desc != UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_cl_av_open AV channel is already opened. Do nothing.");
        return TRUE;
    }

    /* Open a FIFO in TX/write direction */
    uipc_cl_av_cb.desc = uipc_rb_open((UINT8*)BSA_AV_RB_NAME, channel_id,
            BSA_AV_UIPC_BUFFER_SIZE, UIPC_RB_MODE_WR);
    if (uipc_cl_av_cb.desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_cl_av_open fails to open Tx AV channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_cl_av_open AV channel opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_close
 **
 ** Description     Open the client AV UIPC channel
 **
 ** Parameters      None
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_cl_av_close(void)
{
    uipc_rb_close(uipc_cl_av_cb.desc);
    uipc_cl_av_cb.desc = UIPC_RB_BAD_DESC;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_send
 **
 ** Description     Send data to the server through the AV UIPC channel
 **
 ** Parameters      p_buf: pointer to the buffer to send
 **                 msglen: length of the buffer to send
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_send(UINT8 *p_buf, UINT16 msglen)
{
    INT32 rv;

    rv = uipc_rb_write(uipc_cl_av_cb.desc, p_buf, msglen);

    if (rv == msglen)
    {
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_av_ioctl
 **
 ** Description     Control the client AV UIPC channel
 **
 ** Parameters      request:
 **                 param:
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_ioctl(UINT32 request, void *param)
{
    BOOLEAN uipc_status = TRUE;

    /* Call the ioctl */
    uipc_status = uipc_rb_ioctl(uipc_cl_av_cb.desc, request, param);

    return uipc_status;
}


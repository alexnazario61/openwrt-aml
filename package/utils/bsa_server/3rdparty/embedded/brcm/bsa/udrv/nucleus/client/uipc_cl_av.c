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

/* #define DEBUG_BSA_CL_AV */
#define BSA_AV_RB_NAME "rb_av"

/*typedef struct
{
    tUIPC_RB_DESC desc;
} tUIPC_CL_AV_CB;
*/

/*
 * Global variables
 */
/*static tUIPC_CL_AV_CB uipc_cl_av_cb;*/

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

    return uipc_status;
}


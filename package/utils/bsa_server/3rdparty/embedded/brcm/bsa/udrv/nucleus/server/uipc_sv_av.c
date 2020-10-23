/*****************************************************************************
 **
 **  Name:           uipc_sv_av.c
 **
 **  Description:    Server API for BSA UIPC AV
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
#include "uipc_sv_av.h"
#include "uipc_thread.h"

#define UIPC_DEBUG_SV_AV
#define UIPC_AV_RB_NAME "rb_av"

#define UIPC_AV_RX_WM 1000

typedef struct {
    UINT32 rx_wm; /* Receive water mark */
} tUIPC_SV_AV_CB;

/*
 * Global variable
 */
static tUIPC_SV_AV_CB uipc_sv_av_cb;

/* local prototypes */
static void uipc_sv_av_read_task(void *arg);
void uipc_sv_av_close(void);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_init
 **
 ** Description     Initialize the client AV UIPC
 **
 ** Parameters      None
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_init(void)
{
    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_av_reg_cback
 **
 ** Description     start a call back thread and store the call back function.
 **
 ** Parameters
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
static BOOLEAN uipc_sv_av_reg_cback(tUIPC_RCV_CBACK *p_cback)
{
    int status;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_av_open
 **
 ** Description     Open a server AV UIPC channel
 **
 ** Parameters      channel_id: identifier of the channel to open
 **                 p_cback: call back to register, may be null.
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_av_close
 **
 ** Description     Open the server AV UIPC channel
 **
 ** Parameters      None
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_sv_av_close(void)
{
}

/*******************************************************************************
 **
 ** Function        uipc_sv_av_ioctl
 **
 ** Description     Control the server AV UIPC channel
 **
 ** Parameters      request:
 **                 param:
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_ioctl(UINT32 request, void *param)
{
    BOOLEAN uipc_status = TRUE;

    return uipc_status;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_av_read
 **
 ** Description     Read data from client through AV UIPC channel
 **
 ** Parameters      p_buf: pointer to the buffer to send
 **                 len: length of the buffer to send
 **
 ** Returns         length read
 **
 *******************************************************************************/
UINT32 uipc_sv_av_read(UINT8 *p_buf, UINT32 len)
{
    return 1;
}

/*******************************************************************************
 **
 ** Function           uipc_hs_sco_in_read_fifo_task
 **
 ** Description        Thread in charge of continuously reading data
 **
 ** Output Parameter
 **
 ** Returns            nothing
 **
 *******************************************************************************/
static void uipc_sv_av_read_task(void *arg)
{
    return;
}


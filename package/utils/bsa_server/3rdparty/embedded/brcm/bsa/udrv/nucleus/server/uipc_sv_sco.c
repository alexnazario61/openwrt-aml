/*****************************************************************************
 **
 **  Name:           uipc_sv_sco.c
 **
 **  Description:    Server API for BSA UIPC for Audio gateway
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
#include "bsa_task.h"
#include "uipc_thread.h"
/*#include "uipc_rb.h"*/
#include "uipc_sv_sco.h"

#ifndef UIPC_SCO_TX_RB_NAME
#define UIPC_SCO_TX_RB_NAME "sco-tx-rb"
#endif

#ifndef UIPC_SCO_RX_RB_NAME
#define UIPC_SCO_RX_RB_NAME "sco-rx-rb"
#endif

#ifndef UIPC_SV_SCO_RX_WM
#define UIPC_SV_SCO_RX_WM            240       /* RX watermark for ring buffer configuration */
#endif

typedef struct
{
    tUIPC_RCV_CBACK *p_cback;   /* Rx Call back function */
    UINT32 rx_wm;               /* Receive water mark */
} tUIPC_SV_SCO_CB;

/*
 * Global variables
 */
tUIPC_SV_SCO_CB uipc_sv_sco_cb;

/*
 * Local static functions
 */
static void uipc_sv_sco_read_task(void *arg);
static BOOLEAN uipc_sv_sco_reg_cback(tUIPC_RCV_CBACK *p_cback);
static BOOLEAN uipc_sv_sco_channel_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_sv_sco_init
 **
 ** Description      UIPC SCO Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_sco_init(void)
{
    /* init control block default value */
    memset(&uipc_sv_sco_cb, 0, sizeof(uipc_sv_sco_cb));
    uipc_sv_sco_cb.rx_wm = UIPC_SV_SCO_RX_WM;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_rx_open
 **
 ** Description     Open server SCO RX UIPC channels.
 **                 This is ring buffer from server to application
 **
 ** Parameters
 **                 p_cback: call back to register, may be null.
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_sv_sco_open(tUIPC_RCV_CBACK *p_cback)
{
    BOOLEAN status = TRUE;

    if(uipc_sv_sco_channel_open(UIPC_CH_ID_SCO_TX,NULL) == TRUE)
    {
        if(uipc_sv_sco_channel_open(UIPC_CH_ID_SCO_RX,NULL) != TRUE)
        {
            /* Close UIPC TX path (RX for server point of view) */
            status = FALSE;
        }
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_channel_open
 **
 ** Description     Open server SCO UIPC channels.
 **                 This is ring buffer from application to server
 **
 ** Parameters      channel_id: identifier of the channel to open
 **                 p_cback: call back to register, may be null.
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
static BOOLEAN uipc_sv_sco_channel_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_close
 **
 ** Description     Close the server SCO UIPC channels
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_sv_sco_close(void)
{
    APPL_TRACE_DEBUG0("Close SCO RX channel");
}

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_reg_cback
 **
 ** Description     start a call back thread and store the call back function.
 **
 ** Parameters
 **         tUIPC_RCV_CBACK *p_cback: Rx callback
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
static BOOLEAN uipc_sv_sco_reg_cback(tUIPC_RCV_CBACK *p_cback)
{
    return TRUE;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_ioctl
 **
 ** Description     Control the server SCO UIPC channels
 **
 ** Parameters
 **             UINT32 request: IOCTL request
 **             void* param: request parameter
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_sco_ioctl(UINT32 request, void *param)
{
    BOOLEAN uipc_status = TRUE;
    return uipc_status;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_read
 **
 ** Description     Read data from client through SCO UIPC channel
 **
 ** Parameters
 **                 p_buf: pointer to the buffer to send to BTA
 **                 len: length of the buffer to send to BTA
 **
 ** Returns         length read
 **
 *******************************************************************************/
UINT32 uipc_sv_sco_read(UINT8 *p_buf, UINT32 len)
{
    return len;
}

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_send
 **
 ** Description     Send data to client through SCO UIPC channel
 **
 ** Parameters
 **                 p_buf: pointer to the buffer to send to application
 **                 len: length of the buffer to send to application
 **
 ** Returns         length read
 **
 *******************************************************************************/
BOOLEAN uipc_sv_sco_send(UINT8 *p_buf,UINT16 msglen)
{
    return TRUE;
}

/*******************************************************************************
**
** Function           uipc_sv_sco_read_task
**
** Description        Thread in charge of continuously reading data
**
** Output Parameter
**
** Returns            nothing
**
*******************************************************************************/
static void uipc_sv_sco_read_task(void *arg)
{
    APPL_TRACE_DEBUG0("uipc_sv_sco_read_task()");
}

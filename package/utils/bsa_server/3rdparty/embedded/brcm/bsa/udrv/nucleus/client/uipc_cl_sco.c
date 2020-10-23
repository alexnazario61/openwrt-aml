/*****************************************************************************
 **
 **  Name:           uipc_cl_sco.c
 **
 **  Description:    Client API for BSA UIPC Audio Gateway
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
#include "bsa_client.h"
//#include "uipc_rb.h"
#include "uipc_mutex.h"
#include "uipc_cl_sco.h"
#include "uipc_thread.h"


#ifndef UIPC_SCO_TX_RB_NAME
#define UIPC_SCO_TX_RB_NAME "sco-tx-rb"
#endif

#ifndef UIPC_SCO_RX_RB_NAME
#define UIPC_SCO_RX_RB_NAME "sco-rx-rb"
#endif

#ifndef UIPC_CL_SCO_RX_WM
#define UIPC_CL_SCO_RX_WM            240       /* RX watermark for ring buffer configuration */
#endif

typedef struct
{
    tUIPC_RCV_CBACK *p_cback;   /* Rx Call back function */
    NU_TASK thread;             /* receiving Thread when call back is used */
    UINT32 rx_wm;               /* Receive water mark */
} tUIPC_CL_SCO_CB;

/*
 * Global variables
 */
static tUIPC_CL_SCO_CB uipc_cl_sco_cb;

/*
 * Local functions
 */
//static void uipc_cl_sco_cback(BT_HDR *p_msg);
static void uipc_cl_sco_read_task(void *arg);
static BOOLEAN uipc_cl_sco_reg_cback(tUIPC_RCV_CBACK *p_cback);
static BOOLEAN uipc_cl_sco_channel_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);


/*******************************************************************************
 **
 ** Function         uipc_cl_sco_init
 **
 ** Description      UIPC Audio Gateway Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_sco_init(void)
{
    uipc_cl_sco_cb.p_cback = NULL;
    uipc_cl_sco_cb.rx_wm = UIPC_CL_SCO_RX_WM;

}

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_open
 **
 ** Description      Open Audio Gateway TX and RX ring buffers
 **
 ** Parameters
 **         tUIPC_RCV_CBACK *p_cback: RX callback to register
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_sco_open(tUIPC_RCV_CBACK *p_cback)
{
    BOOLEAN status = TRUE;

    if(uipc_cl_sco_channel_open(UIPC_CH_ID_SCO_TX,NULL) == TRUE)
    {
        if(uipc_cl_sco_channel_open(UIPC_CH_ID_SCO_RX,p_cback) != TRUE)
        {
            /* Close UIPC TX */
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
 ** Function         uipc_cl_sco_channel_open
 **
 ** Description      Open Audio Gateway Socket
 **
 ** Parameters
 **         tUIPC_CH_ID channel_id: client channel id
 **         tUIPC_RCV_CBACK *p_cback: RX callback to register
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN uipc_cl_sco_channel_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_close
 **
 ** Description      Disconnect from SCO server
 **
 ** Returns         void
 **
 *******************************************************************************/
void uipc_cl_sco_close(void)
{
    APPL_TRACE_DEBUG0("Close SCO TX channel");
}

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_send
 **
 ** Description      This function is used to write data to ring buffer
 **
 ** Parameters
 **         UINT8* p_buf: data pointer
 **         UINT16 msglen: messscoe length
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_sco_send(UINT8 *p_buf, UINT16 len)
{
     return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_read
 **
 ** Description      This function is used to read data from ring buffer
 **
 ** Parameters
 **         UINT8* p_buf: data pointer
 **         UINT16 len: number of byte to read
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
UINT32 uipc_cl_sco_read(UINT8 *p_buf, UINT16 len)
{

    APPL_TRACE_WARNING1("uipc_cl_sco_read len:%d", len);
    return len;
}

/*******************************************************************************
 **
 ** Function        uipc_cl_sco_ioctl
 **
 ** Description     Control the client SCO UIPC channel
 **
 ** Parameters
 **         UINT32 request: IOCTL request
 **         void* param: request parameter
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_sco_ioctl(UINT32 request, void *param)
{
    BOOLEAN uipc_status = TRUE;
    return uipc_status;
}

/*******************************************************************************
**
** Function           uipc_cl_sco_read_task
**
** Description        Thread in charge of continuously reading data
**
** Output Parameter
**
** Returns            nothing
**
*******************************************************************************/
static void uipc_cl_sco_read_task(void *arg)
{
    APPL_TRACE_DEBUG2("uipc_cl_sco_read_task p_cback 0x%x , arg 0x%x",
            uipc_cl_sco_cb.p_cback, arg);
    APPL_TRACE_DEBUG2("uipc_cl_sco_read_task exit, p_cback 0x%x , arg 0x%x",
            uipc_cl_sco_cb.p_cback, arg);

    uipc_thread_stop(uipc_cl_sco_cb.thread);
}

/*******************************************************************************
 **
 ** Function        uipc_cl_sco_reg_cback
 **
 ** Description     start a call back thread and store the call back function.
 **
 ** Parameters
 **         tUIPC_RCV_CBACK *p_cback: Rx callback
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
static BOOLEAN uipc_cl_sco_reg_cback(tUIPC_RCV_CBACK *p_cback)
{
    int status;
    /* store the call back function, might be null */
    if(NULL != (uipc_cl_sco_cb.p_cback = p_cback))
    {
        /* we have a call back, let's start a thread that will read the FIFO and call it */
        status = uipc_thread_create(uipc_cl_sco_read_task, (UINT8 *) UIPC_SCO_RX_RB_NAME,
                0, 0, &uipc_cl_sco_cb.thread, (void *) NULL);
        if (status < 0)
        {
            APPL_TRACE_ERROR1("ERROR uipc_cl_sco_reg_cback fails to create thread status %d",
                    status);
            uipc_cl_sco_cb.p_cback = NULL;
            uipc_cl_sco_close();
            return FALSE;
        }
    }

    return TRUE;
}


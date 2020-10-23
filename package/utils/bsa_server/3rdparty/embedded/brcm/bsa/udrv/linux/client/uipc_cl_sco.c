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
#include "uipc_rb.h"
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
    tUIPC_RB_DESC sco_tx_desc; /* To server */
    tUIPC_RB_DESC sco_rx_desc; /* From server*/
    tUIPC_RCV_CBACK *p_cback;   /* Rx Call back function */
    tTHREAD thread;             /* receiving Thread when call back is used */
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
    uipc_cl_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
    uipc_cl_sco_cb.sco_rx_desc = UIPC_RB_BAD_DESC;
    uipc_cl_sco_cb.p_cback = NULL;
    uipc_cl_sco_cb.thread = 0;
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
            uipc_rb_close(uipc_cl_sco_cb.sco_tx_desc);
            uipc_cl_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
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
    if(channel_id == UIPC_CH_ID_SCO_RX)
    {
        if (uipc_cl_sco_cb.sco_rx_desc != UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_DEBUG0("uipc_cl_sco_channel_open SCO channel is already opened. Do nothing.");
            return TRUE;
        }

        /* Open a FIFO in TX/write direction */
        uipc_cl_sco_cb.sco_rx_desc = uipc_rb_open((UINT8*)UIPC_SCO_RX_RB_NAME, UIPC_CH_ID_SCO_RX,
                BSA_SCO_RX_UIPC_BUFFER_SIZE, UIPC_RB_MODE_RD);

        if (uipc_cl_sco_cb.sco_rx_desc == UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_ERROR0("uipc_cl_sco_channel_open fails to open Rx SCO channel");

            if(uipc_cl_sco_cb.sco_tx_desc != UIPC_RB_BAD_DESC)
            {
                uipc_rb_close(uipc_cl_sco_cb.sco_tx_desc);
                uipc_cl_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
            }
            return FALSE;
        }

        APPL_TRACE_DEBUG0("uipc_cl_sco_channel_open SCO channels opened");
#ifdef UIPC_CL_SCO_CALLBACK
        return uipc_cl_sco_reg_cback(p_cback);
#else
        return TRUE;
#endif
    }
    else if(channel_id == UIPC_CH_ID_SCO_TX)
    {
        if (uipc_cl_sco_cb.sco_tx_desc != UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_DEBUG0("uipc_cl_sco_channel_open SCO channel is already opened. Do nothing.");
            return TRUE;
        }

        /* Open a FIFO in TX/write direction */
        uipc_cl_sco_cb.sco_tx_desc = uipc_rb_open((UINT8*)UIPC_SCO_TX_RB_NAME, UIPC_CH_ID_SCO_TX,
                BSA_SCO_TX_UIPC_BUFFER_SIZE, UIPC_RB_MODE_WR);

        if (uipc_cl_sco_cb.sco_tx_desc == UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_ERROR0("uipc_cl_sco_channel_open fails to open Tx SCO channel");

            if(uipc_cl_sco_cb.sco_rx_desc != UIPC_RB_BAD_DESC)
            {
                uipc_rb_close(uipc_cl_sco_cb.sco_rx_desc);
                uipc_cl_sco_cb.sco_rx_desc = UIPC_RB_BAD_DESC;
            }
            return FALSE;
        }

        APPL_TRACE_DEBUG0("uipc_cl_sco_channel_open SCO channels opened");

        /* Set the write mode in blocking mode */
        if(uipc_rb_ioctl(uipc_cl_sco_cb.sco_tx_desc,UIPC_WRITE_BLOCK,NULL) != TRUE)
        {
            APPL_TRACE_ERROR0("uipc_cl_sco_channel_open can not set ring buffer in  write blocking mode");
            uipc_rb_close(uipc_cl_sco_cb.sco_tx_desc);
            uipc_cl_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        APPL_TRACE_ERROR0("uipc_cl_sco_channel_open wrong channel id");
        return FALSE;
    }
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
    if(uipc_cl_sco_cb.sco_tx_desc != UIPC_RB_BAD_DESC)
    {

        APPL_TRACE_DEBUG0("Close SCO TX channel");
        uipc_rb_close(uipc_cl_sco_cb.sco_tx_desc);
        uipc_cl_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
    }

    if(uipc_cl_sco_cb.sco_rx_desc != UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("Close SCO RX channel");
        uipc_rb_close(uipc_cl_sco_cb.sco_rx_desc);
        uipc_cl_sco_cb.sco_rx_desc = UIPC_RB_BAD_DESC;
    }

    if(uipc_cl_sco_cb.p_cback != NULL)
    {
        uipc_cl_sco_cb.p_cback = NULL;
    }

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
    INT32 rv;

    rv = uipc_rb_write(uipc_cl_sco_cb.sco_tx_desc, p_buf, len);

    if (rv < 0)
        return FALSE;
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
    INT32 rv;

    rv = uipc_rb_read(uipc_cl_sco_cb.sco_rx_desc, p_buf, len);

    if (rv < 0)
    {
        APPL_TRACE_ERROR0("uipc_cl_sco_read: uipc_rb_read returned an error");
        return 0;
    }

    if(rv != len)
    {
        APPL_TRACE_WARNING2("uipc_cl_sco_read read fail read:%d expect:%d", rv, len);
    }

    return rv;
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

    switch(request)
    {
    case UIPC_REG_CBACK :
        /* if a call back was already register let's stop the current thread */
        APPL_TRACE_DEBUG1("uipc_cl_sco_ioctl UIPC_REG_CBACK %d", uipc_cl_sco_cb.p_cback);
        if(uipc_cl_sco_cb.p_cback != NULL)
        {
            /* the uipc_sv_av_read_task thread is running with a GKI buffer allocated */
            /* just release it */
            uipc_cl_sco_cb.p_cback = NULL;
            /* release the select to exit the read thread */
            uipc_rb_ioctl(uipc_cl_sco_cb.sco_rx_desc, request, (void *) (UINTPTR)uipc_cl_sco_cb.rx_wm);

        }
        /* restart a new one */
        uipc_cl_sco_reg_cback((tUIPC_RCV_CBACK *) param);
        break;

    case UIPC_SET_RX_WM :
        uipc_cl_sco_cb.rx_wm = (UINTPTR) param;
        APPL_TRACE_DEBUG1("uipc_cl_sco_ioctl UIPC_SET_RX_WM %d", uipc_cl_sco_cb.rx_wm);
        break;

    case UIPC_REQ_RX_FLUSH :
#ifdef UIPC_DEBUG_SV_SCO
        APPL_TRACE_DEBUG0("uipc_sv_sco_ioctl Rx Flush");
#endif
        uipc_rb_ioctl(uipc_cl_sco_cb.sco_rx_desc, request, NULL);
        break;

    default :
        /* Call the ioctl */
        uipc_status = uipc_rb_ioctl(uipc_cl_sco_cb.sco_tx_desc, request, param);
        break;
    }

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
    INT32 len;
    BT_HDR *p_buf;
    UINT8 *p_dest;

    APPL_TRACE_DEBUG2("uipc_cl_sco_read_task p_cback 0x%x , arg 0x%x",
            uipc_cl_sco_cb.p_cback, arg);

    /* Main server loop - until application disconnects from server or cback change */
    while ((uipc_cl_sco_cb.p_cback == arg) && (uipc_cl_sco_cb.p_cback != NULL))
    {
        /*APPL_TRACE_DEBUG0("uipc_sv_av_read_task waiting for read to complete");*/
        len = uipc_rb_select(uipc_cl_sco_cb.sco_rx_desc, uipc_cl_sco_cb.rx_wm);
#ifdef UIPC_DEBUG_SV_SCO
        APPL_TRACE_DEBUG2("uipc_cl_sco_read_task len %d, rx_wm %d",len, uipc_sv_sco_cb.rx_wm);
#endif

        if(len > 0)
        {
            if((p_buf = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR)+len)) != NULL)
            {
                p_buf->event = UIPC_RX_DATA_EVT;
                p_buf->len = len;
                p_dest = (UINT8*) (p_buf+1);

                /* Read the data and call application callback */
                uipc_cl_sco_read(p_dest,len);

                /* Call UIPC read call back */
                ((tUIPC_RCV_CBACK *)arg)((BT_HDR *)p_buf);
            }
            else
            {
                APPL_TRACE_ERROR0("ERROR uipc_cl_sco_read_task no more GKI buffer");
                /* this should never happen. let's exit */
                uipc_cl_sco_cb.p_cback = NULL;
            }

       }
    }

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
                0, 0, &uipc_cl_sco_cb.thread, p_cback);
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


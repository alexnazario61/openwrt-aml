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
#include "uipc_rb.h"
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
    tUIPC_RB_DESC sco_tx_desc;  /* shared mem int descriptor from client to server */
    tUIPC_RB_DESC sco_rx_desc;  /* shared mem int descriptor from server to client */
    tUIPC_RCV_CBACK *p_cback;   /* Rx Call back function */
    tTHREAD thread;             /* receiving Thread when call back is used */
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
    uipc_sv_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
    uipc_sv_sco_cb.sco_rx_desc = UIPC_RB_BAD_DESC;
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
            if(uipc_sv_sco_cb.sco_rx_desc != UIPC_RB_BAD_DESC)
            {
                uipc_rb_close(uipc_sv_sco_cb.sco_rx_desc);
                uipc_sv_sco_cb.sco_rx_desc = UIPC_RB_BAD_DESC;
            }
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
    if(channel_id == UIPC_CH_ID_SCO_TX)
    {
        if (uipc_sv_sco_cb.sco_tx_desc != UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_DEBUG0("uipc_sv_sco_channel_open SCO channel already opened. Do nothing.");
            return TRUE;
        }

        APPL_TRACE_DEBUG0("OPEN SCO TX channel in READ mode");
        /* Open a FIFO in Rx/Read direction */
        uipc_sv_sco_cb.sco_tx_desc = uipc_rb_open((UINT8*)UIPC_SCO_TX_RB_NAME, UIPC_CH_ID_SCO_TX,
            BSA_SCO_TX_UIPC_BUFFER_SIZE, UIPC_RB_MODE_RD);

        if (uipc_sv_sco_cb.sco_tx_desc == UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_ERROR0("uipc_sv_sco_channel_open fails to open SCO Tx channel");

            if (uipc_sv_sco_cb.sco_rx_desc != UIPC_RB_BAD_DESC)
            {
                APPL_TRACE_DEBUG0("Close SCO RX channel");
                uipc_rb_close(uipc_sv_sco_cb.sco_rx_desc);
                uipc_sv_sco_cb.sco_rx_desc = UIPC_RB_BAD_DESC;
            }
            return FALSE;
        }

        APPL_TRACE_DEBUG0("uipc_sv_sco_channel_open SCO channels opened");

        return TRUE;
    }
    else if(channel_id == UIPC_CH_ID_SCO_RX)
    {
        if (uipc_sv_sco_cb.sco_rx_desc != UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_DEBUG0("uipc_sv_sco_channel_open SCO channel already opened. Do nothing.");
            return TRUE;
        }

        APPL_TRACE_DEBUG0("OPEN SCO RX channel in WRITE mode");
        /* Open a FIFO in TX/write direction */
        uipc_sv_sco_cb.sco_rx_desc = uipc_rb_open((UINT8*)UIPC_SCO_RX_RB_NAME, UIPC_CH_ID_SCO_RX,
                BSA_SCO_RX_UIPC_BUFFER_SIZE, UIPC_RB_MODE_WR);

        if (uipc_sv_sco_cb.sco_rx_desc == UIPC_RB_BAD_DESC)
        {
            APPL_TRACE_ERROR0("uipc_sv_sco_channel_open fails to open SCO Rx channel");
            if (uipc_sv_sco_cb.sco_tx_desc != UIPC_RB_BAD_DESC)
            {
                APPL_TRACE_DEBUG0("Close SCO TX channel");
                uipc_rb_close(uipc_sv_sco_cb.sco_tx_desc);
                uipc_sv_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
            }
            return FALSE;
        }

        APPL_TRACE_DEBUG0("uipc_sv_sco_channel_open SCO channels opened");

#ifdef UIPC_SV_SCO_CALLBACK
        return uipc_sv_sco_reg_cback(p_cback);
#else
        return TRUE;
#endif
    }
    else
    {
        APPL_TRACE_ERROR0("uipc_sv_sco_channel_open wrong channel id");
        return FALSE;
    }

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
    if(uipc_sv_sco_cb.sco_tx_desc != UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("Close SCO TX channel");
        uipc_rb_close(uipc_sv_sco_cb.sco_tx_desc);
        uipc_sv_sco_cb.sco_tx_desc = UIPC_RB_BAD_DESC;
    }

    if(uipc_sv_sco_cb.sco_rx_desc != UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("Close SCO RX channel");
        uipc_rb_close(uipc_sv_sco_cb.sco_rx_desc);
        uipc_sv_sco_cb.sco_rx_desc = UIPC_RB_BAD_DESC;
    }

    if(uipc_sv_sco_cb.p_cback != NULL)
    {
        uipc_sv_sco_cb.p_cback = NULL;
    }
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
    int status;
    /* store the call back function, might be null */
    if(NULL != (uipc_sv_sco_cb.p_cback = p_cback))
    {
        /* we have a call back, let's start a thread that will read the FIFO and call it */
        status = uipc_thread_create(uipc_sv_sco_read_task, (UINT8 *) UIPC_SCO_TX_RB_NAME,
                0, 0, &uipc_sv_sco_cb.thread, p_cback);
        if (status < 0)
        {
            APPL_TRACE_ERROR1("ERROR uipc_sv_sco_reg_cback fails to create thread status %d", status);
            uipc_sv_sco_cb.p_cback = NULL;
            uipc_sv_sco_close();
            return FALSE;
        }
    }

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

    switch(request)
    {
    case UIPC_REG_CBACK :
        /* if a call back was already register let's stop the current thread */
        APPL_TRACE_DEBUG1("uipc_sv_sco_ioctl UIPC_REG_CBACK %d", uipc_sv_sco_cb.p_cback);
        if(uipc_sv_sco_cb.p_cback != NULL)
        {
            /* the uipc_sv_av_read_task thread is running with a GKI buffer allocated */
            /* just release it */
            uipc_sv_sco_cb.p_cback = NULL;
            /* release the select to exit the read thread */
            uipc_rb_ioctl(uipc_sv_sco_cb.sco_tx_desc, request, (void *)(UINTPTR)uipc_sv_sco_cb.rx_wm);

        }
        /* restart a new one */
        uipc_sv_sco_reg_cback((tUIPC_RCV_CBACK *) param);
        break;

    case UIPC_SET_RX_WM :
        uipc_sv_sco_cb.rx_wm = (UINTPTR) param;
        APPL_TRACE_DEBUG1("uipc_sv_sco_ioctl UIPC_SET_RX_WM %d", uipc_sv_sco_cb.rx_wm);
        break;

    case UIPC_REQ_RX_FLUSH :
#ifdef UIPC_DEBUG_SV_SCO
        APPL_TRACE_DEBUG0("uipc_sv_sco_ioctl Rx Flush");
#endif
        uipc_rb_ioctl(uipc_sv_sco_cb.sco_tx_desc, request, NULL);
        break;

    default :
        /* Call the ioctl */
        uipc_status = uipc_rb_ioctl(uipc_sv_sco_cb.sco_rx_desc, request, param);
        break;
    }

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
    INT32 rv;
    UINT32 length;
    APPL_TRACE_DEBUG1("uipc_sv_sco_read - desc %d",uipc_sv_sco_cb.sco_tx_desc);

    rv = uipc_rb_read(uipc_sv_sco_cb.sco_tx_desc, p_buf, len);

    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_sco_read read fail ret:%d", rv);
        return 0;
    }
    else
    {
        /* This could cause an error if read size > 2GB, but wont be the case */
        length = (UINT32)rv;
        if (length != len)
        {
            APPL_TRACE_WARNING2("uipc_sv_sco_read read fail ret:%d expect:%d", length, len);
        }
    }

    return length;
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
    INT32 rv;

    APPL_TRACE_DEBUG1("uipc_sv_sco_send - desc %d",uipc_sv_sco_cb.sco_rx_desc);

    rv = uipc_rb_write(uipc_sv_sco_cb.sco_rx_desc, p_buf, msglen);

    if (rv < 0)
        return FALSE;
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
    INT32 len;
    BT_HDR *p_buf;

    APPL_TRACE_DEBUG2("uipc_sv_sco_read_task p_cback 0x%x , arg 0x%x",
            uipc_sv_sco_cb.p_cback, arg);

    /* Main server loop - until application disconnects from server or cback change */
    while ((uipc_sv_sco_cb.p_cback == arg) && (uipc_sv_sco_cb.p_cback != NULL))
    {
        /*APPL_TRACE_DEBUG0("uipc_sv_av_read_task waiting for read to complete");*/
        len = uipc_rb_select(uipc_sv_sco_cb.sco_tx_desc, uipc_sv_sco_cb.rx_wm);
#ifdef UIPC_DEBUG_SV_SCO
        APPL_TRACE_DEBUG2("uipc_sv_sco_read_task len %d, rx_wm %d",len, uipc_sv_sco_cb.rx_wm);
#endif

        if(len)
        {
            if((p_buf = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR))) != NULL)
            {
                p_buf->event = UIPC_RX_DATA_EVT;
                p_buf->len = len;

                /* Call UIPC read call back */
                ((tUIPC_RCV_CBACK *)arg)((BT_HDR *)p_buf);
            }
            else
            {
                APPL_TRACE_ERROR0("ERROR uipc_sv_sco_read_task no more GKI buffer");
                /* this should never happen. let's exit */
                uipc_sv_sco_cb.p_cback = NULL;
            }

       }
    }

    APPL_TRACE_DEBUG2("uipc_sv_sco_read_task exit, p_cback 0x%x , arg 0x%x",
            uipc_sv_sco_cb.p_cback, arg);

    uipc_thread_stop(uipc_sv_sco_cb.thread);
}

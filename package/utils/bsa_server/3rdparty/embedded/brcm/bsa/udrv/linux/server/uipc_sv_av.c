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
#include "uipc_fifo.h"
#include "uipc_rb.h"
#include "uipc_thread.h"

#define UIPC_DEBUG_SV_AV
#define UIPC_AV_RB_NAME "rb_av"

#define UIPC_AV_RX_WM 1000

typedef struct
{
    tUIPC_RB_DESC desc; /* shared mem int descriptor */
    tUIPC_RCV_CBACK *p_cback; /* Rx Call back function */
    tTHREAD thread; /* receiving Thread when call back is used */
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
    /* init control block default value */
    memset(&uipc_sv_av_cb, 0, sizeof(uipc_sv_av_cb));
    uipc_sv_av_cb.desc = UIPC_RB_BAD_DESC;
    uipc_sv_av_cb.rx_wm = UIPC_AV_RX_WM;

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
    tUIPC_RCV_CBACK *p_oldcback = uipc_sv_av_cb.p_cback;

    APPL_TRACE_DEBUG2("uipc_sv_av_reg_cback(%p) previous=%p", p_cback, uipc_sv_av_cb.p_cback);

    /* Store the call back function, might be null */
    uipc_sv_av_cb.p_cback = p_cback;

    /* If there is no callback anymore, but there was one active, end it */
    if ((NULL == p_cback) && (NULL != p_oldcback))
    {
        /* Release the select in the current read thread */
        uipc_rb_ioctl(uipc_sv_av_cb.desc, UIPC_REG_CBACK, (void *)BSA_AV_UIPC_BUFFER_SIZE);
    }
    /* If there is a callback and no previous one, start it */
    else if ((NULL != p_cback) && (NULL == p_oldcback))
    {
        /* we have a call back, let's start a thread that will read the FIFO and call it */
        status = uipc_thread_create(uipc_sv_av_read_task, (UINT8 *) UIPC_AV_RB_NAME,
                0, 0, &uipc_sv_av_cb.thread, NULL);
        if (status < 0)
        {
            APPL_TRACE_ERROR1("ERROR uipc_sv_av_open fails to create thread status %d", status);
            uipc_sv_av_close();
            return FALSE;
        }
    }

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
    if (uipc_sv_av_cb.desc != UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_sv_av_open AV channel is already opened. Do nothing.");
        return TRUE;
    }

    /* Open a FIFO in Rx/Read direction */
    uipc_sv_av_cb.desc = uipc_rb_open((UINT8*)UIPC_AV_RB_NAME, channel_id,
            BSA_AV_UIPC_BUFFER_SIZE, UIPC_RB_MODE_RD);
    if (uipc_sv_av_cb.desc == UIPC_RB_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sv_av_open fails to open Rx AV channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_sv_av_open AV channel opened");

    return uipc_sv_av_reg_cback(p_cback);
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
    uipc_rb_close(uipc_sv_av_cb.desc);
    uipc_sv_av_cb.desc = UIPC_RB_BAD_DESC;

    if (NULL != uipc_sv_av_cb.p_cback)
    {
        uipc_sv_av_cb.p_cback = NULL;
    }
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

    switch(request)
    {
    case UIPC_REG_CBACK:
        APPL_TRACE_DEBUG2("uipc_sv_av_ioctl UIPC_REG_CBACK(%p) previous=%p", param, uipc_sv_av_cb.p_cback);
        /* Stop and restart a new one if needed */
        uipc_sv_av_reg_cback((tUIPC_RCV_CBACK *)param);
        break;

    case UIPC_SET_RX_WM:
        /* In case the watermark is 0, do not update because semop with sem_op = 0 can have unexpected behavior */
        if ((0 != (UINTPTR)param) &&
            (uipc_sv_av_cb.rx_wm != (UINTPTR)param))
        {
            uipc_sv_av_cb.rx_wm = (UINTPTR)param;

            /* Release the select in the current read thread */
            uipc_rb_ioctl(uipc_sv_av_cb.desc, UIPC_REG_CBACK, (void *)BSA_AV_UIPC_BUFFER_SIZE);
        }
        APPL_TRACE_DEBUG1("uipc_sv_av_ioctl UIPC_SET_RX_WM %d", uipc_sv_av_cb.rx_wm);
        break;

    case UIPC_REQ_RX_FLUSH:
#ifdef UIPC_DEBUG_SV_AV
        APPL_TRACE_DEBUG0("uipc_sv_av_ioctl Rx Flush");
#endif
        /* falls through */

    default :
        /* Call the ioctl */
        uipc_status = uipc_rb_ioctl(uipc_sv_av_cb.desc, request, param);
        break;
    }

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
    INT32 rv;
    UINT32 length;

    rv = uipc_rb_read(uipc_sv_av_cb.desc, p_buf, len);

    if (rv < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_av_read read fail ret:%d", rv);
        return 0;
    }
    else
    {
        /* This could cause an error if read size > 2GB, but wont be the case */
        length = (UINT32)rv;
        if (length != len)
        {
            APPL_TRACE_WARNING2("uipc_sv_av_read read fail ret:%d expect:%d", length, len);
        }
    }

    return length;
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
    INT32 len;
    BT_HDR *p_buf;
    tUIPC_RCV_CBACK *p_cback;

    APPL_TRACE_DEBUG1("uipc_sv_av_read_task p_cback %p", uipc_sv_av_cb.p_cback);

    /* Main server loop - until application disconnects from server or cback change */
    while (NULL != uipc_sv_av_cb.p_cback)
    {
        /*APPL_TRACE_DEBUG0("uipc_sv_av_read_task waiting for read to complete");*/
        len = uipc_rb_select(uipc_sv_av_cb.desc, uipc_sv_av_cb.rx_wm);
#ifdef UIPC_DEBUG_SV_AV
        APPL_TRACE_DEBUG2("uipc_sv_av_read_task len %d, rx_wm %d",
                len, uipc_sv_av_cb.rx_wm);
#endif

        /* Make sure there is data to get */
        if (len > 0)
        {
            p_buf = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR));
            if (NULL != p_buf)
            {
                p_buf->event = UIPC_RX_DATA_EVT;
                p_buf->len = len;

                /* Call UIPC read call back */
                p_cback = uipc_sv_av_cb.p_cback;
                if (p_cback != NULL)
                {
                    p_cback((BT_HDR *)p_buf);
                }
            }
            else
            {
                APPL_TRACE_ERROR0("ERROR uipc_sv_av_read_task no more GKI buffer");
                /* this should never happen. let's exit */
                uipc_sv_av_cb.p_cback = NULL;
            }
        }
        else if (len < 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_sv_av_read_task select failed");
            /* this should never happen. let's exit */
            uipc_sv_av_cb.p_cback = NULL;
        }
    }

    APPL_TRACE_DEBUG1("uipc_sv_av_read_task: end p_cback %p",
            uipc_sv_av_cb.p_cback);

    uipc_thread_stop(uipc_sv_av_cb.thread);
}


/*****************************************************************************
 **
 **  Name:           uipc_cl_hh.c
 **
 **  Description:    Server API for BSA UIPC HH
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
#include "uipc_cl_hh.h"
#include "uipc_fifo.h"

#define BSA_HH_FIFO_NAME "./bt-hh-fifo"

#define HH_MIN(a,b) (((a) < (b))? (a): (b))

#ifndef UIPC_CL_RX_HH_MODE
/* Read direction, Data Event */
#define UIPC_CL_RX_HH_MODE (UIPC_FIFO_MODE_READ | UIPC_FIFO_MODE_RX_DATA_EVT)
#endif

typedef struct
{
    tUIPC_FIFO_DESC fifo_desc;
    tUIPC_RCV_CBACK *p_cback;
    BT_HDR *p_hh_report;
    UINT16 rx_len;
    UINT16 data_len;
} tUIPC_CL_HH_CB;


/*
 * Global variable
 */
static tUIPC_CL_HH_CB uipc_cl_hh_cb;

/*
 * Local static function to UIPC callback
 */

/*
 * Local functions
 */
static void uipc_cl_hh_cback(BT_HDR *p_msg);


/*******************************************************************************
 **
 ** Function         uipc_cl_hh_init
 **
 ** Description      Initialize the HH UIPC channel.
 **
 ** Parameters       None.
 **
 ** Returns          TRUE if successful, FALSE otherwise.
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_init(void)
{
    uipc_cl_hh_cb.fifo_desc = UIPC_FIFO_BAD_DESC;
    uipc_cl_hh_cb.p_cback = NULL;
    uipc_cl_hh_cb.rx_len = 0;
    uipc_cl_hh_cb.p_hh_report = NULL;
    uipc_cl_hh_cb.data_len = 0;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_open
 **
 ** Description      Open an HH UIPC channel.
 **
 ** Parameters
 **
 ** Returns          TRUE if successful, FALSE otherwise.
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_hh_open NULL Callback");
        return FALSE;
    }

    if (uipc_cl_hh_cb.fifo_desc != UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_cl_hh_open hh channel is already opened. Do nothing.");
        return TRUE;
    }

    /* Open a FIFO in TX/write direction */
    /* Note that we use our own cback (which will call the app's callback) */
    uipc_cl_hh_cb.fifo_desc = uipc_fifo_open((UINT8*)BSA_HH_FIFO_NAME, channel_id,
                                   uipc_cl_hh_cback, UIPC_CL_RX_HH_MODE);
    if (uipc_cl_hh_cb.fifo_desc == UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_cl_hh_open fails to open Rx hh channel");
        return FALSE;
    }
    /* Save application's callback */
    uipc_cl_hh_cb.p_cback = p_cback;

    APPL_TRACE_DEBUG0("uipc_cl_hh_open hh channel opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_close
 **
 ** Description      Close an HH UIPC channel.
 **
 ** Parameters
 **
 ** Returns          None
 **
 *******************************************************************************/
void uipc_cl_hh_close(tUIPC_CH_ID channel_id)
{
    if (uipc_cl_hh_cb.fifo_desc != UIPC_FIFO_BAD_DESC)
    {
        uipc_fifo_close(uipc_cl_hh_cb.fifo_desc, FALSE);
        uipc_cl_hh_cb.fifo_desc = UIPC_FIFO_BAD_DESC;
    }
}

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_cback
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_cl_hh_cback(BT_HDR *p_msg)
{
    UINT32 rx_len;
    UINT32 copy_len;
    UINT32 missing_len;
    UINT8 *p_src = (UINT8 *)(p_msg + 1);
    UINT8 *p_dst;

#ifdef DEBUG_BSA_CL_HH
    APPL_TRACE_DEBUG1("uipc_cl_hh_cback enter len:%d", p_msg->len);
#endif
    /* If it's a RX data indication, call the callback */
    if (p_msg->event == UIPC_FIFO_DATA_RX_EVT)
    {
        rx_len = p_msg->len;

        while (rx_len)
        {
            /* If the report length has not yet been received, do it */
            if (uipc_cl_hh_cb.rx_len < sizeof(uipc_cl_hh_cb.data_len))
            {
                /* Receive report length byte per byte directly inside data_len */
                p_dst = (UINT8 *)&uipc_cl_hh_cb.data_len;
                p_dst = &p_dst[uipc_cl_hh_cb.rx_len];
                *p_dst = *p_src++;
                uipc_cl_hh_cb.rx_len++;
                rx_len--;

                /* Continue the main loop: to handle the following bytes if any */
                continue;
            }

            /*
             * At this step if the following code is executed the data_len has been
             * extracted and control block can be allocated
             */
#ifdef DEBUG_BSA_CL_HH
            APPL_TRACE_DEBUG1("uipc_cl_hh_cback loop len:%d", rx_len);
#endif
            if (uipc_cl_hh_cb.p_hh_report == NULL)
            {
                if ((uipc_cl_hh_cb.p_hh_report =
                             (BT_HDR *)GKI_getbuf(sizeof(tBSA_HH_UIPC_REPORT))) != NULL)
                {
#ifdef DEBUG_BSA_CL_HH
                    APPL_TRACE_DEBUG0("uipc_cl_hh_cback alloc GKI buffer");
                    APPL_TRACE_DEBUG1("BSA report length = %d",uipc_cl_hh_cb.data_len);
#endif
                    /* Copy the size of report in uipc_cl_hh_cb.p_hh_report->len */
                    uipc_cl_hh_cb.p_hh_report->len = uipc_cl_hh_cb.data_len;

                    uipc_cl_hh_cb.p_hh_report->layer_specific = 0;
                    uipc_cl_hh_cb.p_hh_report->event = UIPC_FIFO_DATA_RX_EVT;
                    uipc_cl_hh_cb.p_hh_report->offset = 0;

                    /* Copy data_len that has already been extracted from fifo */
                    p_dst = (UINT8 *)(uipc_cl_hh_cb.p_hh_report + 1);
                    ((tBSA_HH_REPORT *)p_dst)->report_len = uipc_cl_hh_cb.data_len;
                }
                else
                {
                    APPL_TRACE_ERROR0("ERROR uipc_cl_hh_cback no more buffer");
                    GKI_freebuf(p_msg);
                    return;
                }
            }

            /* Compute the number of bytes needed to complete the report */
            missing_len = uipc_cl_hh_cb.p_hh_report->len - uipc_cl_hh_cb.rx_len;

            /* compute the number of bytes to copy */
            copy_len = HH_MIN(missing_len, rx_len);

#ifdef DEBUG_BSA_CL_HH
            APPL_TRACE_DEBUG2("uipc_cl_hh_cback missing:%d cp_len:%d", missing_len, copy_len);
#endif

            /* compute dest addr */
            p_dst = (UINT8 *)(uipc_cl_hh_cb.p_hh_report + 1 ) + uipc_cl_hh_cb.rx_len;

#ifdef DEBUG_BSA_CL_HH
            /*
             * Sanity check (check memcpy parameters)
             */
            /* Check missing length */
            if (uipc_cl_hh_cb.p_hh_report->len < uipc_cl_hh_cb.rx_len)
            {
                APPL_TRACE_ERROR3(
                        "ERROR uipc_cl_hh_cback bad missing len:%d rx_len:%d missing_len:%d",
                        uipc_cl_hh_cb.p_hh_report->len, uipc_cl_hh_cb.rx_len, missing_len);
            }
            /* Check Destination overwrite */
            if ((p_dst + copy_len) > ((UINT8 *)(uipc_cl_hh_cb.p_hh_report) + sizeof(tBSA_HH_UIPC_REPORT)))
            {
                APPL_TRACE_ERROR1("ERROR uipc_cl_hh_cback dest overwrite copy_len:%d", copy_len);
            }
            /* Check source overwrite */
            if ((p_src + copy_len) > ((UINT8 *)(p_msg + 1) + p_msg->len))
            {
                APPL_TRACE_ERROR1("ERROR uipc_cl_hh_cback src overwrite copy_len:%d", copy_len);
            }
#endif
            /* Copy Data */
            memcpy(p_dst, p_src, copy_len);

            p_src += copy_len;              /* update source ptr */
            rx_len -= copy_len;                 /* update rx len (source) */
            uipc_cl_hh_cb.rx_len += copy_len;   /* update rx len (dest) */

            /* If report fully received */
            if (uipc_cl_hh_cb.data_len == uipc_cl_hh_cb.rx_len)
            {
#ifdef DEBUG_BSA_CL_HH
                APPL_TRACE_DEBUG0("uipc_cl_hh_cback call callback");
#endif
                if (uipc_cl_hh_cb.p_cback)
                {
                    uipc_cl_hh_cb.p_cback(uipc_cl_hh_cb.p_hh_report);
                }
                else
                {
                    APPL_TRACE_ERROR0("ERROR uipc_cl_hh_cback no callback");
                    GKI_freebuf(uipc_cl_hh_cb.p_hh_report);
                }
                uipc_cl_hh_cb.p_hh_report = NULL;
                uipc_cl_hh_cb.rx_len = 0;
                uipc_cl_hh_cb.data_len = 0;
            }
#ifdef DEBUG_BSA_CL_HH
            APPL_TRACE_DEBUG1("uipc_cl_hh_cback end loop remaining len:%d", rx_len);
#endif
        }
    }
    /*  If this is an open */
    else if (p_msg->event == UIPC_FIFO_OPEN_EVT)
    {
        APPL_TRACE_DEBUG0("uipc_cl_hh_cback fifo opened event");
    }
    /*  If this is a close */
    else if (p_msg->event == UIPC_FIFO_CLOSE_EVT)
    {
        APPL_TRACE_DEBUG0("uipc_cl_hh_cback fifo closed event");
    }
    else
    {
        APPL_TRACE_ERROR1("uipc_cl_hh_cback bad event received from UIPC:%d",
                p_msg->event);
    }

    /* Free received buffer */
    GKI_freebuf(p_msg);
}




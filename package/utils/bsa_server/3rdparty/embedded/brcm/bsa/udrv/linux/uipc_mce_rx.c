/*****************************************************************************
**
**  Name:           uipc_mce_rx.c
**
**  Description:    UIPC mce internal services for the BSA client and server
**
**  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "gki.h"

#include "bsa_api.h"
#include "bsa_int.h"

#include "uipc_thread.h"
#include "uipc_fifo.h"

#ifndef UIPC_MCE_RX_FIFO_NAME
#define UIPC_MCE_RX_FIFO_NAME "./bt-mce-rx-fifo"
#endif
#define UIPC_MCE_RX_FIFO_NAME_LEN (BSA_MGT_UIPC_PATH_MAX + sizeof(UIPC_MCE_RX_FIFO_NAME))

#ifdef BSA_SERVER
#ifndef UIPC_SV_TX_MCE_MODE
#define UIPC_SV_TX_MCE_MODE (UIPC_FIFO_MODE_CREATE | UIPC_FIFO_MODE_WRITE )
#endif
#endif

#ifdef BSA_CLIENT
#ifndef UIPC_CL_RX_MCE_MODE
/* Read direction, Data Event */
#define UIPC_CL_RX_MCE_MODE (UIPC_FIFO_MODE_READ | UIPC_FIFO_MODE_RX_DATA_EVT)
#endif
#endif

typedef struct {
    BOOLEAN opened;
    void *p_data;
    /* #ifdef BSA_CLIENT */
    tUIPC_RCV_CBACK *p_cback;
    /* #endif */
    tUIPC_FIFO_DESC desc;
} tUIPC_MCE_RX_CB;

/********************* Variable ********************/

static tUIPC_MCE_RX_CB uipc_mce_rx_cb;

/********************* local functions ********************/
#ifdef BSA_CLIENT
static void uipc_mce_rx_cback(BT_HDR *p_msg);
#endif

/*******************************************************************************
**
** Function         uipc_mce_rx_init
**
** Description
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
void uipc_mce_rx_init(void *p_data)
{
    memset(&uipc_mce_rx_cb, 0, sizeof(uipc_mce_rx_cb));
    uipc_mce_rx_cb.p_data = p_data;
    uipc_mce_rx_cb.desc = UIPC_FIFO_BAD_DESC;
}

/*******************************************************************************
**
** Function         uipc_mce_rx_open
**
** Description
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
BOOLEAN uipc_mce_rx_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_mce_cback)
{

    APPL_TRACE_API0("uipc_mce_rx_open");

    /* Nesting protection */
    if (uipc_mce_rx_cb.opened == TRUE)
    {
        APPL_TRACE_ERROR0("ERROR uipc_mce_rx_open already open");
        return 0;
    }

#ifdef BSA_SERVER
    APPL_TRACE_API0("uipc_mce_rx_open server make the the FIFO for MCE");

    /* Open a fifo in Tx/write direction */
    uipc_mce_rx_cb.desc = uipc_fifo_open((UINT8*)UIPC_MCE_RX_FIFO_NAME, ch_id, NULL, UIPC_SV_TX_MCE_MODE);
    if (uipc_mce_rx_cb.desc == UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_mce_rx_open fails to open Tx MCE channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_mce_rx_open MCE channel opened");

#else
    if (p_mce_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_mce_rx_open NULL Callback");
        return FALSE;
    }

    if (uipc_mce_rx_cb.desc != UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_mce_rx_open MCE channel is already opened. Do nothing.");
        return TRUE;
    }

    /* Open a FIFO in TX/write direction */
    /* Note that we use our own cback (which will call the app's callback) */
    uipc_mce_rx_cb.desc = uipc_fifo_open((UINT8*)UIPC_MCE_RX_FIFO_NAME, ch_id, uipc_mce_rx_cback,
        UIPC_CL_RX_MCE_MODE);

    if (uipc_mce_rx_cb.desc == UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_mce_rx_open fails to open Rx MCE channel");
        return FALSE;
    }

    /* Save application's callback */
    uipc_mce_rx_cb.p_cback = p_mce_cback;

    APPL_TRACE_DEBUG0("uipc_mce_rx_open MCE channel opened");

#endif
    uipc_mce_rx_cb.opened = TRUE;
    return TRUE;
}

#ifdef BSA_CLIENT
/*******************************************************************************
**
** Function         uipc_mce_rx_cback
**
** Description      Callback that is called by FIFO read service
**
** Parameters
**         BT_HDR *p_msg: pointer on data header
**
** Returns          void
**
*******************************************************************************/
static void uipc_mce_rx_cback(BT_HDR *p_msg)
{
    if (uipc_mce_rx_cb.p_cback != NULL)
    {
        uipc_mce_rx_cb.p_cback(p_msg);
    }
}

/*******************************************************************************
**
** Function           uipc_mce_rx_read
**
** Description        API to let the application read the FIFO when no call back
**                    is provided.
**
** Output Parameter
**
** Returns            Length of the data retrieved
**
*******************************************************************************/
UINT32 uipc_mce_rx_read(UINT8 * p_buf, UINT32 len)
{
    ssize_t length;

    /* APPL_TRACE_API0("uipc_mce_rx_read");*/

    /* if the UIPC is not open return not- until application disconnects from server */
    if (uipc_mce_rx_cb.opened == FALSE)
    {
        return 0;
    }
    length = uipc_fifo_read(uipc_mce_rx_cb.desc, p_buf, len);

    APPL_TRACE_DEBUG2("uipc_mce_rx_read %d->%d", len, length);

    return length;
}
#endif

/*******************************************************************************
**
** Function         uipc_mce_rx_send
**
** Description      This function send data to a Server application
**
** Parameters       buffer: Pointer to buffer to send
**                  Length: length of buffer
**
** Returns          TRUE if ok, FALSE if failure
**
*******************************************************************************/
BOOLEAN uipc_mce_rx_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf, UINT16 msglen)
{
#ifdef BSA_SERVER
    BOOLEAN uipc_status;

    uipc_status = uipc_fifo_send(uipc_mce_rx_cb.desc, p_buf, msglen);

    return uipc_status;
#else
    APPL_TRACE_ERROR0("error uipc_mce_rx_send not supported for bsa client");
    return FALSE;
#endif
}

/*******************************************************************************
**
** Function         uipc_mce_rx_send_buf
**
** Description      This function is used to send a data buffer to a client
**
** Parameters       channel_id: UIPC channel id
**                  p_msg: data to send
**
** Returns          BOOLEAN
**
*******************************************************************************/
BOOLEAN uipc_mce_rx_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
    BOOLEAN uipc_status;

    uipc_status = uipc_fifo_send_pbuf(uipc_mce_rx_cb.desc,(UINT16) 0, p_msg);

    return uipc_status;
}

/*******************************************************************************
**
** Function         uipc_mce_rx_close
**
** Description      .
**
** Parameters
**
** Returns          void
**
*******************************************************************************/
void uipc_mce_rx_close(void)
{
    APPL_TRACE_API0("uipc_mce_rx_close");

    if (uipc_mce_rx_cb.desc != UIPC_FIFO_BAD_DESC)
    {
        uipc_fifo_close(uipc_mce_rx_cb.desc, FALSE);
        uipc_mce_rx_cb.desc = UIPC_FIFO_BAD_DESC;
    }

    uipc_mce_rx_cb.opened = FALSE;
}


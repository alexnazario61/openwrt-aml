/*****************************************************************************
 **
 **  Name:           uipc_pbc.c
 **
 **  Description:    UIPC pbc internal services for the BSA client and server
 **
 **  Copyright (c) 2013-2014, Broadcom Corp., All Rights Reserved.
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

#ifndef UIPC_PBC_FIFO_NAME
#define UIPC_PBC_FIFO_NAME "./bt-pbc-fifo"
#endif
#define UIPC_PBC_FIFO_NAME_LEN (BSA_MGT_UIPC_PATH_MAX + sizeof(UIPC_PBC_FIFO_NAME))

#ifdef BSA_SERVER
#ifndef UIPC_SV_TX_PBC_MODE
#define UIPC_SV_TX_PBC_MODE (UIPC_FIFO_MODE_CREATE | UIPC_FIFO_MODE_WRITE )
#endif
#endif

#ifdef BSA_CLIENT
#ifndef UIPC_CL_RX_PBC_MODE
/* Read direction, Data Event */
#define UIPC_CL_RX_PBC_MODE (UIPC_FIFO_MODE_READ | UIPC_FIFO_MODE_RX_DATA_EVT)
#endif
#endif

typedef struct {
    BOOLEAN opened;
    void *p_data;
//#ifdef BSA_CLIENT
    tUIPC_RCV_CBACK *p_cback;
//#endif
    tUIPC_FIFO_DESC desc;
} tUIPC_PBC_CB;

/********************* Variable ********************/

static tUIPC_PBC_CB uipc_pbc_cb;

/********************* local functions ********************/
#ifdef BSA_CLIENT
static void uipc_pbc_cback(BT_HDR *p_msg);
#endif

/*******************************************************************************
 **
 ** Function         uipc_pbc_init
 **
 ** Description
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_pbc_init(void *p_data)
{
    memset(&uipc_pbc_cb, 0, sizeof(uipc_pbc_cb));
    uipc_pbc_cb.p_data = p_data;
    uipc_pbc_cb.desc = UIPC_FIFO_BAD_DESC;
}

/*******************************************************************************
 **
 ** Function         uipc_pbc_open
 **
 ** Description
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_pbc_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_pbc_cback)
{

    APPL_TRACE_API0("uipc_pbc_open");

    /* Nesting protection */
    if (uipc_pbc_cb.opened == TRUE)
    {
        APPL_TRACE_ERROR0("ERROR uipc_pbc_open already open");
        return 0;
    }

#ifdef BSA_SERVER
    APPL_TRACE_API0("uipc_pbc_open server make the the FIFO for PBC");

    /* Open a fifo in Tx/write direction */
    uipc_pbc_cb.desc = uipc_fifo_open((UINT8*)UIPC_PBC_FIFO_NAME, ch_id, NULL, UIPC_SV_TX_PBC_MODE);
    if (uipc_pbc_cb.desc == UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_pbc_open fails to open Tx PBC channel");
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_pbc_open PBC channel opened");

#else
    if (p_pbc_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_pbc_open NULL Callback");
        return FALSE;
    }

    if (uipc_pbc_cb.desc != UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_pbc_open PBC channel is already opened. Do nothing.");
        return TRUE;
    }

    /* Open a FIFO in TX/write direction */
    /* Note that we use our own cback (which will call the app's callback) */
    uipc_pbc_cb.desc = uipc_fifo_open((UINT8*)UIPC_PBC_FIFO_NAME, ch_id, uipc_pbc_cback,
            UIPC_CL_RX_PBC_MODE);

    if (uipc_pbc_cb.desc == UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_pbc_open fails to open Rx PBC channel");
        return FALSE;
    }

    /* Save application's callback */
    uipc_pbc_cb.p_cback = p_pbc_cback;

    APPL_TRACE_DEBUG0("uipc_pbc_open PBC channel opened");

#endif
    uipc_pbc_cb.opened = TRUE;
    return TRUE;
}

#ifdef BSA_CLIENT
/*******************************************************************************
 **
 ** Function         uipc_pbc_cback
 **
 ** Description      Callback that is called by FIFO read service
 **
 ** Parameters
 **         BT_HDR *p_msg: pointer on data header
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_pbc_cback(BT_HDR *p_msg)
{
    if (uipc_pbc_cb.p_cback != NULL)
    {
        uipc_pbc_cb.p_cback(p_msg);
    }
}

/*******************************************************************************
 **
 ** Function           uipc_pbc_read
 **
 ** Description        API to let the application read the FIFO when no call back
 **                    is provided.
 **
 ** Output Parameter
 **
 ** Returns            Length of the data retrieved
 **
 *******************************************************************************/
UINT32 uipc_pbc_read(UINT8 * p_buf, UINT32 len)
{
    ssize_t length;

    /* APPL_TRACE_API0("uipc_pbc_read");*/

    /* if the UIPC is not open return not- until application disconnects from server */
    if (uipc_pbc_cb.opened == FALSE)
    {
        return 0;
    }
    length = uipc_fifo_read(uipc_pbc_cb.desc, p_buf, len);

    APPL_TRACE_DEBUG2("uipc_pbc_read %d->%d", len, length);

    return length;
}
#endif

/*******************************************************************************
 **
 ** Function         uipc_pbc_send
 **
 ** Description      This function send data to a Server application
 **
 ** Parameters       buffer: Pointer to buffer to send
 **                  Length: length of buffer
 **
 ** Returns          TRUE if ok, FALSE if failure
 **
 *******************************************************************************/
BOOLEAN uipc_pbc_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf, UINT16 msglen)
{
#ifdef BSA_SERVER
    BOOLEAN uipc_status;

    uipc_status = uipc_fifo_send(uipc_pbc_cb.desc, p_buf, msglen);

    return uipc_status;
#else
    APPL_TRACE_ERROR0("error uipc_pbc_send not supported for bsa client");
    return FALSE;
#endif
}

/*******************************************************************************
 **
 ** Function         uipc_pbc_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_msg: data to send
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_pbc_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
    BOOLEAN uipc_status;

    uipc_status = uipc_fifo_send_pbuf(uipc_pbc_cb.desc,(UINT16) 0, p_msg);

    return uipc_status;
}

/*******************************************************************************
 **
 ** Function         uipc_pbc_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_pbc_close(void)
{
    APPL_TRACE_API0("uipc_pbc_close");

    if (uipc_pbc_cb.desc != UIPC_FIFO_BAD_DESC)
    {
        uipc_fifo_close(uipc_pbc_cb.desc, FALSE);
        uipc_pbc_cb.desc = UIPC_FIFO_BAD_DESC;
    }

    uipc_pbc_cb.opened = FALSE;
}


/*****************************************************************************
 **
 **  Name:           uipc_sac.c
 **
 **  Description:    UIPC internal services for the BSA SAC client and server
 **
 **  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
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

#define UIPC_SAC_TX_FIFO_NAME "./bt-sac-tx-fifo"
#define UIPC_SAC_RX_FIFO_NAME "./bt-sac-rx-fifo"
#define UIPC_SAC_FIFO_NAME_LEN (BSA_MGT_UIPC_PATH_MAX + sizeof(UIPC_SAC_TX_FIFO_NAME))

#ifdef BSA_CLIENT
#define UIPC_SAC_TX_MODE (UIPC_FIFO_MODE_WRITE)
#define UIPC_SAC_RX_MODE (UIPC_FIFO_MODE_READ | UIPC_FIFO_MODE_RX_READ)
#endif

#ifdef BSA_SERVER
#define UIPC_SAC_TX_MODE (UIPC_FIFO_MODE_CREATE |UIPC_FIFO_MODE_READ | UIPC_FIFO_MODE_RX_READ)
#define UIPC_SAC_RX_MODE (UIPC_FIFO_MODE_CREATE |UIPC_FIFO_MODE_WRITE)
#endif

typedef struct {
    tUIPC_FIFO_DESC desc_tx;
    tUIPC_FIFO_DESC desc_rx;
    void *p_data;
} tUIPC_SAC_CB;

/********************* Variable ********************/

static tUIPC_SAC_CB uipc_sac_cb;

void uipc_sac_close(void);

/*******************************************************************************
 **
 ** Function         uipc_sac_init
 **
 ** Description
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sac_init(void *p_data)
{
    memset(&uipc_sac_cb, 0, sizeof(uipc_sac_cb));
    uipc_sac_cb.p_data = p_data;
    uipc_sac_cb.desc_tx = UIPC_FIFO_BAD_DESC;
    uipc_sac_cb.desc_rx = UIPC_FIFO_BAD_DESC;
}

/*******************************************************************************
 **
 ** Function         uipc_sac_open
 **
 ** Description
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sac_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_pbc_cback)
{

    APPL_TRACE_API0("uipc_sac_open");

    if (uipc_sac_cb.desc_tx != UIPC_FIFO_BAD_DESC &&
        uipc_sac_cb.desc_rx != UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_DEBUG0("uipc_sac_open SAC channel is already open.  Do nothing.");
        return TRUE;
    }

    uipc_sac_cb.desc_tx = uipc_fifo_open((UINT8*)UIPC_SAC_TX_FIFO_NAME, ch_id, NULL,
            UIPC_SAC_TX_MODE);
    uipc_sac_cb.desc_rx = uipc_fifo_open((UINT8*)UIPC_SAC_RX_FIFO_NAME, ch_id, NULL,
            UIPC_SAC_RX_MODE);

    if (uipc_sac_cb.desc_tx == UIPC_FIFO_BAD_DESC ||
        uipc_sac_cb.desc_rx == UIPC_FIFO_BAD_DESC)
    {
        APPL_TRACE_ERROR0("uipc_sac_open failed to open channel");
        uipc_sac_close();
        return FALSE;
    }

    APPL_TRACE_DEBUG0("uipc_sac_open channel opened");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function           uipc_sac_read
 **
 ** Description        API to let the application read the FIFO when no call back
 **                    is provided.
 **
 ** Output Parameter
 **
 ** Returns            Length of the data retrieved
 **
 *******************************************************************************/
UINT32 uipc_sac_read(UINT8 * p_buf, UINT32 len)
{
#ifdef BSA_CLIENT
    tUIPC_FIFO_DESC desc = uipc_sac_cb.desc_rx;
#else
    tUIPC_FIFO_DESC desc = uipc_sac_cb.desc_tx;
#endif
    ssize_t length;

    /* if the UIPC is not open return  */
    if (desc == UIPC_FIFO_BAD_DESC)
    {
        return 0;
    }
    length = uipc_fifo_read(desc, p_buf, len);

    APPL_TRACE_DEBUG2("uipc_sac_read %d->%d", len, length);

    return length;
}

/*******************************************************************************
 **
 ** Function         uipc_sac_send
 **
 ** Description      This function send data through the FIFO
 **
 ** Parameters       buffer: Pointer to buffer to send
 **                  Length: length of buffer
 **
 ** Returns          TRUE if ok, FALSE if failure
 **
 *******************************************************************************/
BOOLEAN uipc_sac_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf, UINT16 msglen)
{
#ifdef BSA_CLIENT
    tUIPC_FIFO_DESC desc = uipc_sac_cb.desc_tx;
#else
    tUIPC_FIFO_DESC desc = uipc_sac_cb.desc_rx;
#endif

    /* if the UIPC is not open return  */
    if (desc == UIPC_FIFO_BAD_DESC)
    {
        return FALSE;
    }
    return uipc_fifo_send(desc, p_buf, msglen);
}

/*******************************************************************************
 **
 ** Function         uipc_sac_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_msg: data to send
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sac_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
#ifdef BSA_CLIENT
    tUIPC_FIFO_DESC desc = uipc_sac_cb.desc_tx;
#else
    tUIPC_FIFO_DESC desc = uipc_sac_cb.desc_rx;
#endif

    /* if the UIPC is not open return  */
    if (desc == UIPC_FIFO_BAD_DESC)
    {
        return FALSE;
    }
    return uipc_fifo_send_pbuf(desc, (UINT16) 0, p_msg);
}

/*******************************************************************************
 **
 ** Function         uipc_sac_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sac_close(void)
{
    APPL_TRACE_API0("uipc_sac_close");

    if (uipc_sac_cb.desc_tx != UIPC_FIFO_BAD_DESC)
    {
        uipc_fifo_close(uipc_sac_cb.desc_tx, FALSE);
        uipc_sac_cb.desc_tx = UIPC_FIFO_BAD_DESC;
    }

    if (uipc_sac_cb.desc_rx != UIPC_FIFO_BAD_DESC)
    {
        uipc_fifo_close(uipc_sac_cb.desc_rx, FALSE);
        uipc_sac_cb.desc_rx = UIPC_FIFO_BAD_DESC;
    }
}


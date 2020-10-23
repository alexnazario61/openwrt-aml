/*****************************************************************************
**
**  Name:           uipc_pbc.c
**
**  Description:    UIPC pbc internal services for the BSA client and server
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "gki.h"

#include "bsa_api.h"
#include "bsa_int.h"

#include "uipc_thread.h"
#include "uipc.h"

#ifndef UIPC_PBC_FIFO_NAME
#define UIPC_PBC_FIFO_NAME "./bt-pbc-fifo"
#endif
#define UIPC_PBC_FIFO_NAME_LEN (BSA_MGT_UIPC_PATH_MAX + sizeof(UIPC_PBC_FIFO_NAME))

#ifndef UIPC_SV_TX_PBC_MODE
#define UIPC_SV_TX_PBC_MODE (UIPC_FIFO_MODE_CREATE | UIPC_FIFO_MODE_WRITE )
#endif

#ifndef UIPC_CL_RX_PBC_MODE
/* Read direction, Data Event */
#define UIPC_CL_RX_PBC_MODE (UIPC_FIFO_MODE_READ | UIPC_FIFO_MODE_RX_DATA_EVT)
#endif


typedef struct {
    BOOLEAN opened;
    void *p_data;
    tUIPC_RCV_CBACK *p_cback;
} tUIPC_PBC_CB;

/********************* Variable ********************/

static tUIPC_PBC_CB uipc_pbc_cb;

/********************* local functions ********************/
static void uipc_pbc_cback(BT_HDR *p_msg);

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
    return TRUE;
}

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
    /* APPL_TRACE_API0("uipc_pbc_read");*/
    return len;
}

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
    BOOLEAN uipc_status = TRUE;
    BOOLEAN is_server;
    UINT8 task_id;

    task_id = GKI_get_taskid();
    is_server = (task_id <= BTE_APPL_TASK)?TRUE:FALSE;

    if(is_server)
    {
#ifdef UIPC_PBC_DATA_CALLBACK
        uipc_status = UIPC_PBC_DATA_CALLBACK(p_buf, msglen);
#else
        APPL_TRACE_API0("uipc_pbc_send() Not implemented!");
#endif
        return uipc_status;
    }
    else
    {
        APPL_TRACE_ERROR0("error uipc_pbc_send not supported for bsa client");
        return FALSE;
    }
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
    BOOLEAN uipc_status = TRUE;
    BOOLEAN is_server;
    UINT8 task_id;

    task_id = GKI_get_taskid();
    is_server = (task_id <= BTE_APPL_TASK)?TRUE:FALSE;

    if(is_server)
    {
#ifdef UIPC_PBC_DATA_CALLBACK
        uipc_status = UIPC_PBC_DATA_CALLBACK(*((UINT16 **)(p_msg + 1)) + (p_msg->offset), p_msg->len);
#else
        APPL_TRACE_API0("uipc_pbc_send_buf() Not implemented!");
#endif
        GKI_freebuf(p_msg);
        return uipc_status;
    }
    else
    {
        APPL_TRACE_ERROR0("error uipc_pbc_send_buf not supported for bsa client");
        GKI_freebuf(p_msg);
        return FALSE;
    }
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
}


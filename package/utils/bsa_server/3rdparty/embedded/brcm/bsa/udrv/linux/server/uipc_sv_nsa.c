/*****************************************************************************
 **
 **  Name:           uipc_sv_nsa.c
 **
 **  Description:    Server API for BSA NSA Interface
 **
 **  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
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
#include "uipc_sv_nsa.h"
#include "uipc_sv_socket.h"
#include "bsa_strm_dec.h"

/* #define UIPC_SV_NSA_DEBUG*/

#ifdef UIPC_SV_NSA_DEBUG
#include "bsa_trace.h"
#endif

/* Defines how many NSA Server/Client can connect to BSA */
#ifndef UIPC_SV_NSA_MAX_CLIENT
#define UIPC_SV_NSA_MAX_CLIENT 5
#endif

#ifndef UIPC_SV_NSA_SOCKET_NAME
#define UIPC_SV_NSA_SOCKET_NAME "./bt-nci-socket"
#endif

#ifndef UIPC_SV_NSA_SOCKET_MODE
/* By default. Send Rx event (with data) and Write is blocking */
#define UIPC_SV_NSA_SOCKET_MODE (UIPC_SV_SOCKET_MODE_RX_DATA_EVT | UIPC_SV_SOCKET_MODE_BLOCK)
#endif

typedef struct
{
    tUIPC_SV_SOCKET_SERVER_DESC socket_desc;
} tUIPC_SV_NSA_CB;

/*
 * Global variables
 */
tUIPC_SV_NSA_CB uipc_sv_nsa_cb;


/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_init
 **
 ** Description      UIPC Control Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_nsa_init(void)
{
    uipc_sv_nsa_cb.socket_desc = UIPC_SV_SOCKET_SERVER_MAX;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_open
 **
 ** Description      Open UIPC control channel
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_nsa_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_sv_nsa_open fail p_cback NULL");
        return FALSE;
    }

    if (uipc_sv_nsa_cb.socket_desc < UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR0("uipc_sv_nsa_open already opened");
        return FALSE;
    }

    /* Starts server with local callback function */
    uipc_sv_nsa_cb.socket_desc = uipc_sv_socket_start_server(
            (UINT8 *) UIPC_SV_NSA_SOCKET_NAME, p_cback,
            UIPC_SV_NSA_MAX_CLIENT, UIPC_SV_NSA_SOCKET_MODE);

    if (uipc_sv_nsa_cb.socket_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR0("uipc_sv_nsa_open fails to start control server");
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_close
 **
 ** Description      Close UIPC control channel
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_nsa_close(tUIPC_CH_ID channel_id)
{
    uipc_sv_socket_stop_server(uipc_sv_nsa_cb.socket_desc, (UINT8 *)UIPC_SV_NSA_SOCKET_NAME);
    uipc_sv_nsa_cb.socket_desc = UIPC_SV_SOCKET_SERVER_MAX;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sv_nsa_send(tUIPC_CH_ID channel_id,
        tBSA_CLIENT_NB client_num, UINT8 *p_buf, UINT16 msglen)
{
    BOOLEAN uipc_status;

#ifdef UIPC_SV_NSA_DEBUG
    APPL_TRACE_DEBUG2("uipc_sv_nsa_send Client:%d length:%d",
            client_num, msglen);
    scru_dump_hex(p_buf, "Data Tx (To NSA Server)", msglen, TRACE_LAYER_NONE,
            TRACE_TYPE_DEBUG);
#endif

    uipc_status = uipc_sv_socket_send(uipc_sv_nsa_cb.socket_desc, client_num, p_buf, msglen);

    return uipc_status;
}


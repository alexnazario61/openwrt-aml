/*****************************************************************************
 **
 **  Name:           bsa_dg_int.h
 **
 **  Description:    Contains private BSA data
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_DG_INT_H
#define BSA_DG_INT_H

#include "bsa_dg_api.h"

/*
 * DG Message definition
 */
enum {
    BSA_DG_MSGID_ENABLE_CMD = BSA_DG_MSGID_FIRST,
    BSA_DG_MSGID_DISABLE_CMD,
    BSA_DG_MSGID_LISTEN_CMD,
    BSA_DG_MSGID_OPEN_CMD,
    BSA_DG_MSGID_CLOSE_CMD,
    BSA_DG_MSGID_SHUTDOWN_CMD,
    BSA_DG_MSGID_FIND_SERVICE_CMD,

    BSA_DG_MSGID_LAST_CMD = BSA_DG_MSGID_FIND_SERVICE_CMD,

    BSA_DG_MSGID_OPEN_EVT, /* Connection to peer is open. */
    BSA_DG_MSGID_CLOSE_EVT, /* Connection to peer closed. */
    BSA_DG_MSGID_FIND_SERVICE_EVT,  /* Find Service Event */

    BSA_DG_MSGID_LAST_EVT = BSA_DG_MSGID_FIND_SERVICE_EVT
};

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_DG_MSGID_STATUS_RSP;

/*
 * Structures used for parameters (transport)
 */

/* Structures used for DG */

typedef tBSA_DG_ENABLE             tBSA_DG_MSGID_ENABLE_CMD_REQ;
typedef tBSA_DG_MSGID_STATUS_RSP    tBSA_DG_MSGID_ENABLE_CMD_RSP;

typedef tBSA_DG_DISABLE            tBSA_DG_MSGID_DISABLE_CMD_REQ;
typedef tBSA_DG_MSGID_STATUS_RSP    tBSA_DG_MSGID_DISABLE_CMD_RSP;

typedef tBSA_DG_LISTEN              tBSA_DG_MSGID_LISTEN_CMD_REQ;
typedef struct
{
    tBSA_STATUS status;
    tBSA_DG_HNDL handle;
} tBSA_DG_MSGID_LISTEN_CMD_RSP;

typedef tBSA_DG_OPEN                tBSA_DG_MSGID_OPEN_CMD_REQ;
typedef tBSA_DG_MSGID_STATUS_RSP    tBSA_DG_MSGID_OPEN_CMD_RSP;

typedef tBSA_DG_CLOSE               tBSA_DG_MSGID_CLOSE_CMD_REQ;
typedef tBSA_DG_MSGID_STATUS_RSP    tBSA_DG_MSGID_CLOSE_CMD_RSP;

typedef tBSA_DG_SHUTDOWN            tBSA_DG_MSGID_SHUTDOWN_CMD_REQ;
typedef tBSA_DG_MSGID_STATUS_RSP    tBSA_DG_MSGID_SHUTDOWN_CMD_RSP;

typedef tBSA_DG_OPEN_MSG            tBSA_DG_MSGID_OPEN_EVT;
typedef tBSA_DG_CLOSE_MSG           tBSA_DG_MSGID_CLOSE_EVT;
typedef tBSA_DG_FIND_SERVICE_MSG    tBSA_DG_MSGID_FIND_SERVICE_EVT;

typedef tBSA_DG_FIND_SERVICE        tBSA_DG_MSGID_FIND_SERVICE_CMD_REQ;
typedef tBSA_DG_MSGID_STATUS_RSP    tBSA_DG_MSGID_FIND_SERVICE_CMD_RSP;

typedef union
{
    tBSA_DG_MSGID_ENABLE_CMD_RSP enable;
    tBSA_DG_MSGID_DISABLE_CMD_RSP disable;
    tBSA_DG_MSGID_LISTEN_CMD_RSP listen;
    tBSA_DG_MSGID_OPEN_CMD_RSP open;
    tBSA_DG_MSGID_CLOSE_CMD_RSP close;
    tBSA_DG_MSGID_SHUTDOWN_CMD_RSP shutdown;
    tBSA_DG_MSGID_FIND_SERVICE_CMD_RSP find_service;
} tBSA_DG_MSGID_CMD_RSP;

typedef union
{
    tBSA_DG_MSGID_OPEN_EVT open;
    tBSA_DG_MSGID_CLOSE_EVT close;
    tBSA_DG_MSGID_FIND_SERVICE_EVT find_service;
} tBSA_DG_MSGID_EVT;

/* Control Block  (used by client) */
typedef struct {
    tBSA_DG_CBACK *p_app_cback;
} tBSA_DG_CB;

extern tBSA_DG_CB bsa_dg_cb;

/*******************************************************************************
 **
 ** Function       bsa_dg_event_hdlr
 **
 ** Description    Handle DG events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_dg_event_hdlr(int message_id, tBSA_DG_MSG *p_buffer, int length);


#endif


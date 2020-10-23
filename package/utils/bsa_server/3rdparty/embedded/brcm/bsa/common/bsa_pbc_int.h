/*****************************************************************************
**
**  Name:           bsa_pbc_int.h
**
**  Description:    Contains private BSA PBAP client data
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
*****************************************************************************/

#ifndef BSA_PBC_INT_H
#define BSA_PBC_INT_H

#include "bsa_pbc_api.h"

#define BSA_PBC_APP_ID  1

/*
* PBC Message definition
*/
enum
{
    BSA_PBC_MSGID_ENABLE_CMD = BSA_PBC_MSGID_FIRST,
    BSA_PBC_MSGID_DISABLE_CMD,
    BSA_PBC_MSGID_OPEN_CMD,
    BSA_PBC_MSGID_CLOSE_CMD,
	BSA_PBC_MSGID_CANCEL_CMD,
    BSA_PBC_MSGID_ABORT_CMD,
    BSA_PBC_MSGID_AUTH_RSP_CMD,
    BSA_PBC_MSGID_GET_CMD,
    BSA_PBC_MSGID_SET_CMD,
    BSA_PBC_MSGID_LAST_CMD = BSA_PBC_MSGID_SET_CMD,

    BSA_PBC_MSGID_OPEN_EVT, /* Connection Open*/
    BSA_PBC_MSGID_CLOSE_EVT, /* Connection Closed */
    BSA_PBC_MSGID_DISABLE_EVT, /* Connection Disable */
    BSA_PBC_MSGID_ABORT_EVT, /* Connection Abort */
    BSA_PBC_MSGID_AUTH_EVT, /* OBEX Authentication event */
    BSA_PBC_MSGID_GET_EVT, /* Get request event */
    BSA_PBC_MSGID_SET_EVT, /* Set request event */
    BSA_PBC_MSGID_LAST_EVT = BSA_PBC_MSGID_SET_EVT
};

/*
* Structures used for parameters (transport)
*/

/*
* PBC Enable
*/
typedef struct
{
    UINT8 app_id;
    tBSA_PBC_FEA_MASK local_features;
} tBSA_PBC_MSGID_ENABLE_CMD_REQ;

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_PBC_MSGID_STATUS_RSP;

typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_ENABLE_CMD_RSP;

/*
* PBC Disable
*/
typedef tBSA_PBC_DISABLE tBSA_PBC_MSGID_DISABLE_CMD_REQ;
typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_DISABLE_CMD_RSP;
typedef tBSA_PBC_DISABLE_MSG tBSA_PBC_MSGID_DISABLE_RSP_EVT;

/*
* PBC Open
*/
typedef tBSA_PBC_OPEN tBSA_PBC_MSGID_OPEN_CMD_REQ;
typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_OPEN_CMD_RSP;
typedef tBSA_PBC_OPEN_MSG tBSA_PBC_MSGID_OPEN_RSP_EVT;

/*
* PBC Close
*/
typedef tBSA_PBC_CLOSE tBSA_PBC_MSGID_CLOSE_CMD_REQ;
typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_CLOSE_CMD_RSP;
typedef tBSA_PBC_CLOSE_MSG tBSA_PBC_MSGID_CLOSE_RSP_EVT;

/* PBC Cancel */
typedef tBSA_PBC_CANCEL tBSA_PBC_MSGID_CANCEL_CMD_REQ;

/*
* PBC Abort
*/
typedef tBSA_PBC_ABORT tBSA_PBC_MSGID_ABORT_CMD_REQ;
typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_ABORT_CMD_RSP;
typedef tBSA_PBC_ABORT_MSG tBSA_PBC_MSGID_ABORT_RSP_EVT;

/*
* PBC AuthRsp
*/
typedef tBSA_PBC_AUTHRSP tBSA_PBC_MSGID_AUTH_RSP_CMD_REQ;
typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_AUTH_RSP_CMD_RSP;
typedef tBSA_PBC_AUTH_MSG tBSA_PBC_MSGID_AUTH_RSP_EVT;

/*
* PBC Get Rsp
*/
typedef tBSA_PBC_GET tBSA_PBC_MSGID_GET_CMD_REQ;
typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_GET_CMD_RSP;
typedef tBSA_PBC_GET_MSG tBSA_PBC_MSGID_GET_EVT;

/*
* PBC Set Rsp
*/
typedef tBSA_PBC_SET tBSA_PBC_MSGID_SET_CMD_REQ;
typedef tBSA_PBC_MSGID_STATUS_RSP tBSA_PBC_MSGID_SET_CMD_RSP;
typedef tBSA_PBC_SET_MSG tBSA_PBC_MSGID_SET_EVT;

/*
* Structure (used by client)
*/
typedef struct
{
    tBSA_PBC_CBACK *p_app_pbc_cback;
} tBSA_PBC_CB;

/*
* Global variables (used by client)
*/
extern tBSA_PBC_CB bsa_pbc_cb;

/*******************************************************************************
**
** Function       bsa_pbc_event_hdlr
**
** Description    Handle PBC events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_pbc_event_hdlr(int message_id, tBSA_PBC_MSG *p_data, int length);

#endif

/*****************************************************************************
 **
 **  Name:           bsa_op_int.h
 **
 **  Description:    Contains private BSA data
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_OP_INT_H
#define BSA_OP_INT_H

#include "bsa_op_api.h"

/*
 * OPS Message definition
 */
enum {
    BSA_OPS_MSGID_ENABLE_CMD = BSA_OPS_MSGID_FIRST,
    BSA_OPS_MSGID_DISABLE_CMD,
    BSA_OPS_MSGID_CLOSE_CMD,
    BSA_OPS_MSGID_ACCESS_CMD,

    BSA_OPS_MSGID_LAST_CMD = BSA_OPS_MSGID_ACCESS_CMD,

    BSA_OPS_MSGID_OPEN_EVT, /* Connection to peer is open. */
    BSA_OPS_MSGID_PROGRESS_EVT, /* Object being sent or received. */
    BSA_OPS_MSGID_OBJECT_EVT, /* Object has been received. */
    BSA_OPS_MSGID_CLOSE_EVT, /* Connection to peer closed. */
    BSA_OPS_MSGID_ACCESS_EVT, /* Request for access to push or pull object */

    BSA_OPS_MSGID_LAST_EVT = BSA_OPS_MSGID_ACCESS_EVT
};

/*
 * OPC Message definition
 */
enum {
    BSA_OPC_MSGID_ENABLE_CMD = BSA_OPC_MSGID_FIRST,
    BSA_OPC_MSGID_DISABLE_CMD,
    BSA_OPC_MSGID_PUSH_CMD,
    BSA_OPC_MSGID_PULL_CARD_CMD,
    BSA_OPC_MSGID_EXCH_CARD_CMD,
    BSA_OPC_MSGID_CLOSE_CMD,

    BSA_OPC_MSGID_LAST_CMD = BSA_OPC_MSGID_CLOSE_CMD,

    BSA_OPC_MSGID_ENABLE_EVT, /* Object push server is enabled. */
    BSA_OPC_MSGID_DISABLE_EVT, /* Object push server is disabled. */
    BSA_OPC_MSGID_OPEN_EVT, /* Connection to peer is open. */
    BSA_OPC_MSGID_PROGRESS_EVT, /* Object being sent or received. */
    BSA_OPC_MSGID_OBJECT_EVT, /* Object has been received. */
    BSA_OPC_MSGID_OBJECT_PSHD_EVT, /* Object has been received. */
    BSA_OPC_MSGID_CLOSE_EVT, /* Connection to peer closed. */

    BSA_OPC_MSGID_LAST_EVT = BSA_OPC_MSGID_CLOSE_EVT
};

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;

} tBSA_OP_MSGID_STATUS_RSP;


/*
 * Structures used for parameters (transport)
 */

/* Structures used for OPS */

typedef tBSA_OPS_ENABLE             tBSA_OPS_MSGID_ENABLE_CMD_REQ;
typedef tBSA_OP_MSGID_STATUS_RSP    tBSA_OPS_MSGID_ENABLE_CMD_RSP;

typedef tBSA_OPS_DISABLE            tBSA_OPS_MSGID_DISABLE_CMD_REQ;
typedef tBSA_OP_MSGID_STATUS_RSP    tBSA_OPS_MSGID_DISABLE_CMD_RSP;

typedef tBSA_OPS_CLOSE              tBSA_OPS_MSGID_CLOSE_CMD_REQ;
typedef tBSA_OP_MSGID_STATUS_RSP    tBSA_OPS_MSGID_CLOSE_CMD_RSP;

typedef tBSA_OPS_OBJECT_EVT         tBSA_OPS_MSGID_OPEN_EVENT;

typedef tBSA_OP_MSGID_STATUS_RSP    tBSA_OPS_MSGID_ACCESS_CMD_RSP;

typedef tBSA_OP_MSGID_STATUS_RSP    tBSA_OPS_MSGID_PROGRESS_CMD_RSP;

typedef tBSA_OP_MSGID_STATUS_RSP    tBSA_OPS_MSGID_OBJECT_CMD_RSP;

/* Control Block  (used by client) */
typedef struct {
    tBSA_OPS_CBACK *p_app_ops_cback;
} tBSA_OPS_CB;

extern tBSA_OPS_CB bsa_ops_cb;

/* Structures used for OPC */

/*
 * OPC Enable
 */
typedef tBSA_OPC_ENABLE tBSA_OPC_MSGID_ENABLE_CMD_REQ;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_ENABLE_CMD_RSP;


 /*
  * OPC Disable
  */
typedef tBSA_OPC_DISABLE tBSA_OPC_MSGID_DISABLE_CMD_REQ;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_DISABLE_CMD_RSP;

 
 /*
 * OPC Open
 */
  
typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_OPEN_EVT;
 
 /*
 * OPC Close
 */
typedef tBSA_OPC_CLOSE tBSA_OPC_MSGID_CLOSE_CMD_REQ;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_CLOSE_CMD_RSP;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_CLOSE_EVT;
  
 /*
 * OPC Push
 */
typedef tBSA_OPC_PUSH tBSA_OPC_MSGID_PUSH_CMD_REQ;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_PUSH_CMD_RSP;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_PUSH_EVT;

 /*
 * OPC Pull
 */
typedef tBSA_OPC_PULL_CARD tBSA_OPC_MSGID_PULL_CARD_CMD_REQ;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_PULL_CARD_CMD_RSP;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_PULL_CARD_EVT;

 /*
 * OPC Exchange card
 */
typedef tBSA_OPC_EXCH_CARD tBSA_OPC_MSGID_EXCH_CARD_CMD_REQ;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_EXCH_CARD_CMD_RSP;

typedef tBSA_OP_MSGID_STATUS_RSP tBSA_OPC_MSGID_EXCH_CARD_EVT;

/* Control Block  (used by client) */
typedef struct {
    tBSA_OPC_CBACK *p_cback;
} tBSA_OPC_CB;

extern tBSA_OPC_CB bsa_opc_cb;

/*******************************************************************************
 **
 ** Function       bsa_ops_event_hdlr
 **
 ** Description    Handle OPS events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_ops_event_hdlr(int message_id, tBSA_OPS_MSG *p_buffer, int length);

/*******************************************************************************
 **
 ** Function       bsa_opc_event_hdlr
 **
 ** Description    Handle OPC events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_opc_event_hdlr(int message_id, tBSA_OPC_MSG *p_buffer, int length);

#endif


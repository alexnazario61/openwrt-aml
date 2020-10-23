/*****************************************************************************
 **
 **  Name:           bsa_sec_int.h
 **
 **  Description:    Contains private BSA security data
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_SEC_INT_H
#define BSA_SEC_INT_H

#include "bsa_sec_api.h"

/*
 * Sec Message definition
 */
enum
{
    BSA_SEC_MSGID_SET_SEC_CMD = BSA_SEC_MSGID_FIRST,
    BSA_SEC_MSGID_BOUND_CMD,
    BSA_SEC_MSGID_BOUND_CANCEL_CMD,
    BSA_SEC_MSGID_PIN_CODE_REPLY_CMD,
    BSA_SEC_MSGID_SP_CFM_REPLY_CMD,
    BSA_SEC_MSGID_AUTHORIZE_REPLY_CMD,
    BSA_SEC_MSGID_ADD_DEV_CMD,
    BSA_SEC_MSGID_DEL_DEV_CMD,
    BSA_SEC_MSGID_READ_OOB_CMD,
    BSA_SEC_MSGID_SET_REMOTE_OOB_CMD,
    BSA_SEC_MSGID_ADD_SI_DEV_CMD,

    BSA_SEC_MSGID_LAST_CMD = BSA_SEC_MSGID_ADD_SI_DEV_CMD,

    BSA_SEC_MSGID_LINK_UP_EVT, /* A device is physically connected (for info) */
    BSA_SEC_MSGID_LINK_DOWN_EVT, /* A device is physically disconnected (for info)*/
    BSA_SEC_MSGID_PIN_REQ_EVT, /* PIN code Request */
    BSA_SEC_MSGID_AUTH_CMPL_EVT, /* pairing/authentication complete */
    BSA_SEC_MSGID_BOND_CANCEL_CMPL_EVT, /* Canceling bonding procedure complete */
    BSA_SEC_MSGID_AUTHORIZE_EVT, /* Authorization request */
    BSA_SEC_MSGID_SP_CFM_REQ_EVT, /* Simple Pairing confirm request */
    BSA_SEC_MSGID_SP_KEY_NOTIF_EVT, /* Simple Pairing Passkey Notification */
    BSA_SEC_MSGID_SP_RMT_OOB_EVT, /* Simple Pairing Remote OOB Data request. */
    BSA_SEC_MSGID_LOCAL_OOB_DATA_EVT,   /* Response to read local OOB info request */
    BSA_SEC_MSGID_SP_KEYPRESS_EVT, /* Simple Pairing Key press notification event. */
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    BSA_SEC_MSGID_BLE_KEY_EVT,
    BSA_SEC_MSGID_BLE_PASSKEY_REQ_EVT,
#endif
    BSA_SEC_MSGID_SUSPENDED_EVT, /* Connection in Suspended mode */
    BSA_SEC_MSGID_RESUMED_EVT, /* Connection in Resumed mode */

    BSA_SEC_MSGID_LAST_EVT = BSA_SEC_MSGID_RESUMED_EVT
};

/*
 * Structures used for parameters (transport)
 */

/* Structures used to set security */
typedef struct
{
    tBSA_SEC_IO_CAP io_cap;
    BOOLEAN ssp_debug;
} tBSA_SEC_MSGID_SET_SEC_CMD_REQ;

/* Structures used to sent pin code */
typedef tBSA_SEC_PIN_CODE_REPLY tBSA_SEC_MSGID_PIN_CODE_REPLY_CMD_REQ;

/* Structures used to reply to SP request */
typedef tBSA_SEC_SP_CFM_REPLY tBSA_SEC_MSGID_SP_CFM_REPLY_CMD_REQ;

/* Structures used for Bounding */
typedef tBSA_SEC_BOND tBSA_SEC_MSGID_BOUND_CMD_REQ;

/* Structures used for Canceling bounding procedure */
typedef tBSA_SEC_BOND_CANCEL tBSA_SEC_MSGID_BOUND_CANCEL_CMD_REQ;

/* Structures used for Deleting Device */
typedef tBSA_SEC_REMOVE_DEV tBSA_SEC_MSGID_REMOVE_DEV_CMD_REQ;

/* Structures used for Adding Device */
typedef tBSA_SEC_ADD_DEV tBSA_SEC_MSGID_ADD_DEV_CMD_REQ;

/* Structures used for authorization reply */
typedef tBSA_SEC_AUTH_REPLY tBSA_SEC_MSGID_AUTHORIZE_REPLY_CMD_REQ;

/* Structures used for Read OOB from local controller */
typedef tBSA_SEC_READ_OOB tBSA_SEC_MSGID_READ_OOB_CMD_REQ;

/* Structures used for Set Remote OOB */
typedef tBSA_SEC_SET_REMOTE_OOB tBSA_SEC_MSGID_SET_REMOTE_OOB_CMD_REQ;

/* Structures used for adding special interest device */
typedef tBSA_SEC_ADD_SI_DEV tBSA_SEC_MSGID_ADD_SI_DEV_CMD_REQ;

/* SEC Control block */
typedef struct
{
    tBSA_SEC_CBACK *p_app_sec_cback;
} tBSA_SEC_CB;

/*
 * Definitions for Client only
 */

extern tBSA_SEC_CB bsa_sec_cb;

/*******************************************************************************
 **
 ** Function       bsa_sec_event_hdlr
 **
 ** Description    Handle security events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_sec_event_hdlr(int message_id, tBSA_SEC_MSG *p_data, int length);

#endif


/*****************************************************************************
 **
 **  Name:           bsa_sec_int.c
 **
 **  Description:    This is the internal interface file for security part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"

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
void bsa_sec_event_hdlr(int message_id, tBSA_SEC_MSG *p_data, int length)
{
    tBSA_SEC_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_SEC_MSGID_LINK_UP_EVT: /* A device is physically connected (for info) */
        event = BSA_SEC_LINK_UP_EVT;
        break;
    case BSA_SEC_MSGID_LINK_DOWN_EVT: /* A device is physically disconnected (for info)*/
        event = BSA_SEC_LINK_DOWN_EVT;
        break;
    case BSA_SEC_MSGID_PIN_REQ_EVT: /* PIN code Request */
        event = BSA_SEC_PIN_REQ_EVT;
        break;
    case BSA_SEC_MSGID_AUTH_CMPL_EVT: /* pairing/authentication complete */
        event = BSA_SEC_AUTH_CMPL_EVT;
        break;
    case BSA_SEC_MSGID_BOND_CANCEL_CMPL_EVT: /* Cancel bonding procedure */
        event = BSA_SEC_BOND_CANCEL_CMPL_EVT;
        break;
    case BSA_SEC_MSGID_AUTHORIZE_EVT: /* Authorization request */
        event = BSA_SEC_AUTHORIZE_EVT;
        break;
    case BSA_SEC_MSGID_SP_CFM_REQ_EVT: /* Simple Pairing confirm request */
        event = BSA_SEC_SP_CFM_REQ_EVT;
        break;
    case BSA_SEC_MSGID_SP_KEY_NOTIF_EVT: /* Simple Pairing Passkey Notification */
        event = BSA_SEC_SP_KEY_NOTIF_EVT;
        break;
    case BSA_SEC_MSGID_SP_RMT_OOB_EVT: /* Simple Pairing Remote OOB Data request. */
        event = BSA_SEC_SP_RMT_OOB_EVT;
        break;
    case BSA_SEC_MSGID_LOCAL_OOB_DATA_EVT:      /* Response to read local OOB data request */
        event = BSA_SEC_LOCAL_OOB_DATA_EVT;
        break;
    case BSA_SEC_MSGID_SP_KEYPRESS_EVT: /* Simple Pairing Key press notification event. */
        event = BSA_SEC_SP_KEYPRESS_EVT;
        break;
    case BSA_SEC_MSGID_SUSPENDED_EVT: /* Link Suspended event */
        event = BSA_SEC_SUSPENDED_EVT;
        break;
    case BSA_SEC_MSGID_RESUMED_EVT: /* Link Resumed event */
        event = BSA_SEC_RESUMED_EVT;
        break;
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    case BSA_SEC_MSGID_BLE_KEY_EVT: /* BLE KEY event */
        event = BSA_SEC_BLE_KEY_EVT;
        break;
    case BSA_SEC_MSGID_BLE_PASSKEY_REQ_EVT: /* BLE PASSKEY req event */
        event = BSA_SEC_BLE_PASSKEY_REQ_EVT;
        break;
#endif
    default:
        APPL_TRACE_ERROR1("bsa_sec_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* Security events are directly sent to user's callback */
    if ((bsa_sec_cb.p_app_sec_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_sec_event_hdlr event:%d", event);

        bsa_sec_cb.p_app_sec_cback(event, p_data);
    }
}

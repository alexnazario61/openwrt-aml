/*****************************************************************************
**
**  Name:           bsa_mce_int.c
**
**  Description:    This is the internal interface file for mce part of
**                  the Bluetooth simplified API
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
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
** Function       bsa_mce_event_hdlr
**
** Description    Handle mce events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_mce_event_hdlr(int message_id, tBSA_MCE_MSG * p_buffer, int length)
{
    tBSA_MCE_EVT event = BSA_MCE_INVALID_EVT;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_MCE_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_MCE_OPEN_EVT;
        break;
    case BSA_MCE_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_MCE_CLOSE_EVT;
        break;
    case BSA_MCE_MSGID_DISABLE_EVT: /* Connection Disabled */
        event = BSA_MCE_DISABLE_EVT;
        break;
    case BSA_MCE_MSGID_ABORT_EVT: /* Connection Aborted */
        event = BSA_MCE_ABORT_EVT;
        break;
    case BSA_MCE_MSGID_START_EVT:
        event = BSA_MCE_START_EVT;
        break;
    case BSA_MCE_MSGID_STOP_EVT:
        event = BSA_MCE_STOP_EVT;
        break;
    case BSA_MCE_MSGID_SET_MSG_STATUS_EVT:
        event = BSA_MCE_SET_MSG_STATUS_EVT;
        break;
    case BSA_MCE_MSGID_SET_FOLDER_EVT:
        event = BSA_MCE_SET_FOLDER_EVT;
        break;
    case BSA_MCE_MSGID_FOLDER_LIST_EVT:
        event = BSA_MCE_FOLDER_LIST_EVT;
        break;
    case BSA_MCE_MSGID_MSG_LIST_EVT:
        event = BSA_MCE_MSG_LIST_EVT;
        break;
    case BSA_MCE_MSGID_GET_MSG_EVT:
        event = BSA_MCE_GET_MSG_EVT;
        break;
    case BSA_MCE_MSGID_PUSH_MSG_EVT:
        event = BSA_MCE_PUSH_MSG_EVT;
        break;
    case BSA_MCE_MSGID_PUSH_MSG_DATA_REQ_EVT:
        event = BSA_MCE_PUSH_MSG_DATA_REQ_EVT;
        break;
    case BSA_MCE_MSGID_MSG_PROG_EVT:
        event = BSA_MCE_MSG_PROG_EVT;
        break;
    case BSA_MCE_MSGID_OBEX_PUT_RSP_EVT:
        event = BSA_MCE_OBEX_PUT_RSP_EVT;
        break;
    case BSA_MCE_MSGID_OBEX_GET_RSP_EVT:
        event = BSA_MCE_OBEX_GET_RSP_EVT;
        break;
    case BSA_MCE_MSGID_NOTIF_EVT:
        event = BSA_MCE_NOTIF_EVT;
        break;
    case BSA_MCE_MSGID_NOTIF_REG_EVT:
        event = BSA_MCE_NOTIF_REG_EVT;
        break;
    case BSA_MCE_MSGID_UPDATE_INBOX_EVT:
        event = BSA_MCE_UPDATE_INBOX_EVT;
        break;
    case BSA_MCE_MSGID_INVALID_EVT:
        event = BSA_MCE_INVALID_EVT;
        break;
    case BSA_MCE_MSGID_GET_MAS_INS_INFO_EVT:
        event = BSA_MCE_GET_MAS_INS_INFO_EVT;
        break;
    case BSA_MCE_MSGID_GET_MAS_INSTANCES_EVT:
        event = BSA_MCE_GET_MAS_INSTANCES_EVT;
        break;
    case BSA_MCE_MSGID_MN_OPEN_EVT:
        event = BSA_MCE_MN_OPEN_EVT;
        break;
    case BSA_MCE_MSGID_MN_CLOSE_EVT:
        event = BSA_MCE_MN_CLOSE_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_mce_event_hdlr unknown message_id:%d",
            message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user callback */
    if ((bsa_mce_cb.p_app_mce_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_mce_event_hdlr event:%d", event);
        bsa_mce_cb.p_app_mce_cback(event, p_buffer);
    }

    /* Delete the callback after DISABLE EVT has been sent to the application */
    if(event == BSA_MCE_DISABLE_EVT)
        bsa_mce_cb.p_app_mce_cback = NULL;

}

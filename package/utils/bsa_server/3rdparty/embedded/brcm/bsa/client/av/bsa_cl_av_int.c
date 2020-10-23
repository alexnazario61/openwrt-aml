/*****************************************************************************
 **
 **  Name:           bsa_cl_av_int.c
 **
 **  Description:    This is the internal interface file for av part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2011-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"
#include "bsa_cl_av_int.h"


/*******************************************************************************
 **
 ** Function       bsa_cl_av_event_hdlr
 **
 ** Description    Handle av events
 **
 ** Parameters     message_id: identifier of the message
 **                p_buffer: message content
 **                length: message length
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_cl_av_event_hdlr(int message_id, tBSA_AV_MSG *p_buffer, int length)
{
    tBSA_AV_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_AV_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_AV_OPEN_EVT;
        break;

    case BSA_AV_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_AV_CLOSE_EVT;
        break;

    case BSA_AV_MSGID_START_EVT: /* Streaming started */
        event = BSA_AV_START_EVT;
        break;

    case BSA_AV_MSGID_STOP_EVT: /* Streaming stopped */
        event = BSA_AV_STOP_EVT;
        break;

    case BSA_AV_MSGID_RC_OPEN_EVT: /* Rc Connection Open */
        event = BSA_AV_RC_OPEN_EVT;
        break;

    case BSA_AV_MSGID_RC_CLOSE_EVT: /* Rc Connection Closed */
        event = BSA_AV_RC_CLOSE_EVT;
        break;

    case BSA_AV_MSGID_REMOTE_CMD_EVT: /* RC command */
        event = BSA_AV_REMOTE_CMD_EVT;
        break;

    case BSA_AV_MSGID_REMOTE_RSP_EVT: /* RC response */
        event = BSA_AV_REMOTE_RSP_EVT;
        break;

    case BSA_AV_MSGID_VENDOR_CMD_EVT: /* Vendor command */
        event = BSA_AV_VENDOR_CMD_EVT;
        break;

    case BSA_AV_MSGID_VENDOR_RSP_EVT: /* Vendor response */
        event = BSA_AV_VENDOR_RSP_EVT;
        break;

    case BSA_AV_MSGID_META_MSG_EVT: /* Meta data message */
        event = BSA_AV_META_MSG_EVT;
        break;

    case BSA_AV_MSGID_PENDING_EVT:  /* Connection Open pending */
        event = BSA_AV_PENDING_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_cl_av_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* Events are directly sent to user's callback */
    if ((bsa_av_cb.p_app_av_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_cl_av_event_hdlr event:%d", event);

        bsa_av_cb.p_app_av_cback(event, p_buffer);
    }
}


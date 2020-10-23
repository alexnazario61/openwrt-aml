/*****************************************************************************
 **
 **  Name:           bsa_hh_int.c
 **
 **  Description:    This is the internal interface file for hh part of
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
 ** Function       bsa_hh_event_hdlr
 **
 ** Description    Handle hh events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_hh_event_hdlr(int message_id, tBSA_HH_MSG * p_buffer, int length)
{
    tBSA_HH_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_HH_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_HH_OPEN_EVT;
        break;

    case BSA_HH_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_HH_CLOSE_EVT;
        break;

    case BSA_HH_MSGID_MIP_START_EVT: /* First MIP Connection Open*/
        event = BSA_HH_MIP_START_EVT;
        break;

    case BSA_HH_MSGID_MIP_STOP_EVT: /* Last MIP Connection Closed */
        event = BSA_HH_MIP_STOP_EVT;
        break;

    case BSA_HH_MSGID_GET_REPORT_EVT: /* Get Report Event */
        event = BSA_HH_GET_REPORT_EVT;
        break;

    case BSA_HH_MSGID_SET_REPORT_EVT: /* Set Report Event */
        event = BSA_HH_SET_REPORT_EVT;
        break;

    case BSA_HH_MSGID_GET_PROTO_EVT: /* Get Proto Event */
        event = BSA_HH_GET_PROTO_EVT;
        break;

    case BSA_HH_MSGID_SET_PROTO_EVT: /* Set Proto Event */
        event = BSA_HH_SET_PROTO_EVT;
        break;

    case BSA_HH_MSGID_GET_IDLE_EVT: /* Get Idle Event */
        event = BSA_HH_GET_IDLE_EVT;
        break;

    case BSA_HH_MSGID_SET_IDLE_EVT: /* Get Idle Event */
        event = BSA_HH_SET_IDLE_EVT;
        break;

    case BSA_HH_MSGID_GET_DSCPINFO_EVT: /* Get DscpInfo */
        event = BSA_HH_GET_DSCPINFO_EVT;
        break;

    case BSA_HH_MSGID_VC_UNPLUG_EVT: /* Virtually unplugged */
        event = BSA_HH_VC_UNPLUG_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_hh_event_hdlr unknown message_id:%d",
                message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user's callback */
    if ((bsa_hh_cb.p_app_hh_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_hh_event_hdlr event:%d", event);

        bsa_hh_cb.p_app_hh_cback(event, p_buffer);
    }
}


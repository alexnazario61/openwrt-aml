/*****************************************************************************
 **
 **  Name:           bsa_hd_int.c
 **
 **  Description:    This is the internal interface file for hd part of
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
 ** Function       bsa_hd_event_hdlr
 **
 ** Description    Handle hd events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_hd_event_hdlr(int message_id, tBSA_HD_MSG * p_buffer, int length)
{
    tBSA_HD_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_HD_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_HD_OPEN_EVT;
        break;

    case BSA_HD_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_HD_CLOSE_EVT;
        break;

    case BSA_HD_MSGID_UNPLUG_EVT: /* Virtually unplugged */
        event = BSA_HD_UNPLUG_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_hd_event_hdlr unknown message_id:%d",
                message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user's callback */
    if ((bsa_hd_cb.p_app_hd_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_hd_event_hdlr event:%d", event);

        bsa_hd_cb.p_app_hd_cback(event, p_buffer);
    }
}


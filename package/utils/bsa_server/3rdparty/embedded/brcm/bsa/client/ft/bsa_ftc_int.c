/*****************************************************************************
 **
 **  Name:           bsa_ftc_int.c
 **
 **  Description:    This is the internal interface file for FTS part of
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
 ** Function       bsa_ftc_event_hdlr
 **
 ** Description    Handle ftc events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_ftc_event_hdlr(int message_id, tBSA_FTC_MSG * p_buffer, int length)
{
    tBSA_FTS_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_FTC_MSGID_OPEN_EVT: /* Connection Open (for info) */
        event = BSA_FTC_OPEN_EVT;
        break;
    case BSA_FTC_MSGID_CLOSE_EVT: /* Connection Closed (for info)*/
        event = BSA_FTC_CLOSE_EVT;
        break;
    case BSA_FTC_MSGID_AUTH_EVT: /* Authorization */
        event = BSA_FTC_AUTH_EVT;
        break;
    case BSA_FTC_MSGID_PROGRESS_EVT: /* Progress Event (for info) */
        event = BSA_FTC_PROGRESS_EVT;
        break;
    case BSA_FTC_MSGID_LIST_EVT: /* Put complete Event (for info) */
        event = BSA_FTC_LIST_EVT;
        break;
    case BSA_FTC_MSGID_PUTFILE_EVT: /* Get complete Event (for info) */
        event = BSA_FTC_PUTFILE_EVT;
        break;
    case BSA_FTC_MSGID_GETFILE_EVT: /* Get complete Event (for info) */
        event = BSA_FTC_GETFILE_EVT;
        break;
    case BSA_FTC_MSGID_MOVE_EVT: /* Move complete Event (for info) */
        event = BSA_FTC_MOVE_EVT;
        break;
    case BSA_FTC_MSGID_REMOVE_EVT: /* Remove complete Event (for info) */
        event = BSA_FTC_REMOVE_EVT;
        break;
    case BSA_FTC_MSGID_COPY_EVT: /* Copy complete Event (for info) */
        event = BSA_FTC_COPY_EVT;
        break;
    case BSA_FTC_MSGID_CHDIR_EVT: /* Change dir complete Event (for info) */
        event = BSA_FTC_CHDIR_EVT;
        break;
    case BSA_FTC_MSGID_MKDIR_EVT: /* Make dir complete Event (for info) */
        event = BSA_FTC_MKDIR_EVT;
        break;
    default:
        APPL_TRACE_ERROR1("bsa_ftc_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* FTC events are directly sent to user's callback */
    if ((bsa_ftc_cb.p_app_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_API1("bsa_ftc_event_hdlr event:%d", event);

        bsa_ftc_cb.p_app_cback(event, p_buffer);
    }
}


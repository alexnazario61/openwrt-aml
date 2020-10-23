/*****************************************************************************
 **
 **  Name:           bsa_fts_int.c
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
 ** Function       bsa_fts_event_hdlr
 **
 ** Description    Handle fts events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_fts_event_hdlr(int message_id, tBSA_FTS_MSG * p_buffer, int length)
{
    tBSA_FTS_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_FTS_MSGID_OPEN_EVT: /* Connection Open (for info) */
        event = BSA_FTS_OPEN_EVT;
        break;
    case BSA_FTS_MSGID_CLOSE_EVT: /* Connection Closed (for info)*/
        event = BSA_FTS_CLOSE_EVT;
        break;
    case BSA_FTS_MSGID_AUTH_EVT: /* Authorization */
        event = BSA_FTS_AUTH_EVT;
        break;
    case BSA_FTS_MSGID_ACCESS_EVT: /* Access requested Event */
        event = BSA_FTS_ACCESS_EVT;
        break;
    case BSA_FTS_MSGID_PROGRESS_EVT: /* Progress Event (for info) */
        event = BSA_FTS_PROGRESS_EVT;
        break;
    case BSA_FTS_MSGID_PUT_CMPL_EVT: /* Put complete Event (for info) */
        event = BSA_FTS_PUT_CMPL_EVT;
        break;
    case BSA_FTS_MSGID_GET_CMPL_EVT: /* Get complete Event (for info) */
        event = BSA_FTS_GET_CMPL_EVT;
        break;
    case BSA_FTS_MSGID_DEL_CMPL_EVT: /* Delete complete Event (for info) */
        event = BSA_FTS_DEL_CMPL_EVT;
        break;
    default:
        APPL_TRACE_ERROR1("bsa_fts_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* FTS events are directly sent to user's callback */
    if ((bsa_fts_cb.p_app_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_fts_event_hdlr event:%d", event);

        bsa_fts_cb.p_app_cback(event, p_buffer);
    }
}


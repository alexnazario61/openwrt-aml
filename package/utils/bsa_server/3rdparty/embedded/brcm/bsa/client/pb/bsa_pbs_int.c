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
 ** Function       bsa_pbs_event_hdlr
 **
 ** Description    Handle pbs events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_pbs_event_hdlr(int message_id, tBSA_PBS_MSG * p_buffer, int length)
{
    tBSA_PBS_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_PBS_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_PBS_OPEN_EVT;
        break;

    case BSA_PBS_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_PBS_CLOSE_EVT;
        break;

    case BSA_PBS_MSGID_AUTH_EVT: /* OBEX Authentication event */
        event = BSA_PBS_AUTH_EVT;
        break;

    case BSA_PBS_MSGID_ACCESS_EVT: /* Access request Event */
        event = BSA_PBS_ACCESS_EVT;
        break;

    case BSA_PBS_MSGID_OPER_CMPL_EVT: /* Operation complete  Event */
        event = BSA_PBS_OPER_CMPL_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_pbs_event_hdlr unknown message_id:%d",
                message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user callback */
    if ((bsa_pbs_cb.p_app_pbs_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_pbs_event_hdlr event:%d", event);

        bsa_pbs_cb.p_app_pbs_cback(event, p_buffer);
    }
}


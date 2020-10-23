/*****************************************************************************
 **
 **  Name:           bsa_opc_int.c
 **
 **  Description:    This is the internal interface file for Object Push Server part of
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
 ** Function       bsa_opc_event_hdlr
 **
 ** Description    Handle opc events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_opc_event_hdlr(int message_id, tBSA_OPC_MSG *p_buffer, int length)
{
    tBSA_OPC_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_OPC_MSGID_OPEN_EVT: /* Connection Closed (for info) */
        event = BSA_OPC_OPEN_EVT;
        break;
    case BSA_OPC_MSGID_PROGRESS_EVT: /* Authorization */
        event = BSA_OPC_PROGRESS_EVT;
        break;
    case BSA_OPC_MSGID_OBJECT_EVT: /* Access requested Event */
        event = BSA_OPC_OBJECT_EVT;
        break;
    case BSA_OPC_MSGID_CLOSE_EVT: /* Progress Event (for info) */
        event = BSA_OPC_CLOSE_EVT;
        break;
    case BSA_OPC_MSGID_OBJECT_PSHD_EVT: /* Put complete Event (for info) */
        event = BSA_OPC_OBJECT_PSHD_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_opc_event_hdlr unknown message_id %d",
                message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user callback */
    if ((bsa_opc_cb.p_cback != NULL) &&  (call_cback == TRUE))
    {
        APPL_TRACE_EVENT1("bsa_opc_event_hdlr event:%d", event);

        bsa_opc_cb.p_cback(event, p_buffer);
    }

}


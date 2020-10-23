/*****************************************************************************
**
**  Name:           bsa_dg_int.c
**
**  Description:    This is the internal interface file for dg part of
**                  the Bluetooth simplified API
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
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
** Function       bsa_dg_event_hdlr
**
** Description    Handle dg events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_dg_event_hdlr(int message_id, tBSA_DG_MSG * p_buffer, int length)
{
    tBSA_DG_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_DG_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_DG_OPEN_EVT;
        break;

    case BSA_DG_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_DG_CLOSE_EVT;
        break;

    case BSA_DG_MSGID_FIND_SERVICE_EVT: /* Find Service event */
        event = BSA_DG_FIND_SERVICE_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_dg_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }
    /* events are directly sent to user's callback */
    if ((bsa_dg_cb.p_app_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_dg_event_hdlr event:%d", event);

        bsa_dg_cb.p_app_cback(event, p_buffer);
    }
}


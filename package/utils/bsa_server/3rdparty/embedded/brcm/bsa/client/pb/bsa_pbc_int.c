/*****************************************************************************
**
**  Name:           bsa_pbc_int.c
**
**  Description:    This is the internal interface file for pbc part of
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
** Function       bsa_pbc_event_hdlr
**
** Description    Handle pbc events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_pbc_event_hdlr(int message_id, tBSA_PBC_MSG * p_buffer, int length)
{
    tBSA_PBC_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_PBC_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_PBC_OPEN_EVT;
        break;

    case BSA_PBC_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_PBC_CLOSE_EVT;
        break;

    case BSA_PBC_MSGID_DISABLE_EVT: /* Connection Disabled */
        event = BSA_PBC_DISABLE_EVT;
        break;

    case BSA_PBC_MSGID_ABORT_EVT: /* Connection Aborted */
        event = BSA_PBC_ABORT_EVT;
        break;

    case BSA_PBC_MSGID_AUTH_EVT: /* OBEX Authentication event */
        event = BSA_PBC_AUTH_EVT;
        break;

    case BSA_PBC_MSGID_GET_EVT: /* Get event */
        event = BSA_PBC_GET_EVT;
        break;

    case BSA_PBC_MSGID_SET_EVT: /* Set event */
        event = BSA_PBC_SET_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_pbc_event_hdlr unknown message_id:%d", message_id);
        event = 0;
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user callback */
    if ((bsa_pbc_cb.p_app_pbc_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_pbc_event_hdlr event:%d", event);

        bsa_pbc_cb.p_app_pbc_cback(event, p_buffer);
    }

    /* Clear the callback after we send disable event to application. */
    if(event == BSA_PBC_DISABLE_EVT)
    {
        bsa_pbc_cb.p_app_pbc_cback = NULL;
    }
}

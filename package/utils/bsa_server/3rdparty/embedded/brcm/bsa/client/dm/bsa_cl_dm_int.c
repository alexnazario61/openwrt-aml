/*****************************************************************************
 **
 **  Name:           bsa_cl_dm_int.c
 **
 **  Description:    This is the internal interface file for DM part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2010-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_cl_dm_int.h"

tBSA_CL_DM_CB bsa_cl_dm_cb;

/*******************************************************************************
 **
 ** Function       bsa_cl_dm_event_hdlr
 **
 ** Description    Handle DM events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_cl_dm_event_hdlr(UINT16 message_id, tBSA_DM_MSG * p_buffer, int length)
{
    tBSA_DM_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_DM_MSGID_3D_ANNOUNCEMENT_EVT:
        event = BSA_DM_3D_ANNOUCEMENT_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_cl_dm_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* DM event are directly sent to user callback */
    if ((bsa_cl_dm_cb.callback != NULL) && (call_cback != FALSE))
    {
        bsa_cl_dm_cb.callback(event, p_buffer);
    }
}


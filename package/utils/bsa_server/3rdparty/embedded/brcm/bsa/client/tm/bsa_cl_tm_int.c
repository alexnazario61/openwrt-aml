/*****************************************************************************
 **
 **  Name:           bsa_cl_tm_int.c
 **
 **  Description:    This is the internal interface file for TM part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_cl_tm_int.h"

/*
 * Global variables
 */
tBSA_CL_TM_CB bsa_cl_tm_cb;

/*******************************************************************************
 **
 ** Function       bsa_cl_tm_event_hdlr
 **
 ** Description    Handle TM events
 **
 ** Parameters     message_id: Received Message Id
 **                p_buffer: pointer on received data
 **                length: received data length
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_cl_tm_event_hdlr(UINT16 message_id, tBSA_TM_MSG * p_buffer, int length)
{
    tBSA_TM_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_TM_MSGID_VSE_EVT: /* VSE event */
        event = BSA_TM_VSE_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_cl_tm_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* TM event are directly sent to user callback */
    if ((bsa_cl_tm_cb.p_callback != NULL) && (call_cback != FALSE))
    {
        bsa_cl_tm_cb.p_callback(event, p_buffer);
    }
}


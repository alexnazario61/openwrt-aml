/*****************************************************************************
**
**  Name:           bsa_hs_int.c
**
**  Description:    This is the internal interface file for Headset part of
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
** Function       bsa_hs_event_hdlr
**
** Description    Handle hs events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_hs_event_hdlr(int message_id, tBSA_HS_MSG * p_buffer, int length)
{
    tBSA_HS_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
    case BSA_HS_MSGID_CONN_EVT:                     /* Service Level connection Open*/
        event = BSA_HS_CONN_EVT;
        break;

    case BSA_HS_MSGID_AUDIO_OPEN_EVT:               /* SCO Audio open */
        event = BSA_HS_AUDIO_OPEN_EVT;
        break;

    case BSA_HS_MSGID_AUDIO_CLOSE_EVT:              /* SCO Audio close */
        event = BSA_HS_AUDIO_CLOSE_EVT;
        break;

    case BSA_HS_MSGID_OPEN_EVT:                    /* Connection Opened */
        event = BSA_HS_OPEN_EVT;
        break;

    case BSA_HS_MSGID_CLOSE_EVT:                    /* Connection Closed */
        event = BSA_HS_CLOSE_EVT;
        break;

    case BSA_HS_MSGID_CIND_EVT: /* AT cmd events */
    case BSA_HS_MSGID_CIEV_EVT:
    case BSA_HS_MSGID_RING_EVT:
    case BSA_HS_MSGID_CLIP_EVT:
    case BSA_HS_MSGID_BSIR_EVT:
    case BSA_HS_MSGID_BVRA_EVT:
    case BSA_HS_MSGID_CCWA_EVT:
    case BSA_HS_MSGID_CHLD_EVT:
    case BSA_HS_MSGID_VGM_EVT:
    case BSA_HS_MSGID_VGS_EVT:
    case BSA_HS_MSGID_BINP_EVT:
    case BSA_HS_MSGID_BTRH_EVT:
    case BSA_HS_MSGID_CNUM_EVT:
    case BSA_HS_MSGID_COPS_EVT:
    case BSA_HS_MSGID_CMEE_EVT:
    case BSA_HS_MSGID_CLCC_EVT:
    case BSA_HS_MSGID_UNAT_EVT:
    case BSA_HS_MSGID_OK_EVT:
    case BSA_HS_MSGID_ERROR_EVT:
    case BSA_HS_MSGID_BCS_EVT:
        event = message_id - BSA_HS_MSGID_CIND_EVT + BTA_HS_CIND_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_hs_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user's callback */
    if ((bsa_hs_cb.p_app_hs_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT2("bsa_hs_event_hdlr event:%d, Command ID: %d", event, p_buffer->val.num);

        bsa_hs_cb.p_app_hs_cback(event, p_buffer);
    }
}


/*****************************************************************************
**
**  Name:           bsa_ag_int.c
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
** Function       bsa_ag_event_hdlr
**
** Description    Handle AG events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_ag_event_hdlr(int message_id, tBSA_AG_MSG * p_buffer, int length)
{
    tBSA_AG_EVT event;
    BOOLEAN call_cback = TRUE;

    switch (message_id)
    {
        case BSA_AG_MSGID_OPEN_EVT:                     /* RFCOM Level connection Open*/
            event = BSA_AG_OPEN_EVT;
            break;

        case BSA_AG_MSGID_CONN_EVT:                     /* Service Level connection Open*/
            event = BSA_AG_CONN_EVT;
            break;

        case BSA_AG_MSGID_AUDIO_OPEN_EVT:               /* SCO Audio open */
            event = BSA_AG_AUDIO_OPEN_EVT;
            break;

        case BSA_AG_MSGID_AUDIO_CLOSE_EVT:              /* SCO Audio close */
            event = BSA_AG_AUDIO_CLOSE_EVT;
            break;

        case BSA_AG_MSGID_CLOSE_EVT:                    /* Connection Closed */
            event = BSA_AG_CLOSE_EVT;
            break;

        case BSA_AG_MSGID_AT_SPK_EVT:                     /* AT commands */
            event = BSA_AG_SPK_EVT;
            break;

        case BSA_AG_MSGID_AT_MIC_EVT:
            event = BSA_AG_MIC_EVT;
            break;
        case BSA_AG_MSGID_AT_CKPD_EVT:
            event = BSA_AG_AT_CKPD_EVT;
            break;
        case BSA_AG_MSGID_AT_A_EVT:
            event = BSA_AG_AT_A_EVT;
            break;
        case BSA_AG_MSGID_AT_D_EVT:
            event = BSA_AG_AT_D_EVT;
            break;
        case BSA_AG_MSGID_AT_CHLD_EVT:
            event = BSA_AG_AT_CHLD_EVT;
            break;
        case BSA_AG_MSGID_AT_CHUP_EVT:
            event = BSA_AG_AT_CHUP_EVT;
            break;
        case BSA_AG_MSGID_AT_CIND_EVT:
            event = BSA_AG_AT_CIND_EVT;
            break;
        case BSA_AG_MSGID_AT_VTS_EVT:
            event = BSA_AG_AT_VTS_EVT;
            break;
        case BSA_AG_MSGID_AT_BINP_EVT:
            event = BSA_AG_AT_BINP_EVT;
            break;
        case BSA_AG_MSGID_AT_BLDN_EVT:
            event = BSA_AG_AT_BLDN_EVT;
            break;
        case BSA_AG_MSGID_AT_BVRA_EVT:
            event = BSA_AG_AT_BVRA_EVT;
            break;
        case BSA_AG_MSGID_AT_NREC_EVT:
            event = BSA_AG_AT_NREC_EVT;
            break;
        case BSA_AG_MSGID_AT_CNUM_EVT:
            event = BSA_AG_AT_CNUM_EVT;
            break;
        case BSA_AG_MSGID_AT_BTRH_EVT:
            event = BSA_AG_AT_BTRH_EVT;
            break;
        case BSA_AG_MSGID_AT_CLCC_EVT:
            event = BSA_AG_AT_CLCC_EVT;
            break;
        case BSA_AG_MSGID_AT_COPS_EVT:
            event = BSA_AG_AT_COPS_EVT;
            break;
        case BSA_AG_MSGID_AT_UNAT_EVT:
            event = BSA_AG_AT_UNAT_EVT;
            break;
        case BSA_AG_MSGID_AT_CBC_EVT:
            event = BSA_AG_AT_CBC_EVT;
            break;
        case BSA_AG_MSGID_AT_BAC_EVT:
            event = BSA_AG_AT_BAC_EVT;
            break;
        case BSA_AG_MSGID_AT_BCS_EVT:
            event = BSA_AG_AT_BCS_EVT;
            break;

        default:
            APPL_TRACE_ERROR1("bsa_ag_event_hdlr unknown message_id:%d", message_id);
            call_cback = FALSE;
            break;
    }

    /* events are directly sent to user callback */
    if ((bsa_ag_cb.p_app_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_ag_event_hdlr event:%d", event);

        bsa_ag_cb.p_app_cback(event, p_buffer);
    }
}


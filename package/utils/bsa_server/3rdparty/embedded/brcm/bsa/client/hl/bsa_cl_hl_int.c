/*****************************************************************************
 **
 **  Name:           bsa_hl_int.c
 **
 **  Description:    This is the internal interface file for Health part of
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

#include "bsa_cl_hl_int.h"

/*******************************************************************************
 **
 ** Function       bsa_cl_hl_event_hdlr
 **
 ** Description    Handle Health events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_cl_hl_event_hdlr(int message_id, tBSA_HL_MSGID_EVT *p_buffer, UINT16 length)
{
    tBSA_HL_MSG bsa_hl_msg;
    tBSA_HL_EVT event = 0xff;
    BOOLEAN call_cback = TRUE;
    tBSA_HL_MSGID_SDP_QUERY_APP_EVT *p_sdp;
    tBSA_HL_MSGID_SDP_QUERY_APP_MDEP_EVT *p_sdp_mdep;
    tBSA_HL_SDP_REC *p_app;

    switch (message_id)
    {
    case BSA_HL_MSGID_SDP_QUERY_APP_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_SDP_QUERY_APP_EVT received");
        if (bsa_hl_cb.p_sdp == NULL)
        {
            APPL_TRACE_ERROR0("ERROR bsa_hl_event_hdlr internal buffer null");
            return;
        }
        p_sdp = &p_buffer->sdp_query_app;
        bsa_hl_cb.p_sdp->status = p_sdp->status;
        bsa_hl_cb.p_sdp->num_records = p_sdp->num_records;
        bdcpy(bsa_hl_cb.p_sdp->bd_addr, p_sdp->bd_addr);
        p_app = &bsa_hl_cb.p_sdp->sdp_records[p_sdp->rec_index];

        p_app->ctrl_psm = p_sdp->ctrl_psm;
        p_app->mcap_sup_features = p_sdp->mcap_sup_features;
        p_app->num_mdeps = p_sdp->num_mdeps;
        strncpy(p_app->service_name, p_sdp->service_name,
                BSA_HL_SERVICE_NAME_LEN_MAX);
        strncpy(p_app->service_desc, p_sdp->service_desc,
                BSA_HL_SERVICE_DESC_LEN_MAX);
        strncpy(p_app->provider_name, p_sdp->provider_name,
                BSA_HL_PROVIDER_NAME_LEN_MAX);

        /* If error or if this is the last app */
        if ((p_sdp->num_records == 0) ||
            (p_sdp->status != BSA_SUCCESS))
        {
            event = BSA_HL_SDP_QUERY_EVT;
            /* copy the full SDP record in the callback data */
            memcpy(&bsa_hl_msg.sdp_query, bsa_hl_cb.p_sdp, sizeof(bsa_hl_msg.sdp_query));

            /* Free the temporary GKI buffer */
            GKI_freebuf(bsa_hl_cb.p_sdp);

            /* Mark that the temporary GKI buffer is free (to allow new SDP) */
            bsa_hl_cb.p_sdp = NULL;
        }
        else
        {
            call_cback = FALSE;
        }
        break;

    case BSA_HL_MSGID_SDP_QUERY_APP_MDEP_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_SDP_QUERY_APP_MDEP_EVT received");
        if (bsa_hl_cb.p_sdp == NULL)
        {
            APPL_TRACE_ERROR0("ERROR bsa_hl_event_hdlr internal buffer null");
            return;
        }
        p_sdp_mdep = &p_buffer->sdp_query_app_mdep;
        p_app = &bsa_hl_cb.p_sdp->sdp_records[p_sdp_mdep->rec_index];
        memcpy(&p_app->mdep[p_sdp_mdep->mdep_index], &p_sdp_mdep->mdep_cfg,
                sizeof(tBSA_HL_MDEP_CFG_REG));

        /* If this is the last app_record and the last MDEP */
        if (((p_sdp_mdep->rec_index + 1) == bsa_hl_cb.p_sdp->num_records) &&
            ((p_sdp_mdep->mdep_index + 1) == bsa_hl_cb.p_sdp->sdp_records[p_sdp_mdep->rec_index].num_mdeps))
        {
            event = BSA_HL_SDP_QUERY_EVT;
            /* copy the full SDP record in the callback data */
            memcpy(&bsa_hl_msg.sdp_query, bsa_hl_cb.p_sdp, sizeof(bsa_hl_msg.sdp_query));

            /* Free the temporary GKI buffer */
            GKI_freebuf(bsa_hl_cb.p_sdp);

            /* Mark that the temporary GKI buffer is free (to allow new SDP) */
            bsa_hl_cb.p_sdp = NULL;
        }
        else
        {
            call_cback = FALSE;
        }
        break;

    case BSA_HL_MSGID_OPEN_REQ_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_OPEN_REQ_EVT received");
        event = BSA_HL_OPEN_REQ_EVT;
        memcpy(&bsa_hl_msg.open_req, p_buffer, sizeof(bsa_hl_msg.open_req));
        break;

    case BSA_HL_MSGID_OPEN_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_OPEN_EVT received");
        event = BSA_HL_OPEN_EVT;
        memcpy(&bsa_hl_msg.open, p_buffer, sizeof(bsa_hl_msg.open));
        break;

    case BSA_HL_MSGID_CLOSE_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_CLOSE_EVT received");
        event = BSA_HL_CLOSE_EVT;
        memcpy(&bsa_hl_msg.close, p_buffer, sizeof(bsa_hl_msg.close));
        break;

    case BSA_HL_MSGID_RECONNECT_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_RECONNECT_EVT received");
        event = BSA_HL_RECONNECT_EVT;
        memcpy(&bsa_hl_msg.reconnect, p_buffer, sizeof(bsa_hl_msg.reconnect));
        break;

    case BSA_HL_MSGID_SEND_DATA_CFM_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_SEND_DATA_CFM_EVT received");
        event = BSA_HL_SEND_DATA_CFM_EVT;
        memcpy(&bsa_hl_msg.send_data_cfm, p_buffer, sizeof(bsa_hl_msg.send_data_cfm));
        break;

    case BSA_HL_MSGID_SAVE_MDL_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_CO_SAVE_MDL_EVT received");
        event = BSA_HL_SAVE_MDL_EVT;
        memcpy(&bsa_hl_msg.save_mdl, p_buffer, sizeof(bsa_hl_msg.save_mdl));
        break;

    case BSA_HL_MSGID_DELETE_MDL_EVT:
        APPL_TRACE_DEBUG0("BSA_HL_MSGID_DELETE_MDL_EVT received");
        event = BSA_HL_DELETE_MDL_EVT;
        memcpy(&bsa_hl_msg.save_mdl, p_buffer, sizeof(bsa_hl_msg.save_mdl));
        break;

    default:
        APPL_TRACE_ERROR1("ERROR bsa_hl_event_hdlr unknown message_id:%d\n",
                message_id);
        call_cback = FALSE;
        break;
    }

    /* Some HL events are sent directly to user callback */
    if ((bsa_hl_cb.p_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT2("bsa_hl_event_hdlr message_id:%d => event:%d\n",
                message_id, event);
        bsa_hl_cb.p_cback(event, &bsa_hl_msg);
    }
}


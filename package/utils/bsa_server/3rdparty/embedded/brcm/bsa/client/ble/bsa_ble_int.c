/*****************************************************************************
 **
 **  Name:           bsa_ble_int.c
 **
 **  Description:    This is the internal interface file for ble part of
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
#include "bsa_ble_int.h"

/*******************************************************************************
 **
 ** Function       bsa_ble_event_hdlr
 **
 ** Description    Handle ble events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_ble_event_hdlr(int message_id, tBSA_BLE_MSG * p_buffer, int length)
{
    tBSA_BLE_EVT event;
    BOOLEAN call_cback = TRUE;
    BOOLEAN ble_client = TRUE;


    switch (message_id)
    {
    case BSA_BLE_CL_MSGID_OPEN_EVT: /* Connection open */
        event = BSA_BLE_CL_OPEN_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_SEARCH_RES_EVT: /* Search result */
        event = BSA_BLE_CL_SEARCH_RES_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_SEARCH_CMPL_EVT: /* Search complete */
        event = BSA_BLE_CL_SEARCH_CMPL_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_READ_EVT: /* Read complete */
        event = BSA_BLE_CL_READ_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_WRITE_EVT: /* Write complete */
        event = BSA_BLE_CL_WRITE_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_CLOSE_EVT: /* Close complete */
        event = BSA_BLE_CL_CLOSE_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_DEREGISTER_EVT: /* app deregistered */
        event = BSA_BLE_CL_DEREGISTER_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_NOTIF_EVT: /* Notification to application */
        event = BSA_BLE_CL_NOTIF_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_CACHE_SAVE_EVT:
        event = BSA_BLE_CL_CACHE_SAVE_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_CL_MSGID_CACHE_LOAD_EVT:
        event = BSA_BLE_CL_CACHE_LOAD_EVT;
        ble_client = TRUE;
        break;

    case BSA_BLE_SE_MSGID_DEREGISTER_EVT: /* app deregistered */
        event = BSA_BLE_SE_DEREGISTER_EVT;
        ble_client = FALSE;
        break;

    case BSA_BLE_SE_MSGID_CREATE_EVT: /* service created */
        event = BSA_BLE_SE_CREATE_EVT;
        ble_client = FALSE;
        break;

    case BSA_BLE_SE_MSGID_ADDCHAR_EVT: /* char added */
        event = BSA_BLE_SE_ADDCHAR_EVT;
        ble_client = FALSE;
        break;

    case BSA_BLE_SE_MSGID_START_EVT: /* service started */
        event = BSA_BLE_SE_START_EVT;
        ble_client = FALSE;
        break;

    case BSA_BLE_SE_MSGID_READ_EVT:
        event = BSA_BLE_SE_READ_EVT;
        ble_client = FALSE;
        break;

    case BSA_BLE_SE_MSGID_WRITE_EVT:
        event = BSA_BLE_SE_WRITE_EVT;
        ble_client = FALSE;
        break;

    case BSA_BLE_SE_MSGID_CONNECT_EVT:
        event = BSA_BLE_SE_CONNECT_EVT;
        ble_client = FALSE;
        break;

    case BSA_BLE_SE_MSGID_DISCONNECT_EVT:
        event = BSA_BLE_SE_DISCONNECT_EVT;
        ble_client = FALSE;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_ble_event_hdlr unknown message_id:%d",
                message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user's callback */
    if( ble_client == TRUE)
    {
        if ((bsa_ble_cb.p_app_ble_client_cback != NULL) && (call_cback != FALSE))
        {
            bsa_ble_cb.p_app_ble_client_cback(event, p_buffer);
        }
    }
    else /* this is for ble server */
    {
        if ((bsa_ble_cb.p_app_ble_server_cback != NULL) && (call_cback != FALSE))
        {
            bsa_ble_cb.p_app_ble_server_cback(event, p_buffer);
        }
    }
}


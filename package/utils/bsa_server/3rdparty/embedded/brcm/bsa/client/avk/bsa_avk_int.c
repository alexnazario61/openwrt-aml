/*****************************************************************************
**
**  Name:           bsa_avk_int.c
**
**  Description:    This is the internal interface file for avk module of
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
** Function       bsa_avk_event_hdlr
**
** Description    Handle server avk events
**
** Parameters   message_id : Server message id 
**                     p_buffer : pointer on a buffer containning data associated with the message
**                     length : length of the message 
**
** Returns        void
**
*******************************************************************************/
void bsa_avk_event_hdlr(int message_id, tBSA_AVK_MSG * p_buffer, UINT16 length)
{
    tBSA_AVK_EVT event;
    BOOLEAN call_cback = TRUE;

    APPL_TRACE_EVENT1("bsa_avk_event_hdlr message_id:%d", message_id);

    switch (message_id)
    {
    case BSA_AVK_MSGID_OPEN_EVT: /* Connection Open*/
        event = BSA_AVK_OPEN_EVT;
        break;

    case BSA_AVK_MSGID_CLOSE_EVT: /* Connection Closed */
        event = BSA_AVK_CLOSE_EVT;
        break;

    case BSA_AVK_MSGID_START_EVT: /* Streaming started */
        event = BSA_AVK_START_EVT;
        break;

    case BSA_AVK_MSGID_STOP_EVT: /* Streaming stopped */
        event = BSA_AVK_STOP_EVT;
        break;

    case BSA_AVK_MSGID_RC_OPEN_EVT: /* RC Connection Open */
        event = BSA_AVK_RC_OPEN_EVT;
        break;

    case BSA_AVK_MSGID_RC_CLOSE_EVT: /* RC Connection Closed */
        event = BSA_AVK_RC_CLOSE_EVT;
        break;

    case BSA_AVK_MSGID_REMOTE_RSP_EVT: /* RC Response */
        event = BSA_AVK_REMOTE_RSP_EVT;
        break;

    case BSA_AVK_MSGID_VENDOR_CMD_EVT: /* Vendor Commmand */
        event = BSA_AVK_VENDOR_CMD_EVT;
        break;

    case BSA_AVK_MSGID_VENDOR_RSP_EVT: /* Vendor Response */
        event = BSA_AVK_VENDOR_RSP_EVT;
        break;

    case BSA_AVK_MSGID_CP_INFO_EVT: /* Content protection info notification */
        event = BSA_AVK_CP_INFO_EVT;
        break;

    case BSA_AVK_MSGID_META_REGISTER_NOTIFICATION_RSP_EVT:
        event = BSA_AVK_REGISTER_NOTIFICATION_EVT;
        break;

    case BSA_AVK_MSGID_META_LIST_PLAYER_APP_ATTR_RSP_EVT:
        event = BSA_AVK_LIST_PLAYER_APP_ATTR_EVT;
        break;

    case BSA_AVK_MSGID_META_LIST_PLAYER_APP_VALUES_RSP_EVT:
        event = BSA_AVK_LIST_PLAYER_APP_VALUES_EVT;
        break;

    case BSA_AVK_MSGID_META_GET_PLAYER_APP_VALUE_RSP_EVT:
        event = BSA_AVK_GET_PLAYER_APP_VALUE_EVT;
        break;

    case BSA_AVK_MSGID_META_SET_PLAYER_APP_VALUE_RSP_EVT:
        event = BSA_AVK_SET_PLAYER_APP_VALUE_EVT;
        break;

    case BSA_AVK_MSGID_META_GET_PLAYER_ATTR_TEXT_RSP_EVT:
        event = BSA_AVK_GET_PLAYER_ATTR_TEXT_EVT;
        break;

    case BSA_AVK_MSGID_META_GET_PLAYER_VALUE_TEXT_RSP_EVT:
        event = BSA_AVK_GET_PLAYER_ATTR_VALUE_EVT;
        break;

    case BSA_AVK_MSGID_META_GET_ELEMENT_ATTR_RSP_EVT:
        event = BSA_AVK_GET_ELEM_ATTR_EVT;
        break;

    case BSA_AVK_MSGID_META_GET_PLAY_STATUS_RSP_EVT:
        event = BSA_AVK_GET_PLAY_STATUS_EVT;
        break;

    case BSA_AVK_MSGID_META_SET_ADDRESSED_PLAYER_RSP_EVT:
        event = BSA_AVK_SET_ADDRESSED_PLAYER_EVT;
        break;

    case BSA_AVK_MSGID_META_SET_BR_PLAYER_RSP_EVT:
        event = BSA_AVK_SET_BROWSED_PLAYER_EVT;
        break;

    case BSA_AVK_MSGID_META_GET_FOLDER_ITEMS_RSP_EVT:
        event = BSA_AVK_GET_FOLDER_ITEMS_EVT;
        break;

    case BSA_AVK_MSGID_META_GET_ITEM_ATTR_RSP_EVT:
        event = BSA_AVK_GET_ITEM_ATTR_EVT;
        break;

    case BSA_AVK_MSGID_META_CHANGE_PATH_RSP_EVT:
        event = BSA_AVK_CHANGE_PATH_EVT;
        break;

    case BSA_AVK_MSGID_META_PLAY_ITEM_RSP_EVT:
        event = BSA_AVK_PLAY_ITEM_EVT;
        break;

    case BSA_AVK_MSGID_META_ADD_TO_NOW_PLAYING_RSP_EVT:
        event = BSA_AVK_ADD_TO_NOW_PLAYING_EVT;
        break;

    default:
        APPL_TRACE_ERROR1("bsa_avk_event_hdlr unknown message_id:%d",
                message_id);
        call_cback = FALSE;
        break;
    }
    /* events are directly sent to user's callback */
    if ((bsa_avk_cb.p_app_avk_cback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_avk_event_hdlr event:%d", event);

        bsa_avk_cb.p_app_avk_cback(event, p_buffer);
    }
}


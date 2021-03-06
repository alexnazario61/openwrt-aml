/*****************************************************************************
**
**  Name:           bsa_mce_int.h
**
**  Description:    Contains private BSA MAP client data
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
*****************************************************************************/

#ifndef BSA_MCE_INT_H
#define BSA_MCE_INT_H

#include "bsa_mce_api.h"

#define BSA_MCE_APP_ID  1

/*
* MCE Message definition
*/
enum
{
    BSA_MCE_MSGID_ENABLE_CMD = BSA_MCE_MSGID_FIRST,
    BSA_MCE_MSGID_DISABLE_CMD,
    BSA_MCE_MSGID_MN_START_CMD,
    BSA_MCE_MSGID_MN_STOP_CMD,
    BSA_MCE_MSGID_OPEN_CMD,
    BSA_MCE_MSGID_CLOSE_CMD,
    BSA_MCE_MSGID_CANCEL_CMD,
    BSA_MCE_MSGID_UPDATE_INBOX_CMD,
    BSA_MCE_MSGID_NOTIFY_REG_CMD,
    BSA_MCE_MSGID_PUSH_MSG_CMD,
    BSA_MCE_MSGID_ABORT_CMD,
    BSA_MCE_MSGID_GET_CMD,
    BSA_MCE_MSGID_SET_CMD,
    BSA_MCE_MSGID_LAST_CMD = BSA_MCE_MSGID_SET_CMD,

    BSA_MCE_MSGID_START_EVT,
    BSA_MCE_MSGID_STOP_EVT,
    BSA_MCE_MSGID_OPEN_EVT,
    BSA_MCE_MSGID_CLOSE_EVT,
    BSA_MCE_MSGID_DISABLE_EVT,
    BSA_MCE_MSGID_MN_OPEN_EVT,
    BSA_MCE_MSGID_MN_CLOSE_EVT,
    BSA_MCE_MSGID_NOTIF_EVT,
    BSA_MCE_MSGID_NOTIF_REG_EVT,
    BSA_MCE_MSGID_UPDATE_INBOX_EVT,
    BSA_MCE_MSGID_SET_MSG_STATUS_EVT,
    BSA_MCE_MSGID_SET_FOLDER_EVT,
    BSA_MCE_MSGID_FOLDER_LIST_EVT,
    BSA_MCE_MSGID_MSG_LIST_EVT,
    BSA_MCE_MSGID_GET_MSG_EVT,
    BSA_MCE_MSGID_PUSH_MSG_EVT,
    BSA_MCE_MSGID_PUSH_MSG_DATA_REQ_EVT,
    BSA_MCE_MSGID_MSG_PROG_EVT,
    BSA_MCE_MSGID_OBEX_PUT_RSP_EVT,
    BSA_MCE_MSGID_OBEX_GET_RSP_EVT,
    BSA_MCE_MSGID_ABORT_EVT,
    BSA_MCE_MSGID_GET_MAS_INS_INFO_EVT,
    BSA_MCE_MSGID_GET_MAS_INSTANCES_EVT,
    BSA_MCE_MSGID_INVALID_EVT, /* Invalid event */
    BSA_MCE_MSGID_LAST_EVT = BSA_MCE_MSGID_INVALID_EVT
};

/*
* Structures used for parameters (transport)
*/

/*
* MCE Enable
*/
typedef struct
{
    UINT8 app_id;
} tBSA_MCE_MSGID_ENABLE_CMD_REQ;

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;

} tBSA_MCE_MSGID_STATUS_RSP;


typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_ENABLE_CMD_RSP;

/*
* MCE Disable
*/
typedef tBSA_MCE_DISABLE tBSA_MCE_MSGID_DISABLE_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_DISABLE_CMD_RSP;
typedef tBSA_MCE_DISABLE_MSG tBSA_MCE_MSGID_DISABLE_RSP_EVT;

/*
* MCE MN Start
*/
typedef tBSA_MCE_MN_START tBSA_MCE_MSGID_MN_START_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_MN_START_CMD_RSP;
typedef tBSA_MCE_MN_START_STOP_MSG tBSA_MCE_MSGID_MN_START_RSP_EVT;

/*
* MCE MN Stop
*/
typedef tBSA_MCE_MN_STOP tBSA_MCE_MSGID_MN_STOP_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_MN_STOP_CMD_RSP;
typedef tBSA_MCE_MN_START_STOP_MSG tBSA_MCE_MSGID_MN_STOP_RSP_EVT;

/*
* MCE Open
*/
typedef tBSA_MCE_OPEN tBSA_MCE_MSGID_OPEN_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_OPEN_CMD_RSP;
typedef tBSA_MCE_OPEN_CLOSE_MSG tBSA_MCE_MSGID_OPEN_RSP_EVT;

/*
* MCE Close
*/
typedef tBSA_MCE_CLOSE tBSA_MCE_MSGID_CLOSE_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_CLOSE_CMD_RSP;
typedef tBSA_MCE_OPEN_CLOSE_MSG tBSA_MCE_MSGID_CLOSE_RSP_EVT;

/*
* MCE Abort
*/
typedef tBSA_MCE_ABORT tBSA_MCE_MSGID_ABORT_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_ABORT_CMD_RSP;
typedef tBSA_MCE_ABORT_MSG tBSA_MCE_MSGID_ABORT_RSP_EVT;

/* MCE Cancel */
typedef tBSA_MCE_CANCEL tBSA_MCE_MSGID_CANCEL_CMD_REQ;

/*
* MCE Get Rsp
*/
typedef tBSA_MCE_GET tBSA_MCE_MSGID_GET_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_GET_CMD_RSP;


/*
* MCE Set Rsp
*/
typedef tBSA_MCE_SET tBSA_MCE_MSGID_SET_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_SET_CMD_RSP;

typedef tBSA_MCE_SET_MSG_STATUS_MSG     tBSA_MCE_MSGID_SET_MSG_STATUS_EVT;
typedef tBSA_MCE_SET_FOLDER_MSG     tBSA_MCE_MSGID_SET_FOLDER_EVT;
typedef tBSA_MCE_LIST_DATA_MSG  tBSA_MCE_MSGID_FOLDER_LIST_EVT;
typedef tBSA_MCE_LIST_DATA_MSG  tBSA_MCE_MSGID_MSG_LIST_EVT;
typedef tBSA_MCE_GETMSG_MSG     tBSA_MCE_MSGID_GET_MSG_EVT;

typedef tBSA_MCE_MSG_PROG_MSG   tBSA_MCE_MSGID_MSG_PROG_EVT;
typedef tBSA_MCE_OBEX_RSP_MSG   tBSA_MCE_MSGID_OBEX_PUT_RSP_EVT;
typedef tBSA_MCE_OBEX_RSP_MSG   tBSA_MCE_MSGID_OBEX_GET_RSP_EVT;
typedef tBSA_MCE_GET_MAS_INS_INFO_MSG   tBSA_MCE_MSGID_GET_MAS_INS_INFO_EVT;
typedef tBSA_MCE_GET_MAS_INSTANCES_MSG   tBSA_MCE_MSGID_GET_MAS_INSTANCES_EVT;

/*
* MCE Update Inbox
*/
typedef tBSA_MCE_UPDATEINBOX tBSA_MCE_MSGID_UPDATE_INBOX_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_UPDATE_INBOX_CMD_RSP;
typedef tBSA_MCE_UPDATEINBOX_MSG tBSA_MCE_MSGID_UPDATE_INBOX_EVT;

/*
* MCE Notify Reg Rsp
*/
typedef tBSA_MCE_NOTIFYREG tBSA_MCE_MSGID_NOTIFY_REG_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_NOTIFY_REG_CMD_RSP;
typedef tBSA_MCE_NOTIFREG_MSG tBSA_MCE_MSGID_NOTIFY_REG_EVT;

/*
* MCE Push Msg Rsp
*/
typedef tBSA_MCE_PUSHMSG tBSA_MCE_MSGID_PUSH_MSG_CMD_REQ;
typedef tBSA_MCE_MSGID_STATUS_RSP tBSA_MCE_MSGID_PUSH_MSG_CMD_RSP;
typedef tBSA_MCE_PUSHMSG_MSG tBSA_MCE_MSGID_PUSH_MSG_EVT;


/*
* Structure (used by client)
*/
typedef struct
{
    tBSA_MCE_CBACK *p_app_mce_cback;
} tBSA_MCE_CB;

/*
* Global variables (used by client)
*/
extern tBSA_MCE_CB bsa_mce_cb;

/*******************************************************************************
**
** Function       bsa_mce_event_hdlr
**
** Description    Handle MCE events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_mce_event_hdlr(int message_id, tBSA_MCE_MSG *p_data, int length);

#endif

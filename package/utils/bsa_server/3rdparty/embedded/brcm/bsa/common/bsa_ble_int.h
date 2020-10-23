/*****************************************************************************
 **
 **  Name:           bsa_ble_int.h
 **
 **  Description:    Contains private BSA BLE data
 **
 **  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_BLE_INT_H
#define BSA_BLE_INT_H

#include "bsa_ble_api.h"


/*
 * HH Message definition
 */
enum
{
    BSA_BLE_MSGID_ENABLE_CMD = BSA_BLE_MSGID_FIRST,
    BSA_BLE_MSGID_DISABLE_CMD,
    BSA_BLE_MSGID_WAKE_CFG_CMD,
    BSA_BLE_MSGID_WAKE_ENABLE_CMD,
    BSA_BLE_CL_MSGID_REGISTER_CMD,
    BSA_BLE_CL_MSGID_DEREGISTER_CMD,
    BSA_BLE_CL_MSGID_OPEN_CMD,
    BSA_BLE_CL_MSGID_CLOSE_CMD,
    BSA_BLE_CL_MSGID_READ_CMD,
    BSA_BLE_CL_MSGID_WRITE_CMD,
    BSA_BLE_CL_MSGID_SEARCH_CMD,
    BSA_BLE_CL_MSGID_NOTIFREG_CMD,
    BSA_BLE_CL_MSGID_NOTIFDEREG_CMD,
    BSA_BLE_CL_MSGID_CACHE_LOAD_CMD,
    BSA_BLE_CL_MSGID_INDCONF_CMD,

    BSA_BLE_SE_MSGID_REGISTER_CMD,
    BSA_BLE_SE_MSGID_DEREGISTER_CMD,
    BSA_BLE_SE_MSGID_CREATE_CMD,
    BSA_BLE_SE_MSGID_ADDCHAR_CMD,
    BSA_BLE_SE_MSGID_SENDIND_CMD,
    BSA_BLE_SE_MSGID_SENDRSP_CMD,
    BSA_BLE_SE_MSGID_START_CMD,
    BSA_BLE_MSGID_LAST_CMD = BSA_BLE_SE_MSGID_START_CMD,

    BSA_BLE_CL_MSGID_DEREGISTER_EVT,
    BSA_BLE_CL_MSGID_OPEN_EVT,
    BSA_BLE_CL_MSGID_CLOSE_EVT,
    BSA_BLE_CL_MSGID_READ_EVT,
    BSA_BLE_CL_MSGID_WRITE_EVT,
    BSA_BLE_CL_MSGID_SEARCH_RES_EVT,
    BSA_BLE_CL_MSGID_SEARCH_CMPL_EVT,
    BSA_BLE_CL_MSGID_NOTIF_EVT,
    BSA_BLE_CL_MSGID_CACHE_SAVE_EVT,
    BSA_BLE_CL_MSGID_CACHE_LOAD_EVT,

    BSA_BLE_SE_MSGID_DEREGISTER_EVT,
    BSA_BLE_SE_MSGID_CREATE_EVT,
    BSA_BLE_SE_MSGID_ADDCHAR_EVT,
    BSA_BLE_SE_MSGID_READ_EVT,
    BSA_BLE_SE_MSGID_WRITE_EVT,
    BSA_BLE_SE_MSGID_CONNECT_EVT,
    BSA_BLE_SE_MSGID_DISCONNECT_EVT,
    BSA_BLE_SE_MSGID_START_EVT,
    BSA_BLE_MSGID_LAST_EVT = BSA_BLE_SE_MSGID_START_EVT
};

/*
 * Structures used for parameters (transport)
 */
typedef struct
{
    tBSA_STATUS status;
} tBSA_BLE_MSGID_STATUS_RSP;

/*
 * BLE Enable
 */
typedef tBSA_BLE_ENABLE tBSA_BLE_MSGID_ENABLE_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_MSGID_ENABLE_CMD_RSP;

/*
 * BLE disable
 */
typedef tBSA_BLE_DISABLE tBSA_BLE_MSGID_DISABLE_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_MSGID_DISABLE_CMD_RSP;

/*
* BLE Wake Cfg
*/
typedef tBSA_BLE_WAKE_CFG tBSA_BLE_MSGID_WAKE_CFG_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_MSGID_WAKE_CFG_CMD_RSP;

/*
* BLE Wake Enable
*/
typedef tBSA_BLE_WAKE_ENABLE tBSA_BLE_MSGID_WAKE_ENABLE_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_MSGID_WAKE_ENABLE_CMD_RSP;

/*
 * BLE Client register
 */
typedef tBSA_BLE_CL_REGISTER tBSA_BLE_CL_MSGID_REGISTER_CMD_REQ;

typedef struct
{
    tBSA_STATUS     status;
    tBSA_BLE_IF     client_if; /* client interface ID set by BSA */
} tBSA_BLE_CL_MSGID_REGISTER_CMD_RSP;

/*
 * BLE Client connect
 */
typedef tBSA_BLE_CL_OPEN tBSA_BLE_CL_MSGID_OPEN_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_OPEN_CMD_RSP;

typedef tBSA_BLE_CL_OPEN_MSG tBSA_BLE_CL_MSGID_OPEN_EVT;

/*
 * BLE Client service search
 */
typedef tBSA_BLE_CL_SEARCH tBSA_BLE_CL_MSGID_SEARCH_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_SEARCH_CMD_RSP;

typedef tBSA_BLE_CL_SEARCH_RES_MSG tBSA_BLE_CL_MSGID_SEARCH_RES_EVT;
typedef tBSA_BLE_CL_SEARCH_CMPL_MSG tBSA_BLE_CL_MSGID_SEARCH_CMPL_EVT;

/*
 * BLE Client read data
 */
typedef tBSA_BLE_CL_READ tBSA_BLE_CL_MSGID_READ_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_READ_CMD_RSP;

typedef tBSA_BLE_CL_READ_MSG tBSA_BLE_CL_MSGID_READ_EVT;

/*
 * BLE Client write data
 */
typedef tBSA_BLE_CL_WRITE tBSA_BLE_CL_MSGID_WRITE_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_WRITE_CMD_RSP;

typedef tBSA_BLE_CL_WRITE_MSG tBSA_BLE_CL_MSGID_WRITE_EVT;

/*
 * BLE Client Notificationi register
 */
typedef tBSA_BLE_CL_NOTIFREG tBSA_BLE_CL_MSGID_NOTIFREG_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_NOTIFREG_CMD_RSP;

typedef tBSA_BLE_CL_NOTIF_MSG tBSA_BLE_CL_MSGID_NOTIF_EVT;

/*
 * BLE Client close
 */
typedef tBSA_BLE_CL_CLOSE tBSA_BLE_CL_MSGID_CLOSE_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_CLOSE_CMD_RSP;

typedef tBSA_BLE_CL_CLOSE_MSG tBSA_BLE_CL_MSGID_CLOSE_EVT;

/*
 * BLE Client deregister
 */
typedef tBSA_BLE_CL_DEREGISTER tBSA_BLE_CL_MSGID_DEREGISTER_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_DEREGISTER_CMD_RSP;

typedef tBSA_BLE_CL_DEREGISTER_MSG tBSA_BLE_CL_MSGID_DEREGISTER_EVT;

/*
 * BLE Client notification deregister
 */
typedef tBSA_BLE_CL_NOTIFDEREG tBSA_BLE_CL_MSGID_NOTIFDEREG_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_NOTIFDEREG_CMD_RSP;


/*
 * BLE Client load cache
 */
typedef tBSA_BLE_CL_CACHE_LOAD tBSA_BLE_CL_MSGID_CACHE_LOAD_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_CACHE_LOAD_CMD_RSP;

typedef tBSA_BLE_CL_CACHE_LOAD_MSG tBSA_BLE_CL_MSGID_CACHE_LOAD_EVT;


/*
 * BLE Client Indication Confirm
 */
typedef tBSA_BLE_CL_INDCONF tBSA_BLE_CL_MSGID_INDCONF_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_CL_MSGID_INDCONF_CMD_RSP;



/*
 * BLE Server register
 */
typedef tBSA_BLE_SE_REGISTER tBSA_BLE_SE_MSGID_REGISTER_CMD_REQ;

typedef struct
{
    tBSA_STATUS     status;
    tBSA_BLE_IF     server_if; /* Server interface ID set by BSA */
} tBSA_BLE_SE_MSGID_REGISTER_CMD_RSP;

/*
 * BLE Server deregister
 */
typedef tBSA_BLE_SE_DEREGISTER tBSA_BLE_SE_MSGID_DEREGISTER_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_SE_MSGID_DEREGISTER_CMD_RSP;

typedef tBSA_BLE_SE_DEREGISTER_MSG tBSA_BLE_SE_MSGID_DEREGISTER_EVT;

/*
 * BLE Server create service
 */
typedef tBSA_BLE_SE_CREATE tBSA_BLE_SE_MSGID_CREATE_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_SE_MSGID_CREATE_CMD_RSP;

typedef tBSA_BLE_SE_CREATE_MSG tBSA_BLE_SE_MSGID_CREATE_EVT;

/*
 * BLE Server add char
 */
typedef tBSA_BLE_SE_ADDCHAR tBSA_BLE_SE_MSGID_ADDCHAR_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_SE_MSGID_ADDCHAR_CMD_RSP;

typedef tBSA_BLE_SE_ADDCHAR_MSG tBSA_BLE_SE_MSGID_ADDCHAR_EVT;

/*
 * BLE Server send indication
 */
typedef tBSA_BLE_SE_SENDIND tBSA_BLE_SE_MSGID_SENDIND_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_SE_MSGID_SENDIND_CMD_RSP;

/*
* BLE Server send response
*/
typedef tBSA_BLE_SE_SENDRSP tBSA_BLE_SE_MSGID_SENDRSP_CMD_REQ;

/*
 * BLE Server start service
 */
typedef tBSA_BLE_SE_START tBSA_BLE_SE_MSGID_START_CMD_REQ;

typedef tBSA_BLE_MSGID_STATUS_RSP tBSA_BLE_SE_MSGID_START_CMD_RSP;

typedef tBSA_BLE_SE_START_MSG tBSA_BLE_SE_MSGID_START_EVT;

/*
 * Structure (used by client)
 */
typedef struct
{
    tBSA_BLE_CBACK *p_app_ble_client_cback;
    tBSA_BLE_CBACK *p_app_ble_server_cback;
    tBSA_STATUS    status;
} tBSA_BLE_CB;

/*
 * Global variables (used by client)
 */
extern tBSA_BLE_CB bsa_ble_cb;


/*******************************************************************************
 **
 ** Function       bsa_ble_event_hdlr
 **
 ** Description    Handle BLE events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_ble_event_hdlr(int message_id, tBSA_BLE_MSG *p_data, int length);

#endif


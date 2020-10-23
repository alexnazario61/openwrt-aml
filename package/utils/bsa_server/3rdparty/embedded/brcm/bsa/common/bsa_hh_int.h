/*****************************************************************************
 **
 **  Name:           bsa_hh_int.h
 **
 **  Description:    Contains private BSA HH data
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_HH_INT_H
#define BSA_HH_INT_H

#include "bsa_hh_api.h"


/*
 * HH Message definition
 */
enum
{
    BSA_HH_MSGID_ENABLE_CMD = BSA_HH_MSGID_FIRST,
    BSA_HH_MSGID_DISABLE_CMD,
    BSA_HH_MSGID_OPEN_CMD,
    BSA_HH_MSGID_CLOSE_CMD,
    BSA_HH_MSGID_GET_REPORT_CMD,
    BSA_HH_MSGID_SET_REPORT_CMD,
    BSA_HH_MSGID_GET_PROTO_CMD,
    BSA_HH_MSGID_SET_PROTO_CMD,
    BSA_HH_MSGID_GET_IDLE_CMD,
    BSA_HH_MSGID_SET_IDLE_CMD,
    BSA_HH_MSGID_SEND_CTRL_CMD,
    BSA_HH_MSGID_SEND_DATA_CMD,
    BSA_HH_MSGID_ADD_DEV_CMD,
    BSA_HH_MSGID_REMOVE_DEV_CMD,
    BSA_HH_MSGID_GET_DSCPINFO_CMD,
    BSA_HH_MSGID_GET_CMD,
    BSA_HH_MSGID_SET_CMD,

    BSA_HH_MSGID_LAST_CMD = BSA_HH_MSGID_SET_CMD,

    BSA_HH_MSGID_OPEN_EVT, /* Connection Open*/
    BSA_HH_MSGID_CLOSE_EVT, /* Connection Closed */
    BSA_HH_MSGID_MIP_START_EVT,
    BSA_HH_MSGID_MIP_STOP_EVT,
    BSA_HH_MSGID_GET_REPORT_EVT,
    BSA_HH_MSGID_SET_REPORT_EVT,
    BSA_HH_MSGID_GET_PROTO_EVT,
    BSA_HH_MSGID_SET_PROTO_EVT,
    BSA_HH_MSGID_GET_IDLE_EVT,
    BSA_HH_MSGID_SET_IDLE_EVT,
    BSA_HH_MSGID_SEND_CONTROL_EVT,
    BSA_HH_MSGID_SEND_DATA_EVT,
    BSA_HH_MSGID_GET_DSCPINFO_EVT,
    BSA_HH_MSGID_VC_UNPLUG_EVT,

    BSA_HH_MSGID_LAST_EVT = BSA_HH_MSGID_VC_UNPLUG_EVT
};

/*
 * Structures used for parameters (transport)
 */
/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_HH_MSGID_STATUS_RSP;

/*
 * HH Enable
 */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;
} tBSA_HH_MSGID_ENABLE_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
    tUIPC_CH_ID uipc_channel;
} tBSA_HH_MSGID_ENABLE_CMD_RSP;

/*
 * HH disable
 */

typedef tBSA_HH_DISABLE tBSA_HH_MSGID_DISABLE_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_DISABLE_CMD_RSP;

/*
 * HH Open
 */
typedef tBSA_HH_OPEN tBSA_HH_MSGID_OPEN_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_OPEN_CMD_RSP;

typedef tBSA_HH_OPEN_MSG tBSA_HH_MSGID_OPEN_EVT;

/*
 * HH Close
 */
typedef tBSA_HH_CLOSE tBSA_HH_MSGID_CLOSE_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_CLOSE_CMD_RSP;

typedef tBSA_HH_CLOSE_MSG tBSA_HH_MSGID_CLOSE_EVT;

/*
 * MIP
 */
typedef tBSA_HH_MIP_START_MSG tBSA_HH_MSGID_MIP_START_EVT;

typedef tBSA_HH_MIP_STOP_MSG tBSA_HH_MSGID_MIP_STOP_EVT;

/*
 * Set Proto Mode
 */
typedef tBSA_HH_SET_PROTO tBSA_HH_MSGID_SET_PROTO_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_SET_PROTO_CMD_RSP;

typedef tBSA_HH_SET_PROTO_MSG tBSA_HH_MSGID_SET_PROTO_EVT;

/*
 * Get Proto Mode
 */
typedef tBSA_HH_GET_PROTO tBSA_HH_MSGID_GET_PROTO_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_GET_PROTO_CMD_RSP;

typedef tBSA_HH_GET_PROTO_MSG tBSA_HH_MSGID_GET_PROTO_EVT;

/*
 * Set Report
 */
typedef tBSA_HH_SET_REPORT tBSA_HH_MSGID_SET_REPORT_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_SET_REPORT_CMD_RSP;

typedef tBSA_HH_SET_REPORT_MSG tBSA_HH_MSGID_SET_REPORT_EVT;

/*
 * Get Report
 */
typedef tBSA_HH_GET_REPORT tBSA_HH_MSGID_GET_REPORT_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_GET_REPORT_CMD_RSP;

typedef tBSA_HH_GET_REPORT_MSG tBSA_HH_MSGID_GET_REPORT_EVT;

/*
 * Set Idle
 */
typedef tBSA_HH_SET_IDLE tBSA_HH_MSGID_SET_IDLE_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_SET_IDLE_CMD_RSP;

typedef tBSA_HH_SET_IDLE_MSG tBSA_HH_MSGID_SET_IDLE_EVT;

/*
 * Get Idle
 */
typedef tBSA_HH_GET_IDLE tBSA_HH_MSGID_GET_IDLE_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_GET_IDLE_CMD_RSP;

typedef tBSA_HH_GET_IDLE_MSG tBSA_HH_MSGID_GET_IDLE_EVT;

/*
 * Send Control
 */
typedef tBSA_HH_SEND_CTRL tBSA_HH_MSGID_SEND_CTRL_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_SEND_CTRL_CMD_RSP;

/*
 * Send Data
 */
typedef tBSA_HH_SEND_DATA tBSA_HH_MSGID_SEND_DATA_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_SEND_DATA_CMD_RSP;

/*
 * HH Get DSCPINFO
 */
typedef struct
{
    tBSA_HH_HANDLE handle;
} tBSA_HH_MSGID_GET_DSCPINFO_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_GET_DSCPINFO_CMD_RSP;

typedef tBSA_HH_DSCPINFO_MSG tBSA_HH_MSGID_DSCPINFO_EVT;

/*
 * HH Add Dev
 */
typedef tBSA_HH_ADD_DEV tBSA_HH_MSGID_ADD_DEV_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
    tBSA_HH_HANDLE handle;
} tBSA_HH_MSGID_ADD_DEV_CMD_RSP;

/*
 * HH Remove Dev
 */
typedef tBSA_HH_REMOVE_DEV tBSA_HH_MSGID_REMOVE_DEV_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_REMOVE_DEV_CMD_RSP;

/*
 * Set
 */
typedef tBSA_HH_SET tBSA_HH_MSGID_SET_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_SET_CMD_RSP;

/*
 * Get
 */
typedef tBSA_HH_GET tBSA_HH_MSGID_GET_CMD_REQ;

typedef tBSA_HH_MSGID_STATUS_RSP tBSA_HH_MSGID_GET_CMD_RSP;

/*
 * HH VC Unplug
 */
typedef tBSA_HH_VC_UNPLUG_MSG tBSA_HH_MSGID_VC_UNPLUG_EVT;

/*
 * Structure (used by client)
 */
typedef struct
{
    tBSA_HH_CBACK *p_app_hh_cback;
    BOOLEAN open_ongoing;
    BD_ADDR open_bd_addr;
    tBSA_HH_HANDLE open_handle;
    tBSA_STATUS open_status;
    UINT8 open_uipc_channel;
} tBSA_HH_CB;

/*
 * Global variables (used by client)
 */
extern tBSA_HH_CB bsa_hh_cb;

/*******************************************************************************
 **
 ** Function       bsa_hh_event_hdlr
 **
 ** Description    Handle HH events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_hh_event_hdlr(int message_id, tBSA_HH_MSG *p_data, int length);

#endif


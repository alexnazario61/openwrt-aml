/*****************************************************************************
 **
 **  Name:           bsa_av_int.h
 **
 **  Description:    Contains private BSA AV data
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_AV_INT_H
#define BSA_AV_INT_H

#include "bsa_av_api.h"

/*
 * AV Message definition
 */
enum {
    BSA_AV_MSGID_ENABLE_CMD = BSA_AV_MSGID_FIRST,
    BSA_AV_MSGID_DISABLE_CMD,
    BSA_AV_MSGID_REGISTER_CMD,
    BSA_AV_MSGID_DEREGISTER_CMD,
    BSA_AV_MSGID_OPEN_CMD,
    BSA_AV_MSGID_CLOSE_CMD,
    BSA_AV_MSGID_START_CMD,
    BSA_AV_MSGID_STOP_CMD,
    BSA_AV_MSGID_RC_CMD_CMD,
    BSA_AV_MSGID_VD_CMD_CMD,
    BSA_AV_MSGID_VD_RSP_CMD,
    BSA_AV_MSGID_META_RSP_CMD,
    BSA_AV_MSGID_RC_CLOSE_CMD,
    BSA_AV_MSGID_BUSY_LEVEL_CMD,

    BSA_AV_MSGID_LAST_CMD = BSA_AV_MSGID_BUSY_LEVEL_CMD,

    BSA_AV_MSGID_OPEN_EVT, /* AV Connection Open*/
    BSA_AV_MSGID_CLOSE_EVT, /* AV Connection Closed */
    BSA_AV_MSGID_RC_OPEN_EVT, /* RC Connection Open*/
    BSA_AV_MSGID_RC_CLOSE_EVT, /* RC Connection Closed */
    BSA_AV_MSGID_START_EVT, /* streaming started */
    BSA_AV_MSGID_STOP_EVT, /* streaming stopped */
    BSA_AV_MSGID_REMOTE_CMD_EVT,
    BSA_AV_MSGID_REMOTE_RSP_EVT,
    BSA_AV_MSGID_VENDOR_CMD_EVT,
    BSA_AV_MSGID_VENDOR_RSP_EVT,
    BSA_AV_MSGID_META_MSG_EVT,
    BSA_AV_MSGID_PENDING_EVT, /* AV Connection Pending */
    BSA_AV_MSGID_LAST_EVT = BSA_AV_MSGID_PENDING_EVT
};

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;

} tBSA_AV_MSGID_STATUS_RSP;


/*
 * AV Enable
 */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    tBSA_AV_FEAT features;
    tBSA_AV_CODEC_INFO aptx_caps;
    tBSA_AV_CODEC_INFO sec_caps;
} tBSA_AV_MSGID_ENABLE_CMD_REQ;

typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_ENABLE_CMD_RSP;

/*
 * AV Disable
 */
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_DISABLE_CMD_RSP;

/*
 * AV Register
 */
typedef struct
{
    tBSA_AV_CHNL channel; /* audio/video */
    UINT8 lt_addr;
    char service_name[BSA_AV_SERVICE_NAME_LEN_MAX];
} tBSA_AV_MSGID_REGISTER_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
    tBSA_AV_CHNL channel; /* audio/video */
    tBSA_AV_HNDL handle; /* Handle associated with the stream. */
    tUIPC_CH_ID uipc_channel;
} tBSA_AV_MSGID_REGISTER_CMD_RSP;

/*
 * AV Deregister
 */
typedef tBSA_AV_DEREGISTER tBSA_AV_MSGID_DEREGISTER_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_DEREGISTER_CMD_RSP;

/*
 * AV Open
 */
typedef tBSA_AV_OPEN tBSA_AV_MSGID_OPEN_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_OPEN_CMD_RSP;
typedef tBSA_AV_OPEN_MSG tBSA_AV_MSGID_OPEN_EVT;

/*
 * AV Pend
 */
typedef tBSA_AV_PEND_MSG tBSA_AV_MSGID_PEND_EVT;

/*
 * AV Close
 */
typedef tBSA_AV_CLOSE tBSA_AV_MSGID_CLOSE_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_CLOSE_CMD_RSP;
typedef tBSA_AV_CLOSE_MSG tBSA_AV_MSGID_CLOSE_EVT;

/*
 * AV Start
 */
typedef tBSA_AV_START  tBSA_AV_MSGID_START_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_START_CMD_RSP;
typedef tBSA_AV_START_MSG tBSA_AV_MSGID_START_EVT;

/*
 * AV Stop
 */
typedef tBSA_AV_STOP tBSA_AV_MSGID_STOP_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_STOP_CMD_RSP;
typedef tBSA_AV_STOP_MSG tBSA_AV_MSGID_STOP_EVT;

/*
 * AV RC open
 */

typedef tBSA_AV_RC_OPEN_MSG tBSA_AV_MSGID_RC_OPEN_EVT;

/*
 * AV RC close
 */
typedef tBSA_AV_CLOSE_RC tBSA_AV_MSGID_RC_CLOSE_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_RC_CLOSE_CMD_RSP;
typedef tBSA_AV_RC_CLOSE_MSG tBSA_AV_MSGID_RC_CLOSE_EVT;

/*
 * AV RC command
 */
typedef tBSA_AV_REM_CMD tBSA_AV_MSGID_RC_CMD_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_RC_CMD_CMD_RSP;

/*
 * AV Vendor command
 */
typedef tBSA_AV_VEN_CMD tBSA_AV_MSGID_VD_CMD_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_VD_CMD_CMD_RSP;
typedef tBSA_AV_VENDOR_CMD_MSG tBSA_AV_MSGID_VENDOR_CMD_EVT;
typedef tBSA_AV_VENDOR_RSP_MSG tBSA_AV_MSGID_VENDOR_RSP_EVT;

/*
 * AV Vendor command Response
 */
typedef tBSA_AV_VEN_RSP tBSA_AV_MSGID_VD_RSP_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_VD_RSP_CMD_RSP;

/*
 * AV Meta command Response
 */
typedef tBSA_AV_META_RSP_CMD tBSA_AV_MSGID_META_RSP_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_META_RSP_CMD_RSP;

typedef tBSA_AV_META_MSG_MSG tBSA_AV_MSGID_META_MSG_EVT;

/*
 * AV remote command
 */
typedef tBSA_AV_REMOTE_CMD_MSG tBSA_AV_MSGID_RC_CMD_EVT;
typedef tBSA_AV_REMOTE_RSP_MSG tBSA_AV_MSGID_RC_RSP_EVT;

/*
 * Busy level command
 */
typedef tBSA_AV_BUSY_LEVEL tBSA_AV_MSGID_BUSY_LEVEL_CMD_REQ;
typedef tBSA_AV_MSGID_STATUS_RSP tBSA_AV_MSGID_BUSY_LEVEL_CMD_RSP;



/* union of data associated with AV callback */
typedef union {
    tBSA_AV_MSGID_ENABLE_CMD_REQ enable_req;
    tBSA_AV_MSGID_ENABLE_CMD_RSP enable_rsp;

    tBSA_AV_MSGID_DISABLE_CMD_RSP disable_rsp;

    tBSA_AV_MSGID_REGISTER_CMD_REQ register_req;
    tBSA_AV_MSGID_REGISTER_CMD_RSP register_rsp;

    tBSA_AV_MSGID_DEREGISTER_CMD_REQ deregister_req;
    tBSA_AV_MSGID_DEREGISTER_CMD_RSP deregister_rsp;

    tBSA_AV_MSGID_OPEN_CMD_REQ open_req;
    tBSA_AV_MSGID_OPEN_CMD_RSP open_rsp;
    tBSA_AV_MSGID_OPEN_EVT     open_evt;

    tBSA_AV_MSGID_CLOSE_CMD_REQ close_req;
    tBSA_AV_MSGID_CLOSE_CMD_RSP close_rsp;
    tBSA_AV_MSGID_CLOSE_EVT     close_evt;

    tBSA_AV_MSGID_PEND_EVT      pend_evt;

    tBSA_AV_MSGID_START_CMD_REQ start_req;
    tBSA_AV_MSGID_START_CMD_RSP start_rsp;
    tBSA_AV_MSGID_START_EVT     start_evt;

    tBSA_AV_MSGID_STOP_CMD_REQ stop_req;
    tBSA_AV_MSGID_STOP_CMD_RSP stop_rsp;
    tBSA_AV_MSGID_STOP_EVT     stop_evt;

    tBSA_AV_MSGID_RC_OPEN_EVT   rc_open_evt;
    tBSA_AV_MSGID_RC_CLOSE_EVT  rc_close_evt;

    tBSA_AV_MSGID_VENDOR_CMD_EVT    vendor_cmd_evt;
    tBSA_AV_MSGID_VENDOR_RSP_EVT    vendor_rsp_evt;

    tBSA_AV_MSGID_META_MSG_EVT      meta_msg_evt;

    tBSA_AV_MSGID_RC_CMD_EVT    rc_cmd_evt;
    tBSA_AV_MSGID_RC_RSP_EVT    rc_rsp_evt;
} tBSA_AV_MSG_ID;

/* Control Block  (used by client) */
typedef struct {
    tBSA_AV_CBACK *p_app_av_cback;
} tBSA_AV_CB;

extern tBSA_AV_CB bsa_av_cb;

/*******************************************************************************
 **
 ** Function       bsa_av_event_hdlr
 **
 ** Description    Handle AV events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_av_event_hdlr(int message_id, tBSA_AV_MSG *p_data, int length);

#endif


/*****************************************************************************
**
**  Name:           bsa_hs_int.h
**
**  Description:    Contains private BSA Headset data
**
**  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef BSA_HS_INT_H
#define BSA_HS_INT_H

#include "bsa_hs_api.h"

/*
 * HS Message definition
 */
enum
{
        BSA_HS_MSGID_ENABLE_CMD = BSA_HS_MSGID_FIRST,
        BSA_HS_MSGID_DISABLE_CMD,
        BSA_HS_MSGID_REGISTER_CMD,
        BSA_HS_MSGID_DEREGISTER_CMD,
        BSA_HS_MSGID_OPEN_CMD,
        BSA_HS_MSGID_CLOSE_CMD,
        BSA_HS_MSGID_CANCEL_CMD,
        BSA_HS_MSGID_COMMAND_CMD,
        BSA_HS_MSGID_AUDIO_OPEN_CMD,
        BSA_HS_MSGID_AUDIO_CLOSE_CMD,
        BSA_HS_MSGID_CIEV_CMD,
        BSA_HS_MSGID_LAST_CMD = BSA_HS_MSGID_CIEV_CMD,

        BSA_HS_MSGID_OPEN_EVT,             /* Connection Open*/
        BSA_HS_MSGID_CLOSE_EVT,            /* Connection Closed */
        BSA_HS_MSGID_CONN_EVT,             /* Service level connection open */
        BSA_HS_MSGID_AUDIO_OPEN_EVT,       /* Audio open */
        BSA_HS_MSGID_AUDIO_CLOSE_EVT,      /* Audio Closed */

        BSA_HS_MSGID_CIND_EVT,             /* Indicator string from AG */
        BSA_HS_MSGID_CIEV_EVT,             /* Indicator status from AG */
        BSA_HS_MSGID_RING_EVT,             /* RING alert from AG */
        BSA_HS_MSGID_CLIP_EVT,             /* Calling subscriber information from AG */
        BSA_HS_MSGID_BSIR_EVT,             /* In band ring tone setting */
        BSA_HS_MSGID_BVRA_EVT,             /* Voice recognition activation/deactivation */
        BSA_HS_MSGID_CCWA_EVT,             /* Call waiting notification */
        BSA_HS_MSGID_CHLD_EVT,             /* Call hold and multi party service in AG */
        BSA_HS_MSGID_VGM_EVT,              /* MIC volume setting */
        BSA_HS_MSGID_VGS_EVT,              /* Speaker volume setting */
        BSA_HS_MSGID_BINP_EVT,             /* Input data response from AG */
        BSA_HS_MSGID_BTRH_EVT,             /* CCAP incoming call hold */
        BSA_HS_MSGID_CNUM_EVT,             /* Subscriber number */
        BSA_HS_MSGID_COPS_EVT,             /* Operator selection info from AG */
        BSA_HS_MSGID_CMEE_EVT,             /* Enhanced error result from AG */
        BSA_HS_MSGID_CLCC_EVT,             /* Current active call list info */
        BSA_HS_MSGID_UNAT_EVT,             /* AT command response fro AG which is not specified in HFP or HSP */
        BSA_HS_MSGID_OK_EVT,               /* OK response */
        BSA_HS_MSGID_ERROR_EVT,            /* ERROR response */
        BSA_HS_MSGID_BCS_EVT,              /* Codec selection from AG */


        BSA_HS_MSGID_LAST_EVT = BSA_HS_MSGID_BCS_EVT
};


/*
 * Structures used for parameters (transport)
 */

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_HS_MSGID_STATUS_RSP;

/*
 * HS Enable
 */
typedef struct
{
        int             dummy;
} tBSA_HS_MSGID_ENABLE_CMD_REQ;


typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_ENABLE_CMD_RSP;


/*
 * HS disable
 */
typedef tBSA_HS_MSGID_ENABLE_CMD_REQ tBSA_HS_MSGID_DISABLE_CMD_REQ;
typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_DISABLE_CMD_RSP;

/*
 * HS Register
 */
typedef tBSA_HS_REGISTER tBSA_HS_MSGID_REGISTER_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
    UINT16      hndl;
    tUIPC_CH_ID uipc_channel;
} tBSA_HS_MSGID_REGISTER_CMD_RSP;

/*
 * HS Deregister
 */
typedef tBSA_HS_DEREGISTER tBSA_HS_MSGID_DEREGISTER_CMD_REQ;

typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_DEREGISTER_CMD_RSP;


/*
 * HS Open
 */
typedef tBSA_HS_OPEN tBSA_HS_MSGID_OPEN_CMD_REQ;

typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_OPEN_CMD_RSP;

typedef tBSA_HS_OPEN_MSG tBSA_HS_MSGID_OPEN_EVT;

/*
 * HS Close
 */
typedef tBSA_HS_CLOSE tBSA_HS_MSGID_CLOSE_CMD_REQ;

typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_CLOSE_CMD_RSP;

/* HS Cancel */
typedef tBSA_HS_CANCEL tBSA_HS_MSGID_CANCEL_CMD_REQ;

/*
 * HS Audio Open
 */
typedef tBSA_HS_AUDIO_OPEN tBSA_HS_MSGID_AUDIO_OPEN_CMD_REQ;

typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_AUDIO_OPEN_CMD_RSP;

/*
 * HS Audio Close
 */
typedef tBSA_HS_AUDIO_CLOSE tBSA_HS_MSGID_AUDIO_CLOSE_CMD_REQ;

typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_AUDIO_CLOSE_CMD_RSP;

/*
 * HS Command
 */
typedef tBSA_HS_COMMAND tBSA_HS_MSGID_COMMAND_CMD_REQ;

typedef tBSA_HS_MSGID_STATUS_RSP tBSA_HS_MSGID_COMMAND_CMD_RSP;

typedef tBSA_HS_CONN_MSG tBSA_HS_MSGID_CONN_EVT;

typedef tBSA_HS_HDR_MSG  tBSA_HS_MSGID_HDR_EVT;

typedef tBSA_HS_VAL_MSG  tBSA_HS_MSGID_VAL_EVT;

/* union of data associated with message id from the server to the client */
typedef union
{
    tBSA_HS_MSGID_ENABLE_CMD_RSP      enable_rsp;
    tBSA_HS_MSGID_DISABLE_CMD_REQ     disable_rsp;
    tBSA_HS_MSGID_REGISTER_CMD_RSP    reg_rsp;
    tBSA_HS_MSGID_DEREGISTER_CMD_RSP  dereg_rsp;
    tBSA_HS_MSGID_OPEN_CMD_RSP        open_rsp;
    tBSA_HS_MSGID_CLOSE_CMD_RSP       close_rsp;
    tBSA_HS_MSGID_AUDIO_OPEN_CMD_RSP  audio_open_rsp;
    tBSA_HS_MSGID_AUDIO_CLOSE_CMD_RSP audio_close_rsp;

    /* asynchronous event */
    tBSA_HS_MSGID_HDR_EVT             hdr; /* AUDIO_OPEN and AUDIO_CLOSE event */
    tBSA_HS_MSGID_OPEN_EVT            open;
    tBSA_HS_MSGID_CONN_EVT            conn;
    tBSA_HS_MSGID_VAL_EVT             val; /* AT_XXX event */
} tBSA_HS_MSGID_EVT;

/*
 * Structure (used by client)
 */
typedef struct
{
        tBSA_HS_CBACK   *p_app_hs_cback;
} tBSA_HS_CB;


/*
 * Global variables (used by client)
 */
extern tBSA_HS_CB bsa_hs_cb;


/*******************************************************************************
**
** Function       bsa_hs_event_hdlr
**
** Description    Handle HS events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_hs_event_hdlr(int message_id, tBSA_HS_MSG *p_data, int length);

#endif



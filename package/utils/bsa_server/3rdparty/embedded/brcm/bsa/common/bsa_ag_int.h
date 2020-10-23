/*****************************************************************************
**
**  Name:           bsa_ag_int.h
**
**  Description:    Contains private BSA ag internal definition
**
**  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef BSA_AG_INT_H
#define BSA_AG_INT_H

#include "bsa_ag_api.h"
/*
 * AG Message definition
 */
enum
{
        BSA_AG_MSGID_ENABLE_CMD = BSA_AG_MSGID_FIRST,
        BSA_AG_MSGID_DISABLE_CMD,
        BSA_AG_MSGID_REGISTER_CMD,
        BSA_AG_MSGID_DEREGISTER_CMD,
        BSA_AG_MSGID_OPEN_CMD,
        BSA_AG_MSGID_CLOSE_CMD,
        BSA_AG_MSGID_RESULT_CMD,
        BSA_AG_MSGID_AUDIO_OPEN_CMD,
        BSA_AG_MSGID_AUDIO_CLOSE_CMD,
        BSA_AG_MSGID_LAST_CMD = BSA_AG_MSGID_AUDIO_CLOSE_CMD,

        BSA_AG_MSGID_OPEN_EVT,                          /* Connection Open*/
        BSA_AG_MSGID_CLOSE_EVT,                         /* Connection Closed */
        BSA_AG_MSGID_CONN_EVT,                          /* Service level connection open */
        BSA_AG_MSGID_AUDIO_OPEN_EVT,                    /* Audio open */
        BSA_AG_MSGID_AUDIO_CLOSE_EVT,                   /* Audio Closed */


        /* Values below are for HFP and HSP (order must match BTA_AG_AT_*_EVT) */
        BSA_AG_MSGID_AT_SPK_EVT,                        /* speaker gain from the HS */
        BSA_AG_MSGID_AT_MIC_EVT,                        /* microphone gain from the HS */
        BSA_AG_MSGID_AT_CKPD_EVT,                       /* CKPD from the HS */

        /* Values below are for HFP only (order must match BTA_AG_AT_*_EVT) */
        BSA_AG_MSGID_AT_A_EVT,                          /* Answer a call */
        BSA_AG_MSGID_AT_D_EVT,                          /* Place a call using number or memory dial */
        BSA_AG_MSGID_AT_CHLD_EVT,                       /* Call hold */
        BSA_AG_MSGID_AT_CHUP_EVT,                       /* Hang up a call */
        BSA_AG_MSGID_AT_CIND_EVT,                       /* Read indicator settings */
        BSA_AG_MSGID_AT_VTS_EVT,                        /* Transmit DTMF tone */
        BSA_AG_MSGID_AT_BINP_EVT,                       /* Retrieve number from voice tag */
        BSA_AG_MSGID_AT_BLDN_EVT,                       /* Place call to last dialed number */
        BSA_AG_MSGID_AT_BVRA_EVT,                       /* Enable/disable voice recognition */
        BSA_AG_MSGID_AT_NREC_EVT,                       /* Disable echo canceling */
        BSA_AG_MSGID_AT_CNUM_EVT,                       /* Retrieve subscriber number */
        BSA_AG_MSGID_AT_BTRH_EVT,                       /* CCAP-style incoming call hold */
        BSA_AG_MSGID_AT_CLCC_EVT,                       /* Query list of current calls */
        BSA_AG_MSGID_AT_COPS_EVT,                       /* Query list of current calls */
        BSA_AG_MSGID_AT_UNAT_EVT,                       /* Unknown AT command */
        BSA_AG_MSGID_AT_CBC_EVT,                        /* Battery Level report from HF */
        BSA_AG_MSGID_AT_BAC_EVT,                        /* Codec select */

        BSA_AG_MSGID_AT_BCS_EVT,                        /* Codec select */


        BSA_AG_MSGID_LAST_EVT = BSA_AG_MSGID_AT_BCS_EVT
};


/*
 * Structures used for parameters (transport)
 */

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_AG_MSGID_STATUS_RSP;

/*
 * AG Enable
 */
typedef struct
{
    tBSA_AG_CBACK *p_cback;
} tBSA_AG_MSGID_ENABLE_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP    tBSA_AG_MSGID_ENABLE_CMD_RSP;

/*
 * AG Disable
 */
typedef tBSA_AG_DISABLE tBSA_AG_MSGID_DISABLE_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP    tBSA_AG_MSGID_DISABLE_CMD_RSP;

/*
 * AG Register
 */

typedef tBSA_AG_REGISTER    tBSA_AG_MSGID_REGISTER_CMD_REQ;

typedef struct
{
    UINT16      hndl;
    tBSA_STATUS status;
    tUIPC_CH_ID uipc_channel;
} tBSA_AG_MSGID_REGISTER_CMD_RSP;

/*
 * AG Register
 */

typedef tBSA_AG_DEREGISTER    tBSA_AG_MSGID_DEREGISTER_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP    tBSA_AG_MSGID_DEREGISTER_CMD_RSP;

/*
 * AG Open
 */
typedef tBSA_AG_OPEN     tBSA_AG_MSGID_OPEN_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP    tBSA_AG_MSGID_OPEN_CMD_RSP;


/*
* AG Close
*/
typedef tBSA_AG_CLOSE tBSA_AG_MSGID_CLOSE_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP tBSA_AG_MSGID_CLOSE_CMD_RSP;

/*
 * AG Audio Open
 */
typedef tBSA_AG_AUDIO_OPEN tBSA_AG_MSGID_AUDIO_OPEN_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP    tBSA_AG_MSGID_AUDIO_OPEN_CMD_RSP;

/*
 * AG Audio Close
 */
typedef tBSA_AG_AUDIO_CLOSE tBSA_AG_MSGID_AUDIO_CLOSE_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP    tBSA_AG_MSGID_AUDIO_CLOSE_CMD_RSP;

/*
 * asynchronous event
 */
typedef tBSA_AG_RES tBSA_AG_MSGID_RES_CMD_REQ;

typedef tBSA_AG_MSGID_STATUS_RSP    tBSA_AG_MSGID_RES_CMD_RSP;

typedef tBSA_AG_OPEN_MSG tBSA_AG_MSGID_OPEN_EVT;

typedef tBSA_AG_CONN_MSG tBSA_AG_MSGID_CONN_EVT;

typedef tBSA_AG_HDR_MSG  tBSA_AG_MSGID_HDR_EVT;

typedef tBSA_AG_VAL_MSG  tBSA_AG_MSGID_VAL_EVT;

typedef tBSA_AG_CLOSE_MSG  tBSA_AG_MSGID_CLOSE_EVT;

/* union of data associated with message id from the server to the client */
typedef union
{
    tBSA_AG_MSGID_ENABLE_CMD_RSP      enable_rsp;
    tBSA_AG_MSGID_DISABLE_CMD_REQ     disable_rsp;
    tBSA_AG_MSGID_REGISTER_CMD_RSP    reg_rsp;
    tBSA_AG_MSGID_DEREGISTER_CMD_RSP  dereg_rsp;
    tBSA_AG_MSGID_OPEN_CMD_RSP        open_rsp;
    tBSA_AG_MSGID_CLOSE_CMD_RSP       close_rsp;
    tBSA_AG_MSGID_AUDIO_OPEN_CMD_RSP  audio_open_rsp;
    tBSA_AG_MSGID_AUDIO_CLOSE_CMD_RSP audio_close_rsp;

    /* asynchronous event */
    tBSA_AG_MSGID_HDR_EVT             hdr; /* AUDIO_OPEN and AUDIO_CLOSE event */
    tBSA_AG_MSGID_OPEN_EVT            open;
    tBSA_AG_MSGID_CONN_EVT            conn;
    tBSA_AG_MSGID_VAL_EVT             val; /* AT_XXX event */
    tBSA_AG_MSGID_CLOSE_EVT           close; 
} tBSA_AG_MSGID_EVT;

/*
 * Structure (used by client)
 */
typedef struct
{
        tBSA_AG_CBACK   *p_app_cback;
} tBSA_AG_CB;


extern tBSA_AG_CB bsa_ag_cb;

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
void bsa_ag_event_hdlr(int message_id, tBSA_AG_MSG *p_data, int length);

#endif



/*****************************************************************************
**
**  Name:           bsa_sac_int.h
**
**  Description:    Contains private BSA SAP Client data
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
*****************************************************************************/

#ifndef BSA_SAC_INT_H
#define BSA_SAC_INT_H

#include "bsa_sac_api.h"

/*
* SAC Message definition
*/
enum
{
    BSA_SAC_MSGID_ENABLE_CMD = BSA_SAC_MSGID_FIRST,
    BSA_SAC_MSGID_DISABLE_CMD,
    BSA_SAC_MSGID_CONNECT_CMD,
    BSA_SAC_MSGID_DISCONNECT_CMD,
    BSA_SAC_MSGID_TRANS_APDU_CMD,
    BSA_SAC_MSGID_GET_ATR_CMD,
    BSA_SAC_MSGID_SET_SIM_CMD,
    BSA_SAC_MSGID_GET_CARD_READER_STATUS_CMD,
    BSA_SAC_MSGID_SET_PROTOCOL_CMD,
    BSA_SAC_MSGID_LAST_CMD = BSA_SAC_MSGID_SET_PROTOCOL_CMD,

    BSA_SAC_MSGID_ENABLE_EVT,
    BSA_SAC_MSGID_DISABLE_EVT,
    BSA_SAC_MSGID_CONNECT_EVT,
    BSA_SAC_MSGID_DISCONNECT_EVT,
    BSA_SAC_MSGID_TRANS_APDU_EVT,
    BSA_SAC_MSGID_GET_ATR_EVT,
    BSA_SAC_MSGID_SET_SIM_EVT,
    BSA_SAC_MSGID_CARD_READER_STATUS_EVT,
    BSA_SAC_MSGID_SET_PROTOCOL_EVT,
    BSA_SAC_MSGID_DISCONNECT_IND_EVT,
    BSA_SAC_MSGID_STATUS_IND_EVT,
    BSA_SAC_MSGID_ERROR_RESP_EVT,
    BSA_SAC_MSGID_LAST_EVT = BSA_SAC_MSGID_ERROR_RESP_EVT
};

/*
* Structures used for parameters (transport)
*/

/*
* SAC Enable
*/
 typedef struct
 {
     int                     dummy;              /* Dummy place holder */
 } tBSA_SAC_MSGID_ENABLE_CMD_REQ;

/*
* SAC Disable
*/
typedef tBSA_SAC_DISABLE tBSA_SAC_MSGID_DISABLE_CMD_REQ;

/*
* SAC Connect
*/
typedef tBSA_SAC_CONNECT tBSA_SAC_MSGID_CONNECT_CMD_REQ;

/*
* SAC Disconnect
*/
typedef tBSA_SAC_DISCONNECT tBSA_SAC_MSGID_DISCONNECT_CMD_REQ;

/*
* SAC Transfer APDU
*/
typedef tBSA_SAC_TRANS_APDU tBSA_SAC_MSGID_TRANS_APDU_CMD_REQ;

/*
* SAC Get ATR
*/
typedef tBSA_SAC_GET_ATR tBSA_SAC_MSGID_GET_ATR_CMD_REQ;

/*
* SAC Set SIM
*/
typedef tBSA_SAC_SET_SIM tBSA_SAC_MSGID_SET_SIM_CMD_REQ;

/*
* SAC Get Card Reader Status
*/
typedef tBSA_SAC_GET_CARD_READER_STATUS tBSA_SAC_MSGID_GET_CARD_READER_STATUS_CMD_REQ;

/*
* SAC Set Transport Protocol
*/
typedef tBSA_SAC_SET_PROTOCOL tBSA_SAC_MSGID_SET_PROTOCOL_CMD_REQ;


/*
* Structure (used internally by BSA client-side API)
*/

typedef struct
{
     tBSA_SAC_CBACK *p_app_sac_cback;
} tBSA_SAC_CB;

/*
* Global variables (used by client-side API)
*/
extern tBSA_SAC_CB bsa_sac_cb;


/*******************************************************************************
**
** Function       bsa_sac_event_hdlr
**
** Description    Handle SAC events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_sac_event_hdlr(int message_id, tBSA_SAC_MSG *p_data, int length);

#endif

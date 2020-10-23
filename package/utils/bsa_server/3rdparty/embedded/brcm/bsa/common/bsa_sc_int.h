/*****************************************************************************
**
**  Name:           bsa_sc_int.h
**
**  Description:    Contains private BSA SAP Server data
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
*****************************************************************************/

#ifndef BSA_SC_INT_H
#define BSA_SC_INT_H

#include "bsa_sc_api.h"

#define BSA_SC_APP_ID  1

/*
* SC Message definition
*/
enum
{
    BSA_SC_MSGID_ENABLE_CMD = BSA_SC_MSGID_FIRST,
    BSA_SC_MSGID_DISABLE_CMD,
    BSA_SC_MSGID_CLOSE_CMD,
    BSA_SC_MSGID_SET_CMD,
    BSA_SC_MSGID_LAST_CMD = BSA_SC_MSGID_SET_CMD,

    BSA_SC_MSGID_ENABLE_EVT, /* Connection enable */
    BSA_SC_MSGID_DISABLE_EVT, /* Connection Disable */
    BSA_SC_MSGID_OPEN_EVT, /* Connection Open*/
    BSA_SC_MSGID_CLOSE_EVT, /* Connection Closed */
    BSA_SC_MSGID_SET_EVT, /* Set request event */
    BSA_SC_MSGID_LAST_EVT = BSA_SC_MSGID_SET_EVT
};

/*
* Structures used for parameters (transport)
*/

/*
* SC Enable
*/
typedef struct
{
    tBSA_SEC_AUTH           sec_mask;           /* Security options */
    char                    service_name[BSA_SC_SERVICE_NAME_LEN_MAX+1]; /* Service name for SDP record */
    UINT8                   reader_id;          /* SIM Card Reader ID (for TRANSFER_CARD_READER_STATUS requests) */
    tBSA_SC_READER_FLAGS    reader_flags;       /* Flags describing card reader */
    UINT16                  msg_size_min;       /* Min message size for SIM APDU commands */
    UINT16                  msg_size_max;       /* Max message size for SIM APDU commands */
    tBSA_SC_CBACK           *p_cback;           /* Callback for BSA_SC event notification */
    UINT8 app_id;
} tBSA_SC_MSGID_ENABLE_CMD_REQ;

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_SC_MSGID_STATUS_RSP;

typedef tBSA_SC_MSGID_STATUS_RSP tBSA_SC_MSGID_ENABLE_CMD_RSP;

/*
* SC Disable
*/
typedef tBSA_SC_DISABLE tBSA_SC_MSGID_DISABLE_CMD_REQ;
typedef tBSA_SC_MSGID_STATUS_RSP tBSA_SC_MSGID_DISABLE_CMD_RSP;
typedef tBSA_SC_DISABLE_MSG tBSA_SC_MSGID_DISABLE_RSP_EVT;

/*
* SC Open
*/
typedef tBSA_SC_OPEN_MSG tBSA_SC_MSGID_OPEN_EVT;

/*
* SC Close
*/
typedef tBSA_SC_CLOSE tBSA_SC_MSGID_CLOSE_CMD_REQ;
typedef tBSA_SC_MSGID_STATUS_RSP tBSA_SC_MSGID_CLOSE_CMD_RSP;
typedef tBSA_SC_CLOSE_MSG tBSA_SC_MSGID_CLOSE_RSP_EVT;

/*
* SC Set Rsp
*/
typedef tBSA_SC_SET tBSA_SC_MSGID_SET_CMD_REQ;
typedef tBSA_SC_MSGID_STATUS_RSP tBSA_SC_MSGID_SET_CMD_RSP;
typedef tBSA_SC_SET_MSG tBSA_SC_MSGID_SET_EVT;

/*
* Structure (used internally by BSA client-side API)
*/


typedef struct
{
     tBSA_SC_CBACK *p_app_sc_cback;
} tBSA_SC_CB;


/*
* Global variables (used by client-side API)
*/
extern tBSA_SC_CB bsa_sc_cb;

/*******************************************************************************
**
** Function       bsa_sc_event_hdlr
**
** Description    Handle SC events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_sc_event_hdlr(int message_id, tBSA_SC_MSG *p_data, int length);

#endif

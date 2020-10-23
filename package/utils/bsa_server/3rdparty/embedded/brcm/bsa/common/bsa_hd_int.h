/*****************************************************************************
 **
 **  Name:           bsa_hd_int.h
 **
 **  Description:    Contains private BSA HD data
 **
 **  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_HD_INT_H
#define BSA_HD_INT_H

#include "bsa_hd_api.h"


/*
 * HH Message definition
 */
enum
{
    BSA_HD_MSGID_ENABLE_CMD = BSA_HD_MSGID_FIRST,
    BSA_HD_MSGID_DISABLE_CMD,
    BSA_HD_MSGID_OPEN_CMD,
    BSA_HD_MSGID_CLOSE_CMD,
    BSA_HD_MSGID_SEND_CMD,

    BSA_HD_MSGID_LAST_CMD = BSA_HD_MSGID_SEND_CMD,

    BSA_HD_MSGID_OPEN_EVT, /* Connection Open*/
    BSA_HD_MSGID_CLOSE_EVT, /* Connection Closed */
    BSA_HD_MSGID_UNPLUG_EVT,

    BSA_HD_MSGID_LAST_EVT = BSA_HD_MSGID_UNPLUG_EVT
};

/*
 * Structures used for parameters (transport)
 */
/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_HD_MSGID_STATUS_RSP;

/*
 * HD Enable
 */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    BD_ADDR       bd_addr;
} tBSA_HD_MSGID_ENABLE_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
} tBSA_HD_MSGID_ENABLE_CMD_RSP;

/*
 * HD disable
 */

typedef tBSA_HD_DISABLE tBSA_HD_MSGID_DISABLE_CMD_REQ;

typedef tBSA_HD_MSGID_STATUS_RSP tBSA_HD_MSGID_DISABLE_CMD_RSP;

/*
 * HD Open
 */
typedef tBSA_HD_OPEN tBSA_HD_MSGID_OPEN_CMD_REQ;

typedef tBSA_HD_MSGID_STATUS_RSP tBSA_HD_MSGID_OPEN_CMD_RSP;

typedef tBSA_HD_OPEN_MSG tBSA_HD_MSGID_OPEN_EVT;

/*
 * HD Close
 */
typedef tBSA_HD_CLOSE tBSA_HD_MSGID_CLOSE_CMD_REQ;

typedef tBSA_HD_MSGID_STATUS_RSP tBSA_HD_MSGID_CLOSE_CMD_RSP;

typedef tBSA_HD_CLOSE_MSG tBSA_HD_MSGID_CLOSE_EVT;

/*
 * Send Data
 */
typedef tBSA_HD_SEND tBSA_HD_MSGID_SEND_CMD_REQ;

typedef tBSA_HD_MSGID_STATUS_RSP tBSA_HD_MSGID_SEND_CMD_RSP;


/*
 * HD VC Unplug
 */
typedef tBSA_HD_UNPLUG_MSG tBSA_HD_MSGID_UNPLUG_EVT;

/*
 * Structure (used by client)
 */
typedef struct
{
    tBSA_HD_CBACK *p_app_hd_cback;
    BD_ADDR open_bd_addr;
    tBSA_STATUS open_status;
} tBSA_HD_CB;

/*
 * Global variables (used by client)
 */
extern tBSA_HD_CB bsa_hd_cb;

/*******************************************************************************
 **
 ** Function       bsa_hd_event_hdlr
 **
 ** Description    Handle HD events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_hd_event_hdlr(int message_id, tBSA_HD_MSG *p_data, int length);

#endif


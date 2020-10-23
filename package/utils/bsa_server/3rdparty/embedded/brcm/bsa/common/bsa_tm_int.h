/*****************************************************************************
 **
 **  Name:           bsa_tm_int.h
 **
 **  Description:    Contains private BSA data
 **
 **  Copyright (c) 2010-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_TM_INT_H
#define BSA_TM_INT_H

#include "bsa_tm_api.h"

/*
 * TM Message definition
 */
enum {
    /* Commands */
    BSA_TM_MSGID_SET_TEST_MODE_CMD = BSA_TM_MSGID_FIRST,
    BSA_TM_MSGID_GET_MEM_USAGE_CMD,
    BSA_TM_MSGID_VSC_CMD,
    BSA_TM_MSGID_PING_CMD,
    BSA_TM_MSGID_SET_TRACE_LEVEL_CMD,
    BSA_TM_MSGID_READ_VERSION_CMD,
    BSA_TM_MSGID_VSE_REGISTER_CMD,
    BSA_TM_MSGID_VSE_DEREGISTER_CMD,
    BSA_TM_MSGID_GET_HANDLE_CMD,
    BSA_TM_MSGID_READ_RAWRSSI_CMD,
    BSA_TM_MSGID_DISCONNECT_CMD,
    BSA_TM_MSGID_QOS_CMD,
    BSA_TM_MSGID_DIAG_STATS_CMD,
    BSA_TM_MSGID_LE_TEST_CMD,

    BSA_TM_MSGID_LAST_CMD = BSA_TM_MSGID_LE_TEST_CMD,

    /* Events */
    BSA_TM_MSGID_VSE_EVT,

    BSA_TM_MSGID_LAST_EVT = BSA_TM_MSGID_VSE_EVT
};


#define BSA_TM_PING_SIZE 996 /* Must be smaller than 1000 (cf tBSA_CLIENT_RX_MSG) */

/*
 * Structures used for parameters (transport)
 */

/* Structure used for SetTestMode */
typedef tBSA_TM_SET_TEST_MODE tBSA_TM_MSGID_SET_TEST_MODE_CMD_REQ;

/* Structures used for GetMemUsage */
typedef struct
{
    UINT8 location;
}tBSA_TM_MSGID_GET_MEM_USAGE_CMD_REQ;
typedef tBSA_TM_GET_MEM_USAGE tBSA_TM_MSGID_GET_MEM_USAGE_CMD_RSP;

/* Structures used for Ping */
typedef struct
{
    UINT8 data[BSA_TM_PING_SIZE];
} tBSA_TM_MSGID_PING_CMD_REQ;
typedef tBSA_TM_MSGID_PING_CMD_REQ tBSA_TM_MSGID_PING_CMD_RSP;

/* Structures used for VSC */
typedef tBSA_TM_VSC tBSA_TM_MSGID_VSC_CMD_REQ;
typedef tBSA_TM_VSC tBSA_TM_MSGID_VSC_CMD_RSP;

/* Structures used for SetTraceLevel */
typedef tBSA_TM_SET_TRACE_LEVEL tBSA_TM_MSGID_SET_TRACE_LEVEL_CMD_REQ;

/* Structures used for read Version */
typedef tBSA_TM_DUMMY tBSA_TM_MSGID_READ_VERSION_CMD_REQ;
typedef tBSA_TM_READ_VERSION tBSA_TM_MSGID_READ_VERSION_CMD_RSP;

/* Structure used for VSE Register */
typedef struct
{
    UINT16 sub_event;
}tBSA_TM_MSGID_VSE_REGISTER_CMD_REQ;

/* Structure used for VSE Deregister */
typedef tBSA_TM_MSGID_VSE_REGISTER_CMD_REQ tBSA_TM_MSGID_VSE_DEREGISTER_CMD_REQ;

/* Structure used for Disconnect */
typedef tBSA_TM_DISCONNECT tBSA_TM_MSGID_DISCONNECT_CMD_REQ;

/* Structure used for QoS */
typedef tBSA_TM_QOS tBSA_TM_MSGID_QOS_CMD_REQ;

/* Structure used for Get Handle */
typedef tBSA_TM_GET_HANDLE tBSA_TM_MSGID_GET_HANDLE_CMD_REQ;
typedef tBSA_TM_GET_HANDLE tBSA_TM_MSGID_GET_HANDLE_CMD_RSP;

/* Structures used for Diag Stats */
typedef tBSA_TM_DIAG_STATS tBSA_TM_MSGID_DIAG_STATS_CMD_REQ;
typedef tBSA_TM_DIAG_STATS tBSA_TM_MSGID_DIAG_STATS_CMD_RSP;

/* Structure used for Read Raw RSSI */
typedef tBSA_TM_READRAWRSSI tBSA_TM_MSGID_READ_RAWRSSI_CMD_REQ;
typedef tBSA_TM_READRAWRSSI tBSA_TM_MSGID_READ_RAWRSSI_CMD_RSP;
#endif /* BSA_TM_INT_H */


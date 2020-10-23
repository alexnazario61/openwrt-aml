/*****************************************************************************
 **
 **  Name:           bsa_sys_int.h
 **
 **  Description:    Contains system BSA data
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_SYS_INT_H
#define BSA_SYS_INT_H

#include "bsa_status.h"

/*
 * SYSTEM Message definition
 */
enum
{
    BSA_SYS_MSGID_PING_CMD = BSA_SYS_MSGID_FIRST,
    BSA_SYS_MSGID_KILL_SERVER_CMD,
    BSA_SYS_MSGID_LAST_CMD = BSA_SYS_MSGID_KILL_SERVER_CMD
};

/*
 * SYSTEM messages parameters definitions
 */

/* Structures used to ping server (for test) */
typedef struct
{
    int nb_ping;
    int nb_event;
} tBSA_DEBUG_SYS;

#define BSA_PING_SIZE 996 /* Must be smaller than 1000 (cf tBSA_CLIENT_RX_MSG) */
typedef struct
{
    unsigned char data[BSA_PING_SIZE];
} tBSA_SYS_MSGID_PING_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
    unsigned char data[BSA_PING_SIZE];
} tBSA_SYS_MSGID_PING_CMD_RSP;

/* Structures used to kill server */
typedef struct
{
    int dummy;
} tBSA_SYS_MSGID_KILL_SERVER_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
} tBSA_SYS_MSGID_KILL_SERVER_CMD_RSP;

#endif


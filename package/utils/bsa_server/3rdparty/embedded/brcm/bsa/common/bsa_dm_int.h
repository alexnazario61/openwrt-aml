/*****************************************************************************
 **
 **  Name:           bsa_dm_int.h
 **
 **  Description:    Contains DM BSA data
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_DM_INT_H
#define BSA_DM_INT_H

#include "bsa_dm_api.h"
#include "bsa_int.h"

/*
 * DM Message definition
 */
enum
{
    BSA_DM_MSGID_GET_CONFIG_CMD = BSA_DM_MSGID_FIRST,
    BSA_DM_MSGID_SET_CONFIG_CMD,

    BSA_DM_MSGID_LAST_CMD = BSA_DM_MSGID_SET_CONFIG_CMD,

    BSA_DM_MSGID_3D_ANNOUNCEMENT_EVT,

    BSA_DM_MSGID_LAST_EVT = BSA_DM_MSGID_3D_ANNOUNCEMENT_EVT
};

/*
 * DM messages parameters definitions
 */

/* Structure used to get configuration */
typedef struct
{
    tBSA_STATUS status;
    tBSA_DM_GET_CONFIG config;
} tBSA_DM_MSGID_GET_CONFIG_CMD_RSP;

/* Structure used to set configuration */
typedef tBSA_DM_SET_CONFIG tBSA_DM_MSGID_SET_CONFIG_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
} tBSA_DM_MSGID_SET_CONFIG_CMD_RSP;

/* 3D Events */
typedef tBSA_DM_3D_ANNOUCEMENT_MSG tBSA_DM_MSGID_3D_ANNOUCEMENT_EVT;

#endif


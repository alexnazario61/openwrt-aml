/*****************************************************************************
 **
 **  Name:           bsa_disc_int.h
 **
 **  Description:    Contains private BSA data
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_DISC_INT_H
#define BSA_DISC_INT_H

#include "bsa_disc_api.h"

/*
 * DISC Message definition
 */
enum
{
    BSA_DISC_MSGID_DISC_START_CMD = BSA_DISC_MSGID_FIRST,
    BSA_DISC_MSGID_DISC_ABORT_CMD,

    BSA_DISC_MSGID_LAST_CMD = BSA_DISC_MSGID_DISC_ABORT_CMD,

    BSA_DISC_MSGID_DISC_NEW_EVT,
    BSA_DISC_MSGID_DISC_CMPL_EVT,
    BSA_DISC_MSGID_DISC_DEV_INFO_EVT,

    BSA_DISC_MSGID_LAST_EVT = BSA_DISC_MSGID_DISC_DEV_INFO_EVT
};

/*
 * DISC messages parameters definitions
 */
typedef tBSA_DISC_START tBSA_DISC_MSGID_DISC_START_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
}tBSA_DISC_MSGID_DISC_START_CMD_RSP;

typedef tBSA_DISC_ABORT tBSA_DISC_MSGID_DISC_ABORT_CMD_REQ;

#endif


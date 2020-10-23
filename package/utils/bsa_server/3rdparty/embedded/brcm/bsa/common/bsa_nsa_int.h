/*****************************************************************************
**
**  Name:           bsa_nsa_int.h
**
**  Description:    Contains private BSA NSA internal definition
**
**  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef BSA_NSA_INT_H
#define BSA_NSA_INT_H

#include "bsa_api.h"

/*
 * BSA NSA Message definition
 */
enum
{
        BSA_NSA_MSGID_ADD_IF_CMD = BSA_NSA_MSGID_FIRST,
        BSA_NSA_MSGID_REMOVE_IF_CMD,

        BSA_NSA_MSGID_LAST_CMD = BSA_NSA_MSGID_REMOVE_IF_CMD,

        BSA_NSA_MSGID_LAST_EVT = BSA_NSA_MSGID_REMOVE_IF_CMD
};


/*
 * Structures used for parameters (transport)
 */

/*
 * BSA NSA Add Interface
 */
typedef tBSA_NSA_ADD_IF tBSA_NSA_MSGID_ADD_IF_CMD_REQ;
typedef struct
{
    tBSA_STATUS status;
    tBSA_NSA_PORT port;
} tBSA_NSA_MSGID_ADD_IF_CMD_RSP;

/*
 * BSA NSA Remove
 */
typedef tBSA_NSA_REMOVE_IF tBSA_NSA_MSGID_REMOVE_IF_CMD_REQ;

#endif /* BSA_NSA_INT_H */



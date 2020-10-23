/*****************************************************************************
**
**  Name:           bsa_mgt_int.h
**
**  Description:    Contains private BSA data
**
**  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef BSA_MGT_INT_H
#define BSA_MGT_INT_H

#include "bsa_mgt_api.h"

/*
 * MGT Message definition
 */
enum
{
    BSA_MGT_MSGID_LAST_CMD = BSA_MGT_MSGID_FIRST,

    BSA_SYS_MSGID_STATUS_EVT,
    BSA_MGT_MSGID_LAST_EVT = BSA_SYS_MSGID_STATUS_EVT
};

/*
 * MGT messages parameters definitions
 */
/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_MGT_MSGID_STATUS_RSP;

typedef tBSA_MSG_STATUS_MSG tBSA_MGT_MSGID_STATUS_EVT;

/*******************************************************************************
 **
 ** Function       bsa_mgt_event_hdlr
 **
 ** Description    Function to handle MGT messages from server.
 **
 ** Parameters     ID of the message, content of the message and length of the
 **                message.
 **
 ** Returns        None
 **
 *******************************************************************************/
extern void bsa_mgt_event_hdlr(int message_id, tBSA_MGT_MSG *p_data,
        int length);

#endif



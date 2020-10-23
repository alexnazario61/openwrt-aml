/*****************************************************************************
 **
 **  Name:           bsa_cl_dm_int.h
 **
 **  Description:    This is the internal interface file for DM part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2010-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CL_DM_INT_H
#define BSA_CL_DM_INT_H

#include "bt_target.h"

typedef struct
{
    tBSA_DM_CBACK *callback;
} tBSA_CL_DM_CB;

extern tBSA_CL_DM_CB bsa_cl_dm_cb;

/*******************************************************************************
 **
 ** Function       bsa_cl_dm_event_hdlr
 **
 ** Description    Handle DM events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_cl_dm_event_hdlr(UINT16 message_id, tBSA_DM_MSG * p_buffer, int length);

#endif

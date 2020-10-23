/*****************************************************************************
 **
 **  Name:           bsa_cl_hl_int.h
 **
 **  Description:    This is the internal interface file for Health part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CL_HL_INT_H
#define BSA_CL_HL_INT_H

typedef struct
{
    tBSA_HL_CBACK *p_cback;
    tBSA_HL_SDP_QUERY_MSG *p_sdp; /* Used temporary */
} tBSA_HL_CB;

/*
 * Global variables (used by client)
 */
extern tBSA_HL_CB bsa_hl_cb;

/*******************************************************************************
 **
 ** Function       bsa_cl_hl_event_hdlr
 **
 ** Description    Handle Health events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_cl_hl_event_hdlr(int message_id, tBSA_HL_MSGID_EVT* p_buffer, UINT16 length);


#endif


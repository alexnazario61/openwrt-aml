/*****************************************************************************
 **
 **  Name:           bsa_cl_tm_int.h
 **
 **  Description:    This is the internal interface file for TM part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CL_TM_INT_H
#define BSA_CL_TM_INT_H

/*
 * Definitions
 */
typedef struct
{
    tBSA_TM_CBACK *p_callback;
} tBSA_CL_TM_CB;

/*
 * Global varialbes
 */
extern tBSA_CL_TM_CB bsa_cl_tm_cb;

/*******************************************************************************
 **
 ** Function        bsa_cl_tm_event_hdlr
 **
 ** Description     Handle TM events
 **
 ** Parameters      message_id: Message Id received from server
 **                 p_buffer: pointer on asssociated data
 **                 length: data length
 **
 ** Returns         None
 **
 *******************************************************************************/
void bsa_cl_tm_event_hdlr(UINT16 message_id, tBSA_TM_MSG * p_buffer, int length);

#endif


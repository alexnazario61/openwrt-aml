/*****************************************************************************
 **
 **  Name:           bsa_cl_av_int.h
 **
 **  Description:    This is the internal interface file for av part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CL_AV_INT_H
#define BSA_CL_AV_INT_H

/*******************************************************************************
 **
 ** Function       bsa_cl_av_event_hdlr
 **
 ** Description    Handle av events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_cl_av_event_hdlr(int message_id, tBSA_AV_MSG * p_buffer, int length);

#endif


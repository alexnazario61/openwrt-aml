/*****************************************************************************
 **
 **  Name:           uipc_cl_av.h
 **
 **  Description:    Client API for BSA UIPC AV
 **
 **  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CL_AA_H
#define UIPC_CL_AA_H

/*******************************************************************************
 **
 ** Function        uipc_cl_av_init
 **
 ** Description     Initialize the client AV UIPC
 **
 ** Parameters      None
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_init(void);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_open
 **
 ** Description     Open a client AV UIPC channel
 **
 ** Parameters      channel_id: identifier of the channel to open
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_open(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_close
 **
 ** Description     Open the client AV UIPC channel
 **
 ** Parameters      None
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_cl_av_close(void);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_send
 **
 ** Description     Send data to the server through the AV UIPC channel
 **
 ** Parameters      p_buf: pointer to the buffer to send
 **                 msglen: length of the buffer to send
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_send(UINT8 *p_buf, UINT16 msglen);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_ioctl
 **
 ** Description     Control the AV UIPC channel
 **
 ** Parameters      request:
 **                 param:
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_ioctl(UINT32 request, void *param);

#endif

/*****************************************************************************
 **
 **  Name:           uipc_sv_av.h
 **
 **  Description:    Server API for BSA UIPC AV
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_AV_H
#define UIPC_SV_AV_H

/*******************************************************************************
 **
 ** Function        uipc_sv_av_init
 **
 ** Description     Initialize the client AV UIPC
 **
 ** Parameters      None
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_init(void);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_open
 **
 ** Description     Open a server AV UIPC channel
 **
 ** Parameters      channel_id: identifier of the channel to open
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_close
 **
 ** Description     Open the server AV UIPC channel
 **
 ** Parameters      None
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_sv_av_close(void);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_ioctl
 **
 ** Description     Control the server AV UIPC channel
 **
 ** Parameters      request:
 **                 param:
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_ioctl(UINT32 request, void *param);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_read
 **
 ** Description     Read data from client through AV UIPC channel
 **
 ** Parameters      p_buf: pointer to the buffer to send
 **                 len: length of the buffer to send
 **
 ** Returns         length read
 **
 *******************************************************************************/
UINT32 uipc_sv_av_read(UINT8 *p_buf, UINT32 len);

#endif



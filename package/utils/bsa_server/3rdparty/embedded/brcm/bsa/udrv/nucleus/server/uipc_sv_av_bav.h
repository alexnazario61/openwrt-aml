/*****************************************************************************
 **
 **  Name:           uipc_sv_av_bav.h
 **
 **  Description:    Server API for BSA UIPC AV Broadcast
 **
 **  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_AV_BAV_H
#define UIPC_SV_AV_BAV_H


/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_init
 **
 ** Description     Initialize the Server AV Broadcast UIPC
 **
 ** Parameters      None
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_bav_init(void);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_open
 **
 ** Description     Open a server AV Broadcast UIPC channel
 **
 ** Parameters      channel_id: identifier of the channel to open
 **                 p_cback: call back to register, may be null.
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_sv_av_bav_open(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_close
 **
 ** Description     Open the server AV UIPC channel
 **
 ** Parameters      None
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_sv_av_bav_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function        uipc_sv_av_bav_read
 **
 ** Description     Read data from client through AV UIPC channel
 **
 ** Parameters      p_buf: pointer to the buffer to send
 **                 len: length of the buffer to send
 **
 ** Returns         length read
 **
 *******************************************************************************/
UINT32 uipc_sv_av_bav_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT32 len);

#endif



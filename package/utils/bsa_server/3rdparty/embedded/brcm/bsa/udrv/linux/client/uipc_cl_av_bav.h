/*****************************************************************************
 **
 **  Name:           uipc_cl_av_bav.h
 **
 **  Description:    Client API for BSA UIPC AV Broadcast
 **
 **  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CL_AV_BAV_H
#define UIPC_CL_AV_BAV_H


/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_init
 **
 ** Description     Initialize the client AV Broadcast UIPC
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_init(void);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_open
 **
 ** Description     Open a client AV Broadcast UIPC channel
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_open(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_close
 **
 ** Description     Open the client AV Broadcast UIPC channel
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_cl_av_bav_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_send
 **
 ** Description     Send data to the server through the AV Broadcast UIPC channel
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/*******************************************************************************
 **
 ** Function        uipc_cl_av_bav_ioctl
 **
 ** Description     Control the client AV Broadcast UIPC channel
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_av_bav_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param);

#endif


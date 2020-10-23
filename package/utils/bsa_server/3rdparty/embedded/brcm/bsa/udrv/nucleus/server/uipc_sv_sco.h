/*****************************************************************************
 **
 **  Name:           uipc_sv_sco.h
 **
 **  Description:    Server API header file for BSA UIPC for Audio gateway
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef UIPC_SV_SCO_H
#define UIPC_SV_SCO_H

/*******************************************************************************
 **
 ** Function         uipc_sv_sco_init
 **
 ** Description      UIPC SCO Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_sco_init(void);

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_rx_open
 **
 ** Description     Open server SCO RX UIPC channels.
 **                 This is ring buffer from server to application
 **
 ** Parameters
 **                 p_cback: call back to register, may be null.
 **
 ** Returns         TRUE if successful, FALSE in case of error
 **
 *******************************************************************************/
BOOLEAN uipc_sv_sco_open(tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_close
 **
 ** Description     Close the server SCO UIPC channels
 **
 ** Returns         None
 **
 *******************************************************************************/
void uipc_sv_sco_close(void);

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_ioctl
 **
 ** Description     Control the server SCO UIPC channels
 **
 ** Parameters
 **             UINT32 request: IOCTL request
 **             void* param: request parameter
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_sco_ioctl(UINT32 request, void *param);

/*******************************************************************************
 **
 ** Function         uipc_sv_sco_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Parameters
 **         UINT8* p_buf: data pointer
 **         UINT16 msglen: messscoe length
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_sco_send(UINT8 *p_buf,UINT16 msglen);

/*******************************************************************************
 **
 ** Function        uipc_sv_sco_read
 **
 ** Description     Read data from client through SCO UIPC channel
 **
 ** Parameters
 **                 p_buf: pointer to the buffer to send to BTA
 **                 len: length of the buffer to send to BTA
 **
 ** Returns         length read
 **
 *******************************************************************************/
UINT32 uipc_sv_sco_read(UINT8 *p_buf, UINT32 len);
#endif

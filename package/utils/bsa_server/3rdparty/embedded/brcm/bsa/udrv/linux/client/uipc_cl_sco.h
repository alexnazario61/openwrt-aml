/*****************************************************************************
 **
 **  Name:           uipc_cl_ag.h
 **
 **  Description:    Client API header file for BSA UIPC Audio Gateway
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef UIPC_CL_SCO_H
#define UIPC_CL_SCO_H

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_init
 **
 ** Description      UIPC Audio Gateway Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_sco_init(void);

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_open
 **
 ** Description      Open Audio Gateway TX and RX ring buffers
 **
 ** Parameters
 **         tUIPC_RCV_CBACK *p_cback: RX callback to register
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_sco_open(tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_close
 **
 ** Description      Disconnect from SCO server
 **
 ** Returns         void
 **
 *******************************************************************************/
void uipc_cl_sco_close(void);

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_send
 **
 ** Description      This function is used to write data to ring buffer
 **
 ** Parameters
 **         UINT8* p_buf: data pointer
 **         UINT16 msglen: messscoe length
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_sco_send(UINT8 *p_buf, UINT16 len);

/*******************************************************************************
 **
 ** Function         uipc_cl_sco_read
 **
 ** Description      This function is used to read data from ring buffer
 **
 ** Parameters
 **         UINT8* p_buf: data pointer
 **         UINT16 len: number of byte to read
 **
 ** Returns
 **         BOOLEAN: TRUE if success/FALSE otherwise
 **
 *******************************************************************************/
UINT32 uipc_cl_sco_read(UINT8 *p_buf, UINT16 len);

/*******************************************************************************
 **
 ** Function        uipc_cl_sco_ioctl
 **
 ** Description     Control the client SCO UIPC channel
 **
 ** Parameters
 **         UINT32 request: IOCTL request
 **         void* param: request parameter
 **
 ** Returns         TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_sco_ioctl(UINT32 request, void *param);
#endif

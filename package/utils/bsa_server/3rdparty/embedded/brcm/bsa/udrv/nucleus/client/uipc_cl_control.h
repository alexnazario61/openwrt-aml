/*****************************************************************************
 **
 **  Name:           uipc_cl_control.h
 **
 **  Description:    Server API for BSA UIPC
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CL_CONTROL_H
#define UIPC_CL_CONTROL_H

/*******************************************************************************
 **
 ** Function         uipc_cl_control_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_control_init(void);

/*******************************************************************************
 **
 ** Function         uipc_cl_control_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_control_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_cl_control_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_control_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_cl_control_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_control_send(tUIPC_CH_ID channel_id, UINT16 msg_id, UINT8 *p_buf, UINT16 msglen);

#endif


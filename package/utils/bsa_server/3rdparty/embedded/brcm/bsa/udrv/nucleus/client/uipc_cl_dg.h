/*****************************************************************************
 **
 **  Name:           uipc_cl_dg.h
 **
 **  Description:    Server API for BSA UIPC Data Gateway
 **
 **  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CL_DG_H
#define UIPC_CL_DG_H

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_dg_init(void);

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_dg_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_dg_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_cl_dg_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_dg_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/*******************************************************************************
**
** Function         uipc_cl_dg_send_buf
**
** Description      This function is used to send a data buffer to a client
**
** Parameters       channel_id: UIPC channel id
**                  p_msg: data to send
**
** Returns          BOOLEAN
**
*******************************************************************************/
BOOLEAN uipc_cl_dg_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);

#endif

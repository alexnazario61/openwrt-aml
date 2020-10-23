/*****************************************************************************
 **
 **  Name:           uipc_sv_dg.h
 **
 **  Description:    Server API for BSA UIPC Data Gateway
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_DG_H
#define UIPC_SV_DG_H

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_init(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_dg_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

#endif

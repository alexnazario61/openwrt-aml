/*****************************************************************************
 **
 **  Name:           uipc_sv_hh.c
 **
 **  Description:    Server API for BSA UIPC HH
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_HH_H
#define UIPC_SV_HH_H

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_init(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_hh_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_sv_hh_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hh_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

#endif

/*****************************************************************************
 **
 **  Name:           uipc_sv_nsa.h
 **
 **  Description:    Server API for BSA NSA Interface
 **
 **  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_NSA_H
#define UIPC_SV_NSA_H


/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_init
 **
 ** Description      UIPC Control Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_nsa_init(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_open
 **
 ** Description      Open UIPC control channel
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_nsa_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_close
 **
 ** Description      Close UIPC control channel
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_nsa_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_sv_nsa_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sv_nsa_send(tUIPC_CH_ID channel_id,
        tBSA_CLIENT_NB client_num, UINT8 *p_buf, UINT16 msglen);

#endif /* UIPC_SV_NSA_H */


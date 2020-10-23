/*****************************************************************************
 **
 **  Name:           uipc_sv_control.h
 **
 **  Description:    Server API for BSA UIPC
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_CONTROL_H
#define UIPC_SV_CONTROL_H

/*redefine if we are compiling for NSA server*/
#if defined (NSA) && (NSA == TRUE)
typedef tNSA_SV_CONTROL_CLIENT_MSG tBSA_SV_CONTROL_CLIENT_MSG;
#endif

/*******************************************************************************
 **
 ** Function         uipc_sv_control_init
 **
 ** Description      UIPC Control Init
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_control_init(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_control_open
 **
 ** Description      Connect UIPC control connection to server
 **
 ** Returns          Open Status  (TRUE:OK, FALSE:Fail)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_control_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_sv_control_close
 **
 ** Description      Disconnect UIPC control connection to server
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_control_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_sv_control_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          Send status  (TRUE:OK, FALSE:Fail)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_control_send(tUIPC_CH_ID channel_id,
        tBSA_CLIENT_NB client_num, UINT8 *p_buf, UINT16 msglen);

#endif


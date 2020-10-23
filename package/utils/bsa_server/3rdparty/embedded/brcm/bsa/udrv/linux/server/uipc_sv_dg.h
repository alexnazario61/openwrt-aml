/*****************************************************************************
 **
 **  Name:           uipc_sv_dg.h
 **
 **  Description:    Server API for BSA UIPC DG
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_DG_H
#define UIPC_SV_DG_H

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_init
 **
 ** Description      UIPC SV initialization
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_dg_init(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_open
 **
 ** Description      Open an UIPC DG channel (socket server)
 **
 ** Returns          open status (TRUE:OK, FALSE:Fail)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_close
 **
 ** Description      Close an UIPC DG channel (stop socket server)
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
 ** Returns          Send status (TRUE:OK, FALSE:Fail)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Returns          Send status (TRUE:OK, FALSE:Fail or FlowOff)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);

/*******************************************************************************
 **
 ** Function         uipc_sv_dg_read
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          Number of bytes read
 **
 *******************************************************************************/
UINT32 uipc_sv_dg_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/*******************************************************************************
 **
 ** Function           uipc_sv_dg_ioctl
 **
 ** Description        Control the DG channel
 **
 ** Parameters         desc: socket server descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_dg_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param);

#endif


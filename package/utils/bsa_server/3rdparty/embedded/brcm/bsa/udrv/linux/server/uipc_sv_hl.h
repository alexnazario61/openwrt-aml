/*****************************************************************************
 **
 **  Name:           uipc_sv_hl.h
 **
 **  Description:    Server API for BSA UIPC HL
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_HL_H
#define UIPC_SV_HL_H

/*******************************************************************************
 **
 ** Function         uipc_sv_hl_init
 **
 ** Description      UIPC SV initialization
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_hl_init(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_hl_open
 **
 ** Description      Open an UIPC HL channel (socket server)
 **
 ** Returns          open status (TRUE:OK, FALSE:Fail)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hl_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_sv_hl_close
 **
 ** Description      Close an UIPC HL channel (stop socket server)
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_hl_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_sv_hl_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          Send status (TRUE:OK, FALSE:Fail)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hl_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/*******************************************************************************
 **
 ** Function         uipc_sv_hl_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Returns          Send status (TRUE:OK, FALSE:Fail or FlowOff)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hl_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);

/*******************************************************************************
 **
 ** Function         uipc_sv_hl_read
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          Number of bytes read
 **
 *******************************************************************************/
UINT32 uipc_sv_hl_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/*******************************************************************************
 **
 ** Function           uipc_sv_hl_ioctl
 **
 ** Description        Control the HL channel
 **
 ** Parameters         desc: socket server descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_hl_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param);

#endif


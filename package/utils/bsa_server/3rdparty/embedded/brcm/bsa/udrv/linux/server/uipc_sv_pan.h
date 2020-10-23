/*****************************************************************************
 **
 **  Name:           uipc_sv_pan.h
 **
 **  Description:    Server API for BSA UIPC PAN
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_PAN_H
#define UIPC_SV_PAN_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 **
 ** Function         uipc_sv_pan_init
 **
 ** Description      UIPC SV initialization
 **
 ** Returns          void
 **
 ******************************************************************************/
void uipc_sv_pan_init(void);

/******************************************************************************
 **
 ** Function         uipc_sv_pan_open
 **
 ** Description      Open an UIPC PAN channel (socket server)
 **
 ** Returns          open status (TRUE:OK, FALSE:Fail)
 **
 ******************************************************************************/
BOOLEAN uipc_sv_pan_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/******************************************************************************
 **
 ** Function         uipc_sv_pan_close
 **
 ** Description      Close an UIPC PAN channel (stop socket server)
 **
 ** Returns          void
 **
 ******************************************************************************/
void uipc_sv_pan_close(tUIPC_CH_ID channel_id);

/******************************************************************************
 **
 ** Function         uipc_sv_pan_send
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          Send status (TRUE:OK, FALSE:Fail)
 **
 ******************************************************************************/
BOOLEAN uipc_sv_pan_send(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/******************************************************************************
 **
 ** Function         uipc_sv_pan_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Returns          Send status (TRUE:OK, FALSE:Fail or FlowOff)
 **
 ******************************************************************************/
BOOLEAN uipc_sv_pan_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);

/******************************************************************************
 **
 ** Function         uipc_sv_pan_read
 **
 ** Description      This function is used to send data to a client
 **
 ** Returns          Number of bytes read
 **
 ******************************************************************************/
UINT32 uipc_sv_pan_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 msglen);

/******************************************************************************
 **
 ** Function           uipc_sv_pan_ioctl
 **
 ** Description        Control the PAN channel
 **
 ** Parameters         desc: socket server descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            TRUE if successful, FALSE otherwise
 **
 ******************************************************************************/
BOOLEAN uipc_sv_pan_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param);

#ifdef __cplusplus
}
#endif

#endif

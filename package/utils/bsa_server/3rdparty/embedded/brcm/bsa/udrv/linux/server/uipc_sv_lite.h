/*****************************************************************************
 **
 **  Name:           uipc_sv_lite.h
 **
 **  Description:    Server API to conenct to Lite Stack
 **
 **  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_LITE_H
#define UIPC_LITE_H


/*******************************************************************************
 **
 ** Function         uipc_sv_lite_init
 **
 ** Description      Initialize the Lite UIPC channel.
 **
 ** Parameters       None.
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_lite_init(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_lite_open
 **
 ** Description      Open the Lite UIPC channel.
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_cback: UIPC callback function
 **
 ** Returns          Boolean
 **
 *******************************************************************************/
BOOLEAN uipc_sv_lite_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         bsa_sv_stop_hl_server
 **
 ** Description      Close an HL UIPC channel.
 **
 ** Parameters       channel_id: UIPC channel id
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_lite_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_sv_lite_send
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_msg: data to send
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sv_lite_send(tUIPC_CH_ID channel_id, UINT16 msg_evt, UINT8 *p_buf, UINT16 msg_len);

/*******************************************************************************
 **
 ** Function         uipc_sv_lite_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_msg: data to send
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sv_lite_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);

#endif


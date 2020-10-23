/*****************************************************************************
 **
 **  Name:           uipc_cl_hl.h
 **
 **  Description:    client API for BSA UIPC HL
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CL_HL_H
#define UIPC_CL_HL_H


/*******************************************************************************
 **
 ** Function         uipc_cl_hl_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_hl_init(void);

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hl_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_hl_close(tUIPC_CH_ID channel_id);

/*******************************************************************************
 **
 ** Function         uipc_cl_hl_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hl_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);


/*******************************************************************************
 **
 ** Function         uipc_cl_hl_read
 **
 ** Description      This function is used to read data from server
 **
 ** Returns          UINT32
 **
 *******************************************************************************/
UINT32 uipc_cl_hl_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 buflen);

/*******************************************************************************
 **
 ** Function           uipc_cl_hl_ioctl
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
BOOLEAN uipc_cl_hl_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param);

#endif


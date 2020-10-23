/*****************************************************************************
 **
 **  Name:           uipc_cl_dg.h
 **
 **  Description:    client API for BSA UIPC DG
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
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
void uipc_cl_dg_init(void);

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
 ** Function         uipc_cl_dg_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_cl_dg_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);


/*******************************************************************************
 **
 ** Function         uipc_cl_dg_read
 **
 ** Description      This function is used to read data from server
 **
 ** Returns          UINT32
 **
 *******************************************************************************/
UINT32 uipc_cl_dg_read(tUIPC_CH_ID channel_id, UINT8 *p_buf, UINT16 buflen);

/*******************************************************************************
 **
 ** Function           uipc_cl_dg_ioctl
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
BOOLEAN uipc_cl_dg_ioctl(tUIPC_CH_ID channel_id, UINT32 request, void *param);

#endif


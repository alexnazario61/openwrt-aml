/*****************************************************************************
 **
 **  Name:           uipc_cl_hh.c
 **
 **  Description:    Server API for BSA UIPC HH
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CL_HH_H
#define UIPC_CL_HH_H

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_init(void);

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_open
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_close
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_cl_hh_close(tUIPC_CH_ID channel_id);

#endif

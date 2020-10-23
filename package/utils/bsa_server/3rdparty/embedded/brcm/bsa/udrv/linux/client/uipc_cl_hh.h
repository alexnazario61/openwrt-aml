/*****************************************************************************
 **
 **  Name:           uipc_cl_hh.c
 **
 **  Description:    Client API for BSA UIPC HH
 **
 **  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CL_HH_H
#define UIPC_CL_HH_H

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_init
 **
 ** Description      Initialize the HH UIPC channel.
 **
 ** Parameters       None.
 **
 ** Returns          TRUE if successful, FALSE otherwise.
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_init(void);

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_open
 **
 ** Description      Open an HH UIPC channel.
 **
 ** Parameters
 **
 ** Returns          TRUE if successful, FALSE otherwise.
 **
 *******************************************************************************/
BOOLEAN uipc_cl_hh_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function         uipc_cl_hh_close
 **
 ** Description      Close an HH UIPC channel.
 **
 ** Parameters
 **
 ** Returns          None
 **
 *******************************************************************************/
void uipc_cl_hh_close(tUIPC_CH_ID channel_id);

#endif

/*****************************************************************************
 **
 **  Name:           bsa_cl_mgt_int.h
 **
 **  Description:    This is the internal interface file for MGT part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CL_MGT_INT_H
#define BSA_CL_MGT_INT_H

/*
 * Defines
 */
typedef struct
{
    /* For ease of use, the GKI should be initialized once and for all */
    BOOLEAN gki_initialized;
    /* The connection to the BSA server can go up and down */
    BOOLEAN bsa_connected_to_server;
    tBSA_MGT_CBACK *bsa_mgt_callback;
}tBSA_CL_MGT_CB;

/*
 * Variables
 */

extern tBSA_CL_MGT_CB bsa_cl_mgt_cb;
extern tUIPC_RCV_CBACK bsa_mgt_cback;

/*******************************************************************************
 **
 ** Function        bsa_cl_mgt_init
 **
 ** Description     Initialize all the Client data
 **
 ** Parameters      none
 **
 ** Returns         Status
 **
 *******************************************************************************/
tBSA_STATUS bsa_cl_mgt_init(void);

/*******************************************************************************
 **
 ** Function       bsa_callback_task
 **
 ** Description    Task in charge of calling the callback functions of application
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
extern void bsa_callback_task(UINT32 param);

#endif


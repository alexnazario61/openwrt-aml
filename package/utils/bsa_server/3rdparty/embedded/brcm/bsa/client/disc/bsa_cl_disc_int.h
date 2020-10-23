/*****************************************************************************
 **
 **  Name:           bsa_cl_disc_int.h
 **
 **  Description:    Contains private BSA data for Client Discovery
 **
 **  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CL_DISC_INT_H
#define BSA_CL_DISC_INT_H

/* Definition */
typedef struct
{
    /* Device discovery section */
    tBSA_DISC_CBACK *cback; /* Callback for new devices found and completion */
    int nb_devices; /* Max number of elements in devices table*/

    /* Device Info section */
    tBSA_DISC_CBACK *di_cback; /* Callback for Device Info */
    BOOLEAN di_in_use; /* True if a device info is pending */
} tBSA_DISC_CB;


/* External variable definition */

extern tBSA_DISC_CB bsa_disc_cb;

/*******************************************************************************
 **
 ** Function        bsa_cl_disc_init
 **
 ** Description     Initialize Client Discovery data
 **
 ** Parameters      none
 **
 ** Returns         Status
 **
 *******************************************************************************/
tBSA_STATUS bsa_cl_disc_init(void);

/*******************************************************************************
 **
 ** Function       bsa_disc_event_hdlr
 **
 ** Description
 **
 ** Parameters     none
 **
 ** Returns        0 if ok, -1 otherwise
 **
 *******************************************************************************/
extern void bsa_disc_event_hdlr(int message_id, tBSA_DISC_MSG *p_data,
        int length);

#endif


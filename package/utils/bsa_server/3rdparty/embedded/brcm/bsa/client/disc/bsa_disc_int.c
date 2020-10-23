/*****************************************************************************
 **
 **  Name:           bsa_disc_int.c
 **
 **  Description:    This is the internal  interface file for discovery part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"
#include "bsa_cl_disc_int.h"

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
tBSA_STATUS bsa_cl_disc_init(void)
{
    memset(&bsa_disc_cb, 0, sizeof(bsa_disc_cb));
    return BSA_SUCCESS;
}

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
void bsa_disc_event_hdlr(int message_id, tBSA_DISC_MSG *p_data, int length)
{
    tBSA_DISC_CBACK *disc_cback;

    switch (message_id)
    {
    case BSA_DISC_MSGID_DISC_NEW_EVT: /* New Device discovered*/
        /* If user provided a callback */
        if (bsa_disc_cb.cback != NULL)
        {
            /* Call user callback (forward the event) */
            bsa_disc_cb.cback(BSA_DISC_NEW_EVT, p_data);
        }
        break;

    case BSA_DISC_MSGID_DISC_CMPL_EVT:
        /* If user provided a callback */
        if (bsa_disc_cb.cback != NULL)
        {
            /* Call user callback (forward the event) */
            disc_cback = bsa_disc_cb.cback;
            bsa_disc_cb.cback = NULL;
            disc_cback(BSA_DISC_CMPL_EVT, NULL);
        }
        break;

    case BSA_DISC_MSGID_DISC_DEV_INFO_EVT:
        /* If user provided a callback */
        if (bsa_disc_cb.cback != NULL)
        {
            /* Call user callback (forward the event) */
            bsa_disc_cb.cback(BSA_DISC_DEV_INFO_EVT, p_data);
        }
        break;

    default:
        APPL_TRACE_ERROR1("bsa_disc_event_hdlr unknown message_id:%d", message_id);
        break;
    }
}



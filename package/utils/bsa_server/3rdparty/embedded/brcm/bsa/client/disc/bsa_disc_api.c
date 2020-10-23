/*****************************************************************************
 **
 **  Name:           bsa_disc_api.c
 **
 **  Description:    This is the public interface file for discovery part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include "bsa_api.h"
#include "bsa_int.h"
#include "bsa_cl_disc_int.h"

/*
 * Global variables
 */
tBSA_DISC_CB bsa_disc_cb;

/*******************************************************************************
 **
 ** Function         BSA_DiscStartInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscStartInit(tBSA_DISC_START *p_disc_start)
{
    APPL_TRACE_API0("BSA_DiscStartInit");

    if (p_disc_start == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_DiscStartInit p_disc_start is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disc_start, 0, sizeof(*p_disc_start));
    p_disc_start->duration = 8;
    p_disc_start->mode = BSA_DM_GENERAL_INQUIRY;
    p_disc_start->filter_type = BSA_DM_INQ_CLR;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function       BSA_DiscStart
 **
 ** Description    Discovers devices and associated services.
 **                Table element's with in_use field set to FALSE will be filled
 **                with discovered device's parameters.
 **                The in_use fields will be set to TRUE
 **
 ** Parameters     pointer on discovery parameters structure
 **
 ** Returns        tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscStart(tBSA_DISC_START *p_disc_start)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_DiscStart");

    if (p_disc_start == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_DiscStart p_disc_start is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* If cback == NULL, this is a blocking call */
    if (p_disc_start->cback == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_DiscStart requires a callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Update Disc CB */
    bsa_disc_cb.cback = p_disc_start->cback;
    bsa_disc_cb.nb_devices = p_disc_start->nb_devices;

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_DISC_MSGID_DISC_START_CMD, p_disc_start,
            sizeof(tBSA_DISC_MSGID_DISC_START_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1(
                "BSA_DiscStart bsa_send_receive_message fails status:%d",
                ret_code);
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_DiscAbortInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscAbortInit(tBSA_DISC_ABORT *p_disc_abort)
{
    APPL_TRACE_API0("BSA_DiscAbortInit");
    if (p_disc_abort == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_DiscAbortInit p_disc_abort is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disc_abort, 0, sizeof(tBSA_DISC_ABORT));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function       BSA_DiscAbort
 **
 ** Description    Cancels a device Discovery request
 **
 ** Parameters     Pointer on structure containing API parameters
 **
 ** Returns        tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DiscAbort(tBSA_DISC_ABORT *p_req)
{
    APPL_TRACE_API0("BSA_DiscAbort");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_DISC_MSGID_DISC_ABORT_CMD,
            p_req, sizeof(tBSA_DISC_ABORT));
}


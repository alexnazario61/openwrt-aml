/*****************************************************************************
 **
 **  Name:           bsa_nsa_api.c
 **
 **  Description:    This is the public interface file for NSA part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include "bsa_api.h"
#include "bsa_int.h"

/*
 * Global variables
 */

/*******************************************************************************
 **
 ** Function         BSA_NsaAddInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaAddIfInit(tBSA_NSA_ADD_IF *p_req)
{
    APPL_TRACE_API0("BSA_NsaAddIfInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_NsaAddIf
 **
 ** Description      This function adds an NSA Interface
 **
 ** Returns          tNSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaAddIf(tBSA_NSA_ADD_IF *p_req)
{
    tBSA_NSA_MSGID_ADD_IF_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_NsaAddIf");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_NsaAddIf param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_NSA_MSGID_ADD_IF_CMD, p_req,
            sizeof(tBSA_NSA_MSGID_ADD_IF_CMD_REQ), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_NsaAddIf fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    p_req->port = rsp.port;
    ret_code = rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_NsaRemoveIfInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaRemoveIfInit(tBSA_NSA_REMOVE_IF *p_req)
{
    APPL_TRACE_API0("BSA_NsaRemoveIfInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}


/*******************************************************************************
 **
 ** Function         BSA_NsaRemove
 **
 ** Description      This function removes an NSA Interface
 **
 ** Returns          tNSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_NsaRemoveIf(tBSA_NSA_REMOVE_IF *p_req)
{
    APPL_TRACE_API0("BSA_NsaRemoveIf");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_NsaRemoveIf param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_NSA_MSGID_REMOVE_IF_CMD, p_req,
            sizeof(tBSA_NSA_MSGID_REMOVE_IF_CMD_REQ));
}


/*****************************************************************************
 **
 **  Name:           bsa_ops_api.h
 **
 **  Description:    This is the public interface file for Object Push Server part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"

/*
 * Global variables
 */
tBSA_OPS_CB bsa_ops_cb = { NULL };

void default_app_ops_cback(tBSA_OPS_EVT event, tBSA_OPS_MSG *p_data)
{
        APPL_TRACE_ERROR0("default_app_ops_cback NULL Callback");
}

/*******************************************************************************
 **
 ** Function         BSA_OpsEnableInit
 **
 ** Description      Initialize the tBSA_OPS_ENABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_OpsEnableInit(tBSA_OPS_ENABLE *ops_enable_req)
{

    APPL_TRACE_API0("BSA_OpsEnableInit");

    ops_enable_req->sec_mask = BSA_SEC_NONE;
    ops_enable_req->api_fmt = BSA_OP_OBJECT_FMT;
    ops_enable_req->formats = BSA_OP_ANY_MASK;
    strncpy(ops_enable_req->service_name, "Object Push Server Service", BSA_OP_SERVICE_NAME_LEN_MAX-1);
    ops_enable_req->service_name[BSA_OP_SERVICE_NAME_LEN_MAX-1] = 0;
    ops_enable_req->p_cback = default_app_ops_cback;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_OpsEnable
 **
 ** Description      Enable the object push server.  This function must be
 **                  called before any other functions in the OPS API are called.
 **                  When the enable operation is complete the function returns.
 **
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpsEnable(tBSA_OPS_ENABLE *p_ops_enable_req)
{
    tBSA_OPS_MSGID_ENABLE_CMD_RSP ops_enable_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_OpsEnable");

    if (p_ops_enable_req->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_OpsEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    if (strlen(p_ops_enable_req->service_name) > BSA_OP_SERVICE_NAME_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_OpsEnable NAME_TOO_LONG");
        return BSA_ERROR_CLI_NAME_TOO_LONG;
    }

    /* Save application's Security callback */
    bsa_ops_cb.p_app_ops_cback = p_ops_enable_req->p_cback;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_OPS_MSGID_ENABLE_CMD, p_ops_enable_req,
            sizeof(tBSA_OPS_ENABLE), &ops_enable_rsp, sizeof(ops_enable_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_OpsEnable bsa_send_receive_message error %x", ret_code);
        return ret_code;
    }

    return ops_enable_rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_OpsDisableInit
 **
 ** Description      Initialize the tBSA_OPS_DISABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpsDisableInit(tBSA_OPS_DISABLE *ops_disable_req)
{
    APPL_TRACE_API0("BSA_OpsDisableInit");

    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_OpsDisable
 **
 ** Description      Disable the file transfer server.  If the server is currently
 **                  connected to a peer device the connection will be closed.
 **
 ** Returns          int: Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_OpsDisable(tBSA_OPS_DISABLE *ops_disable_req)
{
    APPL_TRACE_API0("BSA_OpsDisable");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_OPS_MSGID_DISABLE_CMD, ops_disable_req, sizeof(tBSA_OPS_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_OpsCloseInit
 **
 ** Description      Initialize the tBSA_OPS_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_OpsCloseInit(tBSA_OPS_CLOSE *ops_close_req)
{
    tBSA_STATUS ret_code = 0;

    APPL_TRACE_API0("BSA_OpsCloseInit");

    return ret_code;
}
/*******************************************************************************
 **
 ** Function         BSA_OpsClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the object
 **                  push is completed.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpsClose(tBSA_OPS_CLOSE *ops_close_req)
{
    APPL_TRACE_API0("BSA_OpsClose");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_OPS_MSGID_CLOSE_CMD, ops_close_req, sizeof(tBSA_OPS_CLOSE));
}

/*******************************************************************************
 **
 ** Function         BSA_OpsAccessInit
 **
 ** Description      Initialize the tBSA_OPS_ACCESS structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_OpsAccessInit(tBSA_OPS_ACCESS *ops_access)
{

    APPL_TRACE_API0("BSA_OpsAccessInit");

    ops_access->oper = BSA_OP_OPER_PUSH;
    ops_access->access = BSA_OP_ACCESS_FORBID;
    strncpy(ops_access->object_name, "./None", BSA_OP_OBJECT_NAME_LEN_MAX-1);
    ops_access->object_name[BSA_OP_OBJECT_NAME_LEN_MAX-1] = 0;
    return 0;
}
/*******************************************************************************
 **
 ** Function         BSA_OpsAccess
 **
 ** Description      Can be sent to enable auto accept or in reply to an access request event
 **                 (BSA_OPS_ACCESS_EVT).
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpsAccess(tBSA_OPS_ACCESS *ops_access)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_OpsAccess");

    if (strlen(ops_access->object_name) > BSA_OP_OBJECT_NAME_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_OpsAccess NAME_TOO_LONG");
        return BSA_ERROR_CLI_NAME_TOO_LONG;
    }


    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_OPS_MSGID_ACCESS_CMD, ops_access,
            sizeof(tBSA_OPS_ACCESS), &ret_code, sizeof(ret_code));
    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_OpsAccess bsa_send_receive_message error %x", ret_code);
        return ret_code;
    }
    return ret_code;

}


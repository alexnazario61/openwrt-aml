/*****************************************************************************
 **
 **  Name:           bsa_pbs_api.c
 **
 **  Description:    This is the public interface file for PBAP server part of
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
tBSA_PBS_CB bsa_pbs_cb =
{
    NULL
};

/*******************************************************************************
 **
 ** Function         BSA_PbsEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsEnableInit(tBSA_PBS_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_PbsEnableInit");

    if (p_enable == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_PBS_ENABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_PbsEnable
 **
 ** Description      This function enables PBAP server and registers PBAB with
 **                  lower layers.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsEnable(tBSA_PBS_ENABLE *p_enable)
{
    tBSA_PBS_MSGID_ENABLE_CMD_REQ pbs_enable_req;

    APPL_TRACE_API0("BSA_PbsEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsEnable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (strlen(p_enable->service_name) > BSA_FTS_SERVICE_NAME_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_PbsEnable Service Name too long");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    if (strlen(p_enable->root_path) > BSA_FTS_ROOT_PATH_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_PbsEnable Root path too long");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    if (strlen(p_enable->realm) > BSA_FT_REALM_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_PbsEnable Realm too long");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(&pbs_enable_req, 0, sizeof(pbs_enable_req));

    /* Save application's Security callback */
    bsa_pbs_cb.p_app_pbs_cback = p_enable->p_cback;

    /* Prepare request parameters */
    pbs_enable_req.sec_mask = p_enable->sec_mask;

    strncpy(pbs_enable_req.service_name, p_enable->service_name,
            sizeof(pbs_enable_req.service_name) - 1);
    pbs_enable_req.service_name[BSA_FTS_SERVICE_NAME_LEN_MAX - 1] = '\0';

    strncpy(pbs_enable_req.root_path, p_enable->root_path,
            sizeof(pbs_enable_req.root_path) - 1);

    pbs_enable_req.root_path[BSA_FTS_ROOT_PATH_LEN_MAX - 1] = '\0';

    pbs_enable_req.enable_authen = p_enable->enable_authen;

    strncpy(pbs_enable_req.realm, p_enable->realm, sizeof(p_enable->realm) - 1);
    pbs_enable_req.realm[BSA_FT_REALM_LEN_MAX - 1] = '\0';

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_PBS_MSGID_ENABLE_CMD,
            &pbs_enable_req, sizeof(tBSA_PBS_MSGID_ENABLE_CMD_REQ));

}

/*******************************************************************************
 **
 ** Function         BSA_PbsDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDisableInit(tBSA_PBS_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_PbsDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_PBS_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_PbsDisable
 **
 ** Description      This function is called when the host is about power down.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDisable(tBSA_PBS_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_PbsDisable");
    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsDisable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    bsa_pbs_cb.p_app_pbs_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_PBS_MSGID_DISABLE_CMD, p_disable,
            sizeof(tBSA_PBS_DISABLE));

}

/*******************************************************************************
 **
 ** Function         BSA_PbsAuthRspInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAuthRspInit(tBSA_PBS_AUTHRSP *p_authrsp)
{
    APPL_TRACE_API0("BSA_PbsAuthRspInit");

    if (p_authrsp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsAuthRspInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_authrsp, 0, sizeof(tBSA_PBS_AUTHRSP));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_PbsAuthRsp
 **
 ** Description      This function is called to send OBEX an authentication
 **                  challenge to the connected OBEX client.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAuthRsp(tBSA_PBS_AUTHRSP *p_authrsp)
{
    APPL_TRACE_API0("BSA_PbsAuthRsp");
    if (p_authrsp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsAuthRsp param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_PBS_MSGID_AUTH_RSP_CMD, p_authrsp,
            sizeof(tBSA_PBS_AUTHRSP));

}

/*******************************************************************************
 **
 ** Function         BSA_PbsAccessRspInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAccessRspInit(tBSA_PBS_ACCESSRSP *p_accessrsp)
{
    APPL_TRACE_API0("BSA_PbsAccessRspInit");
    if (p_accessrsp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsAccessRspInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_accessrsp, 0, sizeof(tBSA_PBS_ACCESSRSP));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_PbsAccessRsp
 **
 ** Description      This function is called to reply to an access request event
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAccessRsp(tBSA_PBS_ACCESSRSP *p_accessrsp)
{
    APPL_TRACE_API0("BSA_PbsAccessRsp");
    if (p_accessrsp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbsAccessRsp param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_PBS_MSGID_ACCESS_RSP_CMD, p_accessrsp,
            sizeof(tBSA_PBS_ACCESSRSP));

}


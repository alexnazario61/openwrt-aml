/*****************************************************************************
 **
 **  Name:           bsa_hl_api.h
 **
 **  Description:    This is the public interface file for Health part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2011-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_cl_hl_int.h"

/*
 * Global variables
 */
tBSA_HL_CB bsa_hl_cb = {NULL, NULL};

/*******************************************************************************
 **
 ** Function         BSA_HlEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlEnableInit(tBSA_HL_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_HlEnableInit");
    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlEnableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlEnable
 **
 ** Description      This function enable Health profile.
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlEnable(tBSA_HL_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_HlEnable");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlEnable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_req->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Save application's HL callback */
    bsa_hl_cb.p_cback = p_req->p_cback;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_ENABLE_CMD,
            p_req, sizeof(tBSA_HL_MSGID_ENABLE_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HlDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlDisableInit(tBSA_HL_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_HlDisableInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlDisable
 **
 ** Description      This function is called when the host is about power down.
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlDisable(tBSA_HL_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_HlDisable");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlDisable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Clear application's Security callback */
    bsa_hl_cb.p_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_DISABLE_CMD, p_req,
            sizeof(tBSA_HL_MSGID_DISABLE_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HlRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlRegisterInit(tBSA_HL_REGISTER *p_req)
{
    APPL_TRACE_API0("BSA_HlRegisterInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlRegisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_REGISTER));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlRegister
 **
 ** Description      This function is called to register an application.
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlRegister(tBSA_HL_REGISTER *p_req)
{
    tBSA_HL_MSGID_REGISTER_CMD_REQ hl_register_req;
    tBSA_HL_MSGID_REGISTER_CMD_RSP hl_register_rsp;
    tBSA_HL_MSGID_MDEP_CFG_CMD_REQ hl_mdep_cfg;
    tBSA_HL_MDEP_CFG_REG *p_mdep_cfg_reg;
    tBSA_STATUS ret_code;
    UINT8 mdep_idx;

    APPL_TRACE_API0("BSA_HlRegister");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlRegister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if ((p_req->num_of_mdeps == 0) ||
        (p_req->num_of_mdeps > BSA_HL_NUM_MDEPS_MAX))
    {
        APPL_TRACE_ERROR1("BSA_HlRegister bad num_of_mdeps:%d", p_req->num_of_mdeps);
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /*
     * Full Health application/Service does not fit in a packet sent to the Control
     * Socket. Send the configuration of every MDEP one by one before sending
     * the main HL Register message
     */
    for (mdep_idx = 0; mdep_idx < p_req->num_of_mdeps ; mdep_idx++)
    {
        p_mdep_cfg_reg = &p_req->mdep[mdep_idx];

        /* Check the Number of Data Type of this MDEP */
        if ((p_mdep_cfg_reg->num_of_mdep_data_types == 0) ||
            (p_mdep_cfg_reg->num_of_mdep_data_types > BSA_HL_NUM_DATA_TYPES_MAX))
        {
            APPL_TRACE_ERROR2("BSA_HlRegister bad NumDataType:%d for MDEP index:%d",
                    p_mdep_cfg_reg->num_of_mdep_data_types, mdep_idx);
            return BSA_ERROR_CLI_BAD_PARAM;
        }

        /* Prepare a request to send one MDEP configuration */
        hl_mdep_cfg.mdep_index = mdep_idx;
        hl_mdep_cfg.mdep_role = p_mdep_cfg_reg->mdep_role;
        hl_mdep_cfg.num_of_mdep_data_types = p_mdep_cfg_reg->num_of_mdep_data_types;
        memcpy(&hl_mdep_cfg.data_cfg[0], p_mdep_cfg_reg->data_cfg ,
                sizeof(tBSA_HL_MDEP_DATA_TYPE_CFG) * p_mdep_cfg_reg->num_of_mdep_data_types);

        /* Call server (send/receive message) */
        ret_code = bsa_send_message_receive_status(BSA_HL_MSGID_MDEP_CFG_CMD, &hl_mdep_cfg,
                sizeof(tBSA_HL_MDEP_CFG_REG));
        if (ret_code != BSA_SUCCESS)
        {
            APPL_TRACE_ERROR2("BSA_HlRegister fails (MDEP Cfg) Index:%d status:%d",
                    mdep_idx, ret_code);
            return ret_code;
        }
    }

    /* Build the Request (abstract of tBSA_HL_REGISTER) */
    BCM_STRNCPY_S(hl_register_req.service_name, BSA_HL_SERVICE_NAME_LEN_MAX,
                p_req->service_name, BSA_HL_SERVICE_NAME_LEN_MAX-1);
    hl_register_req.service_name[BSA_HL_SERVICE_NAME_LEN_MAX-1] = '0';

    BCM_STRNCPY_S(hl_register_req.service_desc, BSA_HL_SERVICE_DESC_LEN_MAX,
                p_req->service_desc, BSA_HL_SERVICE_DESC_LEN_MAX-1);
    hl_register_req.service_desc[BSA_HL_SERVICE_DESC_LEN_MAX-1] = '0';

    BCM_STRNCPY_S(hl_register_req.provider_name, BSA_HL_PROVIDER_NAME_LEN_MAX,
                p_req->provider_name, BSA_HL_PROVIDER_NAME_LEN_MAX-1);
    hl_register_req.provider_name[BSA_HL_PROVIDER_NAME_LEN_MAX-1] = '0';

    hl_register_req.num_of_mdeps = p_req->num_of_mdeps;
    hl_register_req.advertize_source = p_req->advertize_source;
    hl_register_req.sec_mask = p_req->sec_mask;
    memcpy(hl_register_req.saved_mdl, p_req->saved_mdl, sizeof(hl_register_req.saved_mdl));

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HL_MSGID_REGISTER_CMD, &hl_register_req,
            sizeof(tBSA_HL_MSGID_REGISTER_CMD_REQ),
            &hl_register_rsp, sizeof(hl_register_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HlRegister fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hl_register_rsp.status;
    p_req->app_handle = hl_register_rsp.app_handle;

    /* Retrieve MDEP ID allocated by server for every MDEP */
    for (mdep_idx = 0 ; mdep_idx < p_req->num_of_mdeps ; mdep_idx++)
    {
        p_req->mdep[mdep_idx].mdep_id = hl_register_rsp.mdep_id_tab[mdep_idx];
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HlDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlDeregisterInit(tBSA_HL_DEREGISTER *p_req)
{
    APPL_TRACE_API0("BSA_HlDeregisterInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlDeregisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_DEREGISTER));
    p_req->app_handle = BSA_HL_BAD_APP_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlDeregister
 **
 ** Description      This function is called to deregister an application.
 **
 ** Returns          void
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_HlDeregister(tBSA_HL_DEREGISTER *p_req)
{
    APPL_TRACE_API0("BSA_HlDeregister");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlDeregister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_DEREGISTER_CMD, p_req,
            sizeof(tBSA_HL_MSGID_DEREGISTER_CMD_REQ));
}
/*******************************************************************************
 **
 ** Function         BSA_HlSdpQueryInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlSdpQueryInit(tBSA_HL_SDP_QUERY *p_req)
{
    APPL_TRACE_API0("BSA_HlSdpQueryInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlSdpQueryInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_SDP_QUERY));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlSdpQuery
 **
 ** Description      This function is used to SdpQuery (SDP) an Health peer device
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlSdpQuery(tBSA_HL_SDP_QUERY *p_req)
{
    APPL_TRACE_API0("BSA_HlSdpQuery");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlSdpQuery param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (bsa_hl_cb.p_sdp != NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlSdpQuery SDP query already pending");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Allocate a buffer to hold the answers */
    bsa_hl_cb.p_sdp = GKI_getbuf(sizeof(tBSA_HL_SDP_QUERY_MSG));
    if (bsa_hl_cb.p_sdp == NULL)
    {
        APPL_TRACE_ERROR1("ERROR BSA_HlSdpQuery cannot allocate GKI buffer size:%d",
                sizeof(tBSA_HL_SDP_QUERY_MSG));
        return BSA_ERROR_CLI_MEM_FULL;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_SDP_QUERY_CMD, p_req,
            sizeof(tBSA_HL_MSGID_SDP_QUERY_CMD_REQ));
}
/*******************************************************************************
 **
 ** Function         BSA_HlOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlOpenInit(tBSA_HL_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_HlOpenInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_OPEN));
    p_req->app_handle = BSA_HL_BAD_APP_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlOpen
 **
 ** Description      This function is used to Open a Data Link to a peer Health device
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlOpen(tBSA_HL_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_HlOpen");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_OPEN_CMD, p_req,
            sizeof(tBSA_HL_MSGID_OPEN_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HlOpenRspInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlOpenRspInit(tBSA_HL_OPEN_RSP *p_req)
{
    APPL_TRACE_API0("BSA_HlOpenRspInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlOpenRspInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_OPEN_RSP));
    p_req->app_handle = BSA_HL_BAD_APP_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlOpen
 **
 ** Description      This function is used to Respond to a Data Link Open Request from
 **                  a peer Health device
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlOpenRsp(tBSA_HL_OPEN_RSP *p_req)
{
    APPL_TRACE_API0("BSA_HlOpenRsp");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlOpenRsp param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_OPEN_RSP_CMD, p_req,
            sizeof(tBSA_HL_MSGID_OPEN_RSP_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HlCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlCloseInit(tBSA_HL_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_HlCloseInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_CLOSE));
    p_req->data_handle = BSA_HL_BAD_DATA_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlOpen
 **
 ** Description      This function is used to close a Data Link
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlClose(tBSA_HL_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_HlClose");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HlClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_CLOSE_CMD, p_req,
            sizeof(tBSA_HL_MSGID_CLOSE_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HlReconnectInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlReconnectInit(tBSA_HL_RECONNECT *p_req)
{
    APPL_TRACE_API0("BSA_HlReconnectInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlReconnectInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HL_RECONNECT));
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_HlReconnect
 **
 ** Description      This function is used to Reconnect a Data Link
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_HlReconnect(tBSA_HL_RECONNECT *p_req)
{
    APPL_TRACE_API0("BSA_HlReconnect");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlReconnect param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_RECONNECT_CMD, p_req,
            sizeof(tBSA_HL_MSGID_RECONNECT_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HlDeleteMdlInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDeleteMdlInit(tBSA_HL_DELETE_MDL *p_req)
{
    APPL_TRACE_API0("BSA_HlDeleteMdlInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlDeleteMdlInit Init param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HlDeleteMdl
 **
 ** Description      This function is used to Delete a Data Link
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
BTA_API extern tBSA_STATUS BSA_HlDeleteMdl(tBSA_HL_DELETE_MDL *p_req)
{
    APPL_TRACE_API0("BSA_HlDeleteMdl");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_HlDeleteMdl param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HL_MSGID_DELETE_MDL_CMD, p_req,
            sizeof(tBSA_HL_MSGID_DELETE_MDL_CMD_REQ));
}

#ifdef __cplusplus
}
#endif


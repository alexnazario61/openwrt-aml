/*****************************************************************************
 **
 **  Name:           bsa_ble_api.c
 **
 **  Description:   This is the public interface file for BLE part of
 **                 the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include "bsa_api.h"
#include "bsa_int.h"

/*
 * Global variables
 */
tBSA_BLE_CB bsa_ble_cb;


/*******************************************************************************
 **
 ** Function         BSA_BleEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleEnableInit(tBSA_BLE_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_BleEnableInit");
    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleEnableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_BLE_ENABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleEnable
 **
 ** Description      This function enable BLE 
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleEnable(tBSA_BLE_ENABLE *p_enable)
{
    tBSA_BLE_MSGID_ENABLE_CMD_REQ ble_enable_req;

    APPL_TRACE_API0("BSA_BleEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleEnable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_MSGID_ENABLE_CMD,
            &ble_enable_req, sizeof(tBSA_BLE_MSGID_ENABLE_CMD_REQ));

}

/*******************************************************************************
 **
 ** Function         BSA_BleDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleDisableInit(tBSA_BLE_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_BleDisableInit");
    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_BLE_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleDisable
 **
 ** Description      This function disable BLE
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleDisable(tBSA_BLE_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_BleDisable");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleDisable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_MSGID_DISABLE_CMD,
            p_disable, sizeof(tBSA_BLE_DISABLE));

}

/*******************************************************************************
 **
 ** Function         BSA_BleClAppRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppRegisterInit(tBSA_BLE_CL_REGISTER *p_reg)
{
    APPL_TRACE_API0("BSA_BleClAppRegisterInit");

    if (p_reg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClAppRegisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_reg, 0, sizeof(tBSA_BLE_CL_REGISTER));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClAppRegister
 **
 ** Description      This function is called to register client application 
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppRegister(tBSA_BLE_CL_REGISTER *p_reg)
{
    tBSA_STATUS ret_code;
    tBSA_BLE_CL_MSGID_REGISTER_CMD_RSP rsp;

    APPL_TRACE_API0("BSA_BleClAppRegister");

    if (p_reg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClAppRegister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_reg->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClAppRegister call back is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Save application's callback */
    bsa_ble_cb.p_app_ble_client_cback = p_reg->p_cback;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_BLE_CL_MSGID_REGISTER_CMD, p_reg,
            sizeof(tBSA_BLE_CL_REGISTER), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_BleSeAppRegister fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    p_reg->client_if = rsp.client_if;
    APPL_TRACE_API1("BSA_BleClAppRegister if num:%d",p_reg->client_if);

    return rsp.status;

}

/*******************************************************************************
 **
 ** Function         BSA_BleClConnectInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClConnectInit(tBSA_BLE_CL_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_BleClConnectInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClConnectInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_open, 0, sizeof(tBSA_BLE_CL_OPEN));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClConnect
 **
 ** Description      This function is called to open an BLE connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClConnect(tBSA_BLE_CL_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_BleClConnect");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClConnect param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_OPEN_CMD, p_open,
            sizeof(tBSA_BLE_CL_OPEN));

}

/*******************************************************************************
 **
 ** Function         BSA_BleClSearchInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClSearchInit(tBSA_BLE_CL_SEARCH *p_search)
{
    APPL_TRACE_API0("BSA_BleClSearchInit");

    if (p_search == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClSearchInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_search, 0, sizeof(tBSA_BLE_CL_SEARCH));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClServiceSearch
 **
 ** Description      This function is called to search service list to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClSearch(tBSA_BLE_CL_SEARCH *p_search)
{
    APPL_TRACE_API0("BSA_BleClSearch");

    if (p_search == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClSearch param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_SEARCH_CMD, p_search,
            sizeof(tBSA_BLE_CL_SEARCH));

}

/*******************************************************************************
 **
 ** Function         BSA_BleClReadInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClReadInit(tBSA_BLE_CL_READ *p_read)
{
    APPL_TRACE_API0("BSA_BleClReadInit");

    if (p_read == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClReadInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_read, 0, sizeof(tBSA_BLE_CL_READ));
    p_read->char_id.srvc_id.id.uuid.len = 2;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClRead
 **
 ** Description      This function is called to read a data to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClRead(tBSA_BLE_CL_READ *p_read)
{
    APPL_TRACE_API0("BSA_BleClRead");

    if (p_read == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClRead param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if(p_read->char_id.srvc_id.id.uuid.len != 2)
    {
        APPL_TRACE_ERROR0("BSA_BleClRead param uuid invalid length");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_READ_CMD, p_read,
            sizeof(tBSA_BLE_CL_READ));

}

/*******************************************************************************
 **
 ** Function         BSA_BleClWriteInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClWriteInit(tBSA_BLE_CL_WRITE *p_write)
{
    APPL_TRACE_API0("BSA_BleClWriteInit");

    if (p_write == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClWriteInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_write, 0, sizeof(tBSA_BLE_CL_WRITE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClWrite
 **
 ** Description      This function is called to write a data to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClWrite(tBSA_BLE_CL_WRITE *p_write)
{
    APPL_TRACE_API0("BSA_BleClWrite");

    if (p_write == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClWrite param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_WRITE_CMD, p_write,
            sizeof(tBSA_BLE_CL_WRITE));
}

/*******************************************************************************
 **
 ** Function         BSA_BleClIndConfInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClIndConfInit(tBSA_BLE_CL_INDCONF *p_indc)
{
    APPL_TRACE_API0("BSA_BleClIndConfInit");

    if (p_indc == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClIndConfInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_indc, 0, sizeof(tBSA_BLE_CL_INDCONF));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClIndConf
 **
 ** Description      This function is called to send a confirmation to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClIndConf(tBSA_BLE_CL_INDCONF *p_indc)
{
    APPL_TRACE_API0("BSA_BleClIndConf");

    if (p_indc == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClIndConf param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_INDCONF_CMD, p_indc,
            sizeof(tBSA_BLE_CL_INDCONF));
}

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifRegisterInit(tBSA_BLE_CL_NOTIFREG *p_reg)
{
    APPL_TRACE_API0("BSA_BleClNotifRegisterInit");

    if (p_reg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClNotifRegisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_reg, 0, sizeof(tBSA_BLE_CL_NOTIFREG));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifRegister
 **
 ** Description      This function is called to register a notification
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifRegister(tBSA_BLE_CL_NOTIFREG *p_reg)
{
    APPL_TRACE_API0("BSA_BleClNotifRegister");

    if (p_reg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClNotifRegister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_NOTIFREG_CMD, p_reg,
            sizeof(tBSA_BLE_CL_NOTIFREG));

}

/*******************************************************************************
 **
 ** Function         BSA_BleClCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClCloseInit(tBSA_BLE_CL_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_BleClCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_close, 0, sizeof(tBSA_BLE_CL_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClClose
 **
 ** Description      This function is called to close an BLE connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClClose(tBSA_BLE_CL_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_BleClClose");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_CLOSE_CMD, p_close,
            sizeof(tBSA_BLE_CL_CLOSE));
}


/*******************************************************************************
 **
 ** Function         BSA_BleClAppDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppDeregisterInit(tBSA_BLE_CL_DEREGISTER *p_dereg)
{
    APPL_TRACE_API0("BSA_BleClAppDeregisterInit");

    if (p_dereg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClAppDeregisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_dereg, 0, sizeof(tBSA_BLE_CL_DEREGISTER));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClAppDeregister
 **
 ** Description      This function is called to deregister app
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClAppDeregister(tBSA_BLE_CL_DEREGISTER *p_dereg)
{
    APPL_TRACE_API0("BSA_BleClAppDeregister");

    if (p_dereg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClAppDeregister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_DEREGISTER_CMD, p_dereg,
            sizeof(tBSA_BLE_CL_DEREGISTER));
}

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifDeregisterInit(tBSA_BLE_CL_NOTIFDEREG *p_dereg)
{
    APPL_TRACE_API0("BSA_BleClNotifDeregisterInit");

    if (p_dereg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClNotifDeregisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_dereg, 0, sizeof(tBSA_BLE_CL_NOTIFDEREG));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClNotifDeregister
 **
 ** Description      This function is called to deregister app
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClNotifDeregister(tBSA_BLE_CL_NOTIFDEREG *p_dereg)
{
    APPL_TRACE_API0("BSA_BleClNotifDeregister");

    if (p_dereg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClNotifDeregister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_NOTIFDEREG_CMD, p_dereg,
            sizeof(tBSA_BLE_CL_NOTIFDEREG));
}


/*******************************************************************************
 **
 ** Function         BSA_BleClCacheLoadInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClCacheLoadInit(tBSA_BLE_CL_CACHE_LOAD *p_load)
{
    APPL_TRACE_API0("BSA_BleClCacheLoadInit");

    if (p_load == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClCacheLoadInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_load, 0, sizeof(tBSA_BLE_CL_CACHE_LOAD));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleClCacheLoad
 **
 ** Description      This function is called to load attributes to bsa_server
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleClCacheLoad(tBSA_BLE_CL_CACHE_LOAD *p_load)
{
    APPL_TRACE_API0("BSA_BleClCacheLoad");

    if (p_load == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleClCacheLoad param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_CL_MSGID_CACHE_LOAD_CMD, p_load,
            sizeof(tBSA_BLE_CL_CACHE_LOAD));
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppRegisterInit(tBSA_BLE_SE_REGISTER *p_reg)
{
    APPL_TRACE_API0("BSA_BleSeAppRegisterInit");

    if (p_reg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeAppRegisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_reg, 0, sizeof(tBSA_BLE_SE_REGISTER));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppRegister
 **
 ** Description      This function is called to register server application 
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppRegister(tBSA_BLE_SE_REGISTER *p_reg)
{
    tBSA_STATUS ret_code;
    tBSA_BLE_SE_MSGID_REGISTER_CMD_RSP rsp;

    APPL_TRACE_API0("BSA_BleSeAppRegister");

    if (p_reg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeAppRegister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Save application's callback */
    bsa_ble_cb.p_app_ble_server_cback = p_reg->p_cback;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_BLE_SE_MSGID_REGISTER_CMD, p_reg,
            sizeof(tBSA_BLE_SE_REGISTER), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_BleSeAppRegister fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    p_reg->server_if = rsp.server_if;

    return rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppDeregisterInit(tBSA_BLE_SE_DEREGISTER *p_dereg)
{
    APPL_TRACE_API0("BSA_BleSeAppDeregisterInit");

    if (p_dereg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeAppDeregisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_dereg, 0, sizeof(tBSA_BLE_SE_DEREGISTER));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeAppDeregister
 **
 ** Description      This function is called to register server application 
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAppDeregister(tBSA_BLE_SE_DEREGISTER *p_dereg)
{

    APPL_TRACE_API0("BSA_BleSeAppDeregister");

    if (p_dereg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeAppDeregister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_SE_MSGID_DEREGISTER_CMD, p_dereg,
            sizeof(tBSA_BLE_SE_DEREGISTER));

}

/*******************************************************************************
 **
 ** Function         BSA_BleSeCreateServiceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeCreateServiceInit(tBSA_BLE_SE_CREATE *p_create)
{
    APPL_TRACE_API0("BSA_BleSeCreateServiceInit");

    if (p_create == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeCreateServiceInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_create, 0, sizeof(tBSA_BLE_SE_CREATE));
    p_create->inst = 1;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeCreateService
 **
 ** Description      Create service 
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeCreateService(tBSA_BLE_SE_CREATE *p_create)
{
    APPL_TRACE_API0("BSA_BleSeCreateService");

    if (p_create == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeCreateService param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_SE_MSGID_CREATE_CMD, p_create,
            sizeof(tBSA_BLE_SE_CREATE));
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeAddCharInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAddCharInit(tBSA_BLE_SE_ADDCHAR *p_add)
{
    APPL_TRACE_API0("BSA_BleSeAddCharInit");

    if (p_add == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeAddCharInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_add, 0, sizeof(tBSA_BLE_SE_ADDCHAR));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeAddChar
 **
 ** Description      Create service 
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeAddChar(tBSA_BLE_SE_ADDCHAR *p_add)
{
    APPL_TRACE_API0("BSA_BleSeAddChar");

    if (p_add == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeAddChar param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_SE_MSGID_ADDCHAR_CMD, p_add,
            sizeof(tBSA_BLE_SE_ADDCHAR));
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeStartServiceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeStartServiceInit(tBSA_BLE_SE_START *p_start)
{
    APPL_TRACE_API0("BSA_BleSeStartServiceInit");

    if (p_start == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeStartServiceInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_start, 0, sizeof(tBSA_BLE_SE_START));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeStartService
 **
 ** Description      Start service 
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeStartService(tBSA_BLE_SE_START *p_start)
{
    APPL_TRACE_API0("BSA_BleSeStartService");

    if (p_start == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeStartService param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_SE_MSGID_START_CMD, p_start,
            sizeof(tBSA_BLE_SE_START));

}

/*******************************************************************************
 **
 ** Function         BSA_BleSeSendIndInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeSendIndInit(tBSA_BLE_SE_SENDIND *p_sendind)
{
    APPL_TRACE_API0("BSA_BleSeSendIndInit");

    if (p_sendind == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeSendIndInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sendind, 0, sizeof(tBSA_BLE_SE_SENDIND));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleSeSendInd
 **
 ** Description      Send Indication
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeSendInd(tBSA_BLE_SE_SENDIND *p_sendind)
{
    APPL_TRACE_API0("BSA_BleSeSendInd");

    if (p_sendind == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeSendInd param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_SE_MSGID_SENDIND_CMD, p_sendind,
            sizeof(tBSA_BLE_SE_SENDIND));

}

/*******************************************************************************
 **
 ** Function         BSA_BleSeSendRsp
 **
 ** Description      Send Response to client for write/read request received
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleSeSendRsp(tBSA_BLE_SE_SENDRSP *p_sendrsp)
{
    APPL_TRACE_API0("BSA_BleSeSendRsp");

    if (p_sendrsp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleSeSendRsp param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_BLE_SE_MSGID_SENDRSP_CMD, p_sendrsp,
            sizeof(tBSA_BLE_SE_SENDRSP));

}

/*******************************************************************************
 **
 ** Function         BSA_BleWakeCfgInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeCfgInit(tBSA_BLE_WAKE_CFG *p_req)
{
    APPL_TRACE_API0("BSA_BleWakeCfgInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleWakeCfg 
 **
 ** Description      Function to set controller for Wake on BLE
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeCfg(tBSA_BLE_WAKE_CFG *p_req)
{
    tBSA_STATUS bsa_status;

    APPL_TRACE_API0("BSA_BleWakeCfg");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleWakeCfg param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send message, receive message) */
    bsa_status = bsa_send_receive_message(BSA_BLE_MSGID_WAKE_CFG_CMD,
            p_req, sizeof(tBSA_BLE_MSGID_WAKE_CFG_CMD_REQ), p_req,
            sizeof(tBSA_BLE_MSGID_WAKE_CFG_CMD_RSP));

    if (bsa_status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_BleWakeCfg fails status:%d", bsa_status);
    }

    return bsa_status;
}

/*******************************************************************************
 **
 ** Function         BSA_BleWakeEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeEnableInit(tBSA_BLE_WAKE_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_BleWakeEnableInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_BleWakeEnable 
 **
 ** Description      Function to enable Wake on BLE
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_BleWakeEnable(tBSA_BLE_WAKE_ENABLE *p_req)
{
    tBSA_STATUS bsa_status;

    APPL_TRACE_API0("BSA_BleWakeEnable");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_BleWakeEnable param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send message, receive message) */
    bsa_status = bsa_send_receive_message(BSA_BLE_MSGID_WAKE_ENABLE_CMD,
            p_req, sizeof(tBSA_BLE_MSGID_WAKE_ENABLE_CMD_REQ), p_req,
            sizeof(tBSA_BLE_MSGID_WAKE_ENABLE_CMD_RSP));

    if (bsa_status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_BleWakeEnable fails status:%d", bsa_status);
    }

    return bsa_status;
}


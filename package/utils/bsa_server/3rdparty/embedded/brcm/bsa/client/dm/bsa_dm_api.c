/*****************************************************************************
 **
 **  Name:           bsa_dm_api.c
 **
 **  Description:    Contains DM BSA API client functions
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"
#include "bsa_client.h"

#include "bsa_cl_dm_int.h"

/* Default BdAddr */
#define BSA_DEFAULT_BD_ADDR {0xBE, 0xEF, 0xBE, 0xEF, 0x00, 0x01}

/* Default COD: undefined */
#define BSA_DEFAULT_CLASS_OF_DEVICE {0x00, 0x1F, 0x00}

/* Default local Name */
#define BSA_DEFAULT_BT_NAME "BSA Bluetooth Device"

/*******************************************************************************
 **
 ** Function         BSA_DmGetConfigInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmGetConfigInit(tBSA_DM_GET_CONFIG *p_config)
{
    APPL_TRACE_API0("BSA_DmGetConfigInit");

    if (p_config == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DmGetConfigInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_config, 0, sizeof(tBSA_DM_GET_CONFIG));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DmGetConfig
 **
 ** Description      This function retrieve the current configuration of the
 **                  local Bluetooth device
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmGetConfig(tBSA_DM_GET_CONFIG *p_config)
{
    tBSA_DM_MSGID_GET_CONFIG_CMD_RSP get_config_rsp;
    tBSA_STATUS ret_code;
    int bsa_status = BSA_SUCCESS;
    UINT8 dummy;

    APPL_TRACE_API0("BSA_DmGetConfig");

    if (p_config == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DmGetConfig param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* no request parameter */

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_DM_MSGID_GET_CONFIG_CMD, &dummy, sizeof(dummy),
            &get_config_rsp, sizeof(get_config_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_DmGetConfig fails status:%d", ret_code);
        return ret_code;
    }

    /* Copy the get configuration response structure */
    *p_config = get_config_rsp.config;

    p_config->callback = bsa_cl_dm_cb.callback;

    bsa_status = get_config_rsp.status;

    return bsa_status;
}

/*******************************************************************************
 **
 ** Function         BSA_DmSetConfigInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmSetConfigInit(tBSA_DM_SET_CONFIG *p_config)
{
    BD_ADDR local_bd_addr = BSA_DEFAULT_BD_ADDR;
    DEV_CLASS local_class_of_device = BSA_DEFAULT_CLASS_OF_DEVICE;

    APPL_TRACE_API0("BSA_DmSetConfigInit");

    if (p_config == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DmSetConfigInit param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_config, 0, sizeof(tBSA_DM_SET_CONFIG));
    p_config->config_mask = BSA_DM_CONFIG_DEFAULT_MASK;
    p_config->enable = TRUE;
    p_config->discoverable = TRUE;
    p_config->connectable = TRUE;
    strncpy((char *) p_config->name, BSA_DEFAULT_BT_NAME,
            sizeof(p_config->name));
    bdcpy(p_config->bd_addr, local_bd_addr);
    memcpy(p_config->class_of_device, local_class_of_device, sizeof(DEV_CLASS));
    /* By default, no channels are disabled */
    p_config->first_disabled_channel = BSA_DM_LAST_AFH_CHANNEL;
    p_config->last_disabled_channel = BSA_DM_LAST_AFH_CHANNEL;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DmSetConfig
 **
 ** Description      This function change the current configuration of the
 **                  local Bluetooth device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_DmSetConfig(tBSA_DM_SET_CONFIG *p_config)
{
    tBSA_DM_MSGID_SET_CONFIG_CMD_RSP set_config_rsp;
    tBSA_STATUS bsa_status;

    APPL_TRACE_API0("BSA_DmSetConfig");

    if (p_config == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DmSetConfig param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Callback is required for 3D operation only */
    if (p_config->config_mask & BSA_DM_CONFIG_BRCM_MASK)
    {
        if (p_config->callback == NULL)
        {
            APPL_TRACE_ERROR0("BSA_DmSetConfig callback is NULL");
            return BSA_ERROR_CLI_BAD_PARAM;
        }
        /* Save the Callback */
        bsa_cl_dm_cb.callback = p_config->callback;
    }

    /* Call server (send/receive message) */
    bsa_status = bsa_send_receive_message(BSA_DM_MSGID_SET_CONFIG_CMD,
            (tBSA_DM_MSGID_SET_CONFIG_CMD_REQ *) p_config,
            sizeof(tBSA_DM_MSGID_SET_CONFIG_CMD_REQ), &set_config_rsp,
            sizeof(set_config_rsp));

    if (bsa_status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_DmSetConfig fails status:%d", bsa_status);
        return bsa_status;
    }

    /* Extract received parameters */
    bsa_status = set_config_rsp.status;

    return bsa_status;
}

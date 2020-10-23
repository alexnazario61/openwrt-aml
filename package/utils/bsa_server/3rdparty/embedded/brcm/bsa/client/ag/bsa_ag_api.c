/*****************************************************************************
 **
 **  Name:           bsa_ag_api.c
 **
 **  Description:    This is the public interface file for HS part of
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
tBSA_AG_CB bsa_ag_cb = { NULL, };

/*******************************************************************************
 **
 ** Function         BSA_AgEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgEnableInit(tBSA_AG_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_AgEnableInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgEnableInit: param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_ENABLE));
    p_req->p_cback = NULL;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgEnable
 **
 ** Description      This function enables Headset profile.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgEnable(tBSA_AG_ENABLE *p_req)
{
    tBSA_AG_MSGID_ENABLE_CMD_REQ ag_enable_cmd;

    APPL_TRACE_API0("BSA_AgEnable");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_req->p_cback == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Save application's Security callback */
    bsa_ag_cb.p_app_cback = p_req->p_cback;
    ag_enable_cmd.p_cback = p_req->p_cback;

    return bsa_send_message_receive_status(BSA_AG_MSGID_ENABLE_CMD, &ag_enable_cmd,
            sizeof(tBSA_AG_MSGID_ENABLE_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_AgDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgDisableInit(tBSA_AG_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_AgDisableInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgDisable
 **
 ** Description      This function is called when the host is about power down.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgDisable(tBSA_AG_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_AgDisable");

    return bsa_send_message_receive_status(BSA_AG_MSGID_DISABLE_CMD, p_req,
            sizeof(tBSA_AG_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_AgRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgRegisterInit(tBSA_AG_REGISTER *p_req)
{
    APPL_TRACE_API0("BSA_AgRegisterInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgRegisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_REGISTER));
    p_req->sec_mask = BSA_SEC_NONE;
    p_req->services = BSA_HSP_SERVICE_MASK | BSA_HFP_SERVICE_MASK;
    p_req->sco_route = HCI_BRCM_SCO_ROUTE_HCI;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgRegister
 **
 ** Description      This function registers the Headset service(s) with
 **                  the lower layers.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgRegister(tBSA_AG_REGISTER *p_req)
{
    tBSA_AG_MSGID_REGISTER_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AgRegister");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgRegister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    if(BSA_SUCCESS == (ret_code = bsa_send_receive_message(BSA_AG_MSGID_REGISTER_CMD, p_req,
            sizeof(tBSA_AG_REGISTER), &rsp, sizeof(rsp))))
    {
        /* Extract received parameters */
        p_req->hndl = rsp.hndl;
        p_req->uipc_channel = rsp.uipc_channel;
        ret_code = rsp.status;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AgDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgDeregisterInit(tBSA_AG_DEREGISTER *p_req)
{
    APPL_TRACE_API0("BSA_AgDeregisterInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgDeregisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_DEREGISTER));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgDeregister
 **
 ** Description      This function is called to deregister the headset service.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgDeregister(tBSA_AG_DEREGISTER *p_req)
{
    APPL_TRACE_API0("BSA_AgDeregister");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgDeregister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AG_MSGID_DEREGISTER_CMD, p_req,
            sizeof(tBSA_AG_DEREGISTER));
}

/*******************************************************************************
 **
 ** Function         BSA_AgOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgOpenInit(tBSA_AG_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_AgOpenInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_OPEN));
    p_req->services = BSA_HSP_SERVICE_MASK | BSA_HFP_SERVICE_MASK;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgOpen
 **
 ** Description      This function is called to open a service level
 **                  connection to an audio gateway.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgOpen(tBSA_AG_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_AgOpen");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    APPL_TRACE_API6("BSA_AgOpen BDA: %02x:%02x:%02x:%02x:%02x:%02x",
            p_req->bd_addr[0], p_req->bd_addr[1], p_req->bd_addr[2],
            p_req->bd_addr[3], p_req->bd_addr[4], p_req->bd_addr[5]);
    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AG_MSGID_OPEN_CMD, p_req,
            sizeof(tBSA_AG_OPEN));
}

/*******************************************************************************
 **
 ** Function         BSA_AgCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgCloseInit(tBSA_AG_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_AgCloseInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgClose
 **
 ** Description      This function is called to close a service level
 **                  connection to an audio gateway.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgClose(tBSA_AG_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_AgClose");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AG_MSGID_CLOSE_CMD, p_req,
            sizeof(tBSA_AG_CLOSE));
}

/*******************************************************************************
 **
 ** Function         BSA_AgAudioOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgAudioOpenInit(tBSA_AG_AUDIO_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_AgAudioOpenInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgAudioOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_AUDIO_OPEN));
    p_req->sco_route = HCI_BRCM_SCO_ROUTE_HCI;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgAudioOpen
 **
 ** Description      This function is called to open audio on a service level
 **                  connection to an audio gateway.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgAudioOpen(tBSA_AG_AUDIO_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_AgAudioOpen");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgAudioOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AG_MSGID_AUDIO_OPEN_CMD, p_req,
            sizeof(tBSA_AG_AUDIO_OPEN));
}
/*******************************************************************************
 **
 ** Function         BSA_AgAudioCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgAudioCloseInit(tBSA_AG_AUDIO_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_AgAudioCloseInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgAudioCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_AUDIO_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgAudioClose
 **
 ** Description      This function is called to close audio on a service level
 **                  connection to an audio gateway.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgAudioClose(tBSA_AG_AUDIO_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_AgAudioClose");
    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgAudioClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AG_MSGID_AUDIO_CLOSE_CMD, p_req,
            sizeof(tBSA_AG_AUDIO_CLOSE));
}

/*******************************************************************************
 **
 ** Function         BSA_AgResultInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgResultInit(tBSA_AG_RES *p_req)
{
    APPL_TRACE_API0("BSA_AgResultInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgResultInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AG_RES));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AgResult
 **
 ** Description      This function is called to send an AT command to
 **                  an audio gateway.
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_AgResult(tBSA_AG_RES * p_req)
{
    APPL_TRACE_API0("BSA_AgResult");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgResult param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AG_MSGID_RESULT_CMD, p_req,
            sizeof(tBSA_AG_RES));
}


/*****************************************************************************
**
**  Name:           bsa_hs_api.c
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
tBSA_HS_CB bsa_hs_cb =
{
    NULL,
};

/*******************************************************************************
 **
 ** Function         BSA_HsEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsEnableInit(tBSA_HS_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_HsEnableInit");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsEnableInit: param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_HS_ENABLE));
    p_enable->p_cback = NULL;
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_HsEnable
**
** Description      This function enables Headset profile.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsEnable (tBSA_HS_ENABLE *p_enable)
{
    tBSA_HS_MSGID_ENABLE_CMD_REQ            hs_enable_req;

    APPL_TRACE_API0("BSA_HsEnable");

    if (p_enable == NULL)
    {
            APPL_TRACE_ERROR0("BSA_HsEnable param struct pointer is NULL");
            return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
            APPL_TRACE_ERROR0("BSA_HsEnable NULL Callback");
            return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(&hs_enable_req, 0, sizeof(hs_enable_req));

    /* Save application's Security callback */
    bsa_hs_cb.p_app_hs_cback = p_enable->p_cback;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_ENABLE_CMD,
            &hs_enable_req, sizeof(tBSA_HS_MSGID_ENABLE_CMD_REQ));

}

/*******************************************************************************
 **
 ** Function         BSA_HsDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsDisableInit(tBSA_HS_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_HsDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_HS_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsDisable
**
** Description      This function is called when the host is about power down.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsDisable(tBSA_HS_DISABLE *p_disable)
{
    tBSA_HS_MSGID_DISABLE_CMD_REQ           req;

    APPL_TRACE_API0("BSA_HsDisable");

    /* Save application's Security callback */
    bsa_hs_cb.p_app_hs_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_DISABLE_CMD,
            &req, sizeof(req));
}

/*******************************************************************************
 **
 ** Function         BSA_HsRegisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsRegisterInit(tBSA_HS_REGISTER *p_register)
{
    APPL_TRACE_API0("BSA_HsRegisterInit");

    if (p_register == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsRegisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_register, 0, sizeof(tBSA_HS_REGISTER));
    p_register->sec_mask = BSA_SEC_NONE;
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_HsRegister
**
** Description      This function registers the Headset service(s) with
**                  the lower layers.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsRegister (tBSA_HS_REGISTER *p_register)
{
    tBSA_HS_MSGID_REGISTER_CMD_REQ          hs_register_req;
    tBSA_HS_MSGID_REGISTER_CMD_RSP          hs_register_rsp;
    int                                     ret_code;

    APPL_TRACE_API0("BSA_HsRegister");

    if (p_register == NULL)
    {
            APPL_TRACE_ERROR0("BSA_HsRegister param struct pointer is NULL");
            return BSA_ERROR_CLI_BAD_PARAM;
    }


    if (strlen(p_register->service_name[0]) > BSA_HS_SERVICE_NAME_LEN_MAX)
    {
            APPL_TRACE_ERROR0("BSA_HsRegister Service Name (1) too long");
            return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (strlen(p_register->service_name[1]) > BSA_HS_SERVICE_NAME_LEN_MAX)
    {
            APPL_TRACE_ERROR0("BSA_HsRegister Service Name (2) too long");
            return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(&hs_register_req, 0, sizeof(hs_register_req));

    /* Prepare request parameters */
    hs_register_req.services = p_register->services;
    hs_register_req.sec_mask = p_register->sec_mask;
    hs_register_req.features = p_register->features;
    hs_register_req.settings = p_register->settings;

    strncpy(hs_register_req.service_name[0], p_register->service_name[0], strlen(p_register->service_name[0]));
    hs_register_req.service_name[0][BSA_HS_SERVICE_NAME_LEN_MAX - 1] = '\0';
    strncpy(hs_register_req.service_name[1], p_register->service_name[1], strlen(p_register->service_name[1]));
    hs_register_req.service_name[1][BSA_HS_SERVICE_NAME_LEN_MAX - 1] = '\0';
    hs_register_req.sco_route = p_register->sco_route;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HS_MSGID_REGISTER_CMD,
                                            &hs_register_req,
                                            sizeof(tBSA_HS_MSGID_REGISTER_CMD_REQ),
                                            &hs_register_rsp,
                                            sizeof(hs_register_rsp));

    if (ret_code != BSA_SUCCESS)
    {
            APPL_TRACE_ERROR1("BSA_HsRegister fails status:%d", ret_code);
            return ret_code;
    }

    p_register->uipc_channel = hs_register_rsp.uipc_channel;
    p_register->hndl = hs_register_rsp.hndl;
    /* Extract received parameters */
    return hs_register_rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_HsDeregisterInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsDeregisterInit(tBSA_HS_DEREGISTER *p_register)
{
    APPL_TRACE_API0("BSA_HsDeregisterInit");

    if (p_register == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsDeregisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_register, 0, sizeof(tBSA_HS_DEREGISTER));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsDeregister
**
** Description      This function is called to deregister the headset service.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsDeregister(tBSA_HS_DEREGISTER *p_deregister)
{
    APPL_TRACE_API0("BSA_HsDeregister");

    if (p_deregister == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsDeregister param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_DEREGISTER_CMD,
            p_deregister, sizeof(tBSA_HS_DEREGISTER));
}

/*******************************************************************************
 **
 ** Function         BSA_HsOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsOpenInit(tBSA_HS_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_HsOpenInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_open, 0, sizeof(tBSA_HS_OPEN));
    p_open->services = BSA_HSP_HS_SERVICE_MASK | BSA_HFP_HS_SERVICE_MASK;

    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsOpen
**
** Description      This function is called to open a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsOpen (tBSA_HS_OPEN    *p_req)
{
    APPL_TRACE_API0("BSA_HsOpen");
    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AgOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_OPEN_CMD, p_req, sizeof(tBSA_HS_OPEN));
}

/*******************************************************************************
 **
 ** Function         BSA_HsCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsCloseInit(tBSA_HS_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_HsCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_close, 0, sizeof(tBSA_HS_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsClose
**
** Description      This function is called to close a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsClose (tBSA_HS_CLOSE  *p_close)
{
    APPL_TRACE_API0("BSA_HsClose");
    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_CLOSE_CMD,
                                            p_close,
                                            sizeof(tBSA_HS_MSGID_CLOSE_CMD_REQ));

}

/*******************************************************************************
 **
 ** Function         BSA_HsAudioOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsAudioOpenInit(tBSA_HS_AUDIO_OPEN *p_audio_open)
{
    APPL_TRACE_API0("BSA_HsAudioOpenInit");

    if (p_audio_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsAudioOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_audio_open, 0, sizeof(tBSA_HS_AUDIO_OPEN));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsAudioOpen
**
** Description      This function is called to open audio on a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsAudioOpen (tBSA_HS_AUDIO_OPEN *p_audio_open)
{
    APPL_TRACE_API0("BSA_HsAudioOpen");
    if (p_audio_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsAudioOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }


    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_AUDIO_OPEN_CMD,
                                            p_audio_open,
                                            sizeof(tBSA_HS_MSGID_AUDIO_OPEN_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HsAudioCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsAudioCloseInit(tBSA_HS_AUDIO_CLOSE *p_audio_close)
{
    APPL_TRACE_API0("BSA_HsAudioCloseInit");

    if (p_audio_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsAudioCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_audio_close, 0, sizeof(tBSA_HS_AUDIO_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsAudioClose
**
** Description      This function is called to close audio on a service level
**                  connection to an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsAudioClose (tBSA_HS_AUDIO_CLOSE *p_audio_close)
{

    APPL_TRACE_API0("BSA_HsAudioClose");
    if (p_audio_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsAudioClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_AUDIO_CLOSE_CMD,
                                    p_audio_close,
                                    sizeof(tBSA_HS_MSGID_AUDIO_CLOSE_CMD_REQ));

}

/*******************************************************************************
 **
 ** Function         BSA_HsCommandInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsCommandInit(tBSA_HS_COMMAND *p_command)
{
    APPL_TRACE_API0("BSA_HsCommandInit");

    if (p_command == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsCommandInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_command, 0, sizeof(tBSA_HS_COMMAND));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsCommand
**
** Description      This function is called to send an AT command to
**                  an audio gateway.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_HsCommand (tBSA_HS_COMMAND      *p_command)
{

    APPL_TRACE_API0("BSA_HsCommand");
    if (p_command == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsCommand param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_COMMAND_CMD,
                                            p_command,
                                            sizeof(tBSA_HS_MSGID_COMMAND_CMD_REQ));

}


/*******************************************************************************
 **
 ** Function         BSA_HsCancelInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_HsCancelInit(tBSA_HS_CANCEL *pCancel)
{
    APPL_TRACE_API0("BSA_HsCancelInit");

    if (pCancel == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsCancelInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pCancel, 0, sizeof(tBSA_HS_CANCEL));


    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_HsCancel
**
** Description      This function is called to cancel connection
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_HsCancel (tBSA_HS_CANCEL    *pCancel)
{
    APPL_TRACE_API0("BSA_HsCancel");
    if (pCancel == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HsCancel param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HS_MSGID_CANCEL_CMD, pCancel, sizeof(tBSA_HS_CANCEL));
}

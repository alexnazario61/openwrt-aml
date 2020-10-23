/*****************************************************************************
 **
 **  Name:           bsa_cl_av_api.c
 **
 **  Description:    This is the public interface file for Audio/Video part of
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
#include "bsa_cl_av_int.h"
#include "bta_av_co.h"

/*
 * Global variables
 */
tBSA_AV_CB bsa_av_cb = { NULL, };

/*******************************************************************************
 **
 ** Function         BSA_AvEnableInit
 **
 ** Description      Init a structure tBSA_AV_ENABLE to be used with BSA_AvEnable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvEnableInit(tBSA_AV_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_AvEnableInit");
    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvEnableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_AV_ENABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvEnable
 **
 ** Description      Enable the advanced audio/video service. When the enable
 **                  operation is complete the callback function will be
 **                  called with a BSA_AV_ENABLE_EVT. This function must
 **                  be called before other function in the AV API are
 **                  called.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvEnable(tBSA_AV_ENABLE *p_enable)
{
    tBSA_AV_MSGID_ENABLE_CMD_REQ enable_req;
    tBSA_AV_MSGID_ENABLE_CMD_RSP enable_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvEnable NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Prepare request parameters */
    enable_req.sec_mask = p_enable->sec_mask;
    enable_req.features = p_enable->features;
    enable_req.aptx_caps = p_enable->aptx_caps;
    enable_req.sec_caps = p_enable->sec_caps;
    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AV_MSGID_ENABLE_CMD, &enable_req,
            sizeof(tBSA_AV_MSGID_ENABLE_CMD_REQ), &enable_rsp, sizeof(enable_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvEnable fails status:%d", ret_code);
        return ret_code;
    }

    /* Save application's AV callback */
    bsa_av_cb.p_app_av_cback = p_enable->p_cback;

    /* Extract received parameters */
    ret_code = enable_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvDisableInit
 **
 ** Description      Init structure tBSA_AV_DISABLE to be used with BSA_AvDisable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDisableInit(tBSA_AV_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_AvDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_AV_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvDisable
 **
 ** Description      Disable the advanced audio/video service.
 **
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDisable(tBSA_AV_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_AvDisable");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvDisable NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Save application's Security callback */
    bsa_av_cb.p_app_av_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_DISABLE_CMD, p_disable, sizeof(tBSA_AV_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_AvRegisterInit
 **
 ** Description      Init structure tBSA_AV_DEREGISTER to be used with BSA_AvRegister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRegisterInit(tBSA_AV_REGISTER *p_register)
{
    APPL_TRACE_API0("BSA_AvRegisterInit");

    if (p_register == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvRegisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_register,0,sizeof(tBSA_AV_REGISTER));
    p_register->channel = BSA_AV_CHNL_AUDIO;
    strncpy(p_register->service_name, "default av service name", BSA_AV_SERVICE_NAME_LEN_MAX);

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvRegister
 **
 ** Description      Register the audio or video service to stack. When the
 **                  operation is complete the callback function will be
 **                  called with a BSA_AV_REGISTER_EVT. This function must
 **                  be called before AVDT stream is open.
 **
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRegister(tBSA_AV_REGISTER *p_register)
{
    tBSA_AV_MSGID_REGISTER_CMD_REQ req;
    tBSA_AV_MSGID_REGISTER_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvRegister");

    if (p_register == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvRegister NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    req.channel = p_register->channel;
    req.lt_addr = p_register->lt_addr;
    strncpy(req.service_name, p_register->service_name, BSA_AV_SERVICE_NAME_LEN_MAX-1);
    req.service_name[BSA_AV_SERVICE_NAME_LEN_MAX-1] = '\0';

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AV_MSGID_REGISTER_CMD, &req, sizeof(req), &rsp,
            sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvRegister fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = rsp.status;
    p_register->handle = rsp.handle;
    p_register->uipc_channel = rsp.uipc_channel;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvDeregisterInit
 **
 ** Description      Init structure tBSA_AV_DEREGISTER to be used with BSA_AvDeregister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDeregisterInit(tBSA_AV_DEREGISTER *p_deregister)
{
    APPL_TRACE_API0("BSA_AvDeregisterInit");

    if (p_deregister == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvDeregisterInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_deregister, 0, sizeof(tBSA_AV_DEREGISTER));

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvDeregister
 **
 ** Description      Deregister the audio or video service
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvDeregister(tBSA_AV_DEREGISTER *p_req)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvDeregister");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvDeregister NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code =  bsa_send_message_receive_status(BSA_AV_MSGID_DEREGISTER_CMD, p_req,
            sizeof(tBSA_AV_DEREGISTER));

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvOpenInit
 **
 ** Description      Init structure tBSA_AV_OPEN to be used with BSA_AvOpen
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvOpenInit(tBSA_AV_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_AvOpenInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_open, 0, sizeof(tBSA_AV_OPEN));
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_AvOpen
 **
 ** Description      Opens an advanced audio/video connection to a peer device.
 **                  When connection is open callback function is called
 **                  with a BSA_AV_OPEN_EVT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvOpen(tBSA_AV_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_AvOpen");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvOpen NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_OPEN_CMD, p_req,
            sizeof(tBSA_AV_OPEN));
}

/*******************************************************************************
 **
 ** Function         BSA_AvCloseInit
 **
 ** Description      Init structure tBSA_AV_CLOSE to be used with BSA_AvClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvCloseInit(tBSA_AV_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_AvCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_close, 0, sizeof(tBSA_AV_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvClose
 **
 ** Description      Close an AV connection
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvClose(tBSA_AV_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_AvClose");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvClose NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_CLOSE_CMD, p_req,
            sizeof(tBSA_AV_CLOSE));
}

/*******************************************************************************
 **
 ** Function         BSA_AvStartInit
 **
 ** Description      Init sturcture p_close to be used in BSA_AvClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_AvStartInit(tBSA_AV_START *p_start)
{
    APPL_TRACE_API0("BSA_AvStartInit");

    if (p_start == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvStartInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_start, 0, sizeof(*p_start));
    p_start->scmst_flag = BTA_AV_CP_SCMS_COPY_FREE;
    p_start->uipc_channel = UIPC_CH_ID_AV_AUDIO;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvStart
 **
 ** Description      Start audio/video stream data transfer.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_AvStart(tBSA_AV_START *p_req)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvStart");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvStart NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if ((p_req->feeding_mode ==  BSA_AV_FEEDING_SYNCHRONOUS) &&
        (p_req->latency > BSA_AV_MAX_SYNCHRONOUS_LATENCY ||
         p_req->latency < BSA_AV_MIN_SYNCHRONOUS_LATENCY))
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvStart wrong latency %d ms", p_req->latency);
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_AV_MSGID_START_CMD, p_req,
            sizeof(tBSA_AV_START));

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvStopInit
 **
 ** Description      Init structure tBSA_AV_STOP to be used with BSA_AvStopInit
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvStopInit(tBSA_AV_STOP *p_stop)
{
    APPL_TRACE_API0("BSA_AvStopInit");

    if (p_stop == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvStopInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_stop, 0, sizeof(tBSA_AV_STOP));
    p_stop->uipc_channel = UIPC_CH_ID_AV_AUDIO;
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_AvStop
 **
 ** Description      Stop audio/video stream data transfer.
 **                  If suspend is TRUE, this function sends AVDT suspend signal
 **                  to the connected peer(s).
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvStop(tBSA_AV_STOP *p_req)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvStop");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvStop NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_AV_MSGID_STOP_CMD, p_req,
            sizeof(tBSA_AV_STOP));

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvRemoteCmdInit
 **
 ** Description      Init a structure tBSA_AV_REM_CMD to be used with BSA_AvRemoteCmd
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRemoteCmdInit(tBSA_AV_REM_CMD *p_req)
{
    APPL_TRACE_API0("BSA_AvRemoteCmdInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvRemoteCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AV_REM_CMD));
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_AvRemoteCmd
 **
 ** Description      Send a remote control command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_RCCT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvRemoteCmd(tBSA_AV_REM_CMD *p_req)
{
    APPL_TRACE_API0("BSA_AvRemoteCmd");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvRemoteCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_RC_CMD_CMD, p_req,
            sizeof(tBSA_AV_REM_CMD));
}

/*******************************************************************************
 **
 ** Function         BSA_AvVendorCmdInit
 **
 ** Description      Init a structure tBSA_AV_VEN_CMD to be used with BSA_AvVendorCmd
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorCmdInit(tBSA_AV_VEN_CMD *p_req)
{
    APPL_TRACE_API0("BSA_AvVendorCmdInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvVendorCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AV_VEN_CMD));
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_AvVendorCmd
 **
 ** Description      Send a remote control command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_RCCT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorCmd(tBSA_AV_VEN_CMD *p_req)
{
    APPL_TRACE_API0("BSA_AvVendorCmd");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvVendorCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_VD_CMD_CMD, p_req,
            sizeof(tBSA_AV_VEN_CMD));
}

/*******************************************************************************
 **
 ** Function         BSA_AvVendorRspInit
 **
 ** Description      Init a structure tBSA_AV_VEN_RSP to be used with BSA_AvVendorRsp
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorRspInit(tBSA_AV_VEN_RSP *p_req)
{
    APPL_TRACE_API0("BSA_AvVendorRspInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvVendorRspInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AV_VEN_RSP));
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_AvVendorRsp
 **
 ** Description      Send a remote control command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_RCTG.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvVendorRsp(tBSA_AV_VEN_RSP *p_req)
{
    APPL_TRACE_API0("BSA_AvVendorRsp");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvVendorRsp NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_VD_RSP_CMD, p_req,
            sizeof(tBSA_AV_VEN_RSP));
}

/*******************************************************************************
 **
 ** Function         BSA_AvMetaRspInit
 **
 ** Description      Init a structure tBSA_AV_META_RSP_CMD to be used with BSA_AvMetaRsp
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvMetaRspInit(tBSA_AV_META_RSP_CMD *p_req)
{
    APPL_TRACE_API0("BSA_AvMetaRspInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvMetaRspInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AV_META_RSP_CMD));
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_AvMetaRsp
 **
 ** Description      Send response to meta command.  This function can only
 **                  be used if AV is enabled with feature BSA_AV_FEAT_METADATA.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvMetaRsp(tBSA_AV_META_RSP_CMD *p_req)
{
    APPL_TRACE_API0("BSA_AvMetaRsp");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvMetaRsp NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_META_RSP_CMD, p_req,
            sizeof(tBSA_AV_META_RSP_CMD));
}

/*******************************************************************************
 **
 ** Function         BSA_AvCloseRcInit
 **
 ** Description      Init structure tBSA_AV_CLOSE_RC to be used with BSA_AvCloseRc
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvCloseRcInit(tBSA_AV_CLOSE_RC *p_close_rc)
{
    APPL_TRACE_API0("BSA_AvCloseRcInit");

    if (p_close_rc == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvCloseRcInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_close_rc, 0, sizeof(tBSA_AV_CLOSE_RC));
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_AvCloseRc
 **
 ** Description      Close an AVRCP connection
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvCloseRc(tBSA_AV_CLOSE_RC *p_req)
{
    APPL_TRACE_API0("BSA_AvCloseRc");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvCloseRc NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_RC_CLOSE_CMD, p_req,
            sizeof(tBSA_AV_CLOSE_RC));
}


/*******************************************************************************
 **
 ** Function         BSA_AvBusyLevelInit
 **
 ** Description      Init structure tBSA_AV_BUSY_LEVEL
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvBusyLevelInit(tBSA_AV_BUSY_LEVEL *p_req)
{
    APPL_TRACE_API0("BSA_AvBusyLevelInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvBusyLevelInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_AV_BUSY_LEVEL));
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_AvBusyLevel
 **
 ** Description      Change busy level
 **
 ** Returns          BSA_AvBusyLevel
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvBusyLevel(tBSA_AV_BUSY_LEVEL *p_req)
{
    APPL_TRACE_API0("BSA_AvBusyLevel");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvBusyLevel NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AV_MSGID_BUSY_LEVEL_CMD, p_req,
            sizeof(tBSA_AV_BUSY_LEVEL));
}


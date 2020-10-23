/*****************************************************************************
 **
 **  Name:           bsa_avk_api.h
 **
 **  Description:    This is the public interface file for Security part of
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
tBSA_AVK_CB bsa_avk_cb =
{ NULL, };

/*******************************************************************************
 **
 ** Function         BSA_AvkEnableInit
 **
 ** Description      Init a structure tBSA_AVK_ENABLE to be used with BSA_AvkEnable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkEnableInit(tBSA_AVK_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_AvkEnableInit");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkEnableInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_AVK_ENABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkEnable
 **
 ** Description      Enable the advanced audio/video service. When the enable
 **                  operation is complete the callback function will be
 **                  called with a BSA_AVK_ENABLE_EVT. This function must
 **                  be called before other function in the AV API are
 **                  called.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkEnable(tBSA_AVK_ENABLE *p_enable)
{
    tBSA_AVK_MSGID_ENABLE_CMD_REQ enable_req;
    tBSA_AVK_MSGID_ENABLE_CMD_RSP enable_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvkEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkEnable NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Save application's Security callback */
    bsa_avk_cb.p_app_avk_cback = p_enable->p_cback;

    /* Prepare request parameters */
    enable_req.sec_mask = p_enable->sec_mask;
    enable_req.feature = p_enable->features;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AVK_MSGID_ENABLE_CMD, &enable_req,
            sizeof(tBSA_AVK_MSGID_ENABLE_CMD_REQ), &enable_rsp,
            sizeof(enable_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvkEnable fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = enable_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkDisableInit
 **
 ** Description      Init structure tBSA_AVK_DISABLE to be used with BSA_AvkDisable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDisableInit(tBSA_AVK_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_AvkDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_AV_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkDisable
 **
 ** Description      Disable the advanced audio/video service.
 **
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDisable(tBSA_AVK_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_AvkDisable");

    /* Clear  application's callback */
    bsa_avk_cb.p_app_avk_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_DISABLE_CMD, p_disable, sizeof(tBSA_AVK_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_AvkRegisterInit
 **
 ** Description      Init structure tBSA_AVK_DEREGISTER to be used with BSA_AvkRegister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRegisterInit(tBSA_AVK_REGISTER *p_register)
{
    APPL_TRACE_API0("BSA_AvkRegisterInit");

    if (p_register == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkRegisterInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_register, 0, sizeof(tBSA_AVK_REGISTER));

    p_register->channel = 0xff; /* bad channel */

    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_AvkRegister
 **
 ** Description      Register the audio or video service to stack. When the
 **                  operation is complete the function will return BSA_SUCCESS. This function must
 **                  be called before AVDT stream is open.
 **
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRegister(tBSA_AVK_REGISTER *p_register)
{
    tBSA_AVK_MSGID_REGISTER_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvkRegister");

    if (p_register == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkRegister NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_register->channel > BSA_AVK_CHNL_VIDEO)
    {
        /* illegal value */
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_register->channel == BSA_AVK_CHNL_VIDEO)
    {
        /* video is not tested yet */
        return BSA_ERROR_CLI_NYI;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AVK_MSGID_REGISTER_CMD, p_register,
            sizeof(tBSA_AVK_MSGID_REGISTER_CMD_REQ), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvkRegister fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = rsp.status;
    p_register->uipc_channel = rsp.uipc_channel;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkDeregisterInit
 **
 ** Description      Init structure tBSA_AVK_DEREGISTER to be used with BSA_AvkDeregister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDeregisterInit(tBSA_AVK_DEREGISTER *p_deregister)
{
    APPL_TRACE_API0("BSA_AvkDeregisterInit");

    if (p_deregister == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkDeregisterInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_deregister, 0, sizeof(tBSA_AVK_DEREGISTER));

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkDeregister
 **
 ** Description      Deregister the audio or video service
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkDeregister(tBSA_AVK_DEREGISTER *p_deregister)
{
    tBSA_AVK_MSGID_DEREGISTER_CMD_REQ req;
    tBSA_AVK_MSGID_DEREGISTER_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvkDeregister");
    if (p_deregister == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkDeregister NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    req.channel = p_deregister->channel;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AVK_MSGID_DEREGISTER_CMD, &req,
            sizeof(req), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvkRegister fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkOpenInit
 **
 ** Description      Init structure tBSA_AVK_OPEN to be used with BSA_AvkOpen
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpenInit(tBSA_AVK_OPEN * p_open)
{
    APPL_TRACE_API0("BSA_AvkOpenInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkOpenInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_open, 0, sizeof(tBSA_AVK_OPEN));

    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_AvkOpen
 **
 ** Description      Opens an advanced audio/video connection to a peer device.
 **                  When connection is open callback function is called
 **                  with a BSA_AVK_OPEN_EVT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpen(tBSA_AVK_OPEN * p_open)
{
    tBSA_AVK_MSGID_OPEN_CMD_REQ req;
    tBSA_AVK_MSGID_OPEN_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvkOpen");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkOpen NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    bdcpy(req.bd_addr, p_open->bd_addr);
    req.sec_mask = p_open->sec_mask;
    req.channel = p_open->channel;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AVK_MSGID_OPEN_CMD, &req,
            sizeof(req), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvkOpen fails status:%d", ret_code);
    }

    /* Extract received parameters */
    ret_code = rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkCloseInit
 **
 ** Description      Init structure tBSA_AVK_CLOSE to be used with BSA_AvkClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseInit(tBSA_AVK_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_AvkCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkCloseInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_close, 0, sizeof(tBSA_AVK_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkClose
 **
 ** Description      Close an AV connection
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkClose(tBSA_AVK_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_AvkClose");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_CLOSE_CMD, p_close, sizeof(tBSA_AVK_CLOSE));
}

/*******************************************************************************
 **
 ** Function         BSA_AvkStartInit
 **
 ** Description      Init sturcture p_close to be used in BSA_AvkClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_AvkStartInit(tBSA_AVK_START * p_start)
{
    APPL_TRACE_API0("BSA_AvkStartInit");

    if (p_start == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkStartInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_start, 0, sizeof(*p_start));

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkStart
 **
 ** Description      Start audio/video stream data transfer.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTA_API tBSA_STATUS BSA_AvkStart(tBSA_AVK_START * p_start)
{
    tBSA_AVK_MSGID_START_CMD_REQ req;
    tBSA_AVK_MSGID_START_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvkStart");

    if (p_start == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkStart NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
//    req.channel = p_start->channel;
    memcpy(&req.media_receiving, &p_start->media_receiving,
            sizeof(tBSA_AVK_MEDIA_RECEIVING));

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AVK_MSGID_START_CMD, &req,
            sizeof(req), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvkStart fails status:%d", ret_code);
    }

    /* Extract received parameters */
    ret_code = rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkStopInit
 **
 ** Description      Init structure tBSA_AVK_STOP to be used with BSA_AvkStopInit
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkStopInit(tBSA_AVK_STOP *p_stop)
{
    APPL_TRACE_API0("BSA_AvkStopInit");

    if (p_stop == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkStopInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_stop, 0, sizeof(tBSA_AVK_STOP));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkStop
 **
 ** Description      Stop audio/video stream data transfer.
 **                  If suspend is TRUE, this function sends AVDT suspend signal
 **                  to the connected peer(s).
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkStop(tBSA_AVK_STOP *p_stop)
{
    tBSA_AVK_MSGID_STOP_CMD_REQ req;
    tBSA_AVK_MSGID_STOP_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvkStop");

    if (p_stop == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkStop NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

//    req.handle = p_stop->handle;
    req.pause = p_stop->pause;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AVK_MSGID_STOP_CMD, &req,
            sizeof(req), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_AvkStop fails status:%d", ret_code);
    }

    /* Extract received parameters */
    ret_code = rsp.status;

    return ret_code;
}


/*******************************************************************************
**
** Function         BSA_AvkRemoteCmdInit
**
** Description      Init a structure tBSA_AVK_REM_CMD to be used with BSA_AvkRemoteCmd
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRemoteCmdInit(tBSA_AVK_REM_CMD *pRemCmd)
{

    if (pRemCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkRemoteCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pRemCmd, 0, sizeof(tBSA_AVK_REM_CMD));
    return BSA_SUCCESS;

}



/*******************************************************************************
**
** Function         BSA_AvkRemoteCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_RCCT.
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRemoteCmd(tBSA_AVK_REM_CMD *pRemCmd)
{    
    


    if (pRemCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkRemoteCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_RC_CMD_CMD, pRemCmd, sizeof(tBSA_AVK_REM_CMD));    
}

/*******************************************************************************
**
** Function         BSA_AvkVendorCmdInit
**
** Description      Init a structure tBSA_AVK_VEN_CMD to be used with BSA_AvkVendorCmd
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkVendorCmdInit(tBSA_AVK_VEN_CMD *pVenCmd)
{


    if (pVenCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkVendorCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pVenCmd, 0, sizeof(tBSA_AVK_VEN_CMD));
    return BSA_SUCCESS;
}



/*******************************************************************************
**
** Function         BSA_AvkVendorCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_RCCT.
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkVendorCmd(tBSA_AVK_VEN_CMD *pVenCmd)
{    

    if (pVenCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkVendorCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_VD_CMD_CMD, pVenCmd, sizeof(tBSA_AVK_VEN_CMD));    
}


/*******************************************************************************
**
** Function         BSA_AvkCancelCmdInit
**
** Description      Init a structure tBSA_AVK_CANCEL_CMD to be used with BSA_AvkCancelCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCancelCmdInit(tBSA_AVK_CANCEL_CMD *pCancelCmd)
{
    if (pCancelCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkCancelCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pCancelCmd, 0, sizeof(tBSA_AVK_CANCEL_CMD));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkCancelCmd
**
** Description      Send a command to cancel connection.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCancelCmd(tBSA_AVK_CANCEL_CMD *pCancelCmd)
{

    if (pCancelCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkCancelCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_CANCEL_CMD, pCancelCmd, sizeof(tBSA_AVK_CANCEL_CMD));
}


/*******************************************************************************
**
** Function         BSA_AvkListPlayerAttrCmdInit
**
** Description      Init a structure tBSA_AVK_LIST_PLAYER_ATTR to be used with BSA_AvkListPlayerAttrCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerAttrCmdInit(tBSA_AVK_LIST_PLAYER_ATTR *pListPlayerAttrCmd)
{
    if (pListPlayerAttrCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkListPlayerAttrCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pListPlayerAttrCmd, 0, sizeof(tBSA_AVK_LIST_PLAYER_ATTR));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkListPlayerAttrCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature tBSA_AVK_LIST_PLAYER_ATTR.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerAttrCmd(tBSA_AVK_LIST_PLAYER_ATTR *pListPlayerAttrCmd)
{
    if (pListPlayerAttrCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkListPlayerAttrCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_LIST_PLAYER_ATTR_CMD, pListPlayerAttrCmd, sizeof(tBSA_AVK_LIST_PLAYER_ATTR));
}


/*******************************************************************************
**
** Function         BSA_AvkListPlayerValuesCmdInit
**
** Description      Init a structure tBSA_AVK_LIST_PLAYER_VALUES to be used with BSA_AvkListPlayerValuesCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerValuesCmdInit(tBSA_AVK_LIST_PLAYER_VALUES *pListPlayerValuesCmd)
{
    if (pListPlayerValuesCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkListPlayerValuesCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pListPlayerValuesCmd, 0, sizeof(tBSA_AVK_LIST_PLAYER_VALUES));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkListPlayerValuesCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature tBSA_AVK_LIST_PLAYER_VALUES.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkListPlayerValuesCmd(tBSA_AVK_LIST_PLAYER_VALUES *pListPlayerValuesCmd)
{

    if (pListPlayerValuesCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkListPlayerValuesCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_LIST_PLAYER_VALUES_CMD, pListPlayerValuesCmd, sizeof(tBSA_AVK_LIST_PLAYER_VALUES));
}

/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAYER_VALUE to be used with BSA_AvkGetPlayerValueCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueCmdInit(tBSA_AVK_GET_PLAYER_VALUE *pGetPlayerValueCmd)
{
    if (pGetPlayerValueCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkGetPlayerValueCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pGetPlayerValueCmd, 0, sizeof(tBSA_AVK_GET_PLAYER_VALUE));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature tBSA_AVK_GET_PLAYER_VALUE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueCmd(tBSA_AVK_GET_PLAYER_VALUE *pGetPlayerValueCmd)
{
    if (pGetPlayerValueCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkGetPlayerValueCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_GET_PLAYER_VALUE_CMD, pGetPlayerValueCmd, sizeof(tBSA_AVK_GET_PLAYER_VALUE));
}


/*******************************************************************************
**
** Function         BSA_AvkSetPlayerValueCmdInit
**
** Description      Init a structure tBSA_AVK_SET_PLAYER_VALUE to be used with BSA_AvkSetPlayerValueCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetPlayerValueCmdInit(tBSA_AVK_SET_PLAYER_VALUE *pSetPlayerValueCmd)
{
    if (pSetPlayerValueCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkSetPlayerValueCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pSetPlayerValueCmd, 0, sizeof(tBSA_AVK_SET_PLAYER_VALUE));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkSetPlayerValueCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetPlayerValueCmd(tBSA_AVK_SET_PLAYER_VALUE *pSetPlayerValueCmd)
{

    if (pSetPlayerValueCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkSetPlayerValueCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_SET_PLAYER_VALUE_CMD, pSetPlayerValueCmd, sizeof(tBSA_AVK_SET_PLAYER_VALUE));
}


/*******************************************************************************
**
** Function         BSA_AvkGetPlayerAttrTextCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAYER_ATTR_TEXT to be used with BSA_AvkGetPlayerAttrTextCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerAttrTextCmdInit(tBSA_AVK_GET_PLAYER_ATTR_TEXT *pGetPlayerAttrTextCmd)
{
    if (pGetPlayerAttrTextCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkGetPlayerAttrTextCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pGetPlayerAttrTextCmd, 0, sizeof(tBSA_AVK_GET_PLAYER_ATTR_TEXT));
    return BSA_SUCCESS;
}



/*******************************************************************************
**
** Function         BSA_AvkGetPlayerAttrTextCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerAttrTextCmd(tBSA_AVK_GET_PLAYER_ATTR_TEXT *pGetPlayerAttrTextCmd)
{
    if (pGetPlayerAttrTextCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkGetPlayerAttrTextCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_GET_PLAYER_ATTR_TEXT_CMD, pGetPlayerAttrTextCmd, sizeof(tBSA_AVK_GET_PLAYER_ATTR_TEXT));
}


/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueTextCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAYER_VALUE_TEXT to be used with BSA_AvkGetPlayerValueTextCmdInit
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueTextCmdInit(tBSA_AVK_GET_PLAYER_VALUE_TEXT *pGetPlayerValueTextCmd)
{
    if (pGetPlayerValueTextCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkGetPlayerValueTextCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pGetPlayerValueTextCmd, 0, sizeof(tBSA_AVK_GET_PLAYER_VALUE_TEXT));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkGetPlayerValueTextCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayerValueTextCmd(tBSA_AVK_GET_PLAYER_VALUE_TEXT *pGetPlayerValueTextCmd)
{

    if (pGetPlayerValueTextCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkGetPlayerValueTextCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_GET_PLAYER_ATTR_VALUE_TEXT_CMD, pGetPlayerValueTextCmd, sizeof(tBSA_AVK_GET_PLAYER_VALUE_TEXT));
}


/*******************************************************************************
**
** Function         BSA_AvkGetElementAttrCmdInit
**
** Description      Init a structure tBSA_AVK_GET_ELEMENT_ATTR to be used with BSA_AvkGetElementAttrCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetElementAttrCmdInit(tBSA_AVK_GET_ELEMENT_ATTR *pGetElemAttrCmd)
{    
    if (pGetElemAttrCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkGetElementAttrCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pGetElemAttrCmd, 0, sizeof(tBSA_AVK_GET_ELEMENT_ATTR));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkGetElementAttrCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetElementAttrCmd(tBSA_AVK_GET_ELEMENT_ATTR *pGetElemAttrCmd)
{    
    if (pGetElemAttrCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkGetElementAttrCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_GET_ELEMENT_ATTR_CMD, pGetElemAttrCmd, sizeof(tBSA_AVK_GET_ELEMENT_ATTR));
}


/*******************************************************************************
**
** Function         BSA_AvkGetPlayStatusCmdInit
**
** Description      Init a structure tBSA_AVK_GET_PLAY_STATUS to be used with BSA_AvkGetPlayStatusCmdInit
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayStatusCmdInit(tBSA_AVK_GET_PLAY_STATUS *pPlayStatusCmd)
{    
    if (pPlayStatusCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkGetPlayStatusCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pPlayStatusCmd, 0, sizeof(tBSA_AVK_GET_PLAY_STATUS));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkGetPlayStatusCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_METADATA.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetPlayStatusCmd(tBSA_AVK_GET_PLAY_STATUS *pPlayStatusCmd)
{    
    if (pPlayStatusCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkGetPlayStatusCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_GET_PLAY_STATUS_CMD, pPlayStatusCmd, sizeof(tBSA_AVK_GET_PLAY_STATUS));
}


/*******************************************************************************
**
** Function         BSA_AvkSetAddressedPlayerCmdInit
**
** Description      Init a structure tBSA_AVK_SET_ADDR_PLAYER to be used with BSA_AvkSetAddressedPlayerCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetAddressedPlayerCmdInit(tBSA_AVK_SET_ADDR_PLAYER *pAddrPlayerCmd)
{    

    if (pAddrPlayerCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkSetAddressedPlayerCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pAddrPlayerCmd, 0, sizeof(tBSA_AVK_SET_ADDR_PLAYER));
    return BSA_SUCCESS;
}


/*******************************************************************************
**
** Function         BSA_AvkSetAddressedPlayerCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetAddressedPlayerCmd(tBSA_AVK_SET_ADDR_PLAYER *pAddrPlayerCmd)
{    
    if (pAddrPlayerCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkSetAddressedPlayerCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_SET_ADDRESSED_PLAYER, pAddrPlayerCmd, sizeof(tBSA_AVK_SET_ADDR_PLAYER));
}

/*******************************************************************************
**
** Function         BSA_AvkSetBrowsedPlayerCmdInit
**
** Description      Init a structure tBSA_AVK_SET_BROWSED_PLAYER to be used with BSA_AvkSetBrowsedPlayerCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetBrowsedPlayerCmdInit(tBSA_AVK_SET_BROWSED_PLAYER *pBrowsedPlayerCmd)
{    
    if (pBrowsedPlayerCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkSetBrowsedPlayerCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pBrowsedPlayerCmd, 0, sizeof(tBSA_AVK_SET_BROWSED_PLAYER));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkSetBrowsedPlayerCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkSetBrowsedPlayerCmd(tBSA_AVK_SET_BROWSED_PLAYER *pBrowsedPlayerCmd)
{    
    if (pBrowsedPlayerCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkSetBrowsedPlayerCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_SET_BROWSED_PLAYER, pBrowsedPlayerCmd, sizeof(tBSA_AVK_SET_BROWSED_PLAYER));
}

/*******************************************************************************
**
** Function         BSA_AvkChangePathCmdInit
**
** Description      Init a structure tBSA_AVK_CHG_PATH to be used with BSA_AvkChangePathCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkChangePathCmdInit(tBSA_AVK_CHG_PATH *pChangePathCmd)
{    
    if (pChangePathCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkChangePathCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pChangePathCmd, 0, sizeof(tBSA_AVK_CHG_PATH));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkChangePathCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkChangePathCmd(tBSA_AVK_CHG_PATH *pChangePathCmd)
{    
    if (pChangePathCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkChangePathCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_CHANGE_PATH_CMD, pChangePathCmd, sizeof(tBSA_AVK_CHG_PATH));
}


/*******************************************************************************
**
** Function         BSA_AvkGetFolderItemsCmdInit
**
** Description      Init a structure tBSA_AVK_GET_FOLDER_ITEMS to be used with BSA_AvkGetFolderItemsCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetFolderItemsCmdInit(tBSA_AVK_GET_FOLDER_ITEMS *pGetFolderItemsCmd)
{    
    if (pGetFolderItemsCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkGetFolderItemsCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pGetFolderItemsCmd, 0, sizeof(tBSA_AVK_GET_FOLDER_ITEMS));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkGetFolderItemsCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetFolderItemsCmd(tBSA_AVK_GET_FOLDER_ITEMS *pGetFolderItemsCmd)
{    
    if (pGetFolderItemsCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkGetFolderItemsCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_GET_FOLDER_ITEMS_CMD, pGetFolderItemsCmd, sizeof(tBSA_AVK_GET_FOLDER_ITEMS));
}



/*******************************************************************************
**
** Function         BSA_AvkGetItemsAttrCmdInit
**
** Description      Init a structure tBSA_AVK_GET_ITEMS_ATTR to be used with BSA_AvkGetItemsAttrCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetItemsAttrCmdInit(tBSA_AVK_GET_ITEMS_ATTR *pGetItemsAttrCmd)
{    
    if (pGetItemsAttrCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkGetItemsAttrCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pGetItemsAttrCmd, 0, sizeof(tBSA_AVK_GET_ITEMS_ATTR));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkGetItemsAttrCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkGetItemsAttrCmd(tBSA_AVK_GET_ITEMS_ATTR *pGetItemsAttrCmd)
{    
    if (pGetItemsAttrCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkGetItemsAttrCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_GET_ITEMS_ATTR_CMD, pGetItemsAttrCmd, sizeof(tBSA_AVK_GET_ITEMS_ATTR));
}

/*******************************************************************************
**
** Function         BSA_AvkPlayItemCmdInit
**
** Description      Init a structure tBSA_AVK_PLAY_ITEM to be used with BSA_AvkPlayItemCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkPlayItemCmdInit(tBSA_AVK_PLAY_ITEM *pPlayItemCmd)
{    
    if (pPlayItemCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkPlayItemCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pPlayItemCmd, 0, sizeof(tBSA_AVK_PLAY_ITEM));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkPlayItemCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkPlayItemCmd(tBSA_AVK_PLAY_ITEM *pPlayItemCmd)
{
    if (pPlayItemCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkPlayItemCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_PLAY_ITEM_CMD, pPlayItemCmd, sizeof(tBSA_AVK_PLAY_ITEM));
}

/*******************************************************************************
**
** Function         BSA_AvkAddToPlayCmdInit
**
** Description      Init a structure tBSA_AVK_ADD_TO_PLAY to be used with BSA_AvkAddToPlayCmd
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkAddToPlayCmdInit(tBSA_AVK_ADD_TO_PLAY *pAddToPlayCmd)
{   
    if (pAddToPlayCmd == NULL)
    {
        APPL_TRACE_ERROR0("BSA_AvkAddToPlayCmdInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pAddToPlayCmd, 0, sizeof(tBSA_AVK_ADD_TO_PLAY));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkAddToPlayCmd
**
** Description      Send a remote control command.  This function can only
**                  be used if AVK is enabled with feature BSA_AVK_FEAT_BROWSE.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkAddToPlayCmd(tBSA_AVK_ADD_TO_PLAY *pAddToPlayCmd)
{    
    if (pAddToPlayCmd == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkAddToPlayCmd NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_ADD_TO_NOW_PLAYING_CMD, pAddToPlayCmd, sizeof(tBSA_AVK_ADD_TO_PLAY));
}


/*******************************************************************************
 **
 ** Function         BSA_AvkOpenRcInit
 **
 ** Description      Init structure tBSA_AVK_OPEN to be used with BSA_AvkOpenRc
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpenRcInit(tBSA_AVK_OPEN * p_open)
{
    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkOpenInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_open, 0, sizeof(tBSA_AVK_OPEN));

    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_AvkOpenRc
 **
 ** Description      Opens an avrcp controller connection to a peer device. AVK must already be connected.
 **                  When connection is open callback function is called
 **                  with a BSA_AVK_OPEN_RC_EVT.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkOpenRc(tBSA_AVK_OPEN * p_open)
{
    tBSA_AVK_MSGID_OPEN_CMD_REQ req;
    tBSA_AVK_MSGID_OPEN_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_AvkOpenRc");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkOpen NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    bdcpy(req.bd_addr, p_open->bd_addr);

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_AVK_MSGID_OPEN_RC_CMD, &req,
            sizeof(req), &rsp, sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_AvkOpen fails status:%d", ret_code);
    }

    /* Extract received parameters */
    ret_code = rsp.status;

    return ret_code;
}



/*******************************************************************************
 **
 ** Function         BSA_AvkCloseRcInit
 **
 ** Description      Init structure tBSA_AVK_CLOSE to be used with BSA_AvkCloseRc
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseRcInit(tBSA_AVK_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_AvkCloseRcInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkCloseInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_close, 0, sizeof(tBSA_AVK_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_AvkCloseRc
 **
 ** Description      Close an AVRC (controller) connection
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkCloseRc(tBSA_AVK_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_AvkCloseRc");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_AVK_MSGID_CLOSE_RC_CMD, p_close, sizeof(tBSA_AVK_CLOSE));
}

/*******************************************************************************
 **
 ** Function         BSA_AvkRelayAudioInit
 **
 ** Description      Init structure tBSA_AVK_CLOSE to be used with BSA_AvkRelayAudioInit
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRelayAudioInit(tBSA_AVK_RELAY_AUDIO *pRelayAudio)
{
    APPL_TRACE_API0("BSA_AvkRelayAudioInit");

    if (pRelayAudio == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_AvkRelayAudioInit NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pRelayAudio, 0, sizeof(tBSA_AVK_RELAY_AUDIO));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_AvkRelayAudio
**
** Description      Start/stop audio relay from AVK to AV
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_AvkRelayAudio(tBSA_AVK_RELAY_AUDIO *pRelayAudio)
{
     APPL_TRACE_API0("BSA_AvkRelayAudio");

     return bsa_send_message_receive_status(BSA_AVK_MSGID_RELAY_AUDIO_CMD, pRelayAudio, sizeof(tBSA_AVK_RELAY_AUDIO));
}

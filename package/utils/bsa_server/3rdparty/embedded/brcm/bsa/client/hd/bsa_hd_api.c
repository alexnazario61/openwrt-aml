/*****************************************************************************
 **
 **  Name:           bsa_hd_api.c
 **
 **  Description:   This is the public interface file for HD part of
 **                 the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include "bsa_api.h"
#include "bsa_int.h"

/*
 * Global variables
 */
tBSA_HD_CB bsa_hd_cb;

/*******************************************************************************
 **
 ** Function         BSA_HdEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdEnableInit(tBSA_HD_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_HdEnableInit");
    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdEnableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_HD_ENABLE));
    p_enable->p_cback = NULL;
    p_enable->sec_mask = BSA_SEC_NONE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HdEnable
 **
 ** Description      This function enable HID Device and registers HID-Device with
 **                  lower layers.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdEnable(tBSA_HD_ENABLE *p_enable)
{
    tBSA_HD_MSGID_ENABLE_CMD_REQ hd_enable_req;

    APPL_TRACE_API0("BSA_HdEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdEnable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Save application's Security callback */
    bsa_hd_cb.p_app_hd_cback = p_enable->p_cback;

    /* Prepare request parameters */
    hd_enable_req.sec_mask = p_enable->sec_mask;
    bdcpy(hd_enable_req.bd_addr, p_enable->bd_addr);

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HD_MSGID_ENABLE_CMD,
           &hd_enable_req, sizeof(tBSA_HD_MSGID_ENABLE_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_HdDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdDisableInit(tBSA_HD_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_HdDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_HD_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HdDisable
 **
 ** Description      This function is called when the host is about power down.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdDisable(tBSA_HD_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_HdDisable");

    /* Clear application's Security callback */
    bsa_hd_cb.p_app_hd_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HD_MSGID_DISABLE_CMD, 
           p_disable, sizeof(tBSA_HD_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_HdOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdOpenInit(tBSA_HD_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_HdOpenInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_open, 0, sizeof(tBSA_HD_OPEN));
    p_open->sec_mask = BSA_SEC_NONE;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HdOpen
 **
 ** Description      This function is called to open an HD connection to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdOpen(tBSA_HD_OPEN *p_open)
{
    tBSA_HD_MSGID_OPEN_CMD_RSP hd_open_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HdOpen");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HD_MSGID_OPEN_CMD, p_open,
            sizeof(tBSA_HD_OPEN), &hd_open_rsp, sizeof(hd_open_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HdOpen fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hd_open_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HdCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdCloseInit(tBSA_HD_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_HdCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HdClose
 **
 ** Description      This function disconnects the device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdClose(tBSA_HD_CLOSE *p_close)
{
    tBSA_HD_MSGID_CLOSE_CMD_RSP hd_close_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HdClose");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HD_MSGID_CLOSE_CMD, p_close,
            sizeof(tBSA_HD_CLOSE), &hd_close_rsp, sizeof(hd_close_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HdClose fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hd_close_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HdSendInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdSendInit(tBSA_HD_SEND *p_send)
{
    APPL_TRACE_API0("BSA_HdSendInit");

    if (p_send == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdSendInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_HdSend
 **
 ** Description      Send report to a HID host.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HdSend(tBSA_HD_SEND *p_send)
{

    APPL_TRACE_API0("BSA_HdSend");

    if (p_send == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HdSend param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_HD_MSGID_SEND_CMD, 
           p_send, sizeof(tBSA_HD_SEND));
}



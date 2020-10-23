/*****************************************************************************
 **
 **  Name:           bsa_hh_api.c
 **
 **  Description:   This is the public interface file for HH part of
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
tBSA_HH_CB bsa_hh_cb =
{ NULL, FALSE,
{ 0, 0, 0, 0, 0, 0 }, 0, 0, 0 };

/*******************************************************************************
 **
 ** Function         BSA_HhEnableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhEnableInit(tBSA_HH_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_HhEnableInit");
    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhEnableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_HH_ENABLE));
    p_enable->p_cback = NULL;
    p_enable->sec_mask = BSA_SEC_NONE;
    p_enable->uipc_channel = UIPC_CH_ID_BAD;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhEnable
 **
 ** Description      This function enable HID host and registers HID-Host with
 **                  lower layers.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhEnable(tBSA_HH_ENABLE *p_enable)
{
    tBSA_HH_MSGID_ENABLE_CMD_REQ hh_enable_req;
    tBSA_HH_MSGID_ENABLE_CMD_RSP hh_enable_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhEnable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Save application's Security callback */
    bsa_hh_cb.p_app_hh_cback = p_enable->p_cback;

    /* Prepare request parameters */
    hh_enable_req.sec_mask = p_enable->sec_mask;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_ENABLE_CMD,
            &hh_enable_req, sizeof(tBSA_HH_MSGID_ENABLE_CMD_REQ),
            &hh_enable_rsp, sizeof(hh_enable_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhEnable fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_enable_rsp.status;
    p_enable->uipc_channel = hh_enable_rsp.uipc_channel;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhDisableInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhDisableInit(tBSA_HH_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_HhDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_HH_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhDisable
 **
 ** Description      This function is called when the host is about power down.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhDisable(tBSA_HH_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_HhDisable");

    /* Clear application's Security callback */
    bsa_hh_cb.p_app_hh_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HH_MSGID_DISABLE_CMD, p_disable, sizeof(tBSA_HH_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_HhOpenInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhOpenInit(tBSA_HH_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_HhOpenInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_open, 0, sizeof(tBSA_HH_OPEN));
    p_open->mode = BSA_HH_PROTO_RPT_MODE;
    p_open->sec_mask = BSA_SEC_NONE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhOpen
 **
 ** Description      This function is called to open an HH connection  to a remote
 **                  device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhOpen(tBSA_HH_OPEN *p_open)
{
    tBSA_HH_MSGID_OPEN_CMD_RSP hh_open_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhOpen");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    APPL_TRACE_DEBUG1("BSA_HhOpen mode:%d", p_open->mode);

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_OPEN_CMD, p_open,
            sizeof(tBSA_HH_OPEN), &hh_open_rsp, sizeof(hh_open_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhOpen fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_open_rsp.status;

#if 0 /* to be used later for blocking mode */
    bsa_hh_cb.open_ongoing = FALSE;
#endif

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhCloseInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhCloseInit(tBSA_HH_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_HhCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_close->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhClose
 **
 ** Description      This function disconnects the device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhClose(tBSA_HH_CLOSE *p_close)
{
    tBSA_HH_MSGID_CLOSE_CMD_RSP hh_close_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhClose");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_CLOSE_CMD, p_close,
            sizeof(tBSA_HH_CLOSE), &hh_close_rsp, sizeof(hh_close_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhClose fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_close_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSetProtoModeInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetProtoModeInit(tBSA_HH_SET_PROTO *p_set_proto)
{
    APPL_TRACE_API0("BSA_HhSetProtoModeInit");

    if (p_set_proto == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSetProtoModeInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_set_proto->handle = BSA_HH_INVALID_HANDLE;
    p_set_proto->mode = BSA_HH_PROTO_RPT_MODE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSetProtoMode
 **
 ** Description      This function set the protocol mode at specified HID handle
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetProtoMode(tBSA_HH_SET_PROTO *p_set_proto)
{
    tBSA_HH_MSGID_SET_PROTO_CMD_RSP hh_set_proto_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhSetProtoMode");

    if (p_set_proto == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSetProtoMode param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_SET_PROTO_CMD,
            p_set_proto, sizeof(tBSA_HH_SET_PROTO), &hh_set_proto_rsp,
            sizeof(hh_set_proto_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhSetProtoMode fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_set_proto_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGetProtoModeInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetProtoModeInit(tBSA_HH_GET_PROTO *p_get_proto)
{
    APPL_TRACE_API0("BSA_HhGetProtoModeInit");

    if (p_get_proto == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetProtoModeInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_get_proto->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGetProtoMode
 **
 ** Description      This function get the protocol mode of a specified HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetProtoMode(tBSA_HH_GET_PROTO *p_get_proto)
{
    tBSA_HH_MSGID_GET_PROTO_CMD_REQ hh_get_proto_req;
    tBSA_HH_MSGID_GET_PROTO_CMD_RSP hh_get_proto_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhGetProtoMode");

    if (p_get_proto == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetProtoMode param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    hh_get_proto_req.handle = p_get_proto->handle;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_GET_PROTO_CMD,
            &hh_get_proto_req, sizeof(tBSA_HH_MSGID_GET_PROTO_CMD_REQ),
            &hh_get_proto_rsp, sizeof(hh_get_proto_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhGetProtoMode fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_get_proto_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSetReportInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetReportInit(tBSA_HH_SET_REPORT *p_set_report)
{
    APPL_TRACE_API0("BSA_HhSetReportInit");

    if (p_set_report == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSetReportInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_set_report->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSetReport
 **
 ** Description      send SET_REPORT to device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetReport(tBSA_HH_SET_REPORT *p_set_report)
{
    tBSA_HH_MSGID_SET_REPORT_CMD_RSP hh_set_report_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhSetReport");

    if (p_set_report == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSetReport param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_SET_REPORT_CMD,
            p_set_report, sizeof(tBSA_HH_SET_REPORT), &hh_set_report_rsp,
            sizeof(hh_set_report_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhSetReport fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_set_report_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGetReportInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetReportInit(tBSA_HH_GET_REPORT *p_get_report)
{
    APPL_TRACE_API0("BSA_HhGetReportInit");

    if (p_get_report == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetReportInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_get_report->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGetReport
 **
 ** Description      send SET_REPORT to device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetReport(tBSA_HH_GET_REPORT *p_get_report)
{
    tBSA_HH_MSGID_GET_REPORT_CMD_REQ hh_get_report_req;
    tBSA_HH_MSGID_GET_REPORT_CMD_RSP hh_get_report_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhGetReport");

    if (p_get_report == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetReport param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    hh_get_report_req.handle = p_get_report->handle;
    hh_get_report_req.report_id = p_get_report->report_id;
    hh_get_report_req.report_type = p_get_report->report_type;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_GET_REPORT_CMD,
            &hh_get_report_req, sizeof(hh_get_report_req), &hh_get_report_rsp,
            sizeof(hh_get_report_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("%s fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_get_report_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSetIdleInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetIdleInit(tBSA_HH_SET_IDLE *p_set_idle)
{
    APPL_TRACE_API0("BSA_HhSetIdleInit");

    if (p_set_idle == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSetIdleInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_set_idle->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSetIdle
 **
 ** Description      This function set the protocol mode at specified HID handle
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetIdle(tBSA_HH_SET_IDLE *p_set_idle)
{
    tBSA_HH_MSGID_SET_IDLE_CMD_RSP hh_set_idle_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhSetIdle");

    if (p_set_idle == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSetIdle param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_SET_IDLE_CMD, p_set_idle,
                    sizeof(tBSA_HH_SET_IDLE), &hh_set_idle_rsp,
                    sizeof(hh_set_idle_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhSetIdle fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_set_idle_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGetIdleInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetIdleInit(tBSA_HH_GET_IDLE *p_get_idle)
{
    APPL_TRACE_API0("BSA_HhGetIdleInit");

    if (p_get_idle == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetIdleInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_get_idle->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGetIdle
 **
 ** Description      This function get the protocol mode of a specified HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetIdle(tBSA_HH_GET_IDLE *p_get_idle)
{
    tBSA_HH_MSGID_GET_IDLE_CMD_REQ hh_get_idle_req;
    tBSA_HH_MSGID_GET_IDLE_CMD_RSP hh_get_idle_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhGetIdle");

    if (p_get_idle == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetIdle param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    hh_get_idle_req.handle = p_get_idle->handle;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_GET_IDLE_CMD,
            &hh_get_idle_req, sizeof(tBSA_HH_MSGID_GET_IDLE_CMD_REQ),
            &hh_get_idle_rsp, sizeof(hh_get_idle_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhGetIdle fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_get_idle_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSendCtrlInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendCtrlInit(tBSA_HH_SEND_CTRL *p_send_ctrl)
{
    APPL_TRACE_API0("BSA_HhSendCtrlInit");

    if (p_send_ctrl == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSendCtrlInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_send_ctrl->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_HhSendCtrl
 **
 ** Description      Send HID_CONTROL request to a HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendCtrl(tBSA_HH_SEND_CTRL *p_send_ctrl)
{
    tBSA_HH_MSGID_SEND_CTRL_CMD_RSP hh_send_ctrl_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhSendCtrl");

    if (p_send_ctrl == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSendCtrl param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_SEND_CTRL_CMD,
            p_send_ctrl, sizeof(tBSA_HH_SEND_CTRL), &hh_send_ctrl_rsp,
            sizeof(hh_send_ctrl_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhSendCtrl fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_send_ctrl_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSendDataInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendDataInit(tBSA_HH_SEND_DATA *p_send_data)
{
    APPL_TRACE_API0("BSA_HhSendDataInit");

    if (p_send_data == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSendDataInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_send_data->handle = BSA_HH_INVALID_HANDLE;
    p_send_data->data.length = 0;
    p_send_data->report_type = BSA_HH_RPTT_OUTPUT;
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_HhSendData
 **
 ** Description      Send HID_CONTROL request to a HID device.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSendData(tBSA_HH_SEND_DATA *p_send_data)
{
    tBSA_HH_MSGID_SEND_DATA_CMD_RSP hh_send_data_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhSendData");

    if (p_send_data == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSendDataInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_SEND_DATA_CMD,
            p_send_data, sizeof(tBSA_HH_SEND_DATA), &hh_send_data_rsp,
            sizeof(hh_send_data_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhSendData fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_send_data_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGetDscpInfoInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetDscpInfoInit(tBSA_HH_GET_DSCPINFO *p_get_dscp)
{
    APPL_TRACE_API0("BSA_HhGetDscpInfoInit");

    if (p_get_dscp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetDscpInfoInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    p_get_dscp->handle = BSA_HH_INVALID_HANDLE;
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_HhGetDscpInfo
 **
 ** Description      Get report descriptor of the device
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetDscpInfo(tBSA_HH_GET_DSCPINFO *p_get_dscp)
{
    tBSA_HH_MSGID_GET_DSCPINFO_CMD_REQ hh_getdscp_req;
    tBSA_HH_MSGID_GET_DSCPINFO_CMD_RSP hh_getdscp_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhGetDscpInfo");

    if (p_get_dscp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetDscpInfo param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Prepare request parameters */
    hh_getdscp_req.handle = p_get_dscp->handle;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_GET_DSCPINFO_CMD,
            &hh_getdscp_req, sizeof(hh_getdscp_req), &hh_getdscp_rsp,
            sizeof(hh_getdscp_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhGetDscpInfo fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    ret_code = hh_getdscp_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhAddDevInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhAddDevInit(tBSA_HH_ADD_DEV *p_add_dev)
{
    APPL_TRACE_API0("BSA_HhAddDevInit");

    if (p_add_dev == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhAddDevInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_add_dev, 0, sizeof(tBSA_HH_ADD_DEV));
    p_add_dev->peerinfo.ssr_max_latency = BTA_HH_SSR_PARAM_INVALID;
    p_add_dev->peerinfo.ssr_min_tout = BTA_HH_SSR_PARAM_INVALID;
    p_add_dev->peerinfo.supervision_tout = BTA_HH_STO_PARAM_INVALID;
    p_add_dev->app_id = 0; /* In order to keep backward compatibility */

    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_HhAddDev
 **
 ** Description      Add a virtually cabled device into HID-Host device list
 **                  to manage and assign a device handle for future API call,
 **                  host application call this API at start-up to initialize its
 **                  virtually cabled devices.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhAddDev(tBSA_HH_ADD_DEV *p_add_dev)
{
    tBSA_HH_MSGID_ADD_DEV_CMD_RSP hh_add_dev_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_HhAddDev");

    if (p_add_dev == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhAddDev param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_HH_MSGID_ADD_DEV_CMD, p_add_dev,
            sizeof(tBSA_HH_ADD_DEV), &hh_add_dev_rsp, sizeof(hh_add_dev_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_HhAddDev fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    p_add_dev->handle = hh_add_dev_rsp.handle;
    ret_code = hh_add_dev_rsp.status;

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_HhRemoveDevInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhRemoveDevInit(tBSA_HH_REMOVE_DEV *p_remove_dev)
{
    APPL_TRACE_API0("BSA_HhRemoveDevInit");

    if (p_remove_dev == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhRemoveDevInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_remove_dev,0,sizeof(tBSA_HH_REMOVE_DEV));

    return BSA_SUCCESS;

}
/*******************************************************************************
 **
 ** Function         BSA_HhRemoveDev
 **
 ** Description      Remove a device from the HID host devices list.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhRemoveDev(tBSA_HH_REMOVE_DEV *p_remove_dev)
{
    APPL_TRACE_API0("BSA_HhRemoveDev");

    if (p_remove_dev == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhRemoveDev param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HH_MSGID_REMOVE_DEV_CMD, p_remove_dev,
            sizeof(tBSA_HH_REMOVE_DEV));
}


/*******************************************************************************
 **
 ** Function         BSA_HhGetInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGetInit(tBSA_HH_GET *p_req)
{
    APPL_TRACE_API0("BSA_HhGetInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGetInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HH_GET));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhGet
 **
 ** Description      HID Get request
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhGet(tBSA_HH_GET *p_req)
{
    APPL_TRACE_API0("BSA_HhGet");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhGet param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HH_MSGID_GET_CMD, p_req,
            sizeof(tBSA_HH_GET));
}

/*******************************************************************************
 **
 ** Function         BSA_HhSetInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSetInit(tBSA_HH_SET *p_req)
{
    APPL_TRACE_API0("BSA_HhSetInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSetInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_HH_SET));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_HhSet
 **
 ** Description      HID Set request
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          status
 **
 *******************************************************************************/
tBSA_STATUS BSA_HhSet(tBSA_HH_SET *p_req)
{
    APPL_TRACE_API0("BSA_HhSet");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_HhSet param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_HH_MSGID_SET_CMD, p_req,
            sizeof(tBSA_HH_SET));
}

/*******************************************************************************
 **
 **              Parsing Utility Functions
 **
 *******************************************************************************/
/*******************************************************************************
 **
 ** Function         BSA_HhParseBootRpt
 **
 ** Description      This utility function parse a boot mode report.
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
#if 0
tBSA_STATUS BSA_HhParseBootRpt(tBSA_HH_BOOT_RPT *p_data, UINT8 *p_report,
        UINT16 report_len)
{
    APPL_TRACE_ERROR0("Function BSA_HhParseBootRpt not yet implemented");
    return -1;
}
#endif


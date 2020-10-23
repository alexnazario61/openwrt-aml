/*****************************************************************************
 **
 **  Name:           bsa_sec_api.c
 **
 **  Description:    This is the public interface file for security part of
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
tBSA_SEC_CB bsa_sec_cb = { NULL };

/*******************************************************************************
 **
 ** Function         BSA_SecSetSecurityInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetSecurityInit(tBSA_SEC_SET_SECURITY *p_set_sec)
{
    APPL_TRACE_API0("BSA_SecSetSecurityInit");

    if (p_set_sec == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecSetSecurityInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_set_sec, 0, sizeof(tBSA_SEC_SET_SECURITY));
    p_set_sec->simple_pairing_io_cap = BSA_SEC_IO_CAP_NONE;
    p_set_sec->ssp_debug = FALSE;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecSetSecurity
 **
 ** Description      Set the Simple Pairing IO capability and security callcack
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetSecurity(tBSA_SEC_SET_SECURITY *p_set_sec)
{
    tBSA_SEC_MSGID_SET_SEC_CMD_REQ sec_set_req;
    int ret_code;

    APPL_TRACE_API0("BSA_SecSetSecurity");

    if (p_set_sec == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecSetSecurity NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_set_sec->sec_cback == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecSetSecurity NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Save application's Security callback */
    bsa_sec_cb.p_app_sec_cback = p_set_sec->sec_cback;

    /* Prepare request parameters */
    sec_set_req.io_cap = p_set_sec->simple_pairing_io_cap;

    /* Set SSP debug mode flag */
    sec_set_req.ssp_debug = p_set_sec->ssp_debug ;

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_SET_SEC_CMD, &sec_set_req,
                    sizeof(tBSA_SEC_MSGID_SET_SEC_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecSetSecurity fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_SecPinCodeReplyInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecPinCodeReplyInit(tBSA_SEC_PIN_CODE_REPLY *p_pin_code_reply)
{
    APPL_TRACE_API0("BSA_SecPinCodeReplyInit");

    if (p_pin_code_reply == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecPinCodeReplyInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_pin_code_reply, 0, sizeof(tBSA_SEC_PIN_CODE_REPLY));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecPinCodeReply
 **
 ** Description      Send back a pin code
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecPinCodeReply(tBSA_SEC_PIN_CODE_REPLY *p_pin_code_reply)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecPinCodeReply");

    if (p_pin_code_reply == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecPinCodeReply NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_pin_code_reply->pin_len > PIN_CODE_LEN)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecPinCodeReply bad pin_len:%d",
                p_pin_code_reply->pin_len);
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_PIN_CODE_REPLY_CMD, p_pin_code_reply,
                    sizeof(tBSA_SEC_MSGID_PIN_CODE_REPLY_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecPinCodeReply fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_SecSpCfmReplyInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSpCfmReplyInit(tBSA_SEC_SP_CFM_REPLY *p_sp_cfm_reply)
{
    APPL_TRACE_API0("BSA_SecSpCfmReplyInit");

    if (p_sp_cfm_reply == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecSpCfmReplyInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sp_cfm_reply, 0, sizeof(tBSA_SEC_SP_CFM_REPLY));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecSpCfmReply
 **
 ** Description      Accept or refuse simple pairing request
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSpCfmReply(tBSA_SEC_SP_CFM_REPLY *p_sp_cfm_reply)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecSpCfmReply");

    if (p_sp_cfm_reply == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecSpCfmReply NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_SP_CFM_REPLY_CMD, p_sp_cfm_reply,
                    sizeof(tBSA_SEC_MSGID_SP_CFM_REPLY_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecSpCfmReply fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_SecBondInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBondInit(tBSA_SEC_BOND *p_sec_bond)
{
    APPL_TRACE_API0("BSA_SecBondInit");

    if (p_sec_bond == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecBondInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_bond, 0, sizeof(tBSA_SEC_BOND));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecBond
 **
 ** Description      Send a pin code in order to bond with a remote device
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBond(tBSA_SEC_BOND *p_sec_bond)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecBond");

    if (p_sec_bond == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecBond NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_BOUND_CMD, p_sec_bond,
                    sizeof(tBSA_SEC_MSGID_BOUND_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecBond fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_SecBondCancelInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBondCancelInit(tBSA_SEC_BOND_CANCEL *p_sec_bond_cancel)
{
    APPL_TRACE_API0("BSA_SecBondCancelInit");

    if (p_sec_bond_cancel == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecBondCancelInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_bond_cancel, 0, sizeof(tBSA_SEC_BOND_CANCEL));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecBondCancel
 **
 ** Description      Cancel bonding procedure
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecBondCancel(tBSA_SEC_BOND_CANCEL *p_sec_bond_cancel)
{
    int ret_code = BSA_SUCCESS;

    APPL_TRACE_API0("BSA_SecBondCancel");

    if (p_sec_bond_cancel == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecBondCancel NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_BOUND_CANCEL_CMD, p_sec_bond_cancel,
                    sizeof(tBSA_SEC_MSGID_BOUND_CANCEL_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecBondCancel fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_SecAuthorizeReplyInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAuthorizeReplyInit(tBSA_SEC_AUTH_REPLY *p_sec_auth_reply)
{
    APPL_TRACE_API0("BSA_SecAuthorizeReplyInit");

    if (p_sec_auth_reply == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecAuthorizeReplyInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_auth_reply, 0, sizeof(tBSA_SEC_AUTH_REPLY));
    p_sec_auth_reply->auth = BSA_SEC_NOT_AUTH;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecAuthorizeReply
 **
 ** Description      Reply to an authorization request for a service
 **
 ** Parameters       Pointer to structure containing API parameters
 **                    BSA_NOT_AUTH to refuse,
 **                    BSA_AUTH_TEMP to grant access temporarily,
 **                    BSA_AUTH_PERM to grant permanent access
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAuthorizeReply(tBSA_SEC_AUTH_REPLY *p_sec_auth_reply)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecAuthorizeReply");

    if (p_sec_auth_reply == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecAuthorizeReply NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_AUTHORIZE_REPLY_CMD, p_sec_auth_reply,
                    sizeof(tBSA_SEC_MSGID_AUTHORIZE_REPLY_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecAuthorizeReply fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_SecRemoveDeviceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecRemoveDeviceInit(tBSA_SEC_REMOVE_DEV *p_sec_del_dev)
{
    APPL_TRACE_API0("BSA_SecRemoveDeviceInit");

    if (p_sec_del_dev == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecDeleteDeviceInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_del_dev, 0, sizeof(tBSA_SEC_REMOVE_DEV));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecRemoveDevice
 **
 ** Description      Remove a device from the local database
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecRemoveDevice(tBSA_SEC_REMOVE_DEV *p_sec_del_dev)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecRemoveDevice");

    if (p_sec_del_dev == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecDeleteDevice NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_DEL_DEV_CMD, p_sec_del_dev,
                    sizeof(tBSA_SEC_MSGID_REMOVE_DEV_CMD_REQ));


    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecDeleteDevice fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_SecAddDeviceInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddDeviceInit(tBSA_SEC_ADD_DEV *p_sec_add_dev)
{
    APPL_TRACE_API0("BSA_SecAddDeviceInit");

    if (p_sec_add_dev == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecAddDeviceInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_add_dev, 0, sizeof(tBSA_SEC_ADD_DEV));

#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    p_sec_add_dev->device_type = BT_DEVICE_TYPE_BREDR;
#endif

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecAddDevice
 **
 ** Description      Add a device to the local database
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddDevice(tBSA_SEC_ADD_DEV *p_sec_add_dev)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecAddDevice");

    if (p_sec_add_dev == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecAddDevice NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_ADD_DEV_CMD, p_sec_add_dev,
                    sizeof(tBSA_SEC_MSGID_ADD_DEV_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecAddDevice fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;

}

/*******************************************************************************
 **
 ** Function         BSA_SecReadOOBInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecReadOOBInit(tBSA_SEC_READ_OOB *p_sec_read_oob)
{
    APPL_TRACE_API0("BSA_SecReadOOBInit");

    if (p_sec_read_oob == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecReadOOBInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_read_oob, 0, sizeof(tBSA_SEC_READ_OOB));

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecReadOOB
 **
 ** Description      Reads OOB information from local controller
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecReadOOB(tBSA_SEC_READ_OOB *p_sec_read_oob)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecReadOOB");

    if (p_sec_read_oob == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecReadOOB NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_READ_OOB_CMD, p_sec_read_oob,
                    sizeof(tBSA_SEC_MSGID_READ_OOB_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecReadOOB fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;

}

/*******************************************************************************
 **
 ** Function         BSA_SecSetRemoteOOBInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetRemoteOOBInit(tBSA_SEC_SET_REMOTE_OOB *p_sec_set_remote_oob)
{
    APPL_TRACE_API0("BSA_SecSetRemoteOOBInit");

    if (p_sec_set_remote_oob == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecSetRemoteOOBInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_set_remote_oob, 0, sizeof(tBSA_SEC_SET_REMOTE_OOB));

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecSetRemoteOOB
 **
 ** Description      Respond to remote OOB information request from the stack
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecSetRemoteOOB(tBSA_SEC_SET_REMOTE_OOB *p_sec_set_remote_oob)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecSetRemoteOOB");

    if (p_sec_set_remote_oob == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecSetRemoteOOB NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_SET_REMOTE_OOB_CMD, p_sec_set_remote_oob,
                    sizeof(tBSA_SEC_MSGID_SET_REMOTE_OOB_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecSetRemoteOOB fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;

}

/*******************************************************************************
 **
 ** Function         BSA_SecAddSiDevInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddSiDevInit(tBSA_SEC_ADD_SI_DEV*p_sec_add_si_dev)
{
    APPL_TRACE_API0("BSA_SecAddSiDevInit");

    if (p_sec_add_si_dev == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecAddSiDevInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_sec_add_si_dev, 0, sizeof(tBSA_SEC_ADD_SI_DEV));

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_SecAddSiDev
 **
 ** Description      Add a special interest device to the local database
 **
 ** Parameters       Pointer to structure containing API parameters
 **
 ** Returns          Status of the function execution (0=OK or error code)
 **
 *******************************************************************************/
tBSA_STATUS BSA_SecAddSiDev(tBSA_SEC_ADD_SI_DEV*p_sec_add_si_dev)
{
    int ret_code;

    APPL_TRACE_API0("BSA_SecAddSiDev");

    if (p_sec_add_si_dev == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_SecAddSiDev NULL param pointer");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_message_receive_status(BSA_SEC_MSGID_ADD_SI_DEV_CMD, p_sec_add_si_dev,
                    sizeof(tBSA_SEC_MSGID_ADD_SI_DEV_CMD_REQ));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_SecAddSiDev fails status:%d", ret_code);
        return ret_code;
    }

    return ret_code;
}

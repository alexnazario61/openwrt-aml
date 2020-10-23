/*****************************************************************************
**
**  Name:           bsa_pbc_api.c
**
**  Description:    This is the public interface file for PBAP client part of
**                  the Bluetooth simplified API
**
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
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
tBSA_PBC_CB bsa_pbc_cb =
{
    NULL
};

/*******************************************************************************
**
** Function         BSA_PbcEnableInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcEnableInit(tBSA_PBC_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_PbcEnableInit");

    if (p_enable == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_PBC_ENABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcEnable
**
** Description      This function enables PBAP server and registers PBAP with
**                  lower layers.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcEnable(tBSA_PBC_ENABLE *p_enable)
{
    tBSA_PBC_MSGID_ENABLE_CMD_REQ pbc_enable_req;

    APPL_TRACE_API0("BSA_PbcEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcEnable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(&pbc_enable_req, 0, sizeof(pbc_enable_req));

    /* Save application's Security callback */
    bsa_pbc_cb.p_app_pbc_cback = p_enable->p_cback;

    /* Prepare request parameters */
    pbc_enable_req.app_id = BSA_PBC_APP_ID;
    pbc_enable_req.local_features = p_enable->local_features;

    return bsa_send_message_receive_status(BSA_PBC_MSGID_ENABLE_CMD,
        &pbc_enable_req, sizeof(tBSA_PBC_MSGID_ENABLE_CMD_REQ));
}

/*******************************************************************************
**
** Function         BSA_PbcDisableInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcDisableInit(tBSA_PBC_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_PbcDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_disable, 0, sizeof(tBSA_PBC_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcDisable
**
** Description      This function is called when the host is about power down.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcDisable(tBSA_PBC_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_PbcDisable");
    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcDisable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_PBC_MSGID_DISABLE_CMD, p_disable,
        sizeof(tBSA_PBC_DISABLE));
}

/*******************************************************************************
**
** Function         BSA_PbcOpenInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcOpenInit(tBSA_PBC_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_PbcOpenInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_open, 0, sizeof(tBSA_PBC_OPEN));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcOpen
**
** Description      Open a connection to an PBAP server.
**
**                  When the connection is open the callback function
**                  will be called with a BSA_PBC_OPEN_EVT.  If the connection
**                  fails or otherwise is closed the callback function will be
**                  called with a BSA_PBC_CLOSE_EVT.
**
**                  Note: Pbc always enable (BSA_SEC_AUTHENTICATE | BSA_SEC_ENCRYPT)
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcOpen(tBSA_PBC_OPEN *p_open)
{
    BD_ADDR zeroBDA;
    memset(zeroBDA, 0, sizeof(BD_ADDR));

    APPL_TRACE_API0("BSA_PbcOpen");
    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_open->sec_mask == BSA_SEC_NONE)
    {
        APPL_TRACE_ERROR0("BSA_PbcOpen Invalid security setting");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (memcmp(zeroBDA, p_open->bd_addr, sizeof(BD_ADDR)) == 0)
    {
        APPL_TRACE_ERROR0("BSA_PbcOpen Invalid BD_ADDR");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_PBC_MSGID_OPEN_CMD, p_open,
        sizeof(tBSA_PBC_OPEN));
}

/*******************************************************************************
**
** Function         BSA_PbcCloseInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcCloseInit(tBSA_PBC_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_PbcCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_close, 0, sizeof(tBSA_PBC_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcClose
**
** Description      Close the current connection to the server. Aborts any
**                  active PBAP transfer.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcClose(tBSA_PBC_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_PbcClose");
    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_PBC_MSGID_CLOSE_CMD, p_close,
        sizeof(tBSA_PBC_CLOSE));
}

/*******************************************************************************
**
** Function         BSA_PbcAbortInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAbortInit(tBSA_PBC_ABORT *p_abort)
{
    APPL_TRACE_API0("BSA_PbcAbortInit");

    if (p_abort == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcAbortInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_abort, 0, sizeof(tBSA_PBC_ABORT));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcAbort
**
** Description      Aborts any active PBC operation.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAbort(tBSA_PBC_ABORT *p_abort)
{
    APPL_TRACE_API0("BSA_PbcAbort");
    if (p_abort == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcAbort param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_PBC_MSGID_ABORT_CMD, p_abort,
        sizeof(tBSA_PBC_ABORT));
}

/*******************************************************************************
**
** Function         BSA_PbcAuthRspInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAuthRspInit(tBSA_PBC_AUTHRSP *p_authrsp)
{
    APPL_TRACE_API0("BSA_PbcAuthRspInit");

    if (p_authrsp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcAuthRspInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_authrsp, 0, sizeof(tBSA_PBC_AUTHRSP));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcAuthRsp
**
** Description      This function is called to send OBEX an authentication
**                  challenge to the connected OBEX client.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcAuthRsp(tBSA_PBC_AUTHRSP *p_authrsp)
{
    APPL_TRACE_API0("BSA_PbcAuthRsp");
    if (p_authrsp == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcAuthRsp param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_PBC_MSGID_AUTH_RSP_CMD, p_authrsp,
        sizeof(tBSA_PBC_AUTHRSP));
}

/*******************************************************************************
**
** Function         BSA_PbcGetInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcGetInit(tBSA_PBC_GET *p_get)
{
    APPL_TRACE_API0("BSA_PbcGetInit");
    if (p_get == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcGetInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_get, 0, sizeof(tBSA_PBC_GET));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcGet
**
** Description      This function is called to for all Get operations
**                  GetCard, GetList, GetPhoneBook etc.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcGet(tBSA_PBC_GET *p_get)
{
    APPL_TRACE_API0("BSA_PbcGet");
    if (p_get == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcGet param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_PBC_MSGID_GET_CMD, p_get,
        sizeof(tBSA_PBC_GET));
}

/*******************************************************************************
**
** Function         BSA_PbcSetInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcSetInit(tBSA_PBC_SET *p_set)
{
    APPL_TRACE_API0("BSA_PbcSetInit");
    if (p_set == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcSetInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_set, 0, sizeof(tBSA_PBC_SET));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcSet
**
** Description      This function is called to for all Set operations like change directory
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcSet(tBSA_PBC_SET *p_set)
{
    APPL_TRACE_API0("BSA_PbcSet");
    if (p_set == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcSet param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_PBC_MSGID_SET_CMD, p_set,
        sizeof(tBSA_PBC_SET));
}

/*******************************************************************************
 **
 ** Function         BSA_PbcCancelInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbcCancelInit(tBSA_PBC_CANCEL *pCancel)
{
    APPL_TRACE_API0("BSA_PbcCancelInit");

    if (pCancel == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcCancelInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pCancel, 0, sizeof(tBSA_PBC_CANCEL));


    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_PbcCancel
**
** Description      This function is called to cancel connection
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_PbcCancel (tBSA_PBC_CANCEL    *pCancel)
{
    APPL_TRACE_API0("BSA_PbcCancel");
    if (pCancel == NULL)
    {
        APPL_TRACE_ERROR0("BSA_PbcCancel param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_PBC_MSGID_CANCEL_CMD, pCancel, sizeof(tBSA_PBC_CANCEL));
}

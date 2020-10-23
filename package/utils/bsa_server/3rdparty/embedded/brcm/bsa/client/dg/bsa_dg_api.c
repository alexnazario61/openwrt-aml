/*****************************************************************************
 **
 **  Name:           bsa_dg_api.c
 **
 **  Description:    This is the public interface file for Security part of
 **                  the Bluetooth simplified API
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

/*
 * Global variables
 */
tBSA_DG_CB bsa_dg_cb = { NULL, };

/*******************************************************************************
 **
 ** Function         BSA_DgEnableInit
 **
 ** Description      Init a structure tBSA_DG_ENABLE to be used with BSA_DgEnable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgEnableInit(tBSA_DG_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_DgEnableInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_DG_ENABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DgEnable
 **
 ** Description      Enable the data gateway service.  This function must be
 **                  called before any other functions in the DG API are called.
 **                  When the enable operation is complete the callback function
 **                  will return BSA_SUCCESS.  After the DG
 **                  service is enabled a server can be started by calling
 **                  BSA_DgListen().
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgEnable(tBSA_DG_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_DgEnable");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_req->p_cback == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* Save application's Security callback */
    bsa_dg_cb.p_app_cback = p_req->p_cback;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_DG_MSGID_ENABLE_CMD, p_req, sizeof(tBSA_DG_ENABLE));


}

/*******************************************************************************
 **
 ** Function         BSA_DgDisableInit
 **
 ** Description      Init structure tBSA_DG_DISABLE to be used with BSA_DgDisable
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgDisableInit(tBSA_DG_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_DgDisableInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DgDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_DG_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DgDisable
 **
 ** Description      Disable the data gateway service.  Before calling this
 **                  function all DG servers must be shut down by calling
 **                  BSA_DgShutdown().
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgDisable(tBSA_DG_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_DgDisable");

    /* Save application's Security callback */
    bsa_dg_cb.p_app_cback = NULL;

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_DG_MSGID_DISABLE_CMD, p_req, sizeof(tBSA_DG_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_DgListenInit
 **
 ** Description      Init structure tBSA_DG_DEREGISTER to be used with BSA_DgRegister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgListenInit(tBSA_DG_LISTEN *p_req)
{
    APPL_TRACE_API0("BSA_DgListenInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DgListenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    strncpy(p_req->service_name, "dg service name", BSA_SERVICE_NAME_LEN);

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DgListen
 **
 ** Description      Create a DG server for DUN or SPP.  After creating a
 **                  server peer devices can open an RFCOMM connection to the
 **                  server.  When the listen operation has started the function will return BSA_SUCCESS
 **                  the handle associated with this server.  The handle
 **                  identifies server when calling other DG functions such as
 **                  BSA_DgClose() or BSA_DgShutdown().
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgListen(tBSA_DG_LISTEN *p_req)
{
    tBSA_DG_MSGID_LISTEN_CMD_RSP rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_DgListen");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_DgListen NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_DG_MSGID_LISTEN_CMD, p_req, sizeof(tBSA_DG_LISTEN), &rsp,
            sizeof(rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_DgListen fails status:%d", ret_code);
        return ret_code;
    }

    /* Extract received parameters */
    p_req->handle = rsp.handle;

    return rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_DgShutdownInit
 **
 ** Description      Init structure tBSA_DG_DEREGISTER to be used with BSA_DgDeregister
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgShutdownInit(tBSA_DG_SHUTDOWN*p_req)
{
    APPL_TRACE_API0("BSA_DgShutdownInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DgShutdownInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_DG_SHUTDOWN));

    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_DgShutdown
**
** Description      Shutdown a DG server previously started by calling
**                  BSA_DgListen().  The server will no longer be available
**                  to peer devices.  If there is currently a connection open
**                  to the server it will be closed.
**
**
** Returns          void
**
*******************************************************************************/
BTAPI tBSA_STATUS BSA_DgShutdown(tBSA_DG_SHUTDOWN *p_req)
{
    APPL_TRACE_API0("BSA_DgShutdown");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_DG_MSGID_SHUTDOWN_CMD, p_req, sizeof(tBSA_DG_SHUTDOWN));

}

/*******************************************************************************
 **
 ** Function         BSA_DgOpenInit
 **
 ** Description      Init structure tBSA_DG_OPEN to be used with BSA_DgOpen
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgOpenInit(tBSA_DG_OPEN * p_req)
{
    APPL_TRACE_API0("BSA_DgOpenInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DgOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_DG_OPEN));
    p_req->service = BSA_SPP_SERVICE_ID;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DgOpen
 **
 ** Description      Open a DG client connection to a peer device.  BSA first
 **                  searches for the requested service on the peer device.  If
 **                  the service name is specified it will also match the
 **                  service name.  Then BSA initiates an RFCOMM connection to
 **                  the peer device.  The handle associated with the connection
 **                  is returned with the BSA_DG_OPEN_EVT.  If the connection
 **                  fails or closes at any time the callback function will be
 **                  called with a BSA_DG_CLOSE_EVT.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgOpen(tBSA_DG_OPEN * p_req)
{
    APPL_TRACE_API0("BSA_DgOpen");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_DG_MSGID_OPEN_CMD, p_req, sizeof(tBSA_DG_OPEN));
}

/*******************************************************************************
 **
 ** Function         BSA_DgCloseInit
 **
 ** Description      Init structure tBSA_DG_CLOSE to be used with BSA_DgClose
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgCloseInit(tBSA_DG_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_DgCloseInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DgCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_DG_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DgClose
 **
 ** Description      Close a DG server connection to a peer device.  BSA will
 **                  close the RFCOMM connection to the peer device.  The server
 **                  will still be listening for subsequent connections.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgClose(tBSA_DG_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_DgClose");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_DG_MSGID_CLOSE_CMD, p_req, sizeof(tBSA_DG_CLOSE));
}


/*******************************************************************************
 **
 ** Function         BSA_DgFindServiceInit
 **
 ** Description      Init structure tBSA_DG_FIND_SERVICE to be used with BSA_DgFindService
 **
 ** Returns          tBSA_STATUS
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgFindServiceInit(tBSA_DG_FIND_SERVICE * p_req)
{
    APPL_TRACE_API0("BSA_DgFindServiceInit");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_DgFindServiceInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_DG_FIND_SERVICE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_DgFindService
 **
 ** Description      Performs SDP on specified device to check if the device supports
 **                  the specified serial service. Response is received in the
 **                  BSA_DG_FIND_SERVICE_EVT indicating if the service is present
 **                  or not.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_DgFindService(tBSA_DG_FIND_SERVICE * p_req)
{
    APPL_TRACE_API0("BSA_DgFindService");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_DG_MSGID_FIND_SERVICE_CMD, p_req, sizeof(tBSA_DG_FIND_SERVICE));
}

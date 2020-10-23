/*****************************************************************************
 **
 **  Name:           bsa_opc_api.h
 **
 **  Description:    This is the public interface file for Object Push Server part of
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
tBSA_OPC_CB bsa_opc_cb = { NULL };

void default_app_opc_cback(tBSA_OPC_EVT event, tBSA_OPC_MSG *p_data)
{
        APPL_TRACE_ERROR0("default_app_opc_cback NULL Callback");
}

/*******************************************************************************
 **
 ** Function         BSA_OpcEnableInit
 **
 ** Description      Initialize the tBSA_OPC_ENABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_OpcEnableInit(tBSA_OPC_ENABLE *p_req)
{

    APPL_TRACE_API0("BSA_OpcEnableInit");

    p_req->single_op = TRUE;
    p_req->sec_mask = BSA_SEC_NONE;
    p_req->p_cback = default_app_opc_cback;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_OpcEnable
 **
 ** Description      Enable the object push server.  This function must be
 **                  called before any other functions in the OPS API are called.
 **                  When the enable operation is complete the function returns.
 **
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcEnable(tBSA_OPC_ENABLE *p_req)
{
    tBSA_OPC_MSGID_ENABLE_CMD_RSP rsp;
    tBSA_STATUS status;

    APPL_TRACE_API0("BSA_OpcEnable");

    if (p_req->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_OpcEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Save application's Security callback */
    bsa_opc_cb.p_cback = p_req->p_cback;

    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_OPC_MSGID_ENABLE_CMD, p_req,
            sizeof(tBSA_OPC_ENABLE), &rsp, sizeof(rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_OpcEnable bsa_send_receive_message error %x", status);
        return status;
    }

    return rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_OpcDisableInit
 **
 ** Description      Initialize the tBSA_OPC_DISABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcDisableInit(tBSA_OPC_DISABLE *p_req)
{
    memset(p_req, 0, sizeof(tBSA_OPC_DISABLE));

    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_OpcDisable
 **
 ** Description      Disable the file transfer server.  If the server is currently
 **                  connected to a peer device the connection will be closed.
 **
 ** Returns          int: Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_OpcDisable(tBSA_OPC_DISABLE *p_req)
{
    tBSA_OPC_MSGID_PUSH_CMD_RSP rsp;
    tBSA_STATUS status;
    
    APPL_TRACE_API0("BSA_OpcDisable");
    
    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_OPC_MSGID_DISABLE_CMD, p_req,
            sizeof(tBSA_OPC_DISABLE), &rsp, sizeof(rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_OpcDisable fails status:%d", status);
        return status;
    }

    /* Extract received parameters */
    return rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_OpcPush
 **
 ** Description      Initialize the tBSA_OPC_PUSH structure to default values.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcPushInit(tBSA_OPC_PUSH *p_req)
{
    memset(p_req, 0, sizeof(tBSA_OPC_PUSH));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_OpcPush
 **
 ** Description      Push an object to a peer device.  p_name must point to
 **                  a fully qualified path and file name.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcPush(tBSA_OPC_PUSH *p_req)
{
    tBSA_OPC_MSGID_PUSH_CMD_RSP rsp;
    tBSA_STATUS status;
    
    APPL_TRACE_API0("BSA_OpcPush");
    
    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_OPC_MSGID_PUSH_CMD, p_req,
            sizeof(tBSA_OPC_PUSH), &rsp, sizeof(rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_OpcPush fails status:%d", status);
        return status;
    }

    /* Extract received parameters */
    return rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_OpcPullCard
 **
 ** Description      Pull default card from peer. p_path must point to a fully
 **                  qualified path specifying where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcPullCardInit(tBSA_OPC_PULL_CARD *p_req)
{
    memset(p_req, 0, sizeof(tBSA_OPC_PULL_CARD));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_OpcPullCard
 **
 ** Description      Pull default card from peer. p_path must point to a fully
 **                  qualified path specifying where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcPullCard(tBSA_OPC_PULL_CARD *p_req)
{
    tBSA_OPC_MSGID_PULL_CARD_CMD_RSP rsp;
    tBSA_STATUS status;
    
    APPL_TRACE_API0("BSA_OpcPullCard");
    
    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_OPC_MSGID_PULL_CARD_CMD, p_req,
            sizeof(tBSA_OPC_PULL_CARD), &rsp, sizeof(rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_OpcPullCard fails status:%d", status);
        return status;
    }

    /* Extract received parameters */
    return rsp.status;

}

/*******************************************************************************
 **
 ** Function         BSA_OpcExchCard
 **
 ** Description      Exchange business cards with a peer device. p_send points to
 **                  a fully qualified path and file name of vcard to push.
 **                  p_recv_path points to a fully qualified path specifying
 **                  where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcExchCardInit(tBSA_OPC_EXCH_CARD *p_req)
{
    memset(p_req, 0, sizeof(tBSA_OPC_EXCH_CARD));
    return BSA_SUCCESS;
}


/*******************************************************************************
 **
 ** Function         BSA_OpcExchCard
 **
 ** Description      Exchange business cards with a peer device. p_send points to
 **                  a fully qualified path and file name of vcard to push.
 **                  p_recv_path points to a fully qualified path specifying
 **                  where to store the pulled card.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcExchCard(tBSA_OPC_EXCH_CARD *p_req)
{
    tBSA_OPC_MSGID_EXCH_CARD_CMD_RSP rsp;
    tBSA_STATUS status;
    
    APPL_TRACE_API0("BSA_OpcExchCard");
    
    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_OPC_MSGID_EXCH_CARD_CMD, p_req,
            sizeof(tBSA_OPC_EXCH_CARD), &rsp, sizeof(rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_OpcExchCard fails status:%d", status);
        return status;
    }

    /* Extract received parameters */
    return rsp.status;
    
}

/*******************************************************************************
 **
 ** Function         BSA_OpcCloseInit
 **
 ** Description      Initialize the tBSA_OPC_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_OpcCloseInit(tBSA_OPC_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_OpcCloseInit");

    memset(p_req, 0, sizeof(tBSA_OPC_CLOSE));
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_OpcClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the object
 **                  push is completed.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_OpcClose(tBSA_OPC_CLOSE *p_req)
{
    tBSA_OPC_MSGID_CLOSE_CMD_RSP rsp;
    tBSA_STATUS status;
    
    APPL_TRACE_API0("BSA_OpcClose");
    
    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_OPC_MSGID_CLOSE_CMD, p_req,
            sizeof(tBSA_OPC_CLOSE), &rsp, sizeof(rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_OpcClose fails status:%d", status);
        return status;
    }

    /* Extract received parameters */
    return rsp.status;
}



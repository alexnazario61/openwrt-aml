/*****************************************************************************
 **
 **  Name:           bsa_fts_api.c
 **
 **  Description:    This is the public interface file for File Transfer Server part of
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
tBSA_FTS_CB bsa_fts_cb = { NULL };

void default_app_fts_cback(tBSA_FTS_EVT event, tBSA_FTS_MSG *p_data)
{
    APPL_TRACE_ERROR0("default_app_fts_cback NULL Callback");
}

/*******************************************************************************
 **
 ** Function         BSA_FtsEnableInit
 **
 ** Description      Initialize the tBSA_FTS_ENABLE_CMD structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsEnableInit(tBSA_FTS_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_FtsEnableInit");

    memset(p_req, 0, sizeof(tBSA_FTS_ENABLE));
    p_req->sec_mask = BSA_SEC_NONE;
    strncpy(p_req->service_name, "FTS", BSA_FTS_SERVICE_NAME_LEN_MAX);
    strncpy(p_req->root_path, "./", BSA_FTS_ROOT_PATH_LEN_MAX);
    p_req->enable_authen = FALSE;
    strncpy(p_req->realm, "guest", BSA_FT_REALM_LEN_MAX);
    p_req->p_cback = default_app_fts_cback;
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtsEnable
 **
 ** Description      Enable the file transfer server.  This function must be
 **                  called before any other functions in the FTS API are called.
 **                  When the enable operation is complete the callback function
 **                  will be called with an BSA_FTS_ENABLE_EVT event.
 **
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsEnable(tBSA_FTS_ENABLE *p_req)
{
    tBSA_FTS_MSGID_ENABLE_CMD_RSP fts_enable_rsp;
    tBSA_STATUS ret_code;
    APPL_TRACE_API0("BSA_FtsEnable");

    if (p_req->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_FtsEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    if (strlen(p_req->service_name) > BSA_FTS_SERVICE_NAME_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_FtsEnable Service Name too long");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (strlen(p_req->root_path) > BSA_FTS_ROOT_PATH_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_FtsEnable Root path too long");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    if (strlen(p_req->realm) > BSA_FT_REALM_LEN_MAX)
    {
        APPL_TRACE_ERROR0("BSA_FtsEnable Realm too long");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

       /* Save application's callback */
    bsa_fts_cb.p_app_cback = p_req->p_cback;


    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_FTS_MSGID_ENABLE_CMD, p_req,
            sizeof(tBSA_FTS_ENABLE), &fts_enable_rsp, sizeof(tBSA_FTS_MSGID_ENABLE_CMD_RSP));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_FtsEnable bsa_send_receive_message error %x", ret_code);
        return ret_code;
    }
    /* Extract received parameters */
    ret_code = fts_enable_rsp.status;

    return ret_code;
}


/*******************************************************************************
 **
 ** Function         BSA_FtsDisableInit
 **
 ** Description      Initialize the tBSA_FTS_DISABLE_CMD structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsDisableInit(tBSA_FTS_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_FtsDisableInit");
    memset(p_req, 0, sizeof(tBSA_FTS_DISABLE));
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_FtsDisable
 **
 ** Description      Disable the file transfer server.  If the server is currently
 **                  connected to a peer device the connection will be closed.
 **
 ** Returns          int: Status
 **
 *******************************************************************************/

tBSA_STATUS BSA_FtsDisable(tBSA_FTS_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_FtsDisable");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_FTS_MSGID_DISABLE_CMD, p_req, sizeof(tBSA_FTS_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_FtsCloseInit
 **
 ** Description      Initialize the tBSA_FTS_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsCloseInit(tBSA_FTS_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_FtsCloseInit");
    memset(p_req, 0, sizeof(tBSA_FTS_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtsClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the transfer
 **                  is completed.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsClose(tBSA_FTS_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_FtsClose");

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_FTS_MSGID_CLOSE_CMD, p_req, sizeof(tBSA_FTS_CLOSE));
}



/*******************************************************************************
 **
 ** Function         BSA_FtsAuthRspInit
 **
 ** Description      Initialize the tBSA_FTS_AUTH_RSP_CMD structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsAuthRspInit (tBSA_FTS_AUTH_RSP *p_req)
{
    APPL_TRACE_API0("BSA_FtsAuthRspInit");

    memset(p_req, 0, sizeof(tBSA_FTS_AUTH_RSP));
    strncpy(p_req->password, "0000", BSA_FT_PASSWORD_LEN_MAX);
    strncpy(p_req->userid, "guest", BSA_FT_USER_ID_LEN_MAX);
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtsAuthRsp
 **
 ** Description      Sends an OBEX authentication challenge to the connected
 **                  OBEX client. Called in response to an BSA_FTS_AUTH_EVT event.
 **                  Used when "enable_authen" is set to TRUE in BSA_FtsEnable().
 **
 **                  Note: If the "userid_required" is TRUE in the BSA_FTS_AUTH_EVT
 **                        event, then p_userid is required, otherwise it is optional.
 **
 **                  password  must be less than BSA_FTS_MAX_AUTH_KEY_SIZE (16 bytes)
 **                  userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsAuthRsp(tBSA_FTS_AUTH_RSP *p_req)
{
    tBSA_FTS_MSGID_AUTH_RSP_CMD_RSP fts_auth_rsp;
    tBSA_STATUS ret_code;
    APPL_TRACE_API0("BSA_FtsAuthRsp");

    ret_code = bsa_send_receive_message(BSA_FTS_MSGID_AUTH_RSP_CMD, p_req, sizeof(tBSA_FTS_AUTH_RSP), &fts_auth_rsp,
            sizeof(tBSA_FTS_MSGID_AUTH_RSP_CMD_RSP));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_FtsAuthRsp bsa_send_receive_message error %x", ret_code);
    }

    /* Extract received parameters */
    return fts_auth_rsp.status;
}

/*******************************************************************************
 **
 ** Function         BSA_FtsAccessInit
 **
 ** Description      Initialize the tBSA_FTS_ACCESS structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtsAccessInit(tBSA_FTS_ACCESS *p_req)
{
    APPL_TRACE_API0("BSA_FtsAccessInit");

    memset(p_req, 0, sizeof(tBSA_FTS_ACCESS));
    p_req->oper = BSA_FT_OPER_DEFAULT;
    strncpy(p_req->p_name, "NONE", BSA_FT_FILENAME_MAX);
    p_req->access = BSA_FT_ACCESS_FORBID;
    p_req->access_mask = 0x00;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtsAccessRsp
 **
 ** Description      Sends a reply to an access request event (BSA_FTS_ACCESS_EVT).
 **                  This call MUST be made whenever the event occurs.
 **
 ** Parameters       oper    - operation being accessed.
 **                  access  - BSA_FT_ACCESS_ALLOW or BSA_FT_ACCESS_FORBID
 **                  p_name  - Full path of file to pulled or pushed.
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS  BSA_FtsAccess(tBSA_FTS_ACCESS *p_req)
{
    tBSA_FTS_MSGID_ACCESS_CMD_RSP fts_access_rsp;
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_FtsAccess");

    ret_code = bsa_send_receive_message(BSA_FTS_MSGID_ACCESS_CMD, p_req, sizeof(tBSA_FTS_ACCESS), &fts_access_rsp,
            sizeof(tBSA_FTS_MSGID_ACCESS_CMD_RSP));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_FtsAccess bsa_send_receive_message error %x", ret_code);
    }

    /* Extract received parameters */
    return fts_access_rsp.status;
}


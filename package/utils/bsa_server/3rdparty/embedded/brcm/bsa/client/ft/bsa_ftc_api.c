/*****************************************************************************
 **
 **  Name:           bsa_ftc_api.h
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
tBSA_FTC_CB bsa_ftc_cb = { NULL };

void default_app_ftc_cback(tBSA_FTC_EVT event, tBSA_FTC_MSG *p_data)
{
    APPL_TRACE_ERROR0("default_app_ftc_cback NULL Callback");
}

/*******************************************************************************
 **
 ** Function         BSA_FtcEnableInit
 **
 ** Description      Initialize the tBSA_FTC_ENABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcEnableInit(tBSA_FTC_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_FtcEnableInit");

    memset(p_req, 0, sizeof(tBSA_FTC_ENABLE));
    bsa_ftc_cb.p_app_cback = default_app_ftc_cback;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtcEnable
 **
 ** Description      Enable the file transfer server.  This function must be
 **                  called before any other functions in the FTC API are called.
 **                  When the enable operation is complete the callback function
 **                  will be called with an BSA_FTC_ENABLE_EVT event.
 **
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcEnable(tBSA_FTC_ENABLE *p_req)
{
    APPL_TRACE_API0("BSA_FtcEnable");

    if (p_req->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_FtcEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Save application's callback */
    bsa_ftc_cb.p_app_cback = p_req->p_cback;
    return bsa_send_message_receive_status(BSA_FTC_MSGID_ENABLE_CMD, p_req, sizeof(tBSA_FTC_ENABLE));
}


/*******************************************************************************
 **
 ** Function         BSA_FtcDisableInit
 **
 ** Description      Initialize the tBSA_FTC_DISABLE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcDisableInit(tBSA_FTC_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_FtcDisableInit");
    memset(p_req, 0, sizeof(tBSA_FTC_DISABLE));
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_FtcDisable
 **
 ** Description      Disable the file transfer server.  If the server is currently
 **                  connected to a peer device the connection will be closed.
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcDisable(tBSA_FTC_DISABLE *p_req)
{
    APPL_TRACE_API0("BSA_FtcDisable");

    /* Save application's callback */
    bsa_ftc_cb.p_app_cback = NULL;

    return bsa_send_message_receive_status(BSA_FTC_MSGID_DISABLE_CMD, p_req, sizeof(tBSA_FTC_DISABLE));
}

/*******************************************************************************
 **
 ** Function         BSA_FtcOpenInit
 **
 ** Description      Initialize the tBSA_FTC_OPEN structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcOpenInit(tBSA_FTC_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_FtcOpenInit");

    memset(p_req, 0, sizeof(tBSA_FTC_OPEN));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtcClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the transfer
 **                  is completed.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcOpen(tBSA_FTC_OPEN *p_req)
{
    APPL_TRACE_API0("BSA_FtcOpen");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_OPEN_CMD, p_req, sizeof(tBSA_FTC_OPEN));

}

/*******************************************************************************
 **
 ** Function         BSA_FtcCloseInit
 **
 ** Description      Initialize the tBSA_FTC_CLOSE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcCloseInit(tBSA_FTC_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_FtcCloseInit");
    memset(p_req, 0, sizeof(tBSA_FTC_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtcClose
 **
 ** Description      Close the current connection.  This function is called if
 **                  the phone wishes to close the connection before the transfer
 **                  is completed.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcClose(tBSA_FTC_CLOSE *p_req)
{
    APPL_TRACE_API0("BSA_FtcClose");
    return bsa_send_message_receive_status(BSA_FTC_MSGID_CLOSE_CMD, p_req, sizeof(tBSA_FTC_CLOSE));

}


/*******************************************************************************
 **
 ** Function         BSA_FtcAuthRspInit
 **
 ** Description      Initialize the tBSA_FTC_AUTH_RSP structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcAuthRspInit (tBSA_FTC_AUTH_RSP *p_req)
{
    APPL_TRACE_API0("BSA_FtcAuthRspInit");

    memset(p_req, 0, sizeof(tBSA_FTC_AUTH_RSP));
    strncpy(p_req->password, "0000", BSA_FT_PASSWORD_LEN_MAX);
    strncpy(p_req->userid, "guest", BSA_FT_USER_ID_LEN_MAX);

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_FtcAuthRsp
 **
 ** Description      Sends an OBEX authentication challenge to the connected
 **                  OBEX client. Called in response to an BSA_FTC_AUTH_EVT event.
 **                  Used when "enable_authen" is set to TRUE in BSA_FtcEnable().
 **
 **                  Note: If the "userid_required" is TRUE in the BSA_FTC_AUTH_EVT
 **                        event, then p_userid is required, otherwise it is optional.
 **
 **                  p_password  must be less than BSA_FTC_MAX_AUTH_KEY_SIZE (16 bytes)
 **                  p_userid    must be less than OBX_MAX_REALM_LEN (defined in target.h)
 **
 ** Returns          int: Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcAuthRsp(tBSA_FTC_AUTH_RSP *p_req)
{
    APPL_TRACE_API0("BSA_FtcAuthRsp");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_AUTH_RSP_CMD, p_req, sizeof(tBSA_FTC_AUTH_RSP));

}

/*******************************************************************************
 **
 ** Function         BSA_FtcCopyInit
 **
 ** Description      Initialize the tBSA_FTC_AUTH_RSP structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcCopyInit (tBSA_FTC_COPY *p_req)
{
    APPL_TRACE_API0("BSA_FtcAuthRspInit");
    memset(p_req, 0, sizeof(tBSA_FTC_COPY));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BTA_FtcCopy
**
** Description      Invoke a Copy action on the FT server.
**                  Create a copy of p_src and name it as p_dest
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_FtcCopy(tBSA_FTC_COPY *p_req)
{
    APPL_TRACE_API0("BTA_FtcCopy");
    return bsa_send_message_receive_status(BSA_FTC_MSGID_COPY_CMD, p_req, sizeof(tBSA_FTC_COPY));
}

/*******************************************************************************
 **
 ** Function         BSA_FtcMoveInit
 **
 ** Description      Initialize the tBSA_FTC_MOVE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcMoveInit(tBSA_FTC_MOVE *p_req)
{
    APPL_TRACE_API0("BSA_FtcMoveInit");
    memset(p_req, 0, sizeof(tBSA_FTC_MOVE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_FtcMove
**
** Description      Invoke a Move action on the server.
**                  Move/rename p_src to p_dest
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_FtcMove(tBSA_FTC_MOVE *p_req)
{
    APPL_TRACE_API0("BSA_FtcMove");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_MOVE_CMD, p_req, sizeof(tBSA_FTC_MOVE));

}

/*******************************************************************************
 **
 ** Function         BSA_FtcSetPermissionInit
 **
 ** Description      Initialize the tBSA_FTC_SET_PERMISION structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcSetPermissionInit(tBSA_FTC_SET_PERMISION *p_req)
{
    APPL_TRACE_API0("BSA_FtcAuthRspInit");
    memset(p_req, 0, sizeof(tBSA_FTC_SET_PERMISION));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_FtcSetPermission
**
** Description      Invoke a SetPermission action on the server.
**
** Returns          void
**
*******************************************************************************/
tBSA_STATUS BSA_FtcSetPermission(tBSA_FTC_SET_PERMISION *p_req)
{
    APPL_TRACE_API0("BSA_FtcSetPermission");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_SET_PERM_CMD, p_req, sizeof(tBSA_FTC_SET_PERMISION));

}

/*******************************************************************************
 **
 ** Function         BSA_FtcPutFileInit
 **
 ** Description      Initialize the tBSA_FTC_PUT structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcPutFileInit(tBSA_FTC_PUT *p_req)
{
    APPL_TRACE_API0("BSA_FtcPutFileInit");
    memset(p_req, 0, sizeof(tBSA_FTC_PUT));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_FtcPutFile
**
** Description      Send a file to the connected server.
**
**                  This function can only be used when the client is connected
**                  in FTP, OPP and BIP mode.
**
** Note:            File name is specified with a fully qualified path.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_FtcPutFile(tBSA_FTC_PUT *p_req)
{
    APPL_TRACE_API0("BSA_FtcPutFile");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_PUT_FILE_CMD, p_req, sizeof(tBSA_FTC_PUT));

}

/*******************************************************************************
 **
 ** Function         BSA_FtcGetFileInit
 **
 ** Description      Initialize the tBSA_FTC_GET structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcGetFileInit(tBSA_FTC_GET *p_req)
{
    APPL_TRACE_API0("BSA_FtcGetFileInit");
    memset(p_req, 0, sizeof(tBSA_FTC_GET));
    return BSA_SUCCESS;
}
/*******************************************************************************
**
** Function         BSA_FtcGetFile
**
** Description      Retrieve a file from the peer device and copy it to the
**                  local file system.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Note:            local file name is specified with a fully qualified path.
**                  Remote file name is specified in UTF-8 format.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_FtcGetFile(tBSA_FTC_GET *p_req)
{
    APPL_TRACE_API0("BSA_FtcGetFile");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_GET_FILE_CMD, p_req, sizeof(tBSA_FTC_GET));
}

/*******************************************************************************
 **
 ** Function         BSA_FtcChDirInit
 **
 ** Description      Initialize the tBSA_FTC_CH_DIR structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcChDirInit(tBSA_FTC_CH_DIR *p_req)
{
    APPL_TRACE_API0("BSA_FtcChDirInit");
    memset(p_req, 0, sizeof(tBSA_FTC_CH_DIR));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_FtcChDir
**
** Description      Change directory on the peer device.
**
**                  This function can only be used when the client is connected
**                  in FTP and PBAP mode.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_FtcChDir(tBSA_FTC_CH_DIR *p_req)
{
    APPL_TRACE_API0("BSA_FtcChDir");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_CH_DIR_CMD, p_req, sizeof(tBSA_FTC_CH_DIR));
}

/*******************************************************************************
 **
 ** Function         BSA_FtcListDirInit
 **
 ** Description      Initialize the tBSA_FTC_LIST_DIR structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcListDirInit(tBSA_FTC_LIST_DIR *p_req)
{
    APPL_TRACE_API0("BSA_FtcListDirInit");
    memset(p_req, 0, sizeof(tBSA_FTC_LIST_DIR));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_FtcListDir
**
** Description      Retrieve a directory listing from the peer device.
**                  When the operation is complete the callback function will
**                  be called with one or more BSA_FTC_LIST_EVT events
**                  containing directory list information formatted as described
**                  in the IrOBEX Spec, Version 1.2, section 9.1.2.3.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Parameters       p_dir - Name of directory to retrieve listing of.  If NULL,
**                          the current working directory is retrieved.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_FtcListDir(tBSA_FTC_LIST_DIR *p_req)
{
    APPL_TRACE_API0("BSA_FtcListDir");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_LIST_DIR_CMD, p_req, sizeof(tBSA_FTC_LIST_DIR));
}

/*******************************************************************************
 **
 ** Function         BSA_FtcRemoveInit
 **
 ** Description      Initialize the tBSA_FTC_REMOVE structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcRemoveInit(tBSA_FTC_REMOVE *p_req)
{
    APPL_TRACE_API0("BSA_FtcRemoveInit");
    memset(p_req, 0, sizeof(tBSA_FTC_REMOVE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_FtcRemove
**
** Description      Remove a file or directory on the peer device.  When the
**                  operation is complete the status is returned with the
**                  BSA_FTC_REMOVE_EVT event.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_FtcRemove(tBSA_FTC_REMOVE *p_req)
{
    APPL_TRACE_API0("BSA_FtcRemove");

    return bsa_send_message_receive_status(BSA_FTC_MSGID_REMOVE_CMD, p_req, sizeof(tBSA_FTC_REMOVE));
}

/*******************************************************************************
 **
 ** Function         BSA_FtcMkDirInit
 **
 ** Description      Initialize the tBSA_FTC_MK_DIR structure to default values.
 **
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_FtcMkDirInit(tBSA_FTC_MK_DIR *p_req)
{
    APPL_TRACE_API0("BSA_FtcMkDirInit");
    memset(p_req, 0, sizeof(tBSA_FTC_MK_DIR));
    return BSA_SUCCESS;
}
/*******************************************************************************
**
** Function         BSA_FtcMkDir
**
** Description      Create a directory on the peer device. When the operation is
**                  complete the status is returned with the BSA_FTC_MKDIR_EVT
**                  event.
**
**                  This function can only be used when the client is connected
**                  in FTP mode.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_FtcMkDir(tBSA_FTC_MK_DIR *p_req)
{
    APPL_TRACE_API0("BSA_FtcMkDir");

    if(p_req && strlen(p_req->dir))
        return bsa_send_message_receive_status(BSA_FTC_MSGID_MK_DIR_CMD, p_req, sizeof(tBSA_FTC_MK_DIR));
    else
        return BSA_ERROR_CLI_BAD_PARAM;
}




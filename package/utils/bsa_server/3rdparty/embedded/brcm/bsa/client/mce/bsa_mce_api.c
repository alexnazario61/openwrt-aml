/*****************************************************************************
**
**  Name:           bsa_mce_api.c
**
**  Description:    This is the public interface file for MAP client part of
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
tBSA_MCE_CB bsa_mce_cb =
{
    NULL
};

/*******************************************************************************
**
** Function         BSA_MceEnableInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceEnableInit(tBSA_MCE_ENABLE *p_enable)
{
    APPL_TRACE_API0("BSA_MceEnableInit");

    if (p_enable == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_enable, 0, sizeof(tBSA_MCE_ENABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceEnable
**
** Description      This function enables MAP client and registers MAP with
**                  lower layers.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceEnable(tBSA_MCE_ENABLE *p_enable)
{
    tBSA_MCE_MSGID_ENABLE_CMD_REQ mce_enable_req;

    APPL_TRACE_API0("BSA_MceEnable");

    if (p_enable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceEnable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_enable->p_cback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceEnable NULL Callback");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(&mce_enable_req, 0, sizeof(mce_enable_req));

    /* Save application's Security callback */
    bsa_mce_cb.p_app_mce_cback = p_enable->p_cback;

    /* Prepare request parameters */
    mce_enable_req.app_id = BSA_MCE_APP_ID;

    return bsa_send_message_receive_status(BSA_MCE_MSGID_ENABLE_CMD,
        &mce_enable_req, sizeof(tBSA_MCE_MSGID_ENABLE_CMD_REQ));

}

/*******************************************************************************
**
** Function         BSA_MceDisableInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceDisableInit(tBSA_MCE_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_MceDisableInit");

    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceDisableInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_disable, 0, sizeof(tBSA_MCE_DISABLE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceDisable
**
** Description      This function is called when the host is about power down.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceDisable(tBSA_MCE_DISABLE *p_disable)
{
    APPL_TRACE_API0("BSA_MceDisable");
    if (p_disable == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceDisable param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_DISABLE_CMD, p_disable,
        sizeof(tBSA_MCE_DISABLE));
}

/*******************************************************************************
**
** Function         BSA_MceOpenInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceOpenInit(tBSA_MCE_OPEN *p_open)
{
    APPL_TRACE_API0("BSA_MceOpenInit");

    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_open, 0, sizeof(tBSA_MCE_OPEN));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceOpen
**
** Description      Open a connection to an MAP client
**
**                  When the connection is open the callback function
**                  will be called with a BSA_MCE_OPEN_EVT.  If the connection
**                  fails or otherwise is closed the callback function will be
**                  called with a BSA_MCE_CLOSE_EVT.
**
**                  Note: Mce always enable (BSA_SEC_AUTHENTICATE | BSA_SEC_ENCRYPT)
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceOpen(tBSA_MCE_OPEN *p_open)
{
    BD_ADDR zeroBDA;
    memset(zeroBDA, 0, sizeof(BD_ADDR));

    APPL_TRACE_API0("BSA_MceOpen");
    if (p_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }


    if (p_open->sec_mask == BSA_SEC_NONE)
    {
        APPL_TRACE_ERROR0("BSA_MceOpen Invalid security setting");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (memcmp(zeroBDA, p_open->bd_addr, sizeof(BD_ADDR)) == 0)
    {
        APPL_TRACE_ERROR0("BSA_MceOpen Invalid BD_ADDR");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_OPEN_CMD, p_open,
        sizeof(tBSA_MCE_OPEN));
}

/*******************************************************************************
**
** Function         BSA_MceCloseInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceCloseInit(tBSA_MCE_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_MceCloseInit");

    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_close, 0, sizeof(tBSA_MCE_CLOSE));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceClose
**
** Description      Close the current connection to the MAP server. Aborts any
**                  active MAP transfer.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceClose(tBSA_MCE_CLOSE *p_close)
{
    APPL_TRACE_API0("BSA_MceClose");
    if (p_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_CLOSE_CMD, p_close,
        sizeof(tBSA_MCE_CLOSE));
}

/*******************************************************************************
**
** Function         BSA_MceAbortInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceAbortInit(tBSA_MCE_ABORT *p_abort)
{
    APPL_TRACE_API0("BSA_MceAbortInit");

    if (p_abort == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceAbortInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_abort, 0, sizeof(tBSA_MCE_ABORT));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceAbort
**
** Description      Aborts any active MCE operation.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceAbort(tBSA_MCE_ABORT *p_abort)
{
    APPL_TRACE_API0("BSA_MceAbort");
    if (p_abort == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceAbort param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_ABORT_CMD, p_abort,
        sizeof(tBSA_MCE_ABORT));
}

/*******************************************************************************
**
** Function         BSA_MceMnStartInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceMnStartInit(tBSA_MCE_MN_START *p_mn_start)
{
    APPL_TRACE_API0("BSA_MceMnStartInit");

    if (p_mn_start == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceMnStartInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_mn_start, 0, sizeof(tBSA_MCE_MN_START));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceMnStart
**
** Description      Start the Message Notification service server.
**                  When the Start operation is complete the callback function
**                  will be called with an BSA_MCE_START_EVT event.
**                  Note: Mas always enable (BSA_SEC_AUTHENTICATE | BSA_SEC_ENCRYPT)
**
** Parameters       p_mn_start - Pointer to structure containing API parameters**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceMnStart(tBSA_MCE_MN_START *p_mn_start)
{
    APPL_TRACE_API0("BSA_MceMnStart");
    if (p_mn_start == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceMnStart param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_MN_START_CMD, p_mn_start,
        sizeof(tBSA_MCE_MN_START));
}


/*******************************************************************************
**
** Function         BSA_MceMnStopInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceMnStopInit(tBSA_MCE_MN_STOP *p_mn_stop)
{
    APPL_TRACE_API0("BSA_MceMnStopInit");

    if (p_mn_stop == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceMnStopInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_mn_stop, 0, sizeof(tBSA_MCE_MN_STOP));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceMnStop
**
** Description      Stop the Message Notification service server.  If the server is currently
**                  connected to a peer device the connection will be closed.
**
** Parameter        p_mn_stop: Pointer to structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceMnStop(tBSA_MCE_MN_STOP *p_mn_stop)
{
    APPL_TRACE_API0("BSA_MceMnStop");
    if (p_mn_stop == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceMnStop param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_MN_STOP_CMD, p_mn_stop,
        sizeof(tBSA_MCE_MN_STOP));
}


/*******************************************************************************
**
** Function         BSA_MceNotifRegInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceNotifRegInit(tBSA_MCE_NOTIFYREG *p_notifyreg)
{
    APPL_TRACE_API0("BSA_MceNotifRegInit");

    if (p_notifyreg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceNotifRegInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_notifyreg, 0, sizeof(tBSA_MCE_NOTIFYREG));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceNotifReg
**
** Description      Set the Message Notification status to On or OFF on the MSE.
**                  When notification is registered, message notification service
**                  must be enabled by calling API BSA_MceMnStart().
**
** Parameter        p_notifyreg - A pointer to the structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceNotifReg(tBSA_MCE_NOTIFYREG *p_notifyreg)
{
    APPL_TRACE_API0("BSA_MceNotifReg");
    if (p_notifyreg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceNotifReg param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_NOTIFY_REG_CMD, p_notifyreg,
        sizeof(tBSA_MCE_NOTIFYREG));
}


/*******************************************************************************
**
** Function         BSA_MceUpdateInboxInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceUpdateInboxInit(tBSA_MCE_UPDATEINBOX *p_updateinbox)
{
    APPL_TRACE_API0("BSA_MceUpdateInboxInit");

    if (p_updateinbox == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceUpdateInboxInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_updateinbox, 0, sizeof(tBSA_MCE_UPDATEINBOX));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceUpdateInbox
**
** Description      This function is used to update the inbox for the
**                  specified MAS session.
**
** Parameter        p_notifyreg - A pointer to the structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceUpdateInbox(tBSA_MCE_UPDATEINBOX *p_updateinbox)
{
    APPL_TRACE_API0("BSA_MceUpdateInbox");
    if (p_updateinbox == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceUpdateInbox param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_UPDATE_INBOX_CMD, p_updateinbox,
        sizeof(tBSA_MCE_UPDATEINBOX));
}

/*******************************************************************************
**
** Function         BSA_McePushMsgInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_McePushMsgInit(tBSA_MCE_PUSHMSG *p_pushmsg)
{
    APPL_TRACE_API0("BSA_McePushMsgInit");

    if (p_pushmsg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_McePushMsgInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_pushmsg, 0, sizeof(tBSA_MCE_PUSHMSG));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_McePushMsg

**
** Description      This function is used to upload a message to the specified folder in MAP server
**
** Parameter        p_pushmsg - A pointer to the structure containing API parameters
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_McePushMsg(tBSA_MCE_PUSHMSG *p_pushmsg)
{
    APPL_TRACE_API0("BSA_McePushMsg");
    if (p_pushmsg == NULL)
    {
        APPL_TRACE_ERROR0("BSA_McePushMsg param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_PUSH_MSG_CMD, p_pushmsg,
        sizeof(tBSA_MCE_PUSHMSG));
}

/*******************************************************************************
**
** Function         BSA_MceGetInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceGetInit(tBSA_MCE_GET *p_get)
{
    APPL_TRACE_API0("BSA_MceGetInit");
    if (p_get == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceGetInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_get, 0, sizeof(tBSA_MCE_GET));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceGet
**
** Description      This function is called to for all Get operations
**                  GetCard, GetList, GetPhoneBook etc.
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceGet(tBSA_MCE_GET *p_get)
{
    APPL_TRACE_API0("BSA_MceGet");
    if (p_get == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceGet param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_GET_CMD, p_get,
        sizeof(tBSA_MCE_GET));
}

/*******************************************************************************
**
** Function         BSA_MceSetInit
**
** Description      Initialize structure containing API parameters with default values
**
** Parameters       Pointer on structure containing API parameters
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceSetInit(tBSA_MCE_SET *p_set)
{
    APPL_TRACE_API0("BSA_MceSetInit");
    if (p_set == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceSetInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    memset(p_set, 0, sizeof(tBSA_MCE_SET));
    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceSet
**
** Description      Performs a Set Operation based on the specified set type and parameters in the tBSA_MCE_SET structure.
**                  Following set operations are supported: 1) Set message status 2) Set folder
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceSet(tBSA_MCE_SET *p_set)
{
    APPL_TRACE_API0("BSA_MceSet");
    if (p_set == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceSet param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    return bsa_send_message_receive_status(BSA_MCE_MSGID_SET_CMD, p_set,
        sizeof(tBSA_MCE_SET));
}

/*******************************************************************************
 **
 ** Function         BSA_MceCancelInit
 **
 ** Description      Initialize structure containing API parameters with default values
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
tBSA_STATUS BSA_MceCancelInit(tBSA_MCE_CANCEL *pCancel)
{
    APPL_TRACE_API0("BSA_MceCancelInit");

    if (pCancel == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceCancelInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(pCancel, 0, sizeof(tBSA_MCE_CANCEL));


    return BSA_SUCCESS;
}

/*******************************************************************************
**
** Function         BSA_MceCancel
**
** Description      This function is called to cancel connection
**
** Returns          tBSA_STATUS
**
*******************************************************************************/
tBSA_STATUS BSA_MceCancel (tBSA_MCE_CANCEL    *pCancel)
{
    APPL_TRACE_API0("BSA_MceCancel");
    if (pCancel == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MceCancel param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_MCE_MSGID_CANCEL_CMD, pCancel, sizeof(tBSA_MCE_CANCEL));
}

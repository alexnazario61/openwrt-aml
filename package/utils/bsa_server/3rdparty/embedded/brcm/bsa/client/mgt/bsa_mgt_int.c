/*****************************************************************************
 **
 **  Name:           bsa_mgt_int.c
 **
 **  Description:    client Management internal function for BSA
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_client.h"

#include "bsa_cl_mgt_int.h"

#if (BSA_AV_INCLUDED == TRUE)
#include "bsa_cl_av_int.h"
#endif

#if (BSA_HL_INCLUDED == TRUE)
#include "bsa_cl_hl_int.h"
#endif

#if (BSA_TM_INCLUDED == TRUE)
#include "bsa_cl_tm_int.h"
#endif

#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
#include "bsa_ble_int.h"
#endif

#include "bsa_cl_disc_int.h"
#include "bsa_cl_dm_int.h"

/*#define BSA_MGT_INT_DEBUG*/

/* Default TimeOut (in OS ticks) to wait for an return value from the server */
#ifndef BSA_MGT_INT_SEND_RCV_TO
#define BSA_MGT_INT_SEND_RCV_TO GKI_MS_TO_TICKS(6000)
#endif

#ifndef BSA_MGT_SND_RCV_RETRY
#define BSA_MGT_SND_RCV_RETRY 10
#endif

/*
 * Global variables
 */
static int message_id_tab[GKI_MAX_TASKS];

#if !defined(BSA_UNIFIED_CLIENT_SERVER) || (BSA_UNIFIED_CLIENT_SERVER == FALSE)
/* Added appl_trace_level here to be able compile client side */
UINT8 appl_trace_level = APPL_INITIAL_TRACE_LEVEL;
#endif

/*
 * Local functions
 */
static int bsa_add_msg_id_tid(UINT8 task_id, int message_id);
static int bsa_del_msg_id_tid(UINT8 task_id);
static int bsa_get_msg_id_from_tid(UINT8 task_id, int *message_id);
static int bsa_get_tid_from_msg_id(UINT8 *task_id, int message_id);


/*******************************************************************************
 **
 ** Function        bsa_cl_mgt_init
 **
 ** Description     Initialize all the Client data
 **
 ** Parameters      none
 **
 ** Returns         Status
 **
 *******************************************************************************/
tBSA_STATUS bsa_cl_mgt_init(void)
{
    APPL_TRACE_DEBUG0("bsa_cl_mgt_init");

    /* MGT initialization */
    memset(message_id_tab, 0, sizeof(message_id_tab));

    /* Discovery initialization */
    bsa_cl_disc_init();

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         bsa_mgt_disc_cback
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void bsa_mgt_disc_hdlr(BT_HDR *p_msg)
{
    tBSA_MGT_MSG disc_mgt_param;
    int i;
    int message_id;
    BT_HDR *disc_msg;

    APPL_TRACE_EVENT0("bsa_mgt_disc_hdlr");

    bsa_cl_mgt_cb.bsa_connected_to_server = FALSE; /* We are not anymore connected to server */

    /* Prepare parameter for user callback */
    disc_mgt_param.disconnect.reason = BSA_ERROR_CLI_INTERNAL;

    /* For every Task */
    for (i = 0 ; i < GKI_MAX_TASKS ; i++)
    {
        /* Check if this task is waiting for an answer from the Server */
        if (bsa_get_msg_id_from_tid(i, &message_id) == 0)
        {
            /* message_id: 0 does not exit => this task was not waiting */
            if (message_id != 0)
            {
                APPL_TRACE_DEBUG1("Send a disconnect message to task_id:%d to unblock it", i);
                APPL_TRACE_DEBUG2("This task was waiting for msg_id:%d (0x%x)", message_id, message_id);
                if ((disc_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR))) != NULL)
                {
                    disc_msg->offset = 0;
                    disc_msg->event = UIPC_CLOSE_EVT;
                    disc_msg->len = 0;
                    bsa_del_msg_id_tid(i);
                    GKI_send_msg(i, TASK_MBOX_0, disc_msg);
                }
                else
                {
                    APPL_TRACE_ERROR1("No GKI buffer to send a disconnect message to task_id:%d", i);
                }
            }
            else
            {
                APPL_TRACE_DEBUG1("Task_id:%d is not waiting for msg", i);
            }
        }
    }
    /* For all UIPCs that could be opened by this client, close them */
    for (i = UIPC_FIRST_BSA_CH_ID; i <= UIPC_LAST_BSA_CH_ID; i++)
    {
        APPL_TRACE_DEBUG1("UIPC_%d: closing", i);
        UIPC_Close(i);
    }
    /* Mark the connection to server closed before invoking callback */
    bsa_cl_mgt_cb.bsa_connected_to_server = FALSE;

    /* Call user cback with param */
    if (bsa_cl_mgt_cb.bsa_mgt_callback != NULL)
    {
        bsa_cl_mgt_cb.bsa_mgt_callback(BSA_MGT_DISCONNECT_EVT, /* Disconnection event */
                &disc_mgt_param); /* parameter */
        bsa_cl_mgt_cb.bsa_mgt_callback = NULL;
    }
    else
    {
        APPL_TRACE_ERROR0("No application's callback");
    }
}

/*******************************************************************************
 **
 ** Function         bsa_mgt_async_rx_hdlr
 **
 ** Description      Handler called when an asynchronous event is received.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void bsa_mgt_async_rx_hdlr(BT_HDR *p_msg)
{
    int message_id = p_msg->layer_specific;
    int length = p_msg->len;
    UINT8 *p_buffer = (UINT8 *)(p_msg + 1);

#ifdef BSA_MGT_INT_DEBUG
    APPL_TRACE_DEBUG2("bsa_mgt_async_rx_hdlr 0x%x =>%d", message_id, message_id
            & ~BSA_ASYNC_EVENT_BIT);
#endif

    /* Remove Async Bit */
    message_id &= ~BSA_ASYNC_EVENT_BIT;

    if ((message_id > BSA_MGT_MSGID_LAST_CMD) && (message_id
            <= BSA_MGT_MSGID_LAST_EVT))
    {
        bsa_mgt_event_hdlr(message_id, (tBSA_MGT_MSG *) p_buffer, length);
    }
    else if ((message_id > BSA_SEC_MSGID_LAST_CMD) && (message_id
            <= BSA_SEC_MSGID_LAST_EVT))
    {
        bsa_sec_event_hdlr(message_id, (tBSA_SEC_MSG *) p_buffer, length);
    }
    else if ((message_id > BSA_DISC_MSGID_LAST_CMD) && (message_id
            <= BSA_DISC_MSGID_LAST_EVT))
    {
        bsa_disc_event_hdlr(message_id, (tBSA_DISC_MSG *) p_buffer, length);
    }
#if (BSA_3DS_INCLUDED == TRUE)
    else if ((message_id >  BSA_DM_MSGID_LAST_CMD) &&
             (message_id <= BSA_DM_MSGID_LAST_EVT))
    {
        bsa_cl_dm_event_hdlr(message_id, (tBSA_DM_MSG *)p_buffer, length);
    }
#endif
#if (BSA_FTS_INCLUDED == TRUE)
    else if ((message_id > BSA_FTS_MSGID_LAST_CMD) && (message_id
            <= BSA_FTS_MSGID_LAST_EVT))
    {
        bsa_fts_event_hdlr(message_id, (tBSA_FTS_MSG *) p_buffer, length);
    }
#endif
#if (BSA_FTC_INCLUDED == TRUE)
    else if ((message_id > BSA_FTC_MSGID_LAST_CMD) && (message_id
            <= BSA_FTC_MSGID_LAST_EVT))
    {
        bsa_ftc_event_hdlr(message_id, (tBSA_FTC_MSG *) p_buffer, length);
    }
#endif
#if (BSA_HH_INCLUDED == TRUE)
    else if ((message_id > BSA_HH_MSGID_LAST_CMD) &&
            (message_id <= BSA_HH_MSGID_LAST_EVT))
    {
        bsa_hh_event_hdlr(message_id, (tBSA_HH_MSG *)p_buffer, length);
    }
#endif
#if (BSA_HD_INCLUDED == TRUE)
    else if ((message_id > BSA_HD_MSGID_LAST_CMD) &&
            (message_id <= BSA_HD_MSGID_LAST_EVT))
    {
        bsa_hd_event_hdlr(message_id, (tBSA_HD_MSG *)p_buffer, length);
    }
#endif
#if (BSA_MCE_INCLUDED == TRUE)
    else if ((message_id >  BSA_MCE_MSGID_LAST_CMD) && (message_id
            <= BSA_MCE_MSGID_LAST_EVT))
    {
        bsa_mce_event_hdlr(message_id, (tBSA_MCE_MSG *)p_buffer, length);
    }
#endif
#if (BSA_PBS_INCLUDED == TRUE)
    else if ((message_id >  BSA_PBS_MSGID_LAST_CMD) && (message_id
            <= BSA_PBS_MSGID_LAST_EVT))
    {
        bsa_pbs_event_hdlr(message_id, (tBSA_PBS_MSG *)p_buffer, length);
    }
#endif
#if (BSA_PBC_INCLUDED == TRUE)
    else if ((message_id >  BSA_PBC_MSGID_LAST_CMD) && (message_id
            <= BSA_PBC_MSGID_LAST_EVT))
    {
        bsa_pbc_event_hdlr(message_id, (tBSA_PBC_MSG *)p_buffer, length);
    }
#endif
#if (BSA_SC_INCLUDED == TRUE)
    else if ((message_id >  BSA_SC_MSGID_LAST_CMD) && (message_id
            <= BSA_SC_MSGID_LAST_EVT))
    {
        bsa_sc_event_hdlr(message_id, (tBSA_SC_MSG *)p_buffer, length);
    }
#endif
#if (BSA_SAC_INCLUDED == TRUE)
    else if ((message_id >  BSA_SAC_MSGID_LAST_CMD) && (message_id
            <= BSA_SAC_MSGID_LAST_EVT))
    {
        bsa_sac_event_hdlr(message_id, (tBSA_SAC_MSG *)p_buffer, length);
    }
#endif
#if (BSA_OPS_INCLUDED == TRUE)
    else if ((message_id > BSA_OPS_MSGID_LAST_CMD) &&
            (message_id <= BSA_OPS_MSGID_LAST_EVT))
    {
        bsa_ops_event_hdlr(message_id, (tBSA_OPS_MSG *)p_buffer, length);
    }
#endif
#if (BSA_OPC_INCLUDED == TRUE)
    else if ((message_id > BSA_OPC_MSGID_LAST_CMD) &&
            (message_id <= BSA_OPC_MSGID_LAST_EVT))
    {
        bsa_opc_event_hdlr(message_id, (tBSA_OPC_MSG *)p_buffer, length);
    }
#endif
#if (BSA_AV_INCLUDED == TRUE)
    else if ((message_id >  BSA_AV_MSGID_LAST_CMD) &&
             (message_id <= BSA_AV_MSGID_LAST_EVT))
    {
        bsa_cl_av_event_hdlr(message_id, (tBSA_AV_MSG *)p_buffer, length);
    }
#endif
#if (BSA_AVK_INCLUDED == TRUE)
    else if ((message_id >  BSA_AVK_MSGID_LAST_CMD) &&
             (message_id <= BSA_AVK_MSGID_LAST_EVT))
    {
        bsa_avk_event_hdlr(message_id, (tBSA_AVK_MSG *)p_buffer, length);
    }
#endif
#if (BSA_HS_INCLUDED == TRUE)
    else if ((message_id >  BSA_HS_MSGID_LAST_CMD) &&
             (message_id <= BSA_HS_MSGID_LAST_EVT))
    {
        bsa_hs_event_hdlr(message_id, (tBSA_HS_MSG *)p_buffer, length);
    }
#endif
#if (BSA_AG_INCLUDED == TRUE)
    else if ((message_id >  BSA_AG_MSGID_LAST_CMD) &&
             (message_id <= BSA_AG_MSGID_LAST_EVT))
    {
        bsa_ag_event_hdlr(message_id, (tBSA_AG_MSG *)p_buffer, length);
    }
#endif
#if (BSA_DG_INCLUDED == TRUE)
    else if ((message_id >  BSA_DG_MSGID_LAST_CMD) &&
             (message_id <= BSA_DG_MSGID_LAST_EVT))
    {
        bsa_dg_event_hdlr(message_id, (tBSA_DG_MSG *)p_buffer, length);
    }
#endif
#if (BSA_PAN_INCLUDED == TRUE)
    else if ((message_id >  BSA_PAN_MSGID_LAST_CMD) &&
             (message_id <= BSA_PAN_MSGID_LAST_EVT))
    {
        bsa_pan_event_hdlr(message_id, (tBSA_PAN_MSG *)p_buffer, length);
    }
#endif
#if (BSA_HL_INCLUDED == TRUE)
    else if ((message_id >  BSA_HL_MSGID_LAST_CMD) &&
             (message_id <= BSA_HL_MSGID_LAST_EVT))
    {
        bsa_cl_hl_event_hdlr(message_id, (tBSA_HL_MSGID_EVT *)p_buffer, length);
    }
#endif
#if (BSA_TM_INCLUDED == TRUE)
    else if ((message_id >  BSA_TM_MSGID_LAST_CMD) &&
             (message_id <= BSA_TM_MSGID_LAST_EVT))
    {
        bsa_cl_tm_event_hdlr(message_id, (tBSA_TM_MSG *)p_buffer, length);
    }
#endif
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    else if ((message_id >  BSA_BLE_MSGID_LAST_CMD) &&
             (message_id <= BSA_BLE_MSGID_LAST_EVT))
    {
        bsa_ble_event_hdlr(message_id, (tBSA_BLE_MSG *)p_buffer, length);
    }
#endif

    else
    {
        APPL_TRACE_ERROR2("bsa_mgt_async_rx_hdlr no handler for this message 0x%x =>%d",
                message_id, message_id & ~BSA_ASYNC_EVENT_BIT);
    }
}

/*******************************************************************************
 **
 ** Function         bsa_callback_task
 **
 ** Description      This callback receives asynchronous events from server
 **                  (after filtering in bsa_mgt_cback).
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void bsa_callback_task(UINT32 param)
{
    UINT32 event;
    BT_HDR *p_msg;

    /* Wait for asynchronous events from UIPC */
    while (TRUE)
    {
        event = GKI_wait(TASK_MBOX_1_EVT_MASK, 0);
#ifdef BSA_MGT_INT_DEBUG
        APPL_TRACE_DEBUG1("bsa_callback_task Event 0x%x", event);
#endif
        /* Messages in MBox 1 are Asynchronous messages */
        if (event & TASK_MBOX_1_EVT_MASK)
        {
            while ((p_msg = GKI_read_mbox(TASK_MBOX_1)) != NULL)
            {
                /* Asynchronous message */
                if (p_msg->event == UIPC_RX_DATA_EVT)
                {
#ifdef BSA_MGT_INT_DEBUG
                    APPL_TRACE_DEBUG0("bsa_callback_task UIPC_RX_DATA_EVT event received");
#endif
                    /* Calls the callback with the parameters */
                    bsa_mgt_async_rx_hdlr(p_msg);
                }
                else if (p_msg->event == UIPC_CLOSE_EVT)
                {
#ifdef BSA_MGT_INT_DEBUG
                    APPL_TRACE_DEBUG0("bsa_callback_task UIPC_CLOSE_EVT event received");
#endif
                    bsa_mgt_disc_hdlr(p_msg);
                }
                else
                {
                    APPL_TRACE_ERROR1("bsa_callback_task Unknown event received:%d",
                            p_msg->event);
                }
#ifdef BSA_MGT_INT_DEBUG
                    APPL_TRACE_DEBUG0("bsa_callback_task free received buffer");
#endif
                GKI_freebuf(p_msg);
            }
        }
    }
}

/*******************************************************************************
 **
 ** Function         bsa_mgt_cback
 **
 ** Description      This callback is invoked upon reception of message from BSA server
 **                  UIPC.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void bsa_mgt_cback(BT_HDR *p_msg)
{
    UINT8 task_id;
    int status;

#ifdef BSA_MGT_INT_DEBUG
    if (p_msg->layer_specific & BSA_FUNCTION_RETURN_BIT)
    {
            APPL_TRACE_DEBUG4("bsa_mgt_cback event:%d msg_id:0x%x (FCTR:%d) len:%d",
                    p_msg->event,
                    p_msg->layer_specific,
                    p_msg->layer_specific & ~(BSA_FUNCTION_RETURN_BIT | BSA_ASYNC_EVENT_BIT),
                    p_msg->len);
    }
    else if (p_msg->layer_specific & BSA_ASYNC_EVENT_BIT)
    {
        APPL_TRACE_DEBUG4("bsa_mgt_cback event:%d msg_id:0x%x (ASYNC:%d) len:%d",
                p_msg->event,
                p_msg->layer_specific,
                p_msg->layer_specific & ~(BSA_FUNCTION_RETURN_BIT | BSA_ASYNC_EVENT_BIT),
                p_msg->len);
    }
#endif

    /* If message has been received from Server */
    if (p_msg->event == UIPC_RX_DATA_EVT)
    {
        /* Check if it's an asynchronous message */
        if (p_msg->layer_specific & BSA_ASYNC_EVENT_BIT)
        {
            /* Send message to Callback task */
#ifdef BSA_MGT_INT_DEBUG
            APPL_TRACE_DEBUG0("bsa_mgt_cback send message to CBACK task");
#endif
            GKI_send_msg(CBACK_TASK, TASK_MBOX_1, p_msg);
        }
        else if (p_msg->layer_specific & BSA_FUNCTION_RETURN_BIT)
        {
            /* Get the task_id of the task which wait for the answer */
            status = bsa_get_tid_from_msg_id(&task_id, p_msg->layer_specific
                    & ~BSA_FUNCTION_RETURN_BIT);
            /* Send message to the task which sent the message to the server */
            /* the message expected has been received, suppress the entry in the table */
            if (status == 0)
            {
#ifdef BSA_MGT_INT_DEBUG
            APPL_TRACE_DEBUG2("bsa_mgt_cback send message to task_id %d task len:%d",
                    task_id, p_msg->len);
#endif
                bsa_del_msg_id_tid(task_id);
                GKI_send_msg(task_id, TASK_MBOX_0, p_msg);
            }
            else
            {
                APPL_TRACE_ERROR1("bsa_mgt_cback dropped unexpected message:%x",
                        p_msg->layer_specific);
                GKI_freebuf(p_msg);
            }
        }
        else
        {
            APPL_TRACE_ERROR1("bsa_mgt_cback bad msg_id:%x type received from UIPC",
                    p_msg->layer_specific);
            GKI_freebuf(p_msg);
        }
    }
    /* else, if it's a server disconnection indication */
    else if (p_msg->event == UIPC_CLOSE_EVT)
    {
        APPL_TRACE_DEBUG0("bsa_mgt_cback disconnected from server");
        /* Send message to Callback task */
        GKI_send_msg(CBACK_TASK, TASK_MBOX_1, p_msg);
    }
    /* else, if it's a server connection indication */
    else if (p_msg->event == UIPC_OPEN_EVT)
    {
        APPL_TRACE_DEBUG0("bsa_mgt_cback connected to server");
        GKI_freebuf(p_msg);
    }
    else
    {
        APPL_TRACE_ERROR1("bsa_mgt_cback bad event:%d received from UIPC",
                 p_msg->event);
        GKI_freebuf(p_msg);
    }
}


/*******************************************************************************
 **
 ** Function         bsa_send_message_receive_status
 **
 ** Description      This function send a message and receive the status response from
 **                  Server application
 **
 ** Parameters       tx_buffer: Pointer to buffer to send
 **                  tx_lngth: length of buffer
 **
 ** Returns          BSA_SUCCESS if ok,  client or server error code if failure
 **
 *******************************************************************************/
tBSA_STATUS bsa_send_message_receive_status(int message_id, void *tx_buffer,
        int tx_length)
{
    tBSA_STATUS status;
    tBSA_STATUS ret_code;

    ret_code = bsa_send_receive_message(message_id, tx_buffer, tx_length, &status,
                sizeof(status));
    if(ret_code != BSA_SUCCESS)
    {
        /* We failed to send the message to the BSA server, return the client error code */
        return ret_code;
    }

    /* Send back received status */
    return status;

}


/*******************************************************************************
 **
 ** Function         bsa_send_receive_message
 **
 ** Description      This function send a message and receive the response to/from
 **                  Server application
 **
 ** Parameters       tx_buffer: Pointer to buffer to send
 **                  tx_lngth: length of buffer
 **                  rx_buffer: Pointer to buffer for reception
 **                  rx_length: length of buffer for reception
 **
 ** Returns          BSA_SUCCESS if ok, client error code if failure
 **
 *******************************************************************************/
tBSA_STATUS bsa_send_receive_message(int message_id, void *tx_buffer,
        int tx_length, void *rx_buffer, int rx_length)
{
    tBSA_STATUS status;
    BOOLEAN bstatus;
    UINT32 event;
    BT_HDR *p_msg;
    UINT8 task_id;
    int msg_id_tmp;
    int retry_count = 0;

#ifdef BSA_MGT_INT_DEBUG
    APPL_TRACE_API2("bsa_send_receive_message msgid:%d (0x%x)", message_id, message_id);
#endif

    if (bsa_cl_mgt_cb.bsa_connected_to_server == FALSE)
    {
        APPL_TRACE_ERROR0("ERROR Not connected to server");
        return BSA_ERROR_CLI_NOT_CONNECTED;
    }

    if (tx_buffer == NULL)
    {
        APPL_TRACE_ERROR0("ERROR bsa_send_receive_message NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Get the Task Id of the caller (the function return message will be sent to this task) */
    task_id = GKI_get_taskid();

    /* check if the task_id of the caller exists in GKI */
    if (task_id >= GKI_MAX_TASKS)
    {
        /* Let's create a temporary task (to be destroyed after reception of answer) */
        GKI_create_task(NULL, GKI_MAX_TASKS, (INT8 *)"TMP Task", 0, 0);
        task_id = GKI_get_taskid();
        if (task_id >= GKI_MAX_TASKS)
        {
            APPL_TRACE_ERROR0("bsa_send_receive_message unable to create temporary task!!");
            return BSA_ERROR_CLI_INTERNAL;
        }
    }

    /* check if this task is already waiting for a message_id (it should not) */
    bsa_get_msg_id_from_tid(task_id, &msg_id_tmp);
    if (msg_id_tmp != 0)
    {
        /* This should not append because the caller will be blocked on GKI_Wait */
        APPL_TRACE_ERROR0("bsa_send_receive_message fail because this task already wait for a msg");

        /* If this is a temporary task => destroy it */
        if (task_id > APP_TASK)
        {
            GKI_exit_task(task_id);
        }
        return BSA_ERROR_CLI_INTERNAL;
    }

    /* Add the message_id in the table (to be able to retrieve tid from msg_id later) */
    bsa_add_msg_id_tid(task_id, message_id);

    /* Send the message to the server */
    bstatus = UIPC_Send(UIPC_CH_ID_CTL, message_id, tx_buffer, tx_length);
    if (bstatus != TRUE)
    {
        APPL_TRACE_ERROR0("bsa_send_receive_message UIPC_Send fail");

        /* This task does not wait anymore for a msg_id */
        bsa_del_msg_id_tid(task_id);

        /* If this is a temporary task => destroy it */
        if (task_id > APP_TASK)
        {
            GKI_exit_task(task_id);
        }
        return BSA_ERROR_CLI_UIPC_SEND;
    }

    /*
     * Now, let's wait for the event sent by UIPC call back which indicates that
     * the answer has been received from the server.
     * We use a timeout in order not to block the caller forever
     */
    status = BSA_ERROR_CLI_BAD_MSG;
    event = GKI_wait(TASK_MBOX_0_EVT_MASK, BSA_MGT_INT_SEND_RCV_TO);

#ifdef BSA_MGT_INT_DEBUG
    APPL_TRACE_API1("bsa_send_receive_message() - GKI_Wait exits with event:%d", event);
#endif

    /* Messages in MBox 0 come from UIPC */
    if (event & TASK_MBOX_0_EVT_MASK)
    {
        /* Read associated message (read several times to avoid spurious events) */
        while (((p_msg = GKI_read_mbox(TASK_MBOX_0)) == NULL)&&(retry_count++ < BSA_MGT_SND_RCV_RETRY))
        {
            APPL_TRACE_ERROR0("bsa_send_receive_message GKI_read_mbox returns NULL => try again");
        }

        if (p_msg != NULL)
        {

            /* If we received a Disconnection (from server) message */
            if (p_msg->event == UIPC_CLOSE_EVT)
            {
                APPL_TRACE_DEBUG0("bsa_send_receive_message received disconnect message");
                status = BSA_ERROR_CLI_NOT_CONNECTED;
            }
            /* It should be an Rx event message AND */
            /* It should be a Return function message AND */
            /* The message_id should be the one we sent */
            else if ((p_msg->event == UIPC_RX_DATA_EVT) &&
                     (p_msg->layer_specific & BSA_FUNCTION_RETURN_BIT) &&
                    ((p_msg->layer_specific & ~BSA_FUNCTION_RETURN_BIT) == message_id))
            {
#ifdef BSA_MGT_INT_DEBUG
                APPL_TRACE_DEBUG1("bsa_send_receive_message received expected message_id (0x%x)",
                         p_msg->layer_specific);
#endif
                if (p_msg->len == rx_length)
                {
                    memcpy(rx_buffer, (UINT8 *)(p_msg + 1), p_msg->len);
                    status = BSA_SUCCESS;
                }
                else
                {
                    APPL_TRACE_ERROR2("bsa_send_receive_message received buffer len:%d do not match (expected:%d)",
                             p_msg->len, rx_length);
                    status = BSA_ERROR_CLI_BAD_RSP_SIZE;
                }
            }
            else
            {
                APPL_TRACE_ERROR3("bsa_send_receive_message received unexpected event:%x message_id:%d (0x%x)",
                        p_msg->event, p_msg->layer_specific, p_msg->layer_specific);
            }
            GKI_freebuf(p_msg);
        }
        else
        {
            APPL_TRACE_ERROR0("bsa_send_receive_message GKI_read_mbox returns NULL");
        }
    }
    else
    {
        APPL_TRACE_ERROR1("bsa_send_receive_message timeout occurred OR bad event received:%d", event);
    }

    /* In case of error, delete the message_id waited by this task */
    if (status != BSA_SUCCESS)
    {
        bsa_del_msg_id_tid(task_id);
    }

    /* If this is a temporary task => destroy it */
    if (task_id > APP_TASK)
    {
        GKI_exit_task(task_id);
    }
    return status;
}

/*******************************************************************************
 **
 ** Function         bsa_add_msg_id_tid
 **
 ** Description      Add a message_id in the task_id table.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static int bsa_add_msg_id_tid(UINT8 task_id, int message_id)
{
    if ((task_id < GKI_MAX_TASKS))
    {
        message_id_tab[task_id] = message_id;
        return 0;
    }
    APPL_TRACE_ERROR1("bsa_add_msg_id_tid bad task id:%d", task_id);
    return -1;
}

/*******************************************************************************
 **
 ** Function         bsa_del_msg_id_tid
 **
 ** Description      Delete a message_id in the task_id table.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static int bsa_del_msg_id_tid(UINT8 task_id)
{
    if ((task_id < GKI_MAX_TASKS))
    {
        message_id_tab[task_id] = 0;
        return 0;
    }
    APPL_TRACE_ERROR1("bsa_del_msg_id_tid bad task id:%d", task_id);
    return -1;
}

/*******************************************************************************
 **
 ** Function         bsa_get_msg_id_from_tid
 **
 ** Description      Get a message_id in the task_id table.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static int bsa_get_msg_id_from_tid(UINT8 task_id, int *message_id)
{
    if ((task_id < GKI_MAX_TASKS))
    {
        *message_id = message_id_tab[task_id];
        return 0;
    }
    APPL_TRACE_ERROR1("bsa_get_msg_id_from_tid bad task id:%d", task_id);
    *message_id = 0;
    return -1;
}

/*******************************************************************************
 **
 ** Function         bsa_get_tid_from_msg_id
 **
 ** Description      Get a message_id in the task_id table.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static int bsa_get_tid_from_msg_id(UINT8 *task_id, int message_id)
{
    int index;

    for (index = 0; index < GKI_MAX_TASKS; index++)
    {
        if (message_id_tab[index] == message_id)
        {
            *task_id = index;
            return 0;
        }
    }
    APPL_TRACE_ERROR1("bsa_get_tid_from_msg_id received unexpected msg_id:%d", message_id);
    *task_id = -1;
    return -1;
}

/*******************************************************************************
 **
 ** Function       bsa_mgt_event_hdlr
 **
 ** Description    Handle Management events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_mgt_event_hdlr(int message_id, tBSA_MGT_MSG *p_data, int length)
{
    tBSA_MGT_EVT event;
    BOOLEAN call_cback = TRUE;
    int uipc_index;
    BOOLEAN bt_enable = TRUE;

    switch (message_id)
    {
    case BSA_SYS_MSGID_STATUS_EVT: /* Bluetooth status */
        event = BSA_MGT_STATUS_EVT;
        /* If this is a BT Status event, extract the BT status (Enable/disable) */
        bt_enable = p_data->status.enable;
        break;
    default:
        APPL_TRACE_ERROR1("bsa_mgt_event_hdlr unknown message_id:%d", message_id);
        call_cback = FALSE;
        break;
    }

    /* events are directly sent to user callback */
    if ((bsa_cl_mgt_cb.bsa_mgt_callback != NULL) && (call_cback != FALSE))
    {
        APPL_TRACE_EVENT1("bsa_mgt_event_hdlr event:%d", event);

        bsa_cl_mgt_cb.bsa_mgt_callback(event, p_data);

        /* If BT has been disabled */
        if (bt_enable == FALSE)
        {
            /* For all UIPCs that could be opened by this client, close them */
            for (uipc_index = 0; uipc_index < UIPC_CH_NUM; uipc_index++)
            {
                /* Close every UIPC channel excepted the Control channel */
                if (uipc_index != UIPC_CH_ID_CTL)
                {
                    APPL_TRACE_DEBUG1("UIPC_%d: closing", uipc_index);
                    UIPC_Close(uipc_index);
                }
            }
        }
    }
}



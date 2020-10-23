/*****************************************************************************
 **
 **  Name:           bsa_tm_api.c
 **
 **  Description:    This is the public interface file for Test Module part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2010-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_cl_tm_int.h"
#include "gki_int.h"
#include "uipc_thread.h"

extern UINT16 GKI_get_stack_used(UINT8 task_id);

/*
 * Global variables
 */


/*******************************************************************************
 **
 ** Function         BSA_TmSetTestModeInit
 **
 ** Description      Initialize the tBSA_TM_SET_TEST_MODE structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmSetTestModeInit(tBSA_TM_SET_TEST_MODE *p_req)
{
    APPL_TRACE_API0("BSA_TmSetTestModeInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_TM_SET_TEST_MODE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_TmSetTestMode
 **
 ** Description      Enable/Disable Bluetooth test mode
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmSetTestMode(tBSA_TM_SET_TEST_MODE *p_req)
{
    APPL_TRACE_API0("BSA_TmSetTestMode");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_TM_MSGID_SET_TEST_MODE_CMD,
            p_req, sizeof(tBSA_TM_SET_TEST_MODE));
}

/*******************************************************************************
 **
 ** Function         BSA_TmGetMemUsageInit
 **
 ** Description      Initialize the tBSA_TM_GET_MEM_USAGE structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmGetMemUsageInit(tBSA_TM_GET_MEM_USAGE *p_req)
{
    APPL_TRACE_API0("BSA_TmGetMemUsageInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_TM_GET_MEM_USAGE));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_TmGetMemUsage
 **
 ** Description      Get Memory usage (Task's stack size, and GKI buffers)
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmGetMemUsage(tBSA_TM_GET_MEM_USAGE *p_req)
{
    tBSA_TM_MSGID_GET_MEM_USAGE_CMD_REQ req;
    tBSA_STATUS ret_code;
    INT32 index;
    UINT8 task_id;
    UINT8 buf_pool_id;

    APPL_TRACE_API0("BSA_TmGetMemUsage");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("ERROR BSA_TmGetMemUsage NULL param");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    /* If Location is Server => send the request to the server */
    if (p_req->location == BSA_TM_SERVER)
    {
        APPL_TRACE_DEBUG0("BSA_TmGetMemUsage: Server");
        req.location = BSA_TM_SERVER;

        /* Call server (send/receive message) */
        ret_code = bsa_send_receive_message(BSA_TM_MSGID_GET_MEM_USAGE_CMD, &req,
                sizeof(req), p_req, sizeof(tBSA_TM_GET_MEM_USAGE));

        if (ret_code != BSA_SUCCESS)
        {
            APPL_TRACE_ERROR1("ERROR BSA_TmGetMemUsage fails status:%d", ret_code);
        }

        return ret_code;
    }
    /* If the Location is client */
    else
    {
        APPL_TRACE_DEBUG0("BSA_TmGetMemUsage: Client");

        /* Reset all */
        memset(p_req, 0, sizeof(tBSA_TM_GET_MEM_USAGE));
        p_req->location = BSA_TM_SERVER;

        APPL_TRACE_DEBUG0("BSA_TmGetMemUsage Get GKI Tasks info");
        index= 0;
        for(task_id=0; (task_id < GKI_MAX_TASKS); task_id++)
        {
            /* If this task is created */
            if (gki_cb.com.OSRdyTbl[task_id] != TASK_DEAD)
            {
                if (index < BSA_TM_TASK_MAX)
                {
                    p_req->task[index].task_used = TRUE;
                    p_req->task[index].task_id = task_id;
                    p_req->task[index].task_state = gki_cb.com.OSRdyTbl[task_id];
                    strncpy((char *) p_req->task[index].task_name,
                            (char *) gki_cb.com.OSTName[task_id], BSA_TM_TASK_NAME_MAX-1);
                    p_req->task[index].task_name[BSA_TM_TASK_NAME_MAX-1] = '\0';
                    p_req->task[index].stack_size = gki_cb.com.OSStackSize[task_id];
                    p_req->task[index].stack_used = GKI_get_stack_used(task_id);
                    index++;
                }
                else
                {
                    APPL_TRACE_WARNING1("BSA_TmGetMemUsage task:%d missed", task_id);
                    p_req->task_missing = TRUE;
                }
            }
        }
        APPL_TRACE_DEBUG0("BSA_TmGetMemUsage Get Other Tasks info (UIPC)");
        /* If space remains in info table */
        if (index < BSA_TM_TASK_MAX)
        {
            /* Get UIPC Task info (pass a pointer on remaining table) */
            if (uipc_thread_get_usage(&p_req->task[index], BSA_TM_TASK_MAX - index) < 0)
            {
                APPL_TRACE_WARNING1("BSA_TmGetMemUsage other task:%d missed", task_id);
                p_req->task_missing = TRUE;
            }
        }


        APPL_TRACE_DEBUG0("BSA_TmGetMemUsage Get Buffer info");
        index = 0;
        for (buf_pool_id = 0; buf_pool_id < GKI_NUM_TOTAL_BUF_POOLS ; buf_pool_id++)
        {
            if (index < BSA_TM_GKI_BUF_POOL_MAX)
            {
                p_req->gki_buffer[index].pool_used = TRUE;
                p_req->gki_buffer[index].pool_id = buf_pool_id;
                p_req->gki_buffer[index].pool_size = gki_cb.com.freeq[buf_pool_id].size;
                p_req->gki_buffer[index].pool_number = gki_cb.com.freeq[buf_pool_id].total;
                p_req->gki_buffer[index].pool_cur_use = gki_cb.com.freeq[buf_pool_id].cur_cnt;
                p_req->gki_buffer[index].pool_max_use = gki_cb.com.freeq[buf_pool_id].max_cnt;
                index++;
            }
            else
            {
                APPL_TRACE_WARNING1("BSA_TmGetMemUsage buf_pool:%d missed", buf_pool_id);
                p_req->gki_buf_missing = TRUE;
            }
        }
    }
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_TmVscInit
 **
 ** Description      Initialize the tBSA_TM_VSC structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmVscInit(tBSA_TM_VSC *p_req)
{
    APPL_TRACE_API0("BSA_TmVscInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_TM_VSC));
    return BSA_SUCCESS;
}


/*******************************************************************************
 **
 ** Function         BSA_TmVsc
 **
 ** Description      Function to send a Vendor Specific Command to the chip
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmVsc(tBSA_TM_VSC *p_req)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_TmVsc");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    if (p_req->length > BSA_TM_VSC_DATA_LEN_MAX)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_TM_MSGID_VSC_CMD,
            p_req, sizeof(tBSA_TM_MSGID_VSC_CMD_REQ), p_req,
            sizeof(tBSA_TM_MSGID_VSC_CMD_RSP));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_TmVsc bsa_send_receive_message fails status:%d", ret_code);
        return ret_code;
    }

    return p_req->status;
}

/*******************************************************************************
 **
 ** Function         BSA_TmPingInit
 **
 ** Description      Initialize the tBSA_TM_PING structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmPingInit(tBSA_TM_PING *p_req)
{
    APPL_TRACE_API0("BSA_TmPingInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_TM_PING));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_TmPing
 **
 ** Description      Function to Ping the BSA server
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmPing(tBSA_TM_PING *p_req)
{
    tBSA_TM_MSGID_PING_CMD_REQ ping_param_req;
    tBSA_TM_MSGID_PING_CMD_REQ ping_param_rsp;
    tBSA_STATUS ret_code;
    static int pattern = 0;

    APPL_TRACE_API0("BSA_TmPing");

    /* change pattern */
    pattern++;

    /* Fill parameter structure with pattern */
    memset(ping_param_req.data, pattern, BSA_TM_PING_SIZE);

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_TM_MSGID_PING_CMD,
            &ping_param_req, sizeof(ping_param_req), &ping_param_rsp,
            sizeof(ping_param_rsp));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_TmPing bsa_send_receive_message fails status:%d", ret_code);
        return ret_code;
    }

    /* check if returned data are the same that sent */
    if (memcmp(ping_param_req.data, ping_param_rsp.data, BSA_TM_PING_SIZE) != 0)
    {
        APPL_TRACE_ERROR0("ERROR BSA_TmPing data differ");
        ret_code = BSA_ERROR_CLI_BAD_PING_RSP;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         BSA_TmSetTraceLevelInit
 **
 ** Description      Initialize the tBSA_TM_SET_TRACE_LEVEL structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmSetTraceLevelInit(tBSA_TM_SET_TRACE_LEVEL *p_req)
{
    APPL_TRACE_API0("BSA_TmSetTraceLevelInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_TM_SET_TRACE_LEVEL));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_TmSetTraceLevel
 **
 ** Description      Function to Set the Trace Level of the BSA server
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmSetTraceLevel(tBSA_TM_SET_TRACE_LEVEL *p_req)
{
    APPL_TRACE_API0("BSA_TmSetTraceLevel");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send/receive message) */
    return bsa_send_message_receive_status(BSA_TM_MSGID_SET_TRACE_LEVEL_CMD,
            p_req, sizeof(tBSA_TM_SET_TRACE_LEVEL));
}

/*******************************************************************************
 **
 ** Function         BSA_TmReadVersionInit
 **
 ** Description      Initialize the tBSA_TM_READ_VERSION structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmReadVersionInit(tBSA_TM_READ_VERSION *p_req)
{
    APPL_TRACE_API0("BSA_TmReadVersionInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(tBSA_TM_READ_VERSION));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_TmReadVersion
 **
 ** Description      Function to Read Version
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmReadVersion(tBSA_TM_READ_VERSION *p_req)
{
    tBSA_STATUS ret_code;
    tBSA_TM_MSGID_READ_VERSION_CMD_REQ version_req;

    APPL_TRACE_API0("BSA_TmReadVersion");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }


    /* To prevent compilation warning */
    version_req.dummy = 0;

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_TM_MSGID_READ_VERSION_CMD,
            &version_req, sizeof(version_req),
            p_req, sizeof(tBSA_TM_MSGID_READ_VERSION_CMD_RSP));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_TmReadVersion bsa_send_receive_message fails status:%d", ret_code);
        return ret_code;
    }

    return p_req->status;
}

/*******************************************************************************
 **
 ** Function         BSA_TmVseRegisterInit
 **
 ** Description      Initialize the tBSA_TM_VSE_REGISTER structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmVseRegisterInit(tBSA_TM_VSE_REGISTER *p_req)
{
    APPL_TRACE_API0("BSA_TmVseRegisterInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_TmVseRegister
 **
 ** Description      Function to Register VSE (sub_event and callback)
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmVseRegister(tBSA_TM_VSE_REGISTER *p_req)
{
    tBSA_TM_MSGID_VSE_REGISTER_CMD_REQ req;
    tBSA_STATUS bsa_status;
    tBSA_TM_CBACK *p_former_callback;

    APPL_TRACE_API0("BSA_TmVseRegister");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmVseRegister param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Callback is required */
    if (p_req->p_callback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmVseRegister callback is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Save former callback (in case of error) */
    p_former_callback = bsa_cl_tm_cb.p_callback;

    /* Save/update TM callback */
    bsa_cl_tm_cb.p_callback = p_req->p_callback;

    /* Prepare Request parameter */
    req.sub_event = p_req->sub_event;

    /* Call server (send message, receive status) */
    bsa_status = bsa_send_message_receive_status(BSA_TM_MSGID_VSE_REGISTER_CMD,
            &req, sizeof(req));
    if (bsa_status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_TmVseRegister fails status:%d", bsa_status);
        /* Restore former TM callback */
        bsa_cl_tm_cb.p_callback = p_former_callback;
    }

    return bsa_status;
}

/*******************************************************************************
 **
 ** Function         BSA_TmVseDeregisterInit
 **
 ** Description      Initialize the tBSA_TM_VSE_DEREGISTER structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmVseDeregisterInit(tBSA_TM_VSE_DEREGISTER *p_req)
{
    APPL_TRACE_API0("BSA_TmVseRegisterInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;

}

/*******************************************************************************
 **
 ** Function         BSA_TmVseDeregister
 **
 ** Description      Function to Deregister VSE (sub_event and callback)
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmVseDeregister(tBSA_TM_VSE_DEREGISTER *p_req)
{

    tBSA_TM_MSGID_VSE_DEREGISTER_CMD_REQ req;
    tBSA_STATUS bsa_status;

    APPL_TRACE_API0("BSA_TmVseDeregister");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmVseDeregister param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* If application asks to deregister the callback */
    if (p_req->deregister_callback)
    {
        /* Reset it */
        bsa_cl_tm_cb.p_callback = NULL;
    }

    /* Prepare Request parameter */
    req.sub_event = p_req->sub_event;

    /* Call server (send message, receive status) */
    bsa_status = bsa_send_message_receive_status(BSA_TM_MSGID_VSE_DEREGISTER_CMD,
            &req, sizeof(req));
    if (bsa_status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_TmVseDeregister fails status:%d", bsa_status);
    }

    return bsa_status;
}

/*******************************************************************************
 **
 ** Function         BSA_TmDisconnectInit
 **
 ** Description      Initialize the tBSA_TM_DISCONNECT structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmDisconnectInit(tBSA_TM_DISCONNECT *p_req)
{
    APPL_TRACE_API0("BSA_TmDisconnectInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}


/*******************************************************************************
 **
 ** Function         BSA_TmDisconnect
 **
 ** Description      Function to Deregister VSE (sub_event and callback)
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmDisconnect(tBSA_TM_DISCONNECT *p_req)
{
    APPL_TRACE_API0("BSA_TmDisconnect");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmDisconnect param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    APPL_TRACE_DEBUG6("BdAddr %02X:%02X:%02X:%02X:%02X:%02X",
            p_req->bd_addr[0], p_req->bd_addr[1],
            p_req->bd_addr[2], p_req->bd_addr[3],
            p_req->bd_addr[4], p_req->bd_addr[5]);

    /* Call server (send message, receive status) */
    return bsa_send_message_receive_status(BSA_TM_MSGID_DISCONNECT_CMD,
            p_req, sizeof(tBSA_TM_MSGID_DISCONNECT_CMD_REQ));
}


/*******************************************************************************
 **
 ** Function         BSA_TmQosInit
 **
 ** Description      Initialize the tBSA_TM_QOS structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmQosInit(tBSA_TM_QOS *p_req)
{
    APPL_TRACE_API0("BSA_TmQosInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}


/*******************************************************************************
 **
 ** Function         BSA_TmQos
 **
 ** Description      Function to configure QoS to ACL connection
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmQos(tBSA_TM_QOS *p_req)
{
    APPL_TRACE_API0("BSA_TmQos");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmQos param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    APPL_TRACE_DEBUG6("BdAddr %02X:%02X:%02X:%02X:%02X:%02X",
            p_req->bd_addr[0], p_req->bd_addr[1],
            p_req->bd_addr[2], p_req->bd_addr[3],
            p_req->bd_addr[4], p_req->bd_addr[5]);

    /* Call server (send message, receive status) */
    return bsa_send_message_receive_status(BSA_TM_MSGID_QOS_CMD,
            p_req, sizeof(tBSA_TM_MSGID_QOS_CMD_REQ));
}

/*******************************************************************************
 **
 ** Function         BSA_TmGetHandleInit
 **
 ** Description      Initialize the tBSA_TM_GET_HANDLE structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmGetHandleInit(tBSA_TM_GET_HANDLE *p_req)
{
    APPL_TRACE_API0("BSA_TmGetHandleInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_TmGetHandle
 **
 ** Description      Function to get connection handle using BDADDR
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmGetHandle(tBSA_TM_GET_HANDLE *p_req)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_TmGetHandle");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmGetHandle param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    APPL_TRACE_DEBUG6("BdAddr %02X:%02X:%02X:%02X:%02X:%02X",
            p_req->bd_addr[0], p_req->bd_addr[1],
            p_req->bd_addr[2], p_req->bd_addr[3],
            p_req->bd_addr[4], p_req->bd_addr[5]);

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_TM_MSGID_GET_HANDLE_CMD,
            p_req, sizeof(tBSA_TM_MSGID_GET_HANDLE_CMD_REQ), p_req,
            sizeof(tBSA_TM_MSGID_GET_HANDLE_CMD_RSP));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_TmGetHandle bsa_send_receive_message fails status:%d", ret_code);
        return ret_code;
    }

    return p_req->status;

}

/*******************************************************************************
 **
 ** Function         BSA_TmDiagStatsInit
 **
 ** Description      Initialize the tBSA_TM_DIAG_STATS structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmDiagStatsInit(tBSA_TM_DIAG_STATS *p_req)
{
    APPL_TRACE_API0("BSA_TmDiagStatsInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}


/*******************************************************************************
 **
 ** Function         BSA_TmDiagStats
 **
 ** Description      Function to send LMP Diagostics command to controller
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
BTAPI tBSA_STATUS BSA_TmDiagStats(tBSA_TM_DIAG_STATS *p_req)
{
    tBSA_STATUS bsa_status;

    APPL_TRACE_API0("BSA_TmDiagStats");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmDiagStats param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Check for valid command */
    if (p_req->diag_cmd >= BSA_TM_DIAG_MAX_CMD)
    {
        APPL_TRACE_ERROR0("BSA_TmDiagStats command is invalid");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Call server (send message, receive message) */
    bsa_status = bsa_send_receive_message(BSA_TM_MSGID_DIAG_STATS_CMD,
            p_req, sizeof(tBSA_TM_MSGID_DIAG_STATS_CMD_REQ), p_req,
            (p_req->diag_cmd == BSA_TM_DIAG_RESET_STATS_CMD) ? sizeof(UINT16) :
            sizeof(tBSA_TM_MSGID_DIAG_STATS_CMD_RSP));

    if (bsa_status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("BSA_TmDiagStats fails status:%d", bsa_status);
    }


    return bsa_status;
}

/*******************************************************************************
 **
 ** Function         BSA_TmReadRawRssiInit
 **
 ** Description      Initialize the tBSA_TM_READRAWRSSI structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmReadRawRssiInit(tBSA_TM_READRAWRSSI *p_req)
{
    APPL_TRACE_API0("BSA_TmReadRawRssiInit");

    if (p_req == NULL)
    {
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_req, 0, sizeof(*p_req));
    return BSA_SUCCESS;
}


/*******************************************************************************
 **
 ** Function         BSA_TmReadRawRssi
 **
 ** Description      Function to Read Raw RSSI
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmReadRawRssi(tBSA_TM_READRAWRSSI *p_req)
{
    tBSA_STATUS ret_code;

    APPL_TRACE_API0("BSA_TmReadRawRssi");

    if (p_req == NULL)
    {
        APPL_TRACE_ERROR0("BSA_TmReadRawRssi param is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    APPL_TRACE_DEBUG6("BdAddr %02X:%02X:%02X:%02X:%02X:%02X",
            p_req->bd_addr[0], p_req->bd_addr[1],
            p_req->bd_addr[2], p_req->bd_addr[3],
            p_req->bd_addr[4], p_req->bd_addr[5]);

    /* Call server (send/receive message) */
    ret_code = bsa_send_receive_message(BSA_TM_MSGID_READ_RAWRSSI_CMD,
            p_req, sizeof(tBSA_TM_MSGID_READ_RAWRSSI_CMD_REQ), p_req,
            sizeof(tBSA_TM_MSGID_READ_RAWRSSI_CMD_RSP));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_TmReadRawRssi bsa_send_receive_message fails status:%d", ret_code);
        return ret_code;
    }

    return p_req->status;

}

/*******************************************************************************
 **
 ** Function         BSA_TmLeTestCmd
 **
 ** Description      Function to send HCI LE Test commands to controller
 **
 ** Parameters       tBSA_TM_LE_CMD
 **                  test: Test command to send:
 **                    0 - tx test, 1 - rx test, 2 - end test
 **                  freq: Frequency for rx and tx test commands
 **                  payload_len: test_data_lenght for tx test command
 **                  pattern: test pattern for tx test command
 **                  retcount: test result. Its valid only test is END test
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmLeTestCmd(tBSA_TM_LE_CMD *pCMD)
{
    tBSA_STATUS resp;
    tBSA_TM_LE_CMD_RESULT result;
    tBSA_STATUS ret_code;

    ret_code = bsa_send_receive_message(BSA_TM_MSGID_LE_TEST_CMD,
                pCMD, sizeof(tBSA_TM_LE_CMD), &result, sizeof(result));

    if (ret_code != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR BSA_TmLeTestCmd() bsa_send_receive_message fails status:%d", ret_code);
        return ret_code;
    }

    resp=result.status;
    if(pCMD->test==BSA_LE_END_TEST_CMD)
    {
        pCMD->retcount=result.count;
    }

    return resp;
}


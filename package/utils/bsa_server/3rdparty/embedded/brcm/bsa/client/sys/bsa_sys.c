/*****************************************************************************
 **
 **  Name:           bsa_client_sys.c
 **
 **  Description:    Contains system BSA client functions
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
#include "bsa_client.h"
#include "bsa_cl_sys_int.h"

/*******************************************************************************
 **
 ** Function         bsa_cl_ping
 **
 ** Description      function used to ping server (for test purposes)
 **
 ** Parameters
 **
 ** Returns          The status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS bsa_cl_ping(void)
{
    tBSA_SYS_MSGID_PING_CMD_REQ param_req;
    tBSA_SYS_MSGID_PING_CMD_RSP param_rsp;
    tBSA_STATUS status;
    static int pattern = 0;

    pattern++; /* change pattern */

    /* Fill parameter structure with pattern */
    memset(param_req.data, pattern, BSA_PING_SIZE);

/*     APPL_TRACE_DEBUG1("bsa_cl_ping :%2x", pattern & 0xff);*/

    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_SYS_MSGID_PING_CMD,
            &param_req, sizeof(param_req), &param_rsp,
            sizeof(param_rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR bsa_cl_ping bsa_send_receive_message fails status:%d", status);
        return status;
    }

    status = param_rsp.status;

    /* if server reported an error => don't go further */
    if (status != BSA_SUCCESS)
    {
        return status;
    }

    /* check if returned data are the same that sent */
    if (memcmp(param_req.data, param_rsp.data, BSA_PING_SIZE) != 0)
    {
        APPL_TRACE_ERROR0("ERROR bsa_cl_ping data differ");
        status = BSA_ERROR_CLI_BAD_PING_RSP;
    }

    return status;
}

/*******************************************************************************
 **
 ** Function         bsa_cl_kill_server
 **
 ** Description      Function used to kill the server
 **
 ** Parameters
 **
 ** Returns          The status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS bsa_cl_kill_server(void)
{
    tBSA_SYS_MSGID_KILL_SERVER_CMD_REQ param_req;
    tBSA_SYS_MSGID_KILL_SERVER_CMD_RSP param_rsp;
    tBSA_STATUS status;

    /* Call server (send/receive message) */
    status = bsa_send_receive_message(BSA_SYS_MSGID_KILL_SERVER_CMD, &param_req,
            sizeof(param_req), &param_rsp, sizeof(param_rsp));

    if (status != BSA_SUCCESS)
    {
        APPL_TRACE_ERROR1("ERROR bsa_cl_exchange_pid bsa_send_receive_message fails status:%d",
                status);
        return status;
    }

    /* Extract received parameters */
    status = param_rsp.status;
    return status;
}


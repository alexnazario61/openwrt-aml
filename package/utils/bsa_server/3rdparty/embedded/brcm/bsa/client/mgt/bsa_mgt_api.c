/*****************************************************************************
 **
 **  Name:           bsa_mgt_api.c
 **
 **  Description:    client Management API for BSA
 **
 **  Copyright (c) 2009-2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_cl_mgt_int.h"
#include "bsa_cl_sys_int.h"

/*
 * Defines
 */
#if !defined(BSA_UNIFIED_CLIENT_SERVER) || (BSA_UNIFIED_CLIENT_SERVER == FALSE)
#if !defined(BSA_START_CBACK_TASK_ON_INIT) || (BSA_START_CBACK_TASK_ON_INIT == FALSE)
#define BTE_CBACK_TASK_STR      ((INT8 *) "CBack")
#define BTE_CBACK_STACK_SIZE    (16*1024)
#define BTE_CBACK_STACK         ((UINT16 *)0)
#endif
#endif

#define BTE_APP_TASK_STR        ((INT8 *) "App")
#define BTE_APP_STACK_SIZE      (16*1024)
#define BTE_APP_STACK           ((UINT16 *)0)

/*
 * Global variables
 */
tBSA_CL_MGT_CB bsa_cl_mgt_cb = {FALSE , FALSE, NULL};


/*******************************************************************************
 **
 ** Function         BSA_MgtOpenInit
 **
 ** Description      Init the Management open structure.
 **
 ** Parameters
 **
 ** Returns          The status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtOpenInit(tBSA_MGT_OPEN *p_mgt_open)
{
    APPL_TRACE_API0("BSA_MgtOpenInit");

    if (p_mgt_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MgtOpenInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_mgt_open, 0, sizeof(tBSA_MGT_OPEN));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_MgtOpen
 **
 ** Description      Open the connection to the server.
 **
 ** Parameters       The management open structure.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtOpen(tBSA_MGT_OPEN *p_mgt_open)
{
    APPL_TRACE_API1("BSA_MgtOpen (%s)", p_mgt_open->uipc_path);

    /* Check parameter structure */
    if (p_mgt_open == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MgtOpen param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* Check Disconnect callback parameter */
    if (p_mgt_open->callback == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MgtOpen mgt_callback is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* check if we are already connected to server */
    if (bsa_cl_mgt_cb.bsa_connected_to_server)
    {
        APPL_TRACE_ERROR0("BSA_MgtOpen already connected to server");
        return BSA_ERROR_CLI_ALREADY_CONNECTED;
    }

    /* Check if GKI has already been initialized */
    if (!bsa_cl_mgt_cb.gki_initialized)
    {
#if (!defined(BSA_UNIFIED_CLIENT_SERVER) || (BSA_UNIFIED_CLIENT_SERVER == FALSE))
        /* Initialize GKI (buffer, mutexes, etc.) */
        GKI_init();

        /* Enable GKI */
        GKI_enable();

#if (!defined(BSA_START_CBACK_TASK_ON_INIT) || (BSA_START_CBACK_TASK_ON_INIT == FALSE))
        /* Start the Callback task */
        GKI_create_task(bsa_callback_task, CBACK_TASK, BTE_CBACK_TASK_STR,
                BTE_CBACK_STACK, BTE_CBACK_STACK_SIZE);
#endif
#endif

        /* Start a fake APP task */
        GKI_create_task(NULL, APP_TASK, BTE_APP_TASK_STR, BTE_APP_STACK,
                BTE_APP_STACK_SIZE);

        /* Remember that GKI was initialized */
        bsa_cl_mgt_cb.gki_initialized = TRUE;
    }

    /* Initialize BSA client structures */
    bsa_cl_mgt_init();

    /* Initialize UIPC */
    UIPC_Init(p_mgt_open->uipc_path);

    /* Save application Management callback */
    bsa_cl_mgt_cb.bsa_mgt_callback = p_mgt_open->callback;

    /* Connect to server */
    if (!UIPC_Open(UIPC_CH_ID_CTL, bsa_mgt_cback))
    {
        APPL_TRACE_ERROR0("BSA_MgtOpen UIPC_open fails (cannot connect to server)");
        bsa_cl_mgt_cb.bsa_mgt_callback = NULL;
        return BSA_ERROR_CLI_UIPC_OPEN;
    }

    /* We are connected */
    bsa_cl_mgt_cb.bsa_connected_to_server = TRUE;

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_MgtKillServerInit
 **
 ** Description      This function initializes the BSA API structure.
 **
 ** Parameters       Pointer to the allocated structure.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtKillServerInit(tBSA_MGT_KILL_SERVER *p_mgt_kill_server)
{
    APPL_TRACE_API0("BSA_MgtKillServerInit");

    if (p_mgt_kill_server == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MgtKillServerInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_mgt_kill_server, 0, sizeof(*p_mgt_kill_server));
    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         BSA_MgtKillServer
 **
 ** Description      This function kills the Bluetooth Daemon.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtKillServer(tBSA_MGT_KILL_SERVER *p_mgt_kill_server)
{
    tBSA_STATUS status;

    APPL_TRACE_API0("BSA_MgtKillServer");

    /* Check parameter structure */
    if (p_mgt_kill_server == NULL)
    {
        /*
         * It's not really an error, but let's force the developer to use
         * a parameter structure for future extensions
         */
        APPL_TRACE_ERROR0("BSA_MgtKillServer param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* check if we are already connected to server */
    if (bsa_cl_mgt_cb.bsa_connected_to_server == FALSE)
    {
        APPL_TRACE_ERROR0("BSA_MgtKillServer not connected to server");
        return BSA_ERROR_CLI_NOT_CONNECTED;
    }

    /* Call the SYSTEM API to kill the server */
    status = bsa_cl_kill_server();

    if(status == BSA_SUCCESS)
    {
        bsa_cl_mgt_cb.bsa_connected_to_server = FALSE;
    }

    return status;
}


/*******************************************************************************
 **
 ** Function         BSA_MgtCloseInit
 **
 ** Description      This function initializes the BSA API structure.
 **
 ** Parameters       Pointer to the allocated structure.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtCloseInit(tBSA_MGT_CLOSE *p_mgt_close)
{
    APPL_TRACE_API0("BSA_MgtCloseInit");

    if (p_mgt_close == NULL)
    {
        APPL_TRACE_ERROR0("BSA_MgtCloseInit param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }
    memset(p_mgt_close, 0, sizeof(tBSA_MGT_CLOSE));
    return BSA_SUCCESS;
}
/*******************************************************************************
 **
 ** Function         BSA_MgtClose
 **
 ** Description      This function close a connection to the Bluetooth Daemon.
 **                  The callback function (of the BSA_MgtOpen) is not called.
 **                  This function should NOT be called from within the callback
 **                  function because it has to JOIN it.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtClose(tBSA_MGT_CLOSE *p_mgt_close)
{
    APPL_TRACE_API0("BSA_MgtClose");

    /* Check parameter structure */
    if (p_mgt_close == NULL)
    {
        /*
         * It's not really an error, but let's force the developer to use
         * a parameter structure for future extensions
         */
        APPL_TRACE_ERROR0("BSA_MgtClose param struct pointer is NULL");
        return BSA_ERROR_CLI_BAD_PARAM;
    }

    /* check if we are already connected to server */
    if (bsa_cl_mgt_cb.bsa_connected_to_server == FALSE)
    {
        APPL_TRACE_ERROR0("BSA_MgtClose not connected to server");
        return BSA_ERROR_CLI_NOT_CONNECTED;
    }

    /* Disconnect from server */
    UIPC_Close(UIPC_CH_ID_CTL);

    bsa_cl_mgt_cb.bsa_connected_to_server = FALSE;
    bsa_cl_mgt_cb.bsa_mgt_callback = NULL;

    return BSA_SUCCESS;
}


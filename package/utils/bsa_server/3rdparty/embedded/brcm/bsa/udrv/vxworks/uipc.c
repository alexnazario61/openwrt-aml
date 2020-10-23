/********************************************************************************
 **                                                                              *
 **  Name        uipc.c                                                    *
 **                                                                              *
 **  Function    UIPC wrapper interface                                          *
 **                                                                              *
 **                                                                              *
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.                    *
 **  Proprietary and confidential.                                               *
 **                                                                              *
 *********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_task.h"

//#include "bsa_client.h"

#include "gki.h"

#include "uipc_sv_control.h"
#include "uipc_cl_control.h"

#if (BSA_HH_INCLUDED == TRUE)
#include "uipc_sv_hh.h"
#include "uipc_cl_hh.h"
#endif

#if (BSA_DG_INCLUDED == TRUE)
#include "uipc_sv_dg.h"
#include "uipc_cl_dg.h"
#endif

BOOLEAN  UIPC_InitDone = FALSE;

//#define DEBUG_UIPC
/*
 * Extern Functions
 */

/*******************************************************************************
 **
 ** Function         UIPC_Init
 **
 ** Description      Initialize UIPC module
 **
 ** Returns          void
 **
 *******************************************************************************/
UDRV_API void UIPC_Init(void *p_data)
{
#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG0("UIPC_Init");
#endif

    if (UIPC_InitDone == TRUE)
    {
        APPL_TRACE_DEBUG0("UIPC_Init was already done (hopefully by server) don't do it again");
        return;
    }
    UIPC_InitDone = TRUE;

    uipc_sv_control_init();

#if (BSA_HH_INCLUDED == TRUE)
    uipc_sv_hh_init();
    uipc_cl_hh_init();
#endif

#if (BSA_DG_INCLUDED == TRUE)
    uipc_sv_dg_init();
    uipc_cl_dg_init();
#endif
}

/*******************************************************************************
 **
 ** Function         UIPC_Open
 **
 ** Description      Open UIPC interface
 **
 ** Returns          void
 **
 *******************************************************************************/
UDRV_API BOOLEAN UIPC_Open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback)
{
    BOOLEAN ret_code = FALSE;
    BOOLEAN is_server;
    UINT8 gki_task_id;

#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG1("UIPC_Open ChId:%d", ch_id);
#endif

    /* Get the taskId of the called */
    gki_task_id = GKI_get_taskid();
    
    /* Server tasks are numbered from 0 to 4:BT_MEDIA_TASK */
    if (gki_task_id <= BT_MEDIA_TASK)
    {
        is_server = TRUE;   /* Server called UIPC_Open */
    }
    else
    {
        is_server = FALSE;  /* Client called UIPC_Open */
    }
    
    switch (ch_id)
    {
    case UIPC_CH_ID_2:  /* Control Channel */
        if (is_server == FALSE)
        {
            ret_code = uipc_cl_control_open(ch_id, p_cback);
        }
        else
        {
            ret_code = uipc_sv_control_open(ch_id, p_cback);
        }
        break;

#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_3:  /* HH channel */
        if (is_server == FALSE)
        {
            ret_code = uipc_cl_hh_open(ch_id, p_cback);
        }
        else
        {
            ret_code = uipc_sv_hh_open(ch_id, p_cback);
        }
        break;
#endif
#if (BSA_DG_INCLUDED == TRUE)
    case UIPC_CH_ID_10:
    case UIPC_CH_ID_11:
    case UIPC_CH_ID_12:
    case UIPC_CH_ID_13:
        if (is_server == FALSE)
        {
            ret_code = uipc_cl_dg_open(ch_id, p_cback);
        }
        else
        {
            ret_code = uipc_sv_dg_open(ch_id, p_cback);
        }
        break;
#endif

    default:
        break;
    }
    return ret_code;
}

/*******************************************************************************
 **
 ** Function         UIPC_Close
 **
 ** Description      Close UIPC interface
 **
 ** Returns          void
 **
 *******************************************************************************/
UDRV_API void UIPC_Close(tUIPC_CH_ID ch_id)
{
    BOOLEAN is_server;
    UINT8 gki_task_id;

#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG1("UIPC_Close ChId:%d", ch_id);
#endif

    /* Get the taskId of the called */
    gki_task_id = GKI_get_taskid();
    
    /* Server tasks are numbered from 0 to 4:BT_MEDIA_TASK */
    if (gki_task_id <= BT_MEDIA_TASK)
    {
        is_server = TRUE;   /* Server called UIPC_Close */
    }
    else
    {
        is_server = FALSE;  /* Client called UIPC_Close */
    }
    
    switch (ch_id)
    {
    case UIPC_CH_ID_2:  /* Control Channel */
        if (is_server == FALSE)
        {
            uipc_cl_control_close(ch_id);
        }
        else
        {
            uipc_sv_control_close(ch_id);
        }
        break;

#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_3:  /* HH channel */
        if (is_server == FALSE)
        {
            uipc_cl_hh_close(ch_id);
        }
        else
        {
            uipc_sv_hh_close(ch_id);
        }
        break;
#endif
#if (BSA_DG_INCLUDED == TRUE)
    case UIPC_CH_ID_10:
    case UIPC_CH_ID_11:
    case UIPC_CH_ID_12:
    case UIPC_CH_ID_13:
        if (is_server == FALSE)
        {
            uipc_cl_dg_close(ch_id);
        }
        else
        {
            uipc_sv_dg_close(ch_id);
        }
        break;
#endif

    default:
        break;
    }
}

/*******************************************************************************
 **
 ** Function         UIPC_SendBuf
 **
 ** Description      Called to transmit a message over UIPC.
 **                  Message buffer will be freed by UIPC_SendBuf.
 **
 ** Returns          void
 **
 *******************************************************************************/
UDRV_API BOOLEAN UIPC_SendBuf(tUIPC_CH_ID ch_id, BT_HDR *p_msg)
{
    BOOLEAN ret_code = FALSE;
    BOOLEAN is_server;
    UINT8 gki_task_id;

#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG1("UIPC_SendBuf ChId:%d", ch_id);
#endif

    /* Get the taskId of the called */
    gki_task_id = GKI_get_taskid();
    
    /* Server tasks are numbered from 0 to 4:BT_MEDIA_TASK */
    if (gki_task_id <= BT_MEDIA_TASK)
    {
        is_server = TRUE;   /* Server called UIPC_Close */
    }
    else
    {
        is_server = FALSE;  /* Client called UIPC_Close */
    }

    switch (ch_id)
    {
    case UIPC_CH_ID_2:  /* control channel */
        if (is_server == FALSE)
        {
    //        ret_code = uipc_cl_control_send_buf(p_client_msg->rx_msg.message_id,
    //                p_client_msg->rx_msg.data, p_client_msg->rx_msg.hdr.len)
        }
        else
        {
            APPL_TRACE_ERROR0("UIPC_SendBuf not implemented for Server");
        }
        GKI_freebuf(p_msg);
        break;

    default:
        APPL_TRACE_ERROR1("UIPC_SendBuf not implemented for this channel:%d", ch_id);
        GKI_freebuf(p_msg);
        break;
    }
    return ret_code;
}

/*******************************************************************************
 **
 ** Function         UIPC_Send
 **
 ** Description      Called to transmit a message over UIPC.
 **
 ** Returns          void
 **
 *******************************************************************************/
UDRV_API BOOLEAN UIPC_Send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
        UINT16 msglen)
{
    BOOLEAN ret_code = FALSE;
    BOOLEAN is_server;
    UINT8 gki_task_id;

#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG1("UIPC_Send ChId:%d", ch_id);
#endif

    /* Get the taskId of the called */
    gki_task_id = GKI_get_taskid();
    
    /* Server tasks are numbered from 0 to 4:BT_MEDIA_TASK */
    if (gki_task_id <= BT_MEDIA_TASK)
    {
#ifdef DEBUG_UIPC
        APPL_TRACE_DEBUG0("UIPC_Send server side");
#endif
        is_server = TRUE;   /* Server called UIPC_Send */
    }
    else
    {
#ifdef DEBUG_UIPC
        APPL_TRACE_DEBUG0("UIPC_Send client side");
#endif
        is_server = FALSE;  /* Client called UIPC_Send */
    }

    switch (ch_id)
    {
    case UIPC_CH_ID_2: /* control Channel */
        if (is_server == FALSE)
        {
            ret_code = uipc_cl_control_send(ch_id, msg_evt, p_buf, msglen);
        }
        else
        {
            ret_code = uipc_sv_control_send(msg_evt, p_buf, msglen);
        }
        break;

#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_3:  /* HH channel */
        if (is_server == FALSE)
        {
            APPL_TRACE_ERROR0("UIPC_Send not allowed for HH client");
        }
        else
        {
              ret_code = uipc_sv_hh_send(ch_id, p_buf, msglen);
        }
        break;
#endif
#if (BSA_DG_INCLUDED == TRUE)
    case UIPC_CH_ID_10:
    case UIPC_CH_ID_11:
    case UIPC_CH_ID_12:
    case UIPC_CH_ID_13:
        if (is_server == FALSE)
        {
            ret_code = uipc_cl_dg_send(ch_id, p_buf, msglen);
        }
        else
        {
            ret_code = uipc_sv_dg_send(ch_id, p_buf, msglen);
        }
        break;
#endif

    default:
        break;
    }

    return ret_code;
}

/*******************************************************************************
 **
 ** Function         UIPC_ReadBuf
 **
 ** Description      Called to read a message from UIPC.
 **
 ** Returns          void
 **
 *******************************************************************************/
UDRV_API void UIPC_ReadBuf(tUIPC_CH_ID ch_id, BT_HDR *p_msg)
{
#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG1("UIPC_ReadBuf ChId:%d NOT implemented", ch_id);
#endif
}

/*******************************************************************************
 **
 ** Function         UIPC_Read
 **
 ** Description      Called to read a message from UIPC.
 **
 ** Returns          void
 **
 *******************************************************************************/
UDRV_API UINT32 UIPC_Read(tUIPC_CH_ID ch_id, UINT16 *p_msg_evt, UINT8 *p_buf,
        UINT32 len)
{
    UINT32 length = 0;

#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG1("UIPC_Read ChId:%d NOT implemented", ch_id);
#endif

    return length;
}


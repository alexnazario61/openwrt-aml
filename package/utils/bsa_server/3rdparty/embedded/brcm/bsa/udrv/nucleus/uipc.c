/*****************************************************************************
**
**  Name:           uipc.c
**
**  Description:    UIPC wrapper interface definition
**
**  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "buildcfg.h"
#include "bsa_api.h"
#include "bsa_int.h"
#include "uipc_bsa.h"

#ifdef BSA_SERVER
#include "bsa_task.h"
#endif

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

#if (BSA_AV_INCLUDED == TRUE)
#include "uipc_sv_av.h"
#if (BSA_BAV_INCLUDED == TRUE)
#include "uipc_sv_av_bav.h"
#endif /* BSA_BAV_INCLUDED */
#include "uipc_cl_av.h"
#if (BSA_BAV_INCLUDED == TRUE)
#include "uipc_cl_av_bav.h"
#endif /* BSA_BAV_INCLUDED */
#endif /* BSA_AV_INCLUDED */

#if (BSA_HL_INCLUDED == TRUE)
#include "uipc_sv_hl.h"
#include "uipc_cl_hl.h"
#endif

#if (BSA_NSA_INCLUDED == TRUE)
#include "uipc_sv_nsa.h"
#endif

#if (BSA_HS_INCLUDED == TRUE) || (BSA_AG_INCLUDED == TRUE)
#include "uipc_sv_sco.h"
#include "uipc_cl_sco.h"
#endif

/*
#define DEBUG_UIPC
*/

#include "gki.h"
#include "uipc.h"
typedef struct
{
    char path[BSA_MGT_UIPC_PATH_MAX];
} tUIPC;

tUIPC uipc_cb;
BOOLEAN UIPC_InitDone = FALSE;

/*
* Extern Functions
*/

#if (BSA_AVK_INCLUDED == TRUE)
extern void uipc_avk_init(void*);
extern UINT32 uipc_avk_read(UINT8 * p_buf, UINT32 len);
extern BOOLEAN uipc_avk_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
                             UINT16 msglen);
extern BOOLEAN uipc_avk_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback);
extern void uipc_avk_close(void);
extern BOOLEAN uipc_avk_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);
#endif

#if (BSA_PBC_INCLUDED == TRUE)
extern void uipc_pbc_init(void*);
extern UINT32 uipc_pbc_read(UINT8 * p_buf, UINT32 len);
extern BOOLEAN uipc_pbc_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
                             UINT16 msglen);
extern BOOLEAN uipc_pbc_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback);
extern void uipc_pbc_close(void);
extern BOOLEAN uipc_pbc_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);
#endif

/*******************************************************************************
**
** Function         UIPC_Init
**
** Description      Initialize UIPC module
**
** Returns          void
**
*******************************************************************************/
void UIPC_Init(void *p_data)
{
    BOOLEAN is_server;
    UINT8 task_id;

    task_id = GKI_get_taskid();
    is_server = (task_id <= BTE_APPL_TASK)?TRUE:FALSE;
    APPL_TRACE_DEBUG2("UIPC_Init(%s) is_server:%s",
        ((p_data != NULL) && (*(char*)p_data != 0))?p_data:"NULL", (is_server?"SERVER":"CLIENT"));

    if (UIPC_InitDone == TRUE)
    {
        APPL_TRACE_DEBUG0("UIPC_Init was already done for Server don't do it again");
        return;
    }
    UIPC_InitDone = TRUE;

    /* Initialize the control block: copy the path unless NULL or empty */
    if ((p_data != NULL) && (*(char*)p_data != 0))
    {
        memcpy(uipc_cb.path, p_data, sizeof(uipc_cb.path));
    }
    else
    {
        /* NULL or empty string: default to CWD */
        uipc_cb.path[0] = '.';
        uipc_cb.path[1] = '/';
        uipc_cb.path[2] = '\0';
    }

    uipc_sv_control_init();
    uipc_cl_control_init();

#if (BSA_HH_INCLUDED == TRUE)
    uipc_sv_hh_init();
    uipc_cl_hh_init();
#endif

#if (BSA_DG_INCLUDED == TRUE)
    uipc_sv_dg_init();
    uipc_cl_dg_init();
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    uipc_pbc_init(uipc_cb.path);
#endif

}

/*******************************************************************************
**
** Function         UIPC_Open
**
** Description      Open UIPC interface
**
** Returns          TRUE in case of success, FALSE in case of failure.
**
*******************************************************************************/
BOOLEAN UIPC_Open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback)
{
    BOOLEAN ret_code = FALSE;
    BOOLEAN is_server;
    UINT8 task_id;

    task_id = GKI_get_taskid();
    is_server = (task_id <= BTE_APPL_TASK)?TRUE:FALSE;
#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG4("UIPC_Open ChId:%d task_id:%d BTE_APPL_TASK:%d is_server>%s", ch_id, task_id,
        BTE_APPL_TASK, (is_server?"SERVER":"CLIENT"));
#endif

    switch(ch_id)
    {
    case UIPC_CH_ID_CTL: /* Control Channel */
        ret_code = (is_server == FALSE)?uipc_cl_control_open(ch_id, p_cback):\
            uipc_sv_control_open(ch_id, p_cback);
        APPL_TRACE_DEBUG3("UIPC_Open uipc_control_open():%d p_cback:%x (is_server:%s)", ret_code, p_cback,
            (is_server?"SERVER":"CLIENT"));
        break;
#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_HH:
        ret_code = (is_server == FALSE)?uipc_cl_hh_open(ch_id, p_cback):\
            uipc_sv_hh_open(ch_id, p_cback);
        break;
#endif
#if (BSA_DG_INCLUDED == TRUE)
    case UIPC_CH_ID_10:
    case UIPC_CH_ID_11:
    case UIPC_CH_ID_12:
    case UIPC_CH_ID_13:
        ret_code = (is_server == FALSE)?uipc_cl_dg_open(ch_id, p_cback):\
            uipc_sv_dg_open(ch_id, p_cback);
        break;
#endif

    case BSA_AVK_CHNL_AUDIO:
        ret_code = uipc_avk_open(ch_id, p_cback);
        break;

#if (BSA_PBC_INCLUDED == TRUE)
    case UIPC_CH_ID_PBC:
        ret_code = uipc_pbc_open(ch_id, p_cback);
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
void UIPC_Close(tUIPC_CH_ID ch_id)
{
    BOOLEAN is_server;
    UINT8 task_id;

    task_id = GKI_get_taskid();
    is_server = (task_id <= BTE_APPL_TASK)?TRUE:FALSE;
    APPL_TRACE_DEBUG1("UIPC_Close ChId:%d", ch_id);

    switch(ch_id)
    {
    case UIPC_CH_ID_CTL: /* Control Channel */
        (is_server == FALSE)?uipc_cl_control_close(ch_id):uipc_sv_control_close(ch_id);
        break;
#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_HH:
        (is_server == FALSE)?uipc_cl_hh_close(ch_id):uipc_sv_hh_close(ch_id);
        break;
#endif
#if (BSA_DG_INCLUDED == TRUE)
    case UIPC_CH_ID_10:
    case UIPC_CH_ID_11:
    case UIPC_CH_ID_12:
    case UIPC_CH_ID_13:
        (is_server == FALSE)?uipc_cl_dg_close(ch_id):uipc_sv_dg_close(ch_id);
        break;
#endif

#if (BSA_AVK_INCLUDED == TRUE)
    case BSA_AVK_CHNL_AUDIO:
        uipc_avk_close();
        break;
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    case UIPC_CH_ID_PBC:
        uipc_pbc_close();
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
** Returns          TRUE in case of success, FALSE in case of failure.
**
*******************************************************************************/
BOOLEAN UIPC_SendBuf(tUIPC_CH_ID ch_id, BT_HDR *p_msg)
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
    if (gki_task_id <= BTE_APPL_TASK)
    {
        is_server = TRUE;   /* Server called UIPC_SendBuf */
    }
    else
    {
        is_server = FALSE;  /* Client called UIPC_SendBuf */
    }

    switch (ch_id)
    {
    case UIPC_CH_ID_2:  /* control channel */
        if (is_server == FALSE)
        {
            /*        ret_code = uipc_cl_control_send_buf(p_client_msg->rx_msg.message_id,
            p_client_msg->rx_msg.data, p_client_msg->rx_msg.hdr.len); */
        }
        else
        {
            APPL_TRACE_ERROR0("UIPC_SendBuf not implemented for Server");
        }
        GKI_freebuf(p_msg);
        break;

#if (BSA_DG_INCLUDED == TRUE)
    case UIPC_CH_ID_10:
    case UIPC_CH_ID_11:
    case UIPC_CH_ID_12:
    case UIPC_CH_ID_13:
        if (is_server == FALSE)
        {
            ret_code = uipc_cl_dg_send_buf(ch_id, p_msg);
        }
        else
        {
            ret_code = uipc_sv_dg_send_buf(ch_id, p_msg);
        }
        break;
#endif


#if (BSA_AVK_INCLUDED == TRUE)
    case UIPC_CH_ID_AVK_AUDIO:
    case UIPC_CH_ID_AVK_VIDEO:
        return uipc_avk_send_buf(ch_id, p_msg);
#endif


#if (BSA_PBC_INCLUDED == TRUE)
        if(ch_id == UIPC_CH_ID_PBC)
            return uipc_pbc_send_buf(ch_id, p_msg);
#endif

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
** Returns          TRUE in case of success, FALSE in case of failure.
**
*******************************************************************************/
BOOLEAN UIPC_Send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
                  UINT16 msglen)
{
    BOOLEAN ret_code = FALSE;
    BOOLEAN is_server;
    UINT8 gki_task_id;

    /* Get the taskId of the called */
    gki_task_id = GKI_get_taskid();

#ifdef DEBUG_UIPC
    APPL_TRACE_DEBUG2("UIPC_Send ChId:%d task_id:%u", ch_id, gki_task_id);
#endif

    /* Server tasks are numbered from 0 to 4:BT_MEDIA_TASK */
    if (gki_task_id <= BTE_APPL_TASK)
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

#if (BSA_AVK_INCLUDED == TRUE)
    case UIPC_CH_ID_AVK_AUDIO:
    case UIPC_CH_ID_AVK_VIDEO:
        return uipc_avk_send(ch_id, msg_evt, p_buf, msglen);
        break;
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    case UIPC_CH_ID_PBC:
        return uipc_pbc_send(ch_id, msg_evt, p_buf, msglen);
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
void UIPC_ReadBuf(tUIPC_CH_ID ch_id, BT_HDR *p_msg)
{
}

/*******************************************************************************
**
** Function         UIPC_Read
**
** Description      Called to read a message from UIPC.
**
** Returns          return the number of bytes read.
**
*******************************************************************************/
UINT32 UIPC_Read(tUIPC_CH_ID ch_id, UINT16 *p_msg_evt, UINT8 *p_buf,
                 UINT32 len)
{
    APPL_TRACE_ERROR1("UIPC_Read not implemented for channel id:%d", ch_id);
    return 0;
}

#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
/*******************************************************************************
**
** Function         uipc_ioctl_get_req_desc
**
** Description      Returns Ioctl Request description
**
** Returns          String
**
*******************************************************************************/
static char *uipc_ioctl_get_req_desc(UINT32 request)
{
    switch(request)
    {
    case UIPC_REQ_TX_FLUSH:
        return "UIPC_REQ_TX_FLUSH";
    case UIPC_REQ_RX_FLUSH:
        return "UIPC_REQ_RX_FLUSH";
    case UIPC_WRITE_BLOCK:
        return "UIPC_WRITE_BLOCK";
    case UIPC_WRITE_NONBLOCK:
        return "UIPC_WRITE_NONBLOCK";
    case UIPC_REG_CBACK:
        return "UIPC_REG_CBACK";
    case UIPC_SET_RX_WM:
        return "UIPC_SET_RX_WM";
    case UIPC_REQ_TX_READY:
        return "UIPC_REQ_TX_READY";
    case UIPC_REQ_RX_READY:
        return "UIPC_REQ_RX_READY";
    case UIPC_RESET:
        return "UIPC_RESET";
    default:
        return "Unknown IOCTL Request";
    }
}
#endif

/*******************************************************************************
**
** Function         UIPC_Ioctl
**
** Description      Called to control UIPC.
**
** Returns          void
**
*******************************************************************************/
BOOLEAN UIPC_Ioctl(tUIPC_CH_ID ch_id, UINT32 request, void *param)
{
#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
    APPL_TRACE_DEBUG3("UIPC_Ioctl ChId:%d Request:%s (%d)",
        ch_id, uipc_ioctl_get_req_desc(request), request);
#endif

    return TRUE;
}


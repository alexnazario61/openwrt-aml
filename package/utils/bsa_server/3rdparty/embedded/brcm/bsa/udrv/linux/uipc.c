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
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"
#define BSA_MGT_UIPC_PATH_MAX NSA_MGT_UIPC_PATH_MAX
#else
#include "bsa_api.h"
#include "bsa_int.h"
#endif

#ifdef BSA_SERVER
#include "bsa_task.h"
#endif

#ifdef NSA_SERVER
#include "nsa_task.h"
#endif

#include "gki.h"

#include "uipc_fifo.h"
#include "uipc_rb.h"

#if (defined(BSA_SERVER) || defined(NSA_SERVER))
#include "uipc_sv_control.h"
#include "uipc_sv_socket.h"
#endif
#if (defined(BSA_CLIENT) || defined(NSA_CLIENT))
#include "uipc_cl_control.h"
#include "uipc_cl_socket.h"
#endif

#if (BSA_HH_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_hh.h"
#endif
#ifdef BSA_CLIENT
#include "uipc_cl_hh.h"
#endif
#endif

#if (BSA_DG_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_dg.h"
#endif
#ifdef BSA_CLIENT
#include "uipc_cl_dg.h"
#endif
#endif

#if (BSA_PAN_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_pan.h"
#endif
#ifdef BSA_CLIENT
#include "uipc_cl_pan.h"
#endif
#endif

#if (BSA_AV_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_av.h"
#if (BSA_BAV_INCLUDED == TRUE)
#include "uipc_sv_av_bav.h"
#endif /* BSA_BAV_INCLUDED */
#endif /* BSA_SERVER */
#ifdef BSA_CLIENT
#include "uipc_cl_av.h"
#if (BSA_BAV_INCLUDED == TRUE)
#include "uipc_cl_av_bav.h"
#endif /* BSA_BAV_INCLUDED */
#endif /* BSA_CLIENT */
#endif /* BSA_AV_INCLUDED */


#if (BSA_HL_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_hl.h"
#endif
#ifdef BSA_CLIENT
#include "uipc_cl_hl.h"
#endif
#endif

#if (BSA_NSA_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_nsa.h"
#endif
#endif

#if (BSA_HS_INCLUDED == TRUE) || (BSA_AG_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_sco.h"
#endif
#ifdef BSA_CLIENT
#include "uipc_cl_sco.h"
#endif
#endif

#if defined(BTU_DUAL_STACK_MM_INCLUDED) && (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
#ifdef BSA_SERVER
#include "uipc_sv_lite.h"
#endif
#endif

typedef struct
{
    char path[BSA_MGT_UIPC_PATH_MAX];
} tUIPC;

tUIPC uipc_cb;
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

#if (BSA_HS_INCLUDED == TRUE)|| (BSA_AG_INCLUDED == TRUE)
/* SCO IN functions  */
extern void uipc_hs_sco_in_init(void*);
extern UINT32 uipc_hs_sco_in_read(UINT8 * p_buf, UINT32 len);
extern BOOLEAN uipc_hs_sco_in_send(tUIPC_CH_ID ch_id, UINT16 msg_evt,
        UINT8 *p_buf, UINT16 msglen);
extern BOOLEAN uipc_hs_sco_in_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback);
extern void uipc_hs_sco_in_close(void);
/* SCO OUT functions */
extern void uipc_hs_sco_out_init(void*);
extern UINT32 uipc_hs_sco_out_read(UINT8 * p_buf, UINT32 len);
extern BOOLEAN uipc_hs_sco_out_send(tUIPC_CH_ID ch_id, UINT16 msg_evt,
        UINT8 *p_buf, UINT16 msglen);
extern BOOLEAN uipc_hs_sco_out_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback);
extern void uipc_hs_sco_out_close(void);
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

#if (BSA_MCE_INCLUDED == TRUE)
extern void uipc_mce_rx_init(void*);
extern UINT32 uipc_mce_rx_read(UINT8 * p_buf, UINT32 len);
extern BOOLEAN uipc_mce_rx_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
        UINT16 msglen);
extern BOOLEAN uipc_mce_rx_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback);
extern void uipc_mce_rx_close(void);
extern BOOLEAN uipc_mce_rx_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);
extern void uipc_mce_tx_init(void*);
extern UINT32 uipc_mce_tx_read(UINT8 * p_buf, UINT32 len);
extern BOOLEAN uipc_mce_tx_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
        UINT16 msglen);
extern BOOLEAN uipc_mce_tx_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback);
extern void uipc_mce_tx_close(void);
extern BOOLEAN uipc_mce_tx_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);
#endif

#if (BSA_SAC_INCLUDED == TRUE)
extern void uipc_sac_init(void*);
extern BOOLEAN uipc_sac_open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback);
extern UINT32 uipc_sac_read(UINT8 * p_buf, UINT32 len);
extern BOOLEAN uipc_sac_send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
        UINT16 msglen);
extern BOOLEAN uipc_sac_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg);
extern void uipc_sac_close(void);
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
UDRV_API void UIPC_Init(void *p_data)
{
    APPL_TRACE_DEBUG0("UIPC_Init");

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

#if (defined(BSA_SERVER) || defined(NSA_SERVER))
    uipc_sv_socket_init(uipc_cb.path);
#endif
#if (defined(BSA_CLIENT) || defined(NSA_CLIENT))
    uipc_cl_socket_init(uipc_cb.path);
#endif

    uipc_fifo_init(uipc_cb.path);
    uipc_rb_init(uipc_cb.path);

#if (defined(BSA_SERVER) || defined(NSA_SERVER))
    uipc_sv_control_init();
#endif
#if (defined(BSA_CLIENT) || defined(NSA_CLIENT))
    uipc_cl_control_init();
#endif

#if (BSA_HH_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_hh_init();
#endif
#ifdef BSA_CLIENT
    uipc_cl_hh_init();
#endif
#endif

#if (BSA_AV_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_av_init();
#if (BSA_BAV_INCLUDED == TRUE)
    uipc_sv_av_bav_init();
#endif /* BSA_BAV_INCLUDED */
#endif /* BSA_SERVER */
#ifdef BSA_CLIENT
    uipc_cl_av_init();
#if (BSA_BAV_INCLUDED == TRUE)
    uipc_cl_av_bav_init();
#endif /* BSA_BAV_INCLUDED */
#endif
#endif


#if (BSA_AVK_INCLUDED == TRUE)
    uipc_avk_init(uipc_cb.path);
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    uipc_pbc_init(uipc_cb.path);
#endif

#if (BSA_MCE_INCLUDED == TRUE)
    uipc_mce_rx_init(uipc_cb.path);
    uipc_mce_tx_init(uipc_cb.path);
#endif

#if (BSA_HS_INCLUDED == TRUE)|| (BSA_AG_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_sco_init();
#endif
#ifdef BSA_CLIENT
    uipc_cl_sco_init();
#endif
#endif
#if (BSA_DG_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_dg_init();
#endif
#endif
#if (BSA_PAN_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_pan_init();
#endif
#endif

#if (BSA_HL_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_hl_init();
#endif
#endif

#if (BSA_NSA_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_nsa_init();
#endif
#endif

#if (BSA_SAC_INCLUDED == TRUE)
    uipc_sac_init(uipc_cb.path);
#endif

#if defined(BTU_DUAL_STACK_MM_INCLUDED) && (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
#ifdef BSA_SERVER
    uipc_sv_lite_init();
#endif
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
UDRV_API BOOLEAN UIPC_Open(tUIPC_CH_ID ch_id, tUIPC_RCV_CBACK *p_cback)
{
    BOOLEAN ret_code = FALSE;

    APPL_TRACE_DEBUG1("UIPC_Open ChId:%d", ch_id);

#if (BSA_DG_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_DG_FIRST) && (ch_id <= UIPC_CH_ID_DG_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_dg_open(ch_id, p_cback);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_dg_open(ch_id, p_cback);
#endif
    }
#endif

#if (BSA_HL_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_HL_FIRST) && (ch_id <= UIPC_CH_ID_HL_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_hl_open(ch_id, p_cback);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_hl_open(ch_id, p_cback);
#endif
    }
#endif

#if (BSA_PAN_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_PAN_FIRST) && (ch_id <= UIPC_CH_ID_PAN_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_pan_open(ch_id, p_cback);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_pan_open(ch_id, p_cback);
#endif
    }
#endif

    switch (ch_id)
    {
    case UIPC_CH_ID_0:
#if defined(BTU_DUAL_STACK_MM_INCLUDED) && (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
#ifdef BSA_SERVER
        ret_code = uipc_sv_lite_open(ch_id, p_cback);
#endif
#endif
        break;

    case UIPC_CH_ID_CTL:
    case UIPC_NSA_CH_ID_CTL:
#if (defined(BSA_SERVER) || defined(NSA_SERVER))
        ret_code = uipc_sv_control_open(ch_id, p_cback);
#endif
#if (defined(BSA_CLIENT) || defined(NSA_CLIENT))
        ret_code = uipc_cl_control_open(ch_id ,p_cback);
#endif
        break;

#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_HH:
#ifdef BSA_SERVER
        ret_code = uipc_sv_hh_open(ch_id, p_cback);
#endif
#ifdef BSA_CLIENT
        ret_code = uipc_cl_hh_open(ch_id, p_cback);
#endif
        break;
#endif

#if (BSA_AV_INCLUDED == TRUE)
    case UIPC_CH_ID_AV_AUDIO:
    case UIPC_CH_ID_AV_VIDEO:
#ifdef BSA_SERVER
        ret_code = uipc_sv_av_open(ch_id, p_cback);
#endif
#ifdef BSA_CLIENT
        ret_code = uipc_cl_av_open(ch_id);
#endif
        break;
#endif

#if (BSA_AVK_INCLUDED == TRUE)
    case UIPC_CH_ID_AVK_AUDIO:
    case UIPC_CH_ID_AVK_VIDEO:
        ret_code = uipc_avk_open(ch_id, p_cback);
        break;
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    case UIPC_CH_ID_PBC:
    ret_code = uipc_pbc_open(ch_id, p_cback);
    break;
#endif

#if (BSA_MCE_INCLUDED == TRUE)
    case UIPC_CH_ID_MCE_RX:
    ret_code = uipc_mce_rx_open(ch_id, p_cback);
    break;
    case UIPC_CH_ID_MCE_TX:
    ret_code = uipc_mce_tx_open(ch_id, p_cback);
    break;
#endif

#if (BSA_HS_INCLUDED == TRUE)|| (BSA_AG_INCLUDED == TRUE)
    case UIPC_CH_ID_SCO:
#ifdef BSA_SERVER
        ret_code = uipc_sv_sco_open(p_cback);
#endif
#ifdef BSA_CLIENT
        ret_code = uipc_cl_sco_open(p_cback);
#endif
        break;
#endif

#if (BSA_NSA_INCLUDED == TRUE)
    case UIPC_CH_ID_NSA:
#ifdef BSA_SERVER
        ret_code = uipc_sv_nsa_open(ch_id, p_cback);
#endif
        break;
#endif

#if (BSA_BAV_INCLUDED == TRUE)
    case UIPC_CH_ID_BAV_1:
    case UIPC_CH_ID_BAV_2:
#ifdef BSA_SERVER
        ret_code = uipc_sv_av_bav_open(ch_id);
#endif
#ifdef BSA_CLIENT
        ret_code = uipc_cl_av_bav_open(ch_id);
#endif
        break;
#endif /* BSA_BAV_INCLUDED */

#if (BSA_SAC_INCLUDED == TRUE)
        case UIPC_CH_ID_SAC:
        ret_code = uipc_sac_open(ch_id, p_cback);
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
    APPL_TRACE_DEBUG1("UIPC_Close ChId:%d", ch_id);

#if (BSA_DG_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_DG_FIRST) && (ch_id <= UIPC_CH_ID_DG_LAST))
    {
#ifdef BSA_SERVER
        uipc_sv_dg_close(ch_id);
#endif
#ifdef BSA_CLIENT
        uipc_cl_dg_close(ch_id);
#endif
    }
#endif

#if (BSA_HL_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_HL_FIRST) && (ch_id <= UIPC_CH_ID_HL_LAST))
    {
#ifdef BSA_SERVER
        uipc_sv_hl_close(ch_id);
#endif
#ifdef BSA_CLIENT
        uipc_cl_hl_close(ch_id);
#endif
    }
#endif

#if (BSA_PAN_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_PAN_FIRST) && (ch_id <= UIPC_CH_ID_PAN_LAST))
    {
#ifdef BSA_SERVER
        uipc_sv_pan_close(ch_id);
#endif
#ifdef BSA_CLIENT
        uipc_cl_pan_close(ch_id);
#endif
    }
#endif

    switch (ch_id)
    {
    case UIPC_CH_ID_0:
#if defined(BTU_DUAL_STACK_MM_INCLUDED) && (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
#ifdef BSA_SERVER
        uipc_sv_lite_close(ch_id);
#endif
#endif
        break;

    case UIPC_CH_ID_CTL:
    case UIPC_NSA_CH_ID_CTL:
#if (defined(BSA_SERVER) || defined(NSA_SERVER))
        uipc_sv_control_close(ch_id);
#endif
#if (defined(BSA_CLIENT) || defined(NSA_CLIENT))
        uipc_cl_control_close(ch_id);
#endif
        break;

#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_HH:
#ifdef BSA_SERVER
        uipc_sv_hh_close(ch_id);
#endif
#ifdef BSA_CLIENT
        uipc_cl_hh_close(ch_id);
#endif
        break;
#endif
#if (BSA_AV_INCLUDED == TRUE)
    case UIPC_CH_ID_AV_AUDIO:
    case UIPC_CH_ID_AV_VIDEO:
#ifdef BSA_SERVER
        uipc_sv_av_close();
#endif
#ifdef BSA_CLIENT
        uipc_cl_av_close();
#endif
        break;
#endif
#if (BSA_AVK_INCLUDED == TRUE)
    case UIPC_CH_ID_AVK_AUDIO:
    case UIPC_CH_ID_AVK_VIDEO:
        uipc_avk_close();
        break;
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    case UIPC_CH_ID_PBC:
        uipc_pbc_close();
        break;
#endif

#if (BSA_MCE_INCLUDED == TRUE)
    case UIPC_CH_ID_MCE_RX:
         uipc_mce_rx_close();
        break;
    case UIPC_CH_ID_MCE_TX:
         uipc_mce_tx_close();
        break;
#endif

#if (BSA_HS_INCLUDED == TRUE)|| (BSA_AG_INCLUDED == TRUE)
    case UIPC_CH_ID_SCO:
#ifdef BSA_CLIENT
        uipc_cl_sco_close();
#endif
#ifdef BSA_SERVER
        uipc_sv_sco_close();
#endif
        break;
#endif

#if (BSA_NSA_INCLUDED == TRUE)
    case UIPC_CH_ID_NSA:
#ifdef BSA_SERVER
        uipc_sv_nsa_close(ch_id);
#endif
        break;
#endif

#if (BSA_BAV_INCLUDED == TRUE)
    case UIPC_CH_ID_BAV_1:
    case UIPC_CH_ID_BAV_2:
#ifdef BSA_SERVER
        uipc_sv_av_bav_close(ch_id);
#endif
#ifdef BSA_CLIENT
        uipc_cl_av_bav_close(ch_id);
#endif
    break;
#endif /* BSA_BAV_INCLUDED */

#if (BSA_SAC_INCLUDED == TRUE)
        case UIPC_CH_ID_SAC:
            uipc_sac_close();
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
UDRV_API BOOLEAN UIPC_SendBuf(tUIPC_CH_ID ch_id, BT_HDR *p_msg)
{
    if(p_msg == NULL)
    {
        return FALSE;
    }

#if (BSA_DG_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_DG_FIRST) && (ch_id <= UIPC_CH_ID_DG_LAST))
    {
#ifdef BSA_SERVER
       return uipc_sv_dg_send_buf(ch_id, p_msg);
#endif
#ifdef BSA_CLIENT
       return uipc_cl_dg_send_buf(ch_id, p_msg);
#endif
    }
#endif

#if (BSA_PAN_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_PAN_FIRST) && (ch_id <= UIPC_CH_ID_PAN_LAST))
    {
#ifdef BSA_SERVER
       return uipc_sv_pan_send_buf(ch_id, p_msg);
#endif
#ifdef BSA_CLIENT
       return uipc_cl_pan_send_buf(ch_id, p_msg);
#endif
    }
#endif

#if (BSA_HL_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_HL_FIRST) && (ch_id <= UIPC_CH_ID_HL_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_hl_send_buf(ch_id, p_msg);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_hl_send_buf(ch_id, p_msg);
#endif
    }
#endif

#if (BSA_AVK_INCLUDED == TRUE)
    if((ch_id == UIPC_CH_ID_AVK_AUDIO) || (ch_id == UIPC_CH_ID_AVK_VIDEO))
        return uipc_avk_send_buf(ch_id, p_msg);
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    if(ch_id == UIPC_CH_ID_PBC)
        return uipc_pbc_send_buf(ch_id, p_msg);
#endif

#if (BSA_MCE_INCLUDED == TRUE)
    if(ch_id == UIPC_CH_ID_MCE_RX)
        return uipc_mce_rx_send_buf(ch_id, p_msg);
    if(ch_id == UIPC_CH_ID_MCE_TX)
        return uipc_mce_tx_send_buf(ch_id, p_msg);
#endif

#if defined(BTU_DUAL_STACK_MM_INCLUDED) && (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
#ifdef BSA_SERVER
    if (ch_id == UIPC_CH_ID_0)
    {
        return  uipc_sv_lite_send_buf(ch_id, p_msg);
    }
#endif
#endif

#if (BSA_SAC_INCLUDED == TRUE)
        if(ch_id == UIPC_CH_ID_SAC)
            return uipc_sac_send_buf(ch_id, p_msg);
#endif

    APPL_TRACE_ERROR1("UIPC_SendBuf not implemented for this channel:%d (use UIPC_Send instead)", ch_id);
    /* Set layer specific to 1 in order to flag an error */
    p_msg->layer_specific = UIPC_LS_TX_FAIL;

    return FALSE;
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
UDRV_API BOOLEAN UIPC_Send(tUIPC_CH_ID ch_id, UINT16 msg_evt, UINT8 *p_buf,
        UINT16 msglen)
{
#if (BSA_DG_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_DG_FIRST) && (ch_id <= UIPC_CH_ID_DG_LAST))
    {
        APPL_TRACE_ERROR1("UIPC_Send not implemented for this channel:%d (use UIPC_SendBuf instead)", ch_id);
        return FALSE;
    }
#endif

#if (BSA_PAN_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_PAN_FIRST) && (ch_id <= UIPC_CH_ID_PAN_LAST))
    {
        APPL_TRACE_ERROR1("UIPC_Send not implemented for this channel:%d (use UIPC_SendBuf instead)", ch_id);
        return FALSE;
    }
#endif

#if (BSA_HL_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_HL_FIRST) && (ch_id <= UIPC_CH_ID_HL_LAST))
    {
        APPL_TRACE_ERROR1("UIPC_Send not implemented for this channel:%d (use UIPC_SendBuf instead)", ch_id);
        return FALSE;
    }
#endif


    switch (ch_id)
    {
    case UIPC_CH_ID_0:
#if defined(BTU_DUAL_STACK_MM_INCLUDED) && (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
#ifdef BSA_SERVER
        return  uipc_sv_lite_send(ch_id, msg_evt, p_buf, msglen);
#endif
#endif
        break;

    case UIPC_CH_ID_CTL:
#ifdef BSA_CLIENT
        return uipc_cl_control_send(ch_id, msg_evt, p_buf, msglen);
#endif
#ifdef BSA_SERVER
        return uipc_sv_control_send(ch_id, (tBSA_CLIENT_NB)msg_evt, p_buf, msglen);
#endif
        break;

#if (BSA_HH_INCLUDED == TRUE)
    case UIPC_CH_ID_HH:
#ifdef BSA_SERVER
        return uipc_sv_hh_send(ch_id, p_buf, msglen);
#endif
        break;
#endif

#if (BSA_AV_INCLUDED == TRUE)
    case UIPC_CH_ID_AV_AUDIO:
    case UIPC_CH_ID_AV_VIDEO:
#ifdef BSA_CLIENT
        return uipc_cl_av_send(p_buf, msglen);
#endif
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

#if (BSA_MCE_INCLUDED == TRUE)
    case UIPC_CH_ID_MCE_RX:
        return uipc_mce_rx_send(ch_id, msg_evt, p_buf, msglen);
        break;
    case UIPC_CH_ID_MCE_TX:
        return uipc_mce_tx_send(ch_id, msg_evt, p_buf, msglen);
        break;
#endif

#if (BSA_HS_INCLUDED == TRUE)|| (BSA_AG_INCLUDED == TRUE)
    case UIPC_CH_ID_SCO:
#ifdef BSA_CLIENT
        return uipc_cl_sco_send(p_buf, msglen);
#endif
#ifdef BSA_SERVER
        return uipc_sv_sco_send(p_buf, msglen);
#endif
        break;
#endif

#if (BSA_NSA_INCLUDED == TRUE)
    case UIPC_CH_ID_NSA:
#ifdef BSA_SERVER
        return uipc_sv_nsa_send(ch_id, (tBSA_CLIENT_NB)msg_evt, p_buf, msglen);
#endif
        break;
#endif

    case UIPC_NSA_CH_ID_CTL:
#if (defined(NSA_CLIENT) || defined(BSA_CLIENT))
        return uipc_cl_control_send(ch_id, msg_evt, p_buf, msglen);
#endif
#ifdef NSA_SERVER
        return uipc_sv_control_send(ch_id, (tNSA_CLIENT_NB)msg_evt, p_buf, msglen);
#endif
        break;

#if (BSA_BAV_INCLUDED == TRUE)
    case UIPC_CH_ID_BAV_1:
    case UIPC_CH_ID_BAV_2:
#ifdef BSA_CLIENT
        return uipc_cl_av_bav_send(ch_id, p_buf, msglen);
#endif
        break;
#endif /* BSA_BAV_INCLUDED */

#if (BSA_SAC_INCLUDED == TRUE)
        case UIPC_CH_ID_SAC:
            return uipc_sac_send(ch_id, msg_evt, p_buf, msglen);
        break;
#endif

    default:
        break;
    }

    APPL_TRACE_ERROR1("UIPC_Send not implemented for this channel:%d", ch_id);

    return FALSE;
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
UDRV_API UINT32 UIPC_Read(tUIPC_CH_ID ch_id, UINT16 *p_msg_evt, UINT8 *p_buf,
        UINT32 len)
{
#if (BSA_DG_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_DG_FIRST) && (ch_id <= UIPC_CH_ID_DG_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_dg_read(ch_id, p_buf, len);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_dg_read(ch_id, p_buf, len);
#endif
    }
#endif

#if (BSA_HL_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_HL_FIRST) && (ch_id <= UIPC_CH_ID_HL_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_hl_read(ch_id, p_buf, len);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_hl_read(ch_id, p_buf, len);
#endif
    }
#endif

#if (BSA_PAN_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_PAN_FIRST) && (ch_id <= UIPC_CH_ID_PAN_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_pan_read(ch_id, p_buf, len);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_pan_read(ch_id, p_buf, len);
#endif
    }
#endif

    switch (ch_id)
    {
#if (BSA_AV_INCLUDED == TRUE)
        case UIPC_CH_ID_AV_AUDIO:
        case UIPC_CH_ID_AV_VIDEO:
#ifdef BSA_SERVER
            return uipc_sv_av_read(p_buf, len);
#endif
#ifdef BSA_CLIENT
            APPL_TRACE_ERROR1("UIPC_Read not implemented for channel id:%d", ch_id);
#endif

            break;
#endif

#if (BSA_AVK_INCLUDED == TRUE)
        case UIPC_CH_ID_AVK_AUDIO:
        case UIPC_CH_ID_AVK_VIDEO:
#ifdef BSA_CLIENT
            return uipc_avk_read(p_buf, len);
#endif
        break;
#endif

#if (BSA_PBC_INCLUDED == TRUE)
    case UIPC_CH_ID_PBC:
#ifdef BSA_CLIENT
    return uipc_pbc_read(p_buf, len);
#endif
        break;
#endif

#if (BSA_MCE_INCLUDED == TRUE)
    case UIPC_CH_ID_MCE_RX:
#ifdef BSA_CLIENT
        return uipc_mce_rx_read(p_buf, len);
#endif
        break;
    case UIPC_CH_ID_MCE_TX:
#ifdef BSA_SERVER
        return uipc_mce_tx_read(p_buf, len);
#endif
        break;
#endif

#if (BSA_HS_INCLUDED == TRUE)|| (BSA_AG_INCLUDED == TRUE)
        case UIPC_CH_ID_SCO:
#ifdef BSA_CLIENT
            return uipc_cl_sco_read(p_buf, len);
#endif
#ifdef BSA_SERVER
            return uipc_sv_sco_read(p_buf, len);
#endif
            break;
#endif

#if (BSA_BAV_INCLUDED == TRUE)
        case UIPC_CH_ID_BAV_1:
        case UIPC_CH_ID_BAV_2:
#ifdef BSA_SERVER
            return uipc_sv_av_bav_read(ch_id, p_buf, len);
#endif
            break;
#endif /* BSA_BAV_INCLUDED */

#if (BSA_SAC_INCLUDED == TRUE)
    case UIPC_CH_ID_SAC:
        return uipc_sac_read(p_buf, len);
        break;
#endif

    default:
        APPL_TRACE_ERROR1("UIPC_Read not implemented for channel id:%d", ch_id);
        break;
    }
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
    case UIPC_READ_ERROR:
        return "UIPC_READ_ERROR";
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
UDRV_API extern BOOLEAN UIPC_Ioctl(tUIPC_CH_ID ch_id, UINT32 request, void *param)
{
#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
    APPL_TRACE_DEBUG3("UIPC_Ioctl ChId:%d Request:%s (%d)", ch_id, uipc_ioctl_get_req_desc(request), request);
#endif

#if (BSA_DG_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_DG_FIRST) && (ch_id <= UIPC_CH_ID_DG_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_dg_ioctl(ch_id, request, param);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_dg_ioctl(ch_id, request, param);
#endif
    }
#endif

#if (BSA_HL_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_HL_FIRST) && (ch_id <= UIPC_CH_ID_HL_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_hl_ioctl(ch_id, request, param);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_hl_ioctl(ch_id, request, param);
#endif
    }
#endif

#if (BSA_PAN_INCLUDED == TRUE)
    if ((ch_id >= UIPC_CH_ID_PAN_FIRST) && (ch_id <= UIPC_CH_ID_PAN_LAST))
    {
#ifdef BSA_SERVER
        return uipc_sv_pan_ioctl(ch_id, request, param);
#endif
#ifdef BSA_CLIENT
        return uipc_cl_pan_ioctl(ch_id, request, param);
#endif
    }
#endif

    switch (ch_id)
    {
#if (BSA_AV_INCLUDED == TRUE)
        case UIPC_CH_ID_AV_AUDIO:
        case UIPC_CH_ID_AV_VIDEO:
#ifdef BSA_SERVER
            return uipc_sv_av_ioctl(request, param);
#endif
#ifdef BSA_CLIENT
            return uipc_cl_av_ioctl(request, param);
#endif
            break;
#endif

#if (BSA_HS_INCLUDED == TRUE)|| (BSA_AG_INCLUDED == TRUE)
        case UIPC_CH_ID_SCO:
#ifdef BSA_CLIENT
            return uipc_cl_sco_ioctl(request, param);
#endif

#ifdef BSA_SERVER
            return uipc_sv_sco_ioctl(request, param);
#endif
            break;
#endif

#if (BSA_BAV_INCLUDED == TRUE)
        case UIPC_CH_ID_BAV_1:
        case UIPC_CH_ID_BAV_2:
#ifdef BSA_CLIENT
            return uipc_cl_av_bav_ioctl(ch_id, request, param);
#endif
            break;
#endif /* BSA_BAV_INCLUDED */

        default:
            APPL_TRACE_ERROR1("UIPC_Ioctl not implemented for channel id:%d", ch_id);
            break;
    }
    return FALSE;
}

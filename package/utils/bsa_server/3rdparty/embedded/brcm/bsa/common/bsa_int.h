/*****************************************************************************
 **
 **  Name:           bsa_int.h
 **
 **  Description:    Contains private BSA data
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_INT_H
#define BSA_INT_H

#define BSA_SYS_MSGID_FIRST                     1
#define BSA_DM_MSGID_FIRST                      100
#define BSA_DISC_MSGID_FIRST                    200
#define BSA_SEC_MSGID_FIRST                     300
#define BSA_HH_MSGID_FIRST                      400
#define BSA_FTS_MSGID_FIRST                     500
#define BSA_FTC_MSGID_FIRST                     550
#define BSA_AV_MSGID_FIRST                      600
#define BSA_AVK_MSGID_FIRST                     700
#define BSA_PBS_MSGID_FIRST                     800
#define BSA_PBC_MSGID_FIRST                     850
#define BSA_OPS_MSGID_FIRST                     900
#define BSA_OPC_MSGID_FIRST                     950
#define BSA_MGT_MSGID_FIRST                    1000
#define BSA_HS_MSGID_FIRST                     1100
#define BSA_DG_MSGID_FIRST                     1200
#define BSA_AG_MSGID_FIRST                     1300
#define BSA_TM_MSGID_FIRST                     1400
#define BSA_HL_MSGID_FIRST                     1500
#define BSA_NSA_MSGID_FIRST                    1600
#define BSA_BLE_MSGID_FIRST                    1700

#define BSA_HD_MSGID_FIRST                     1800
#define BSA_MCS_MSGID_FIRST                    1900
#define BSA_MCE_MSGID_FIRST                    1950
#define BSA_SC_MSGID_FIRST                     2000
#define BSA_SAC_MSGID_FIRST                    2050
#define BSA_PAN_MSGID_FIRST                    2100

#include "gki.h"
#include "uipc.h"
#include "uipc_bsa.h"


#ifdef BSA_CLIENT
#include "bsa_client.h"
#endif

#include "bsa_dm_int.h"
#include "bsa_sys_int.h"
#include "bsa_mgt_int.h"
#include "bsa_sec_int.h"
#include "bsa_disc_int.h"
#include "bsa_hh_int.h"
#include "bsa_hd_int.h"
#include "bsa_ft_int.h"
#include "bsa_mce_int.h"
#include "bsa_pbs_int.h"
#include "bsa_pbc_int.h"
#include "bsa_op_int.h"
#include "bsa_av_int.h"
#include "bsa_avk_int.h"
#include "bsa_hs_int.h"
#include "bsa_ag_int.h"
#include "bsa_dg_int.h"
#include "bsa_pan_int.h"
#include "bsa_tm_int.h"
#include "bsa_hl_int.h"
#include "bsa_nsa_int.h"
#include "bsa_ble_int.h"
#include "bsa_sc_int.h"
#include "bsa_sac_int.h"

#define BSA_FUNCTION_RETURN_BIT     0x8000
#define BSA_ASYNC_EVENT_BIT         0x4000


#define BSA_MSGIG_RESERVED          0

#ifndef BSA_HH_INCLUDED
#define BSA_HH_INCLUDED BTA_HH_INCLUDED
#endif

#ifndef BSA_FTS_INCLUDED
#define BSA_FTS_INCLUDED BTA_FTS_INCLUDED
#endif

#ifndef BSA_FTC_INCLUDED
#define BSA_FTC_INCLUDED BTA_FTC_INCLUDED
#endif

#ifndef BSA_AV_INCLUDED
#define BSA_AV_INCLUDED BTA_AV_INCLUDED
#endif

#ifndef BSA_BAV_INCLUDED
#define BSA_BAV_INCLUDED BTA_BAV_INCLUDED
#endif


#ifndef BSA_AVK_INCLUDED
#define BSA_AVK_INCLUDED BTA_AVK_INCLUDED
#endif

#ifndef BSA_MCE_INCLUDED
#define BSA_MCE_INCLUDED BTA_MCE_INCLUDED
#endif

#ifndef BSA_PBS_INCLUDED
#define BSA_PBS_INCLUDED BTA_PBS_INCLUDED
#endif

#ifndef BSA_PBC_INCLUDED
#define BSA_PBC_INCLUDED BTA_PBC_INCLUDED
#endif

#ifndef BSA_OPS_INCLUDED
#define BSA_OPS_INCLUDED BTA_OPS_INCLUDED
#endif

#ifndef BSA_OPC_INCLUDED
#define BSA_OPC_INCLUDED BTA_OPC_INCLUDED
#endif

#ifndef BSA_OP_INCLUDED
#define BSA_OP_INCLUDED BTA_OP_INCLUDED
#endif

#ifndef BSA_HS_INCLUDED
#define BSA_HS_INCLUDED BTA_HS_INCLUDED
#endif

#ifndef BSA_AG_INCLUDED
#define BSA_AG_INCLUDED BTA_AG_INCLUDED
#endif

#ifndef BSA_DG_INCLUDED
#define BSA_DG_INCLUDED BTA_DG_INCLUDED
#endif

#ifndef BSA_PAN_INCLUDED
#define BSA_PAN_INCLUDED BTA_PAN_INCLUDED
#endif

#ifndef BSA_3DS_INCLUDED
#define BSA_3DS_INCLUDED BTA_3DS_INCLUDED
#endif

#ifndef BSA_TM_INCLUDED
#define BSA_TM_INCLUDED FALSE    /* TM not included per default */
#endif

#ifndef BSA_AV_UIPC_BUFFER_SIZE
#define BSA_AV_UIPC_BUFFER_SIZE 20001
#endif

#ifndef BSA_SCO_RX_UIPC_BUFFER_SIZE
#define BSA_SCO_RX_UIPC_BUFFER_SIZE 2401
#endif

#ifndef BSA_SCO_TX_UIPC_BUFFER_SIZE
#define BSA_SCO_TX_UIPC_BUFFER_SIZE 2401
#endif

#ifndef BSA_HL_INCLUDED
#define BSA_HL_INCLUDED BTA_HL_INCLUDED
#endif

#ifndef BSA_SAC_INCLUDED
#define BSA_SAC_INCLUDED BTA_SAC_INCLUDED
#endif

#endif



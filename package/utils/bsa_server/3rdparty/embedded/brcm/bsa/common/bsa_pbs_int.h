/*****************************************************************************
 **
 **  Name:           bsa_pbs_int.h
 **
 **  Description:    Contains private BSA PBAP server data
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 *****************************************************************************/

#ifndef BSA_PBS_INT_H
#define BSA_PBS_INT_H

#include "bsa_pbs_api.h"

/*
 * PBS Message definition
 */
enum
{
    BSA_PBS_MSGID_ENABLE_CMD = BSA_PBS_MSGID_FIRST,
    BSA_PBS_MSGID_DISABLE_CMD,
    BSA_PBS_MSGID_AUTH_RSP_CMD,
    BSA_PBS_MSGID_ACCESS_RSP_CMD,
    BSA_PBS_MSGID_LAST_CMD = BSA_PBS_MSGID_ACCESS_RSP_CMD,

    BSA_PBS_MSGID_OPEN_EVT, /* Connection Open*/
    BSA_PBS_MSGID_CLOSE_EVT, /* Connection Closed */
    BSA_PBS_MSGID_AUTH_EVT, /* OBEX Authentication event */
    BSA_PBS_MSGID_ACCESS_EVT, /* Access request event */
    BSA_PBS_MSGID_OPER_CMPL_EVT, /* Operation complete event */

    BSA_PBS_MSGID_LAST_EVT = BSA_PBS_MSGID_OPER_CMPL_EVT
};

/*
 * Structures used for parameters (transport)
 */

/*
 * PBS Enable
 */
typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    char service_name[BSA_FTS_SERVICE_NAME_LEN_MAX];
    char root_path[BSA_FTS_ROOT_PATH_LEN_MAX];
    BOOLEAN enable_authen;
    char realm[BSA_FT_REALM_LEN_MAX];
} tBSA_PBS_MSGID_ENABLE_CMD_REQ;

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;

} tBSA_PBS_MSGID_STATUS_RSP;


typedef tBSA_PBS_MSGID_STATUS_RSP tBSA_PBS_MSGID_ENABLE_CMD_RSP;

/*
 * PBS disable
 */
typedef tBSA_PBS_DISABLE tBSA_PBS_MSGID_DISABLE_CMD_REQ;
typedef tBSA_PBS_MSGID_STATUS_RSP tBSA_PBS_MSGID_DISABLE_CMD_RSP;

/*
 * PBS AuthRsp
 */
typedef tBSA_PBS_AUTHRSP tBSA_PBS_MSGID_AUTH_RSP_CMD_REQ;

typedef tBSA_PBS_MSGID_STATUS_RSP tBSA_PBS_MSGID_AUTH_RSP_CMD_RSP;

typedef tBSA_PBS_AUTH_MSG tBSA_PBS_MSGID_AUTH_RSP_EVT;

/*
 * PBS Access Rsp
 */
typedef tBSA_PBS_ACCESSRSP tBSA_PBS_MSGID_ACCESS_RSP_CMD_REQ;

typedef tBSA_PBS_MSGID_STATUS_RSP tBSA_PBS_MSGID_ACCESS_RSP_CMD_RSP;

typedef tBSA_PBS_ACCESS_MSG tBSA_PBS_MSGID_ACCESS_RSP_EVT;

/*
 * Structure (used by client)
 */
typedef struct
{
    tBSA_PBS_CBACK *p_app_pbs_cback;
} tBSA_PBS_CB;

/*
 * Global variables (used by client)
 */
extern tBSA_PBS_CB bsa_pbs_cb;

/*******************************************************************************
 **
 ** Function       bsa_pbs_event_hdlr
 **
 ** Description    Handle PBS events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_pbs_event_hdlr(int message_id, tBSA_PBS_MSG *p_data, int length);

#endif


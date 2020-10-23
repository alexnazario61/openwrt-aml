/*****************************************************************************
 **
 **  Name:           bsa_hh_int.h
 **
 **  Description:    Contains private BSA HL data
 **
 **  Copyright (c) 2011-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_HL_INT_H
#define BSA_HL_INT_H

#include "bsa_hl_api.h"

/*
 * HH Message definition
 */
enum
{
    BSA_HL_MSGID_ENABLE_CMD = BSA_HL_MSGID_FIRST,
    BSA_HL_MSGID_DISABLE_CMD,
    BSA_HL_MSGID_REGISTER_CMD,
    BSA_HL_MSGID_DEREGISTER_CMD,
    BSA_HL_MSGID_OPEN_CMD,
    BSA_HL_MSGID_OPEN_RSP_CMD,
    BSA_HL_MSGID_CLOSE_CMD,
    BSA_HL_MSGID_RECONNECT_CMD,
    BSA_HL_MSGID_DELETE_MDL_CMD,
    BSA_HL_MSGID_MDEP_CFG_CMD,  /* Internal Cmd to configure MDEP Data Type */
    BSA_HL_MSGID_SDP_QUERY_CMD,

    BSA_HL_MSGID_LAST_CMD = BSA_HL_MSGID_SDP_QUERY_CMD,

    BSA_HL_MSGID_OPEN_EVT, /* Connection Open */
    BSA_HL_MSGID_CLOSE_EVT, /* Connection Open */
    BSA_HL_MSGID_RECONNECT_EVT, /* Connection Reconnect */
    BSA_HL_MSGID_OPEN_REQ_EVT, /* Connection Open */
    BSA_HL_MSGID_SEND_DATA_CFM_EVT, /* Data packet sent confirmation event */
    BSA_HL_MSGID_SAVE_MDL_EVT, /* Event to save MDL data*/
    BSA_HL_MSGID_DELETE_MDL_EVT, /* Event to save MDL data*/
    BSA_HL_MSGID_SDP_QUERY_APP_EVT, /* Internal Event holding SDP of one App */
    BSA_HL_MSGID_SDP_QUERY_APP_MDEP_EVT, /* Internal Event holding MDEP */

    BSA_HL_MSGID_LAST_EVT = BSA_HL_MSGID_SDP_QUERY_APP_MDEP_EVT
};

/*
 * Structures used for parameters (transport)
 */

/*
 * HL Enable
 */
typedef tBSA_HL_ENABLE tBSA_HL_MSGID_ENABLE_CMD_REQ;

/*
 * HL Disable
 */

typedef tBSA_HL_DISABLE tBSA_HL_MSGID_DISABLE_CMD_REQ;

/*
 * HL Register
 */
typedef struct
{
    tBSA_SEC_AUTH sec_mask; /* security mask */
    char service_name[BSA_HL_SERVICE_NAME_LEN_MAX];
    char service_desc[BSA_HL_SERVICE_DESC_LEN_MAX];
    char provider_name[BSA_HL_PROVIDER_NAME_LEN_MAX];
    UINT8 num_of_mdeps; /* Number of MDEPs */
    BOOLEAN advertize_source; /* Indicates if Peer Sink can see our source MDEPs */
    tBSA_HL_MDL_CFG saved_mdl[BSA_HL_NUM_MDL_CFGS]; /* Saved MDL */
} tBSA_HL_MSGID_REGISTER_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
    tBSA_HL_APP_HANDLE app_handle; /* OUT Parameter */
    tBSA_HL_MDL_ID mdep_id_tab[BSA_HL_NUM_MDEPS_MAX]; /* OUT Parameter: MDEP IDs allocated */

} tBSA_HL_MSGID_REGISTER_CMD_RSP;

/*
 * Internal Messages used for HL Registration
 */
typedef struct
{
    UINT8 mdep_index;
    tBSA_HL_MDEP_ROLE mdep_role; /* Source or Sink */
    UINT8 num_of_mdep_data_types; /* Number of Data types supported */
    tBSA_HL_MDEP_DATA_TYPE_CFG data_cfg[BSA_HL_NUM_DATA_TYPES_MAX];
} tBSA_HL_MSGID_MDEP_CFG_CMD_REQ;

/*
 * HL Deregister
 */
typedef tBSA_HL_DEREGISTER tBSA_HL_MSGID_DEREGISTER_CMD_REQ;

/*
 * HL SDP Query
 */
typedef tBSA_HL_SDP_QUERY tBSA_HL_MSGID_SDP_QUERY_CMD_REQ;

typedef struct
{
    tBSA_STATUS status;
    BD_ADDR bd_addr; /* BdAddr of Peer Device */
    UINT8 num_records; /* Total Number of records */
    UINT8 rec_index; /* Current record index */
    UINT16 ctrl_psm; /* L2CAP Control PSM */
    UINT8 mcap_sup_features; /* MCAP Features supported */
    UINT8 num_mdeps; /* number of mdep elements from SDP */
    char service_name[BSA_HL_SERVICE_NAME_LEN_MAX + 1];
    char service_desc[BSA_HL_SERVICE_DESC_LEN_MAX + 1];
    char provider_name[BSA_HL_PROVIDER_NAME_LEN_MAX + 1];
} tBSA_HL_MSGID_SDP_QUERY_APP_EVT;

typedef struct
{
    UINT8 rec_index; /* Current record index */
    UINT8 mdep_index; /* Current MDEP index */
    tBSA_HL_MDEP_CFG_REG mdep_cfg;
} tBSA_HL_MSGID_SDP_QUERY_APP_MDEP_EVT;

/*
 * HL Open
 */
typedef tBSA_HL_OPEN tBSA_HL_MSGID_OPEN_CMD_REQ;
typedef tBSA_HL_OPEN_MSG tBSA_HL_MSGID_OPEN_EVT;

/*
 * HL Open Request/Response
 */
typedef tBSA_HL_OPEN_RSP tBSA_HL_MSGID_OPEN_RSP_CMD_REQ;
typedef tBSA_HL_OPEN_REQ_MSG tBSA_HL_MSGID_OPEN_REQ_EVT;

/*
 * HL Close
 */
typedef tBSA_HL_CLOSE tBSA_HL_MSGID_CLOSE_CMD_REQ;
typedef tBSA_HL_CLOSE_MSG tBSA_HL_MSGID_CLOSE_EVT;

/*
 * HL Reconnect
 */
typedef tBSA_HL_RECONNECT tBSA_HL_MSGID_RECONNECT_CMD_REQ;
typedef tBSA_HL_RECONNECT_MSG tBSA_HL_MSGID_RECONNECT_EVT;

/*
 * HL Delete
 */
typedef tBSA_HL_DELETE_MDL tBSA_HL_MSGID_DELETE_MDL_CMD_REQ;
typedef tBSA_HL_DELETE_MDL_MSG tBSA_HL_MSGID_DELETE_MDL_EVT;

/*
 * HL Confirm
 */
typedef tBSA_HL_SEND_DATA_CFM_MSG tBSA_HL_MSGID_SEND_DATA_CFM_EVT;


/*
 * HL Events
 */
typedef union
{
    tBSA_HL_MSGID_REGISTER_CMD_RSP register_rsp;
    tBSA_HL_MSGID_SDP_QUERY_APP_EVT sdp_query_app;
    tBSA_HL_MSGID_SDP_QUERY_APP_MDEP_EVT sdp_query_app_mdep;
    tBSA_HL_MSGID_OPEN_EVT open;
    tBSA_HL_MSGID_OPEN_REQ_EVT open_req;
    tBSA_HL_MSGID_CLOSE_EVT close;
    tBSA_HL_MSGID_RECONNECT_EVT reconnect;
    tBSA_HL_MSGID_DELETE_MDL_EVT delete_mdl;
    tBSA_HL_MSGID_SEND_DATA_CFM_EVT send_data_cfm;
} tBSA_HL_MSGID_EVT;


#endif


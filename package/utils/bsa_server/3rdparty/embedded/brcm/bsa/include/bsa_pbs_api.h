/*****************************************************************************
 **
 **  Name:           bsa_pbs_api.h
 **
 **  Description:    This is the public interface file for PBAP server part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_PBS_API_H
#define BSA_PBS_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"

/*****************************************************************************
 **  Constants and Type Definitions
 *****************************************************************************/
#ifndef BSA_PBS_DEBUG
#define BSA_PBS_DEBUG FALSE
#endif

#define BSA_PBS_FILENAME_MAX            512
#define BSA_PBS_SERVICE_NAME_LEN_MAX    150
#define BSA_PBS_ROOT_PATH_LEN_MAX       255
#define BSA_PBS_MAX_REALM_LEN           30
#define BSA_PBS_MAX_AUTH_KEY_SIZE       16

/* BSA PBAP Server callback events */
typedef enum
{
    BSA_PBS_OPEN_EVT, /* Connection Open*/
    BSA_PBS_CLOSE_EVT, /* Connection Closed */
    BSA_PBS_AUTH_EVT, /* Obex authentication request */
    BSA_PBS_ACCESS_EVT, /* Access to file or directory requested */
    BSA_PBS_OPER_CMPL_EVT,/* Phonebook access operation completed */
} tBSA_PBS_EVT;

/* BSA PBS Access Response */
#define BSA_PBS_ACCESS_ALLOW    0  /* Allow access to operation */
#define BSA_PBS_ACCESS_FORBID   1  /* Deny access to operation */
typedef UINT8 tBSA_PBS_ACCESS_TYPE;

/* BSA PBS Access operation type */
typedef enum
{
    BSA_PBS_OPER_NONE,
    BSA_PBS_OPER_PULL_PB, /* Pull the whole phonebook */
    BSA_PBS_OPER_SET_PB, /* Set phonebook directory */
    BSA_PBS_OPER_PULL_VCARD_LIST, /* Pull Vcard list */
    BSA_PBS_OPER_PULL_VCARD_ENTRY,/* Pull Vcard entry */
} tBSA_PBS_OPER;

/* BSA_PBS_AUTH_EVT callback event data */
typedef struct
{
    char userid[BSA_PBS_MAX_REALM_LEN];
    UINT8 userid_len;
    BOOLEAN userid_required;
} tBSA_PBS_AUTH_MSG;

/* BSA_PBS_ACCESS_EVT callback event data */
typedef struct
{
    char name[BSA_PBS_FILENAME_MAX]; /* ascii name of file or directory */
    BD_NAME dev_name; /* Name of device requesting access */
    tBSA_PBS_OPER oper; /* Operation attempting to gain access */
} tBSA_PBS_ACCESS_MSG;

/* BSA_PBS_OPER_CMPL_EVT callback event data */
typedef struct
{
        char name[BSA_PBS_FILENAME_MAX]; /* ascii name of file or directory */
        tBSA_STATUS status; /* success or failure */
} tBSA_PBS_OBJECT_MSG;

/* BSA_PBS_OPEN_EVT callback event data */
typedef struct
{
        BD_ADDR bd_addr;
} tBSA_PBS_OPEN_MSG;

/* BSA_PBS_CLOSE_EVT callback event data */
typedef struct
{
        tBSA_STATUS status;
} tBSA_PBS_CLOSE_MSG;

/* union of data associated with HD callback */
typedef union
{
    tBSA_PBS_OPEN_MSG open; /* BSA_PBS_OPEN_EVT */
    tBSA_PBS_CLOSE_MSG close; /* BSA_PBS_CLOSE_EVT */
    tBSA_PBS_AUTH_MSG auth; /* BSA_PBS_AUTH_EVT */
    tBSA_PBS_ACCESS_MSG access_req; /* BSA_PBS_ACCESS_EVT */
    tBSA_PBS_OBJECT_MSG oper_complete; /* BSA_PBS_OPER_CMPL_EVT */
} tBSA_PBS_MSG;

/* BSA PBS callback function */
typedef void ( tBSA_PBS_CBACK)(tBSA_PBS_EVT event, tBSA_PBS_MSG *p_data);

/*
 * Structures used to pass parameters to BSA API functions
 */

typedef struct
{
    tBSA_SEC_AUTH sec_mask;
    BOOLEAN enable_authen;
    char service_name[BSA_PBS_SERVICE_NAME_LEN_MAX];
    char root_path[BSA_PBS_ROOT_PATH_LEN_MAX];
    char realm[BSA_PBS_MAX_REALM_LEN];
    tBSA_PBS_CBACK *p_cback;
} tBSA_PBS_ENABLE;

typedef struct
{
        int dummy;
} tBSA_PBS_DISABLE;

typedef struct
{
    char password[BSA_PBS_MAX_AUTH_KEY_SIZE];
    char userid[BSA_PBS_MAX_REALM_LEN];
} tBSA_PBS_AUTHRSP;

typedef struct
{
    tBSA_PBS_OPER oper;
    tBSA_PBS_ACCESS_TYPE access;
    char name[BSA_PBS_FILENAME_MAX];
} tBSA_PBS_ACCESSRSP;

/*****************************************************************************
 **  External Function Declarations
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function            BSA_PbsEnableInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsEnableInit(tBSA_PBS_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function            BSA_PbsEnable
 **
 ** Description         This function enables PBAP Server and registers it with
 **                     the lower layers.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsEnable(tBSA_PBS_ENABLE *p_enable);

/*******************************************************************************
 **
 ** Function            BSA_PbsDisableInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDisableInit(tBSA_PBS_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function            BSA_PbsDisable
 **
 ** Description         This function is called when the host is about power down.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsDisable(tBSA_PBS_DISABLE *p_disable);

/*******************************************************************************
 **
 ** Function            BSA_PbsAuthRspInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAuthRspInit(tBSA_PBS_AUTHRSP *p_authrsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsAuthRsp
 **
 ** Description         This function is called to send an OBEX authentication challenge
 **                     to a connected OBEX client.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAuthRsp(tBSA_PBS_AUTHRSP *p_authrsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsAccessRspInit
 **
 ** Description         Initialize structure containing API parameters with default values
 **
 ** Parameters          Pointer to structure containing API parameters
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAccessRspInit(tBSA_PBS_ACCESSRSP *p_accessrsp);

/*******************************************************************************
 **
 ** Function            BSA_PbsAccessRsp
 **
 ** Description         This function sends a reply to an access request event.
 **
 ** Returns             void
 **
 *******************************************************************************/
tBSA_STATUS BSA_PbsAccessRsp(tBSA_PBS_ACCESSRSP *p_accessrsp);

#ifdef __cplusplus
}
#endif

#endif


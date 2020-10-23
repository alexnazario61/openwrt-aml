/*****************************************************************************
 **
 **  Name:           bsa_ft_int.h
 **
 **  Description:    Contains private BSA FT data
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_FTS_INT_H
#define BSA_FTS_INT_H

#include "bsa_ft_api.h"

/*
 * FT Server Message definition
 */
enum
{
    BSA_FTS_MSGID_ENABLE_CMD = BSA_FTS_MSGID_FIRST,
    BSA_FTS_MSGID_DISABLE_CMD,
    BSA_FTS_MSGID_CLOSE_CMD,
    BSA_FTS_MSGID_AUTH_RSP_CMD,
    BSA_FTS_MSGID_ACCESS_CMD,

    BSA_FTS_MSGID_LAST_CMD = BSA_FTS_MSGID_ACCESS_CMD,

    BSA_FTS_MSGID_OPEN_EVT,             /* Connection Open*/
    BSA_FTS_MSGID_CLOSE_EVT,            /* Connection Closed */
    BSA_FTS_MSGID_AUTH_EVT,             /* Report Event */
    BSA_FTS_MSGID_ACCESS_EVT,           /* Access requested Event */
    BSA_FTS_MSGID_PROGRESS_EVT,         /* Progress Event */
    BSA_FTS_MSGID_PUT_CMPL_EVT,         /* Put complete Event */
    BSA_FTS_MSGID_GET_CMPL_EVT,         /* Get complete Event */
    BSA_FTS_MSGID_DEL_CMPL_EVT,         /* Delete complete Event */

    BSA_FTS_MSGID_LAST_EVT = BSA_FTS_MSGID_DEL_CMPL_EVT
};


/*
 * Structures used for parameters (transport)
 */
/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_FT_MSGID_STATUS_RSP;


typedef tBSA_FT_MSGID_STATUS_RSP    tBSA_FTS_MSGID_ENABLE_CMD_RSP;
typedef tBSA_FT_MSGID_STATUS_RSP    tBSA_FTS_MSGID_DISABLE_CMD_RSP;
typedef tBSA_FT_MSGID_STATUS_RSP    tBSA_FTS_MSGID_CLOSE_CMD_RSP;
typedef tBSA_FT_MSGID_STATUS_RSP    tBSA_FTS_MSGID_AUTH_RSP_CMD_RSP;
typedef tBSA_FT_MSGID_STATUS_RSP    tBSA_FTS_MSGID_ACCESS_CMD_RSP;


/* Control Block  (used by client) */
typedef struct
{
    tBSA_FTS_CBACK *p_app_cback;
} tBSA_FTS_CB;

extern tBSA_FTS_CB bsa_fts_cb;


/*******************************************************************************
**
** Function       bsa_fts_event_hdlr
**
** Description    Handle FTS events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_fts_event_hdlr(int message_id, tBSA_FTS_MSG *p_data, int length);


/*
 * FT Client Message definition
 */
enum
{
    BSA_FTC_MSGID_ENABLE_CMD = BSA_FTC_MSGID_FIRST,
    BSA_FTC_MSGID_DISABLE_CMD,
    BSA_FTC_MSGID_OPEN_CMD,
    BSA_FTC_MSGID_CLOSE_CMD,
    BSA_FTC_MSGID_COPY_CMD,
    BSA_FTC_MSGID_MOVE_CMD,
    BSA_FTC_MSGID_SET_PERM_CMD,
    BSA_FTC_MSGID_PUT_FILE_CMD,
    BSA_FTC_MSGID_GET_FILE_CMD,
    BSA_FTC_MSGID_CH_DIR_CMD,
    BSA_FTC_MSGID_AUTH_RSP_CMD,
    BSA_FTC_MSGID_LIST_DIR_CMD,
    BSA_FTC_MSGID_MK_DIR_CMD,
    BSA_FTC_MSGID_REMOVE_CMD,
    BSA_FTC_MSGID_ABORT_CMD,

    BSA_FTC_MSGID_LAST_CMD = BSA_FTC_MSGID_ABORT_CMD,


    BSA_FTC_MSGID_OPEN_EVT      ,   /* Connection to peer is open. */
    BSA_FTC_MSGID_CLOSE_EVT     ,   /* Connection to peer closed. */
    BSA_FTC_MSGID_AUTH_EVT      ,   /* Request for Authentication key and user id */
    BSA_FTC_MSGID_LIST_EVT      ,   /* Event contains a directory entry (tBSA_FTC_LIST) */
    BSA_FTC_MSGID_PROGRESS_EVT  ,   /* Number of bytes read or written so far */
    BSA_FTC_MSGID_PUTFILE_EVT   ,   /* File Put complete */
    BSA_FTC_MSGID_GETFILE_EVT   ,   /* File Get complete */
    BSA_FTC_MSGID_BI_CAPS_EVT   ,   /* BIP imaging capabilities */
    BSA_FTC_MSGID_THUMBNAIL_EVT ,   /* BIP responder requests for the thumbnail version */
    BSA_FTC_MSGID_CHDIR_EVT     ,   /* Change Directory complete */
    BSA_FTC_MSGID_MKDIR_EVT     ,   /* Make Directory complete */
    BSA_FTC_MSGID_REMOVE_EVT    ,   /* Remove File/Directory complete */
    BSA_FTC_MSGID_PHONEBOOK_EVT ,   /* Report the Application Parameters for BSA_FtcGetPhoneBook response */
    BSA_FTC_MSGID_COPY_EVT      ,   /* Copy File complete */
    BSA_FTC_MSGID_MOVE_EVT      ,   /* Move File complete */
    BSA_FTC_MSGID_PERMISSION_EVT,   /* Set File permission complete */

    BSA_FTC_MSGID_LAST_EVT = BSA_FTC_MSGID_PERMISSION_EVT
};


/* Control Block  (used by client) */
typedef struct
{
    tBSA_FTC_CBACK *p_app_cback;
} tBSA_FTC_CB;

extern tBSA_FTC_CB bsa_ftc_cb;


/*******************************************************************************
**
** Function       bsa_ftc_event_hdlr
**
** Description    Handle FTC events
**
** Parameters
**
** Returns        None
**
*******************************************************************************/
void bsa_ftc_event_hdlr(int message_id, tBSA_FTC_MSG *p_data, int length);

#endif



/*****************************************************************************
 **
 **  Name:           bsa_pan_int.h
 **
 **  Description:    Contains private BSA data
 **
 **  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_PAN_INT_H
#define BSA_PAN_INT_H

#include "bsa_pan_api.h"

/*
 * PAN Message definition
 */
enum {
    BSA_PAN_MSGID_ENABLE_CMD = BSA_PAN_MSGID_FIRST,
    BSA_PAN_MSGID_DISABLE_CMD,
    BSA_PAN_MSGID_SET_ROLE_CMD,
    BSA_PAN_MSGID_OPEN_CMD,
    BSA_PAN_MSGID_CLOSE_CMD,
    BSA_PAN_MSGID_PFILT_CMD,
    BSA_PAN_MSGID_MFILT_CMD,

    BSA_PAN_MSGID_LAST_CMD = BSA_PAN_MSGID_MFILT_CMD,

    BSA_PAN_MSGID_OPEN_EVT,  /* Connection to peer is open. */
    BSA_PAN_MSGID_CLOSE_EVT, /* Connection to peer closed. */
    BSA_PAN_MSGID_PFILT_EVT,
    BSA_PAN_MSGID_MFILT_EVT,

    BSA_PAN_MSGID_LAST_EVT = BSA_PAN_MSGID_MFILT_EVT
};

/* Generic request without parameters */
typedef struct
{
    UINT8 dummy;
} tBSA_PAN_MSGID_DUMMY_REQ;

/* Generic message containing a status */
typedef struct
{
    tBSA_STATUS status;
} tBSA_PAN_MSGID_STATUS_RSP;

/*
 * Structures used for parameters (transport)
 */

/* Structures used for PAN */

typedef tBSA_PAN_MSGID_DUMMY_REQ    tBSA_PAN_MSGID_ENABLE_CMD_REQ;
typedef tBSA_PAN_MSGID_STATUS_RSP   tBSA_PAN_MSGID_ENABLE_CMD_RSP;

typedef tBSA_PAN_DISABLE            tBSA_PAN_MSGID_DISABLE_CMD_REQ;
typedef tBSA_PAN_MSGID_STATUS_RSP   tBSA_PAN_MSGID_DISABLE_CMD_RSP;

typedef tBSA_PAN_SET_ROLE           tBSA_PAN_MSGID_SET_ROLE_CMD_REQ;
typedef struct {
    tBSA_STATUS status;
    tBSA_PAN_ROLE role;
} tBSA_PAN_MSGID_SET_ROLE_CMD_RSP;

typedef tBSA_PAN_OPEN               tBSA_PAN_MSGID_OPEN_CMD_REQ;
typedef tBSA_PAN_MSGID_STATUS_RSP   tBSA_PAN_MSGID_OPEN_CMD_RSP;
typedef tBSA_PAN_OPEN_MSG           tBSA_PAN_MSGID_OPEN_EVT;

typedef tBSA_PAN_CLOSE              tBSA_PAN_MSGID_CLOSE_CMD_REQ;
typedef tBSA_PAN_MSGID_STATUS_RSP   tBSA_PAN_MSGID_CLOSE_CMD_RSP;
typedef tBSA_PAN_CLOSE_MSG          tBSA_PAN_MSGID_CLOSE_EVT;

typedef tBSA_PAN_PFILT              tBSA_PAN_MSGID_PFILT_CMD_REQ;
typedef tBSA_PAN_MSGID_STATUS_RSP   tBSA_PAN_MSGID_PFILT_CMD_RSP;
typedef tBSA_PAN_PFILT_MSG          tBSA_PAN_MSGID_PFILT_EVT;

typedef tBSA_PAN_MFILT              tBSA_PAN_MSGID_MFILT_CMD_REQ;
typedef tBSA_PAN_MSGID_STATUS_RSP   tBSA_PAN_MSGID_MFILT_CMD_RSP;
typedef tBSA_PAN_MFILT_MSG          tBSA_PAN_MSGID_MFILT_EVT;


typedef union
{
    tBSA_PAN_MSGID_ENABLE_CMD_REQ enable;
    tBSA_PAN_MSGID_DISABLE_CMD_REQ disable;
    tBSA_PAN_MSGID_SET_ROLE_CMD_REQ set_role;
    tBSA_PAN_MSGID_OPEN_CMD_REQ open;
    tBSA_PAN_MSGID_CLOSE_CMD_REQ close;
    tBSA_PAN_MSGID_PFILT_CMD_REQ pfilt;
    tBSA_PAN_MSGID_MFILT_CMD_REQ mfilt;
} tBSA_PAN_MSGID_CMD_REQ;

typedef union
{
    tBSA_PAN_MSGID_ENABLE_CMD_RSP enable;
    tBSA_PAN_MSGID_DISABLE_CMD_RSP disable;
    tBSA_PAN_MSGID_SET_ROLE_CMD_RSP set_role;
    tBSA_PAN_MSGID_OPEN_CMD_RSP open;
    tBSA_PAN_MSGID_CLOSE_CMD_RSP close;
    tBSA_PAN_MSGID_PFILT_CMD_RSP pfilt;
    tBSA_PAN_MSGID_MFILT_CMD_RSP mfilt;
} tBSA_PAN_MSGID_CMD_RSP;

typedef union
{
    tBSA_PAN_MSGID_OPEN_EVT open;
    tBSA_PAN_MSGID_CLOSE_EVT close;
    tBSA_PAN_MSGID_PFILT_EVT pfilt;
    tBSA_PAN_MSGID_MFILT_EVT mfilt;
} tBSA_PAN_MSGID_EVT;


#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function       bsa_pan_event_hdlr
 **
 ** Description    Handle PAN events
 **
 ** Parameters
 **
 ** Returns        None
 **
 *******************************************************************************/
void bsa_pan_event_hdlr(int message_id, tBSA_PAN_MSG *p_buffer, int length);

#ifdef __cplusplus
}
#endif

#endif

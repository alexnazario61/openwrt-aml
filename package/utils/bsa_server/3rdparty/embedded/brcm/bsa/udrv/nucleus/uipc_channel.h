/*****************************************************************************
 **
 **  Name:           uipc_channel.h
 **
 **  Description:    Nucleus Message Queue API for BSA UIPC
 **
 **  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_CHANNEL_H
#define UIPC_CHANNEL_H

#include "uipc_bsa.h"

#define UIPC_CHANNEL_BAD_DESC   (-1)
typedef INT32 tUIPC_CHANNEL_DESC;

/*#define UIPC_USE_SEMA */

/* The following definitions define the way used to get Rx data
 * An Rx data event can be received when rx data are available or the
 * Application can use the read API (in blocking or non blocking mode)
 */
#define UIPC_CHANNEL_MODE_CREATE                     0x01    /*  */
#define UIPC_CHANNEL_MODE_READ                       0x02    /*  */
#define UIPC_CHANNEL_MODE_WRITE                      0x04    /*  */

#define UIPC_CHANNEL_MODE_RX_DATA_EVT                 0x08    /* Callback */
#define UIPC_CHANNEL_MODE_RX_MASK                     0x08

#define UIPC_CHANNEL_MODE_TX_DATA_BLOCK               0x10    /* blocking */
#define UIPC_CHANNEL_MODE_TX_MASK                     0x10

typedef INT8 UIPC_CHANNEL_MODE;

typedef struct {
    BOOLEAN nb_open;
    NU_EVENT_GROUP OSEvtGrp;
    BUFFER_Q msg_queue; /* GKI msg queue */
    VOID *p_buff;
    tUIPC_CH_ID channel_id;
    tUIPC_RCV_CBACK *p_uipc_callback;
    NU_TASK read_thread;
    UIPC_CHANNEL_MODE mode;
    void *p_thread_buff;
} tUIPC_CHANNEL;


/* Events generated */
#define UIPC_CHANNEL_OPEN                       0x01
#define UIPC_CHANNEL_CLOSE                      0x02
#define UIPC_CHANNEL_DATA_RX                    0x03

/*******************************************************************************
 **
 ** Function         uipc_channel_init
 **
 ** Description      Init UIPC Server socket.
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int uipc_channel_init(void);

/*******************************************************************************
 **
 ** Function         uipc_channel_open
 **
 ** Description      open UIPC channel.
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
tUIPC_CHANNEL_DESC uipc_channel_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *uipc_callback,
        UIPC_CHANNEL_MODE mode);

/*******************************************************************************
 **
 ** Function         uipc_msg_queue_delete
 **
 ** Description      This function close a channel
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int uipc_channel_close(tUIPC_CHANNEL_DESC uipc_desc);

/*******************************************************************************
 **
 ** Function         uipc_channel_send
 **
 ** Description      This function send data on a channel
 **
 ** Parameters       buffer: Pointer to buffer to send
 **                  Length: length of buffer
 **
 ** Returns
 **
 *******************************************************************************/
BOOLEAN uipc_channel_send(tUIPC_CHANNEL_DESC uipc_desc, UINT8 *buffer, INT16 length);

#endif


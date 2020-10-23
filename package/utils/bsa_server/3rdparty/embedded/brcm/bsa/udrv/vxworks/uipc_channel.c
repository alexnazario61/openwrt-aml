/*****************************************************************************
 **
 **  Name:           uipc_channel.c
 **
 **  Description:    Vxworks MSG queue API for BSA UIPC
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bt_target.h"
#include "bsa_api.h"
#include "bsa_int.h"
#include "gki.h"
#include "uipc.h"
#include "uipc_thread.h"
#include "uipc_channel.h"

/* for the moment, only 3 msg Q channel (control:2, HH:1, DG:2) */
#define UIPC_CHANNEL_DESC_MAX   (2 + 1 + 2)

/* arbitrary max number of 1-byte event messages */
#define MAX_EVENT_MSGS 20

//#define DEBUG_UIPC_CHANNEL

typedef struct
{
    BOOLEAN nb_open;
    MSG_Q_ID msg_event_queue_id; /* VxWorks Q ID */
    BUFFER_Q msg_queue;    /* GKI msg queue */
    tUIPC_CH_ID channel_id;
    tUIPC_RCV_CBACK *p_uipc_callback;
    tTHREAD read_thread;
    UIPC_CHANNEL_MODE mode;
} tUIPC_CHANNEL;

tUIPC_CHANNEL uipc_channel_desc_cb[UIPC_CHANNEL_DESC_MAX];

/* Local functions */
static THREAD_ENTRY uipc_channel_read_task;
static int uipc_channel_notify(UINT16 event, tUIPC_CHANNEL_DESC channel_desc);

/*******************************************************************************
 **
 ** Function         uipc_channel_init
 **
 ** Description      Init UIPC.
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int uipc_channel_init(void)
{
#ifdef DEBUG_UIPC_CHANNEL
        APPL_TRACE_DEBUG0("uipc_channel_init");
#endif
    memset (uipc_channel_desc_cb, 0, sizeof(uipc_channel_desc_cb));
    return 0;
}

/*******************************************************************************
 **
 ** Function         uipc_channel_open
 **
 ** Description      Open UIPC channel.
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
tUIPC_CHANNEL_DESC uipc_channel_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *uipc_callback,
        UIPC_CHANNEL_MODE mode)
{
    INT32 index = 0;
    MSG_Q_ID msg_event_queue_id;

#ifdef DEBUG_UIPC_CHANNEL
        APPL_TRACE_DEBUG0("uipc_channel_open");
#endif

    /* Check Read/Write mode */
    if ((mode & UIPC_CHANNEL_MODE_READ) &&
        (mode & UIPC_CHANNEL_MODE_WRITE))
    {
        APPL_TRACE_ERROR0("uipc_msg_queue_open mode cannot be Rx and Tx");
        return UIPC_CHANNEL_BAD_DESC;
    }

    /* If open in Read, Rx mode must be specified */
    if ((mode & UIPC_CHANNEL_MODE_READ) &&
        ((mode & UIPC_CHANNEL_MODE_RX_MASK) == 0))
    {
        APPL_TRACE_ERROR0("uipc_msg_queue_open bad Rx mode not specified");
        return UIPC_CHANNEL_BAD_DESC;
    }

    if ((mode & UIPC_CHANNEL_MODE_READ) && (uipc_callback == NULL))
    {
        APPL_TRACE_ERROR1("uipc_msg_queue_open no callback for read mode", mode);
        return UIPC_CHANNEL_BAD_DESC;
    }

    /* If open in Write, Tx mode must be specified */
    if ((mode & UIPC_CHANNEL_MODE_WRITE) &&
        ((mode & UIPC_CHANNEL_MODE_TX_MASK) == 0))
    {
        APPL_TRACE_ERROR0("uipc_msg_queue_open bad Tx mode not specified");
        return UIPC_CHANNEL_BAD_DESC;
    }

    /* If this channel must be created */
    if (mode & UIPC_CHANNEL_MODE_CREATE)
    {
#ifdef DEBUG_UIPC_CHANNEL
        APPL_TRACE_DEBUG0("uipc_channel_open tries to create channel");
#endif
        /* Look for a free descriptor  */
        for (index = 0; index < UIPC_CHANNEL_DESC_MAX; index++)
        {
            /* If this one is free */
            if (uipc_channel_desc_cb[index].nb_open == 0)
            {
                /* Create a message event queue, event messages are 8 bits (1 bytes) */
                msg_event_queue_id = msgQCreate(MAX_EVENT_MSGS, sizeof(UINT8), MSG_Q_FIFO);  /* vxWorks call */
                if (msg_event_queue_id == NULL)
                {
                    APPL_TRACE_ERROR0("uipc_msg_queue_open enable to create VxWorks Msg Queue");
                    return UIPC_CHANNEL_BAD_DESC;
                }
                GKI_init_q(&uipc_channel_desc_cb[index].msg_queue);

                /* Let's use it */
                uipc_channel_desc_cb[index].nb_open = 1;
                uipc_channel_desc_cb[index].channel_id = channel_id;
                uipc_channel_desc_cb[index].mode = mode;
                uipc_channel_desc_cb[index].msg_event_queue_id = msg_event_queue_id;
#ifdef DEBUG_UIPC_CHANNEL
        APPL_TRACE_DEBUG1("uipc_channel_open channel:%d created", index);
#endif
                break;
            }
        }
     }
    else
    {
#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG0("uipc_channel_open tries to open an already created channel");
#endif
        /* Look for the descriptor which should be already created  */
        for (index = 0; index < UIPC_CHANNEL_DESC_MAX; index++)
        {
            /* If this one is in use and if the channel_id is the same */
            if ((uipc_channel_desc_cb[index].nb_open == 1) &&
                (uipc_channel_desc_cb[index].channel_id == channel_id))
            {
                /* We found a descriptor which match the channel_id */
                /* Let's check if the direction (read/write) is correct */

                /* If it was already opened in read mode and read mode is asked */
                if ((uipc_channel_desc_cb[index].mode & UIPC_CHANNEL_MODE_READ) &&
                    (mode & UIPC_CHANNEL_MODE_READ))
                {
                    continue; /* This is not the good descriptor, try next one */
                }
                /* If it was already opened in write mode and write mode is asked */
                if ((uipc_channel_desc_cb[index].mode & UIPC_CHANNEL_MODE_WRITE) &&
                    (mode & UIPC_CHANNEL_MODE_WRITE))
                {
                    continue; /* This is not the good descriptor, try next one */
                }
                /* OK, this is the good descritor, let's use it */
                /* opened on both side (read and write) */
                uipc_channel_desc_cb[index].nb_open += 1;
                /* update the mode */
                uipc_channel_desc_cb[index].mode |= mode;
#ifdef DEBUG_UIPC_CHANNEL
                APPL_TRACE_DEBUG1("uipc_channel_open channel:%d opened", index);
#endif
                break;
            }
        }
    }

    if (index >= UIPC_CHANNEL_DESC_MAX)
    {
        APPL_TRACE_ERROR0("uipc_msg_queue_open fail => max number of descriptor reached");
        return UIPC_CHANNEL_BAD_DESC;
    }

    /* If this end is connected in Read mode, let's create a thread in charge of read */
    if (mode & UIPC_CHANNEL_MODE_READ)
    {
#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG0("uipc_channel_open creating read task");
#endif
        if (uipc_create_thread(uipc_channel_read_task, 0, 16*1024,
                    &uipc_channel_desc_cb[index].read_thread,
                    (void *)index) < 0)
        {
            APPL_TRACE_ERROR0("uipc_msg_queue_open cannot create read thread");
            uipc_channel_desc_cb[index].nb_open -= 1;
            /* If we created a VxWorks msg queue => delete it */
            if (mode & UIPC_CHANNEL_MODE_CREATE)
            {
                msgQDelete(uipc_channel_desc_cb[index].msg_event_queue_id);
            }
            return UIPC_CHANNEL_BAD_DESC;
        }
        else
        {
#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG0("uipc_channel_open read task created");
#endif
            uipc_channel_desc_cb[index].p_uipc_callback = uipc_callback;
        }
    }

    /* If both end are connected => notify open event to the reader (via callback) */
    if (uipc_channel_desc_cb[index].nb_open == 2)
    {
        uipc_channel_notify(UIPC_CHANNEL_OPEN, index);
    }
    return index;
}

/*******************************************************************************
 **
 ** Function         uipc_msg_queue_delete
 **
 ** Description      This function stop
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int uipc_channel_close (tUIPC_CHANNEL_DESC channel_desc)
{
#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG1("uipc_channel_close channel:%d", channel_desc);
#endif
    if (channel_desc >= UIPC_CHANNEL_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_channel_close bad channel desc:%d", channel_desc);
        return UIPC_CHANNEL_BAD_DESC;
    }
    /* If both end are connected => notify close event to the reader (via callback) */
    if (uipc_channel_desc_cb[channel_desc].nb_open == 2)
    {
        uipc_channel_notify(UIPC_CHANNEL_CLOSE, channel_desc);
    }
    /* One end closed */
    uipc_channel_desc_cb[channel_desc].nb_open--;

    /* If no reader/writer */
    if (uipc_channel_desc_cb[channel_desc].nb_open == 0)
    {
        #ifdef DEBUG_UIPC_CHANNEL
            APPL_TRACE_DEBUG1("uipc_channel_close deleting :%d", channel_desc);
        #endif
        taskDelete(uipc_channel_desc_cb[channel_desc].read_thread);
        msgQDelete(uipc_channel_desc_cb[channel_desc].msg_event_queue_id);
        while (GKI_IS_QUEUE_EMPTY(&uipc_channel_desc_cb[channel_desc].msg_queue) == FALSE)
        {
            GKI_freebuf(GKI_dequeue(&uipc_channel_desc_cb[channel_desc].msg_queue));
        }
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         uipc_channel_send
 **
 ** Description      This function send data on a channel
 **
 ** Parameters       channel_desc
 **                  buffer: Pointer to buffer to send
 **                  Length: length of buffer
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
BOOLEAN uipc_channel_send(tUIPC_CHANNEL_DESC channel_desc,
        UINT8 *buffer, INT16 length)
{
    BT_HDR *p_msg;
    BOOLEAN ret_code = FALSE;
    UINT8 msg_buf;

#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG1("uipc_channel_send channel:%d", channel_desc);
#endif

    if (channel_desc >= UIPC_CHANNEL_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_channel_send bad channel desc:%d", channel_desc);
        return ret_code;
    }
    if ((buffer == NULL) || (length == 0))
    {
        APPL_TRACE_ERROR2("uipc_channel_send bad data ptr:%x len:%d", buffer, length);
        return ret_code;
    }

    /* Alloc GKI Buffer */
    if ((p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR) + length)) != NULL)
    {
        #ifdef DEBUG_UIPC_CHANNEL
            scru_dump_hex(buffer, "UIPC Data Tx", length >= 32 ? 32 : length, 0, 0);
        #endif
        /* Prepare buffer */
        p_msg->offset = 0;
        p_msg->event = UIPC_CHANNEL_DATA_RX;
        p_msg->len = length;
        p_msg->layer_specific = 0;
        memcpy((UINT8 *)(p_msg + 1), buffer, length); /* Copy data in buffer */
        /* Enqueue buffer */
        GKI_enqueue(&uipc_channel_desc_cb[channel_desc].msg_queue, p_msg);
        /* Send event to indicate that a buffer is ready to be read */
        msg_buf = 0x01;     /* This value is not used (Don't care) */
        if (msgQSend(uipc_channel_desc_cb[channel_desc].msg_event_queue_id, (char *)&msg_buf, sizeof(msg_buf), NO_WAIT, MSG_PRI_NORMAL))  /* vxWorks call */
        {
            /* This is not a killing error (the msg is already enqueued) */
            APPL_TRACE_ERROR0("uipc_channel_send msgQSend fail");
        }
        ret_code = TRUE;
    }
    else
    {
        APPL_TRACE_ERROR0("uipc_channel_send no GKI buffer");
    }
    return ret_code;
}

/*******************************************************************************
**
** Function         uipc_channel_read_task
**
** Description      Task in charge of reading the channel
**
** Parameters       channel desc
**
** Returns
**
*******************************************************************************/
static void uipc_channel_read_task(void *arg)
{
    tUIPC_CHANNEL_DESC channel_desc = (tUIPC_CHANNEL_DESC)arg;
    tUIPC_CHANNEL *p_channel = &uipc_channel_desc_cb[channel_desc];
    UINT8 msg_buf;
    BT_HDR *p_msg;

#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG1("uipc_channel_read_task started channel:%d", channel_desc);
#endif

    while(1)
    {
        /* Wait event on channel event queue */
        msgQReceive(p_channel->msg_event_queue_id, (char *)&msg_buf, sizeof(msg_buf), WAIT_FOREVER);
        /* We don't care of the event received */

#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG1("uipc_channel_read_task channel:%d msgQReceive returns", channel_desc);
#endif

        /* Dequeue all messages in queue */
        while((p_msg = (BT_HDR *)GKI_dequeue(&p_channel->msg_queue)) != NULL)
        {
#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG1("uipc_channel_read_task channel:%d GKI_dequeue returns a message", channel_desc);
    APPL_TRACE_DEBUG3("uipc_channel_read_task event:%d len:%d layer_spec:%x", p_msg->event, p_msg->len, p_msg->layer_specific);
    scru_dump_hex((UINT8 *)(p_msg + 1), "UIPC Data Rx", p_msg->len >= 32 ? 32 :  p_msg->len, 0, 0);
#endif
            if (p_channel->p_uipc_callback != NULL)
            {
                /* Call the callback */
                p_channel->p_uipc_callback(p_msg);
            }
            else
            {
                APPL_TRACE_ERROR1("uipc_msg_queue_read_task no callback for desc:%d", channel_desc);
                APPL_TRACE_ERROR0("uipc_msg_queue_read_task drop buffer");
                GKI_freebuf(p_msg);
            }
        }
    }
}


/*******************************************************************************
**
** Function         uipc_channel_notify
**
** Description      Notify Open/close event
**
** Parameters       channel desc
**
** Returns
**
*******************************************************************************/
static int uipc_channel_notify(UINT16 event, tUIPC_CHANNEL_DESC channel_desc)
{
    BT_HDR *p_msg;

#ifdef DEBUG_UIPC_CHANNEL
    APPL_TRACE_DEBUG1("uipc_channel_notify channel:%d", channel_desc);
#endif

    if (channel_desc >= UIPC_CHANNEL_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_channel_notify bad channel desc:%d", channel_desc);
        return UIPC_CHANNEL_BAD_DESC;
    }
    if (uipc_channel_desc_cb[channel_desc].p_uipc_callback == NULL)
    {
        APPL_TRACE_ERROR1("uipc_channel_notify no callback:%d", channel_desc);
        return UIPC_CHANNEL_BAD_DESC;
    }
    /* Alloc GKI Buffer */
    if ((p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR))) != NULL)
    {
        /* Prepare buffer */
        p_msg->offset = 0;
        p_msg->event = event;
        p_msg->len = 0;
        uipc_channel_desc_cb[channel_desc].p_uipc_callback(p_msg);
    }
    else
    {
        APPL_TRACE_ERROR0("uipc_channel_notify no GKI fuffer");
        return UIPC_CHANNEL_BAD_DESC;
    }
    return 0;
}

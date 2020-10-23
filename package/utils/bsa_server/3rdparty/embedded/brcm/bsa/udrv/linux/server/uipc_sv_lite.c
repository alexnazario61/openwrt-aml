/*****************************************************************************
 **
 **  Name:           uipc_sv_lite.c
 **
 **  Description:    Server API to conenct to Lite Stack
 **
 **  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>


#include "bt_target.h"
#include "bsa_api.h"
#include "bsa_int.h"
#include "uipc.h"
#include "uipc_thread.h"
#include "bsa_task.h"
#include "uipc_sv_lite.h"

#if BT_TRACE_PROTOCOL == TRUE
/* HCI and L2CAP Protocol Trace display functions */
#include "trace_api.h"
#include "bsa_trace.h"
#endif

extern char *userial_get_dev_name(void);

/*
 * Local definitions
 */
/* #define UIPC_SV_LITE_DEBUG */

#ifndef UIPC_SV_LITE_DEV_PATH
#define UIPC_SV_LITE_DEV_PATH "/proc/driver/btusb/"
#endif

#ifndef UIPC_SV_LITE_DEV_EXT
#define UIPC_SV_LITE_DEV_EXT "/lite"
#endif


#define UIPC_SV_LITE_HDR_SIZE     (sizeof(UINT16) + sizeof(UINT16))

#define UIPC_SV_SOCKPAIR_READ_DESC    0
#define UIPC_SV_SOCKPAIR_WRITE_DESC   1
#define UIPC_SV_SOCKPAIR_NB_DESC      2


typedef struct
{
    int fd;
    tUIPC_RCV_CBACK *p_cback;
    tTHREAD thread;
    BT_HDR *p_rx_msg;
    UINT8 rx_header[UIPC_SV_LITE_HDR_SIZE];
    UINT8 rx_header_len;
    UINT16 rx_len;  /* Decoded Rx Payload length */
    int sockpair[UIPC_SV_SOCKPAIR_NB_DESC]; /* sockerpair used to stop the rx thread */
} tUIPC_SV_LITE_CB;

/*
 * Local static function to UIPC callback
 */
static void uipc_sv_lite_read_task(void *arg);
#if (BT_TRACE_PROTOCOL == TRUE)
static char *uipc_sv_lite_evt_desc(UINT16 event);
#endif

/*
 * Globals
 */
static tUIPC_SV_LITE_CB uipc_sv_lite_cb;


/*******************************************************************************
 **
 ** Function         uipc_sv_lite_init
 **
 ** Description      Initialize the Lite UIPC channel.
 **
 ** Parameters       None.
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_lite_init(void)
{
    memset(&uipc_sv_lite_cb, 0, sizeof(uipc_sv_lite_cb));
    uipc_sv_lite_cb.fd = -1;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_lite_open
 **
 ** Description      Open the Lite UIPC channel.
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_cback: UIPC callback function
 **
 ** Returns          Boolean
 **
 *******************************************************************************/
BOOLEAN uipc_sv_lite_open(tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *p_cback)
{
    char filename[40];
    int fd;
    int status;
    char *p_userial_dev;

    if (!p_cback)
    {
        APPL_TRACE_ERROR0("uipc_sv_lite_open fail p_cback NULL");
        return FALSE;
    }

    if (channel_id != UIPC_CH_ID_0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_open wrong ChannelId:%d", channel_id);
        return FALSE;
    }

    if (uipc_sv_lite_cb.fd >= 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_open ChannelId:%d already open", channel_id);
        return FALSE;
    }

    /* Create a socketpair to be able to kill the reading thread */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, uipc_sv_lite_cb.sockpair) < 0)
    {
        APPL_TRACE_ERROR0("uipc_sv_lite_open: socketpair failed");
        return FALSE;
    }

    /* Get the Full Userial Device name (typically /dev/btusbx) */
    p_userial_dev = userial_get_dev_name();
    /* Search the last / */
    p_userial_dev = strrchr(p_userial_dev,'/');
    if (!p_userial_dev)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_open: no '/' found in device %s",
                userial_get_dev_name());
        return FALSE;
    }
    p_userial_dev++;        /* Skip the / */
    if (*p_userial_dev == '\0')
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_open: no device after last '/' found in device %s",
                userial_get_dev_name());
        return FALSE;
    }

    /* Build the Lite device full filename ('/proc/driver/btusb/' + device + '/lite') */
    snprintf(filename, sizeof(filename), "%s%s%s",
            UIPC_SV_LITE_DEV_PATH, p_userial_dev, UIPC_SV_LITE_DEV_EXT);

    APPL_TRACE_DEBUG1("uipc_sv_lite_open filename=%s", filename);

    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        APPL_TRACE_ERROR1("open failed errno=%d ", errno);
        close(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_READ_DESC]);
        close(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC]);
        return FALSE;
    }

    uipc_sv_lite_cb.fd = fd;
    uipc_sv_lite_cb.p_cback = p_cback;

    /* Create the thread in charge of reading the socket */
    status = uipc_thread_create(uipc_sv_lite_read_task, (UINT8 *)"UIPC Lite", NULL, 0,
            &uipc_sv_lite_cb.thread, NULL);
    if (status < 0)
    {
        close (fd);
        uipc_sv_lite_cb.fd = -1;
        uipc_sv_lite_cb.p_cback = NULL;
        close(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_READ_DESC]);
        close(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC]);
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         bsa_sv_stop_hl_server
 **
 ** Description      Close an HL UIPC channel.
 **
 ** Parameters       channel_id: UIPC channel id
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_lite_close(tUIPC_CH_ID channel_id)
{
    UINT8 data;

    if (channel_id != UIPC_CH_ID_0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_close wrong ChannelId:%d", channel_id);
        return;
    }

    if (uipc_sv_lite_cb.fd < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_close ChannelId:%d already closed", channel_id);
        return;
    }

    APPL_TRACE_DEBUG0("uipc_sv_lite_close");
    close(uipc_sv_lite_cb.fd);

    /* Write any value in the "write" side of the socketpair */
    /* This will trig the select and stop the thread */
    data = 0x00;
    if (write(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC], &data, 1) < 0)
    {
        APPL_TRACE_ERROR0("uipc_sv_lite_close: socketpair write failed");
    }

    uipc_sv_lite_cb.fd = -1;
    uipc_sv_lite_cb.p_cback = NULL;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_lite_send
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_msg: data to send
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sv_lite_send(tUIPC_CH_ID channel_id, UINT16 msg_evt, UINT8 *p_buf,
        UINT16 msg_len)
{
    UINT8 header[UIPC_SV_LITE_HDR_SIZE];
    UINT8 *p = header;
    ssize_t size;

#if (BT_TRACE_PROTOCOL == TRUE)
    APPL_TRACE_DEBUG4("uipc_sv_lite_send ChannelId:%d length=%d Event=%s (0x%X)",
            channel_id, msg_len, uipc_sv_lite_evt_desc(msg_evt), msg_evt);
#ifdef UIPC_SV_LITE_DEBUG
    scru_dump_hex (p_buf, "Tx Lite Data", msg_len, 0, 0);
#endif
#endif

    if (channel_id != UIPC_CH_ID_0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send wrong ChannelId:%d", channel_id);
        return FALSE;
    }

    if (uipc_sv_lite_cb.fd < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send ChannelId:%d not opened", channel_id);
        return FALSE;
    }

    /* Prepare Header (Length + Event) */
    UINT16_TO_STREAM(p, msg_len + sizeof(UINT16));
    UINT16_TO_STREAM(p, msg_evt);

    /* Write Header */
    size = write(uipc_sv_lite_cb.fd, header, UIPC_SV_LITE_HDR_SIZE);
    if (size < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send Header write error errno=%d", errno);
        return FALSE;
    }

    /* Write Payload */
    size = write(uipc_sv_lite_cb.fd, p_buf, msg_len);
    if (size < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send Payload write error errno=%d", errno);
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_lite_send_buf
 **
 ** Description      This function is used to send a data buffer to a client
 **
 ** Parameters       channel_id: UIPC channel id
 **                  p_msg: data to send
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN uipc_sv_lite_send_buf(tUIPC_CH_ID channel_id, BT_HDR *p_msg)
{
    UINT8 header[UIPC_SV_LITE_HDR_SIZE];
    UINT8 *p = header;
    ssize_t size;

#if (BT_TRACE_PROTOCOL == TRUE)
    APPL_TRACE_DEBUG4("uipc_sv_lite_send_buf ChannelId:%d length=%d Event=%s (0x%X)",
            channel_id, p_msg->len, uipc_sv_lite_evt_desc(p_msg->event), p_msg->event);
#endif

    if (!p_msg)
    {
        APPL_TRACE_ERROR0("uipc_sv_lite_send_buf no message");
        GKI_freebuf(p_msg);
        return FALSE;
    }

    if (channel_id != UIPC_CH_ID_0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send_buf wrong ChannelId:%d", channel_id);
        GKI_freebuf(p_msg);
        return FALSE;
    }

    if (uipc_sv_lite_cb.fd < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send_buf ChannelId:%d not opened", channel_id);
        GKI_freebuf(p_msg);
        return FALSE;
    }

#if (BT_TRACE_PROTOCOL == TRUE)
    /* Display protocol trace message */
    switch(p_msg->event)
    {
    case BT_EVT_TO_LM_HCI_CMD:
        DispHciCmd(p_msg);
        break;

    case BT_EVT_TO_LM_HCI_ACL:
    case BT_EVT_TO_LM_HCI_ACL | LOCAL_BLE_CONTROLLER_ID:
        DispL2CCmd(p_msg,FALSE);
        DispHciAclData (p_msg, FALSE);
        break;

    case BT_EVT_TO_LM_HCI_SCO:
        DispHciScoData (p_msg, FALSE);
        break;

    default:
#ifdef UIPC_SV_LITE_DEBUG
        scru_dump_hex ((UINT8 *)(p_msg + 1) + p_msg->offset, "Tx Lite Data", p_msg->len, 0, 0);
#endif
        break;
    }
#endif

    /* Prepare Header (Length + Event) */
    UINT16_TO_STREAM(p, p_msg->len + sizeof(UINT16));
    UINT16_TO_STREAM(p, p_msg->event);

    /* Write Header */
    size = write(uipc_sv_lite_cb.fd, header, UIPC_SV_LITE_HDR_SIZE);
    if (size < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send_buf Header write error errno=%d", errno);
        GKI_freebuf(p_msg);
        return FALSE;
    }

    /* Write Payload */
    size = write(uipc_sv_lite_cb.fd, (void *)(p_msg + 1) + p_msg->offset, p_msg->len);
    if (size < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_lite_send_buf Payload write error errno=%d", errno);
        GKI_freebuf(p_msg);
        return FALSE;
    }

    GKI_freebuf(p_msg);

    return TRUE;
}


/*******************************************************************************
 **
 ** Function           uipc_sv_lite_read_task
 **
 ** Description        Thread in charge of continuously reading data from UIPC Lite
 **
 ** Returns            None
 **
 *******************************************************************************/
static void uipc_sv_lite_read_task(void *arg)
{
    ssize_t read_len;
    UINT8 *p;
    UINT16 rx_event;
    BT_HDR *p_msg;
    fd_set socks_rd; /* Read Socket file descriptors we want to wake up for, using select() */
    int highest_fd; /* Highest #'d file descriptor, needed for select() */
    int rv;

    while(uipc_sv_lite_cb.fd >= 0)
    {
        /* FD_ZERO() clears out the fd_set called socks, so that
         it doesn't contain any file descriptors. */

        FD_ZERO(&socks_rd);

        /* FD_SET() adds the file descriptor "sock" to the fd_set,
         so that select() will return if a connection comes in
         on that socket (which means you have to do accept(), etc. */

        FD_SET(uipc_sv_lite_cb.fd, &socks_rd);

        /* Add the socketpair (read side) */
        FD_SET(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_READ_DESC], &socks_rd);

        /* Select needs the highest file descriptor */
        if (uipc_sv_lite_cb.fd > uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_READ_DESC])
        {
            highest_fd = uipc_sv_lite_cb.fd;
        }
        else
        {
            highest_fd = uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_READ_DESC];
        }

        rv = select(highest_fd + 1,
                &socks_rd, /* Read file descriptors */
                (fd_set *)NULL, /* Write file descriptors */
                (fd_set *)NULL, /* OOB file descriptors */
                NULL); /* No Timeout */

        if (BCM_UNLIKELY(rv < 0))
        {
            APPL_TRACE_ERROR1("uipc_sv_lite_read_task: select failed (%d)", rv);
            break;
        }

        /* Check if the select returned because of socketpair write */
        if (FD_ISSET(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_READ_DESC], &socks_rd))
        {
            APPL_TRACE_DEBUG0("uipc_sv_lite_read_task: Socketpair indication. Stop Read Thread");
            break;
        }

        /* If a full Header has not yet been received */
        if (uipc_sv_lite_cb.rx_header_len < UIPC_SV_LITE_HDR_SIZE)
        {
            /* Try to read, the missing part of, the header (1 to 4 bytes) */
            read_len = read(uipc_sv_lite_cb.fd,
                    &uipc_sv_lite_cb.rx_header[uipc_sv_lite_cb.rx_header_len],
                    UIPC_SV_LITE_HDR_SIZE - uipc_sv_lite_cb.rx_header_len);
            if (read_len < 0)
            {
                APPL_TRACE_ERROR1("uipc_sv_lite_read_task Header read error. errno=%d", errno);
                break;
            }

            uipc_sv_lite_cb.rx_header_len += read_len;

            /* If the buffer has been read */
            if (uipc_sv_lite_cb.rx_header_len == UIPC_SV_LITE_HDR_SIZE)
            {
                p = uipc_sv_lite_cb.rx_header;
                STREAM_TO_UINT16(uipc_sv_lite_cb.rx_len, p);
                STREAM_TO_UINT16(rx_event, p);

                uipc_sv_lite_cb.rx_len -= sizeof(UINT16);    /* Do not count Event Field */

                /* Allocate a GKI buffer in a the dedicated pool (Event, ACL, ...) */
                switch(rx_event)
                {
                case BT_EVT_TO_BTU_HCI_EVT:
                    p_msg = (BT_HDR *) GKI_getpoolbuf (HCI_CMD_POOL_ID);
                    break;

                case BT_EVT_BTU_IPC_ACL_EVT:
                    p_msg = (BT_HDR *) GKI_getpoolbuf (HCI_ACL_POOL_ID);
                    break;

                case BT_EVT_TO_LM_HCI_SCO:
                    p_msg = (BT_HDR *) GKI_getpoolbuf (HCI_SCO_POOL_ID);
                    break;

                default:
                    /* Use regular buffer for other events */
                    p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + uipc_sv_lite_cb.rx_len);
                    break;
                }

                if (p_msg == NULL)
                {
                    APPL_TRACE_ERROR2("uipc_sv_lite_read_task Unable to allocate buffer for Event=0x%x len=%d",
                            rx_event, sizeof(BT_HDR) + uipc_sv_lite_cb.rx_len);
                }
                else
                {
                    p_msg->event = rx_event;
                    p_msg->layer_specific = 0;
                    p_msg->offset = 0;
                    p_msg->len = 0;
                    uipc_sv_lite_cb.p_rx_msg = p_msg;
                }
            }
        }
        /* If Header already received */
        else
        {
            p_msg = uipc_sv_lite_cb.p_rx_msg;

            if (!p_msg)
            {
                APPL_TRACE_ERROR0("uipc_sv_lite_read_task No GKI buffer");
                break;
            }
            p = (UINT8 *)(p_msg + 1) + p_msg->len + p_msg->offset;

            /* Try to read, the missing part of, the payload */
            read_len = read(uipc_sv_lite_cb.fd, p, uipc_sv_lite_cb.rx_len);
            if (read_len < 0)
            {
                APPL_TRACE_ERROR1("uipc_sv_lite_read_task Payload read error. errno=%d", errno);
                break;
            }

            uipc_sv_lite_cb.rx_len -= read_len;
            p_msg->len += read_len;

            /* If the fuller has been fully received */
            if (uipc_sv_lite_cb.rx_len == 0)
            {
#if (BT_TRACE_PROTOCOL == TRUE)
                APPL_TRACE_DEBUG3("uipc_sv_lite_read_task Full buffer Event=%s (0x%X) length=%d received",
                        uipc_sv_lite_evt_desc(p_msg->event), p_msg->event, p_msg->len);
                /* Display protocol trace message */
                switch(p_msg->event)
                {
                case BT_EVT_TO_BTU_HCI_EVT:
                    DispHciEvt(p_msg);
                    break;

                case BT_EVT_BTU_IPC_ACL_EVT:
                    DispHciAclData(p_msg, TRUE);
                    DispL2CCmd(p_msg,TRUE);
                    break;

                case BT_EVT_TO_LM_HCI_SCO:
                    DispHciScoData(p_msg, TRUE);
                    break;

                case BT_EVT_TO_LM_DIAG:
                    DispLMDiagEvent(p_msg);
                    break;

                default:
#ifdef UIPC_SV_LITE_DEBUG
                    scru_dump_hex ((UINT8 *)(p_msg + 1) + p_msg->offset, "Rx Lite Data", p_msg->len, 0, 0);
#endif
                    break;
                }
#endif

                /* Call the registered callback with the received message */
                uipc_sv_lite_cb.p_cback(p_msg);

                uipc_sv_lite_cb.p_rx_msg = NULL;
                uipc_sv_lite_cb.rx_header_len = 0; /* Ready to receive a new header */
                p_msg = NULL;
            }
        }
    }
    close(uipc_sv_lite_cb.fd);
    close(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_READ_DESC]);
    close(uipc_sv_lite_cb.sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC]);
    uipc_sv_lite_cb.fd = -1;
    uipc_sv_lite_cb.p_cback = NULL;
    uipc_thread_stop(uipc_sv_lite_cb.thread);
}

#if (BT_TRACE_PROTOCOL == TRUE)
/*******************************************************************************
 **
 ** Function           uipc_sv_lite_evt_desc
 **
 ** Description        Get Lite Event description
 **
 ** Returns            String
 **
 *******************************************************************************/
static char *uipc_sv_lite_evt_desc(UINT16 event)
{
    switch(event)
    {
    case BT_EVT_TO_BTU_HCI_EVT:
        return "BT_EVT_TO_BTU_HCI_EVT";
    case BT_EVT_TO_BTU_HCI_SCO:
        return "BT_EVT_TO_BTU_HCI_SCO";
    case BT_EVT_TO_LM_HCI_CMD:
        return "BT_EVT_TO_LM_HCI_CMD";
    case BT_EVT_TO_LM_HCI_ACL:
        return "BT_EVT_TO_LM_HCI_ACL";
    case BT_EVT_TO_LM_DIAG:
        return "BT_EVT_TO_LM_DIAG";
    case BT_EVT_BTU_IPC_LOGMSG_EVT:
        return "BT_EVT_BTU_IPC_LOGMSG_EVT";
    case BT_EVT_BTU_IPC_ACL_EVT:
        return "BT_EVT_BTU_IPC_ACL_EVT";
    case BT_EVT_BTU_IPC_BTU_EVT:
        return "BT_EVT_BTU_IPC_BTU_EVT";
    case BT_EVT_BTU_IPC_L2C_EVT:
        return "BT_EVT_BTU_IPC_L2C_EVT";
    case BT_EVT_BTU_IPC_L2C_MSG_EVT:
        return "BT_EVT_BTU_IPC_L2C_MSG_EVT";
    case BT_EVT_BTU_IPC_BTM_EVT:
        return "BT_EVT_BTU_IPC_BTM_EVT";
    case BT_EVT_BTU_IPC_AVDT_EVT:
        return "BT_EVT_BTU_IPC_AVDT_EVT";
    case BT_EVT_BTU_IPC_SLIP_EVT:
        return "BT_EVT_BTU_IPC_SLIP_EVT";
    case BT_EVT_BTU_IPC_MGMT_EVT:
        return "BT_EVT_BTU_IPC_MGMT_EVT";
    case BT_EVT_BTU_IPC_BTTRC_EVT:
        return "BT_EVT_BTU_IPC_BTTRC_EVT";
    case BT_EVT_BTU_IPC_BURST_EVT:
        return "BT_EVT_BTU_IPC_BURST_EVT";
    default:
        return "Unknown";
    }
}
#endif

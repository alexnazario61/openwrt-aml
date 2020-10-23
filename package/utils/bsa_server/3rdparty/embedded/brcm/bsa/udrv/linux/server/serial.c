/*******************************************************************************
 **  Name:       serial.c
 **
 **  Description:
 **
 **  This file contains the universal driver wrapper for the serial drivers
 **
 **  Copyright (c) 2005-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *******************************************************************************/

#include <string.h>
#include "gki_int.h"
#include "userial.h"
#include "hcidefs.h"
#include "btu.h"
#include "bte.h"

#include <ctype.h>
#include <pthread.h>
#include <errno.h>
/* For strerror */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
/* for sock_addr_in */
#include <netinet/in.h>
/* for inet_pton */
#include <arpa/inet.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include "brcm_linux_ioctl.h"

#include "uipc_thread.h"

#if defined(__FreeBSD__)
#include <bluetooth.h>
#include <netgraph.h>
#endif

#define USERIAL_DEBUG FALSE

#if !defined(USERIAL_REOPEN_DELAY)
/* Time to sleep in ms between close and open of HCI port upon reopen */
#define USERIAL_REOPEN_DELAY 1000
#endif

#ifndef USERIAL_MAX_LOCAL_RX_BUF_SIZE
#define USERIAL_MAX_LOCAL_RX_BUF_SIZE (8*1024)
#endif

#define USERIAL_CACHE_TRIGGER_WM 4

#ifndef USERIAL_HCI_UART_DEFRAG_INCLUDED
#define USERIAL_HCI_UART_DEFRAG_INCLUDED FALSE
#endif

/* If set scheduler policy to SCHED_OTHER then should set scheduler priority to zero.*/
#if defined(BSA_SET_USERIAL_SCHED_PARAM) && (BSA_SET_USERIAL_SCHED_PARAM == TRUE)
#ifndef USERIAL_SCHED_POLICY
#define USERIAL_SCHED_POLICY SCHED_FIFO
#endif
#ifndef USERIAL_SCHED_PRIORITY
#define USERIAL_SCHED_PRIORITY 90
#endif
#endif

#define USERIAL_UART 0
#define USERIAL_BTUSB 1
#define USERIAL_BTSOCK 2
#define USERIAL_SOCKET 3

typedef struct
{
    /* Name of the opened device (for later usage) */
    char devname[20];
    /* Port number */
    tUSERIAL_PORT port;
    /* File descriptor of the serial device */
    int fd;
    /* Indicate what kind of device is opened */
    UINT8 devicetype;
    /* Thread to use for serial read wait */
    pthread_t read_thread;
    /* Open configuration */
    tUSERIAL_OPEN_CFG cfg;
    /* Serial device callback when data is received */
    tUSERIAL_CBACK *ser_cb;
    /* Pointer passed at init time and saved to override device name at open */
    UINT8 *p_device;
    /* read condition */
    pthread_cond_t read_cv;
    /* read critical section */
    pthread_mutex_t read_protection;
    UINT16 rx_offset;
    volatile UINT16 rx_len;
#if (USERIAL_HCI_UART_DEFRAG_INCLUDED == TRUE)
    UINT16 rx_len_pending;
#endif
    UINT8 rx_buf[USERIAL_MAX_LOCAL_RX_BUF_SIZE];
} tUSERIAL_CB;

static tUSERIAL_CB userial_cb;

#if defined(BSA_HW_HOT_RESET) && (BSA_HW_HOT_RESET == TRUE)
/* not used currently, but this will be used to hold sending BT packet */
BOOLEAN userial_flag_hw_hot_reset = FALSE;
#endif

static const UINT32 userial_baud_tbl[] =
{
    300,        /* USERIAL_BAUD_300          0 */
    600,        /* USERIAL_BAUD_600          1 */
    1200,       /* USERIAL_BAUD_1200         2 */
    2400,       /* USERIAL_BAUD_2400         3 */
    9600,       /* USERIAL_BAUD_9600         4 */
    19200,      /* USERIAL_BAUD_19200        5 */
    57600,      /* USERIAL_BAUD_57600        6 */
    115200,     /* USERIAL_BAUD_115200       7 */
    230400,     /* USERIAL_BAUD_230400       8 */
    460800,     /* USERIAL_BAUD_460800       9 */
    921600,     /* USERIAL_BAUD_921600       10 */
    1000000,    /* USERIAL_BAUD_1M           11 */
    1500000,    /* USERIAL_BAUD_1_5M         12 */
    2000000,    /* USERIAL_BAUD_2M           13 */
    3000000,    /* USERIAL_BAUD_3M           14 */
    4000000     /* USERIAL_BAUD_4M           15 */
};

BOOLEAN gki_chk_buf_damage(void *p_buf);

/* Table of HCI preamble sizes for the different HCI message types */
static const UINT8 userial_preamble_table[] =
{
    HCIC_PREAMBLE_SIZE,
    HCI_DATA_PREAMBLE_SIZE,
    HCI_SCO_PREAMBLE_SIZE,
    HCIE_PREAMBLE_SIZE
};

/*******************************************************************************
 **
 ** Function           userial_read_dev_usb
 **
 ** Description        Read data from a usb driver.
 **
 ** Output Parameter   None
 **
 ** Returns            Number of bytes actually read from the serial port and
 **                    copied into p_data.  This may be less than len.
 **                    or a negative value if error.
 **
 *******************************************************************************/
static int userial_read_dev_usb(UINT8 *p_data, UINT16 len)
{
    fd_set input;
    struct timeval timeout;
    int length;

    FD_ZERO(&input);
    FD_SET(userial_cb.fd, &input);

    /* Initialize the timeout structure */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    /* Do the select */
    length = select(userial_cb.fd + 1, &input, NULL, NULL, &timeout);

#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
    APPL_TRACE_DEBUG1("userial_read_dev_usb select=%d", length);
#endif
    /* See if there was an error, unlikely to happen */
    if (BCM_LIKELY(length > 0))
    {
        /* We have input */
        length = read(userial_cb.fd, p_data, len);
#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
        APPL_TRACE_DEBUG1("userial_read_dev_usb read=%d", length);
#endif
    }

    return length;
}

/*******************************************************************************
 **
 ** Function           userial_read_dev_uart
 **
 ** Description        Read data from a UART driver.
 **
 ** Output Parameter   None
 **
 ** Returns            Number of bytes actually read from the serial port and
 **                    copied into p_data.  This may be less than len.
 **                    or a negative value if error.
 **
 *******************************************************************************/
static int userial_read_dev_uart(UINT8 *p_data, UINT16 len)
{
    return read(userial_cb.fd, p_data, len);
}

/*******************************************************************************
 **
 ** Function           userial_read_dev_socket
 **
 ** Description        Read data from a socket.
 **
 ** Output Parameter   None
 **
 ** Returns            Number of bytes actually read from the socket and
 **                    copied into p_data.  This may be less than len.
 **                    or a negative value if error.
 **
 *******************************************************************************/
static int userial_read_dev_socket(UINT8 *p_data, UINT16 len)
{
    return recv(userial_cb.fd, p_data, len, 0);
}

/*******************************************************************************
 **
 ** Function           userial_read_dev
 **
 ** Description        Read data from a hci driver.
 **
 ** Output Parameter   None
 **
 ** Returns            Num bytes read or < 0 if error
 **
 *******************************************************************************/
static int userial_read_dev(void)
{
    int length;
#if (USERIAL_HCI_UART_DEFRAG_INCLUDED == TRUE)
    UINT16 pl_length = 0;
    UINT16 current_packet_start = 0;
#endif

#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
#if (USERIAL_HCI_UART_DEFRAG_INCLUDED == TRUE)
    APPL_TRACE_DEBUG2("userial_read_dev rx_len_pending %d, offset %d",
                    userial_cb.rx_len_pending, userial_cb.rx_offset);
#else
    APPL_TRACE_DEBUG1("userial_read_dev offset %d", userial_cb.rx_offset);
#endif
#endif

    /* start by clearing the offset in case Read occurs immediately
     * after setting rx_len */
    userial_cb.rx_offset = 0;

    switch (userial_cb.devicetype)
    {
    case USERIAL_BTUSB:
        length = userial_read_dev_usb(userial_cb.rx_buf, USERIAL_MAX_LOCAL_RX_BUF_SIZE);
        /* manage the unplug or kernel failure or timeout */
        if (length <= 0)
        {
            return length;
        }
        userial_cb.rx_len = length;
        break;

    case USERIAL_UART:
#if (USERIAL_HCI_UART_DEFRAG_INCLUDED == FALSE)
        length = userial_read_dev_uart(userial_cb.rx_buf, USERIAL_MAX_LOCAL_RX_BUF_SIZE);
        /* manage the unplug or kernel failure or timeout */
        if (length <= 0)
        {
            return length;
        }
        userial_cb.rx_len = length;
#else
        /* let's consolidate as many hci packet as rx_buf can hold */
        do
        {
            /* we could have read more than one frame last time */
            if (userial_cb.rx_len_pending == 0)
            {
                do
                {
                    /* read the header */
                    length = userial_read_dev_uart(&(userial_cb.rx_buf[userial_cb.rx_len]),
                            USERIAL_MAX_LOCAL_RX_BUF_SIZE-userial_cb.rx_len);

                    /* manage the unplug driver failure or timeout */
                    if (length <= 0)
                    {
                        return length;
                    }
                    userial_cb.rx_len += length;
#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
                    APPL_TRACE_DEBUG3("userial_read_dev length %d, rx_len %d, payload %d",
                                            length, userial_cb.rx_len, pl_length);
                    if (userial_cb.rx_len > USERIAL_MAX_LOCAL_RX_BUF_SIZE)
                        APPL_TRACE_ERROR1("userial_read_dev rx buffer overflow %d", userial_cb.rx_len);
#endif
                } while((userial_cb.rx_len < (pl_length+current_packet_start))
                        && (userial_cb.rx_len<USERIAL_MAX_LOCAL_RX_BUF_SIZE));
                /* To save CPU : read until at least one packet is received, assuming rx_buf can hold at leas one */

            }
            else
            {
                memcpy(userial_cb.rx_buf, &(userial_cb.rx_buf[userial_cb.rx_offset]), userial_cb.rx_len_pending);
                userial_cb.rx_len = userial_cb.rx_len_pending;
                userial_cb.rx_len_pending = 0;
            }

            /* pl_length = ch id length + hci header size */
            pl_length = 1 + userial_preamble_table[userial_cb.rx_buf[current_packet_start]-1];

            /* if we have a full header */
            while (userial_cb.rx_len >= (pl_length+current_packet_start))
            {
                /* let's calculate the payload + header size */
                switch (userial_cb.rx_buf[current_packet_start])
                {
                case 2:
                    pl_length += (userial_cb.rx_buf[current_packet_start+4] << 8) + userial_cb.rx_buf[current_packet_start+3];
                    break;
                case 3 :
                    pl_length += userial_cb.rx_buf[current_packet_start+3];
                    break;
                case 4 :
                    pl_length += userial_cb.rx_buf[current_packet_start+2];
                    break;
                default :
                    APPL_TRACE_ERROR1("userial_read_dev unknown packet %d",
                            userial_cb.rx_buf[0]);
                    return 0;
                }
#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
                APPL_TRACE_DEBUG3("userial_read_dev current_packet_start %d, pl_length %d, rx_len %d",
                        current_packet_start, pl_length, userial_cb.rx_len);
                APPL_TRACE_DEBUG5("[0x%x;0x%x;0x%x;0x%x;0x%x], ",
                        userial_cb.rx_buf[current_packet_start], userial_cb.rx_buf[current_packet_start+1], userial_cb.rx_buf[current_packet_start+2],
                        userial_cb.rx_buf[current_packet_start+3], userial_cb.rx_buf[current_packet_start+4]);

#endif

                if (userial_cb.rx_len == (pl_length+current_packet_start))
                {
                    /* We have read an entire num of hci frame */
                    break;
                }
                if (userial_cb.rx_len > (pl_length+current_packet_start))
                {
                    /* we have read more than one frame */
                    current_packet_start += pl_length;
                    /* get the next packet header : pl_length = ch id length + hci header size */
                    pl_length = 1 + userial_preamble_table[userial_cb.rx_buf[current_packet_start]-1];
                }
            }
        } while ((userial_cb.rx_len < (pl_length+current_packet_start)) &&
                 (userial_cb.rx_len<USERIAL_MAX_LOCAL_RX_BUF_SIZE));

        /* if there is a fragmented frame at the end of rx_buf i.e. (rx_len == USERIAL_MAX_LOCAL_RX_BUF_SIZE)
         * let's remember its length and only report the entire number of hci frame */
        if (userial_cb.rx_len < (pl_length+current_packet_start))
        {
            userial_cb.rx_len_pending = userial_cb.rx_len - current_packet_start;
            userial_cb.rx_len = current_packet_start;
        }

#endif
        break;

    default:
        length = userial_read_dev_socket(userial_cb.rx_buf, USERIAL_MAX_LOCAL_RX_BUF_SIZE);
        /* manage the unplug or kernel failure or timeout */
        if (length <= 0)
        {
            return length;
        }
        userial_cb.rx_len = length;
        break;
    }

    return userial_cb.rx_len;
}

/*******************************************************************************
 **
 ** Function           USERIAL_GetLineSpeed
 **
 ** Description        This function convert USERIAL baud to line speed.
 **
 ** Output Parameter   None
 **
 ** Returns            line speed
 **
 *******************************************************************************/
UDRV_API UINT32 USERIAL_GetLineSpeed(UINT8 baud)
{
    if (baud <= USERIAL_BAUD_4M)
        return (userial_baud_tbl[baud - USERIAL_BAUD_300]);
    else
        return 0;
}

/*******************************************************************************
 **
 ** Function           USERIAL_GetBaud
 **
 ** Description        This function convert line speed to USERIAL baud.
 **
 ** Output Parameter   None
 **
 ** Returns            line speed
 **
 *******************************************************************************/
UDRV_API UINT8 USERIAL_GetBaud(UINT32 line_speed)
{
    UINT8 i;
    for (i = USERIAL_BAUD_300; i <= USERIAL_BAUD_921600; i++)
    {
        if (userial_baud_tbl[i - USERIAL_BAUD_300] == line_speed)
            return i;
    }

    return USERIAL_BAUD_AUTO;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Init
 **
 ** Description        This function initializes the  serial driver.
 **
 ** Output Parameter   None
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
UDRV_API void USERIAL_Init(void *p_opaque)
{
    memset(&userial_cb, 0, sizeof(userial_cb));
    userial_cb.fd = -1;
    userial_cb.ser_cb = NULL;

    /* Save the name of the device to use */
    userial_cb.p_device = p_opaque;
}

/*******************************************************************************
 **
 ** Function           userial_read_thread
 **
 ** Description        Thread in charge of continuously reading data from HCI
 **
 ** Output Parameter
 **
 ** Returns            nothing
 **
 *******************************************************************************/
static void userial_read_thread(void *arg)
{
    int status = 0;

    APPL_TRACE_DEBUG1("userial_read_thread started %p", userial_cb.ser_cb);

    /* in case data came in before we could start reading */
    if ((NULL != userial_cb.ser_cb) && (userial_cb.fd != -1) && (userial_cb.rx_len != 0))
    {
        userial_cb.ser_cb(userial_cb.port, USERIAL_RX_READY_EVT, NULL);

        /* pause the read thread until Userial read */
        pthread_mutex_lock(&userial_cb.read_protection);
        while (userial_cb.rx_len)
        {
            pthread_cond_wait(&userial_cb.read_cv, &userial_cb.read_protection);
        }
        pthread_mutex_unlock(&userial_cb.read_protection);
    }

    while (BCM_LIKELY(userial_cb.fd != -1))
    {
        /* no thread can read the buffer while being filled here because
         * now rx_len = 0. So protection is not required
         */
        status = userial_read_dev();

        if (BCM_UNLIKELY(status < 0))
        {
#if defined(BSA_HW_HOT_RESET) && (BSA_HW_HOT_RESET == TRUE)
            APPL_TRACE_DEBUG0("Wait btusb reinsert!!");
            userial_flag_hw_hot_reset = TRUE;
            APPL_TRACE_DEBUG1("userial_read_thread Turn ON userial_flag_hw_hot_reset %d", userial_flag_hw_hot_reset);
            APPL_TRACE_DEBUG0("Stop while loop");
            break;
#else
            APPL_TRACE_DEBUG1("userial_read_thread userial_read_dev return: %d", status);
            APPL_TRACE_DEBUG0("Kill BSA Server");
            GKI_exit_task(0);
#endif
        }
        /* See if there was an error, optimize the code for the else case */
        if (BCM_UNLIKELY(userial_cb.rx_len == 0))
        {
#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
            if (status == 0)
            {
                APPL_TRACE_DEBUG0("Got a select TIMEOUT");
            }
            else
            {
                APPL_TRACE_ERROR2("userial_read_thread select failed status %d, fd %d", status, userial_cb.fd);
            }
#endif
        }
        else
        {
            /* if we have a call back */
            if (BCM_LIKELY(NULL != userial_cb.ser_cb))
            {
#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
                APPL_TRACE_DEBUG0("userial_read_thread calling cback");
#endif
                userial_cb.ser_cb(userial_cb.port, USERIAL_RX_READY_EVT, NULL);

                /* pause the read thread until Userial read */
                pthread_mutex_lock(&userial_cb.read_protection);
                while (userial_cb.rx_len)
                {
                    pthread_cond_wait(&userial_cb.read_cv, &userial_cb.read_protection);
                }
                pthread_mutex_unlock(&userial_cb.read_protection);

#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
                APPL_TRACE_DEBUG0("userial_read_thread unlocked");
#endif
            }
        }
    }

    APPL_TRACE_DEBUG0("userial_read_thread exited");
    uipc_thread_stop(userial_cb.read_thread);

    APPL_TRACE_DEBUG0("uipc_thread_stop and call USERIAL_Reopen");
    USERIAL_Reopen();
}

/*******************************************************************************
 **
 ** Function           serial_configure
 **
 ** Description        Configure the opened port with new parameters
 **
 ** Output Parameter   None
 **
 ** Returns            TRUE if failed, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN serial_configure(void)
{
    UINT32 baud;
    UINT8 data_bits;
    UINT16 parity;
    UINT8 stop_bits;
    struct termios termios;

    switch(userial_cb.cfg.baud)
    {
    case USERIAL_BAUD_600:
        baud = B600;
        break;
    case USERIAL_BAUD_1200:
        baud = B1200;
        break;
    case USERIAL_BAUD_9600:
        baud = B9600;
        break;
    case USERIAL_BAUD_19200:
        baud = B19200;
        break;
    case USERIAL_BAUD_57600:
        baud = B57600;
        break;
    case USERIAL_BAUD_115200:
        baud = B115200;
        break;
    case USERIAL_BAUD_230400:
        baud = B230400;
        break;
    case USERIAL_BAUD_460800:
        baud = B460800;
        break;
    case USERIAL_BAUD_921600:
        baud = B921600;
        break;
#if defined(B1000000)
    case USERIAL_BAUD_1M:
        baud = B1000000;
        break;
#endif
#if defined(B2000000)
    case USERIAL_BAUD_2M:
        baud = B2000000;
        break;
#endif
#if defined(B3000000)
    case USERIAL_BAUD_3M:
        baud = B3000000;
        break;
#endif
#if defined(B4000000)
    case USERIAL_BAUD_4M:
        baud = B4000000;
        break;
#endif
    default:
        APPL_TRACE_ERROR1("ERROR : serial_configure bad baudrate:%d", userial_cb.cfg.baud);
        return TRUE;
    }

    if(userial_cb.cfg.fmt & USERIAL_DATABITS_8)
        data_bits = CS8;
    else if(userial_cb.cfg.fmt & USERIAL_DATABITS_7)
        data_bits = CS7;
    else if(userial_cb.cfg.fmt & USERIAL_DATABITS_6)
        data_bits = CS6;
    else if(userial_cb.cfg.fmt & USERIAL_DATABITS_5)
        data_bits = CS5;
    else
    {
        APPL_TRACE_ERROR1("ERROR: serial_configure bad size format:0x%x", userial_cb.cfg.fmt);
        return TRUE;
    }

    if(userial_cb.cfg.fmt & USERIAL_PARITY_NONE)
        parity = 0;
    else if(userial_cb.cfg.fmt & USERIAL_PARITY_EVEN)
        parity = PARENB;
    else if(userial_cb.cfg.fmt & USERIAL_PARITY_ODD)
        parity = (PARENB | PARODD);
    else
    {
        APPL_TRACE_ERROR1("ERROR: serial_configure bad parity format:0x%x", userial_cb.cfg.fmt);
        return TRUE;
    }

    if(userial_cb.cfg.fmt & USERIAL_STOPBITS_1)
        stop_bits = 0;
    else if(userial_cb.cfg.fmt & USERIAL_STOPBITS_2)
        stop_bits = CSTOPB;
    else
    {
        APPL_TRACE_ERROR1("ERROR: serial_configure bad stop format:0x%x", userial_cb.cfg.fmt);
        return TRUE;
    }

    tcflush(userial_cb.fd, TCIOFLUSH);

    tcgetattr(userial_cb.fd, &termios);

    /* Configure in default raw mode */
    cfmakeraw(&termios);

    /* Clear out what can be overriden */
    termios.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);

    termios.c_cflag |= parity | data_bits | stop_bits;

    if (userial_cb.cfg.fc == USERIAL_FC_HW)
    {
        termios.c_cflag |= CRTSCTS;
    }

    tcsetattr(userial_cb.fd, TCSANOW, &termios);

    tcflush(userial_cb.fd, TCIOFLUSH);

    tcsetattr(userial_cb.fd, TCSANOW, &termios);

    tcflush(userial_cb.fd, TCIOFLUSH);
    tcflush(userial_cb.fd, TCIOFLUSH);

    cfsetospeed(&termios, baud);
    cfsetispeed(&termios, baud);
    tcsetattr(userial_cb.fd, TCSANOW, &termios);
    return FALSE;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Open
 **
 ** Description        Open the indicated serial port with the given configuration
 **
 ** Output Parameter   None
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
UDRV_API void USERIAL_Open(tUSERIAL_PORT port, tUSERIAL_OPEN_CFG *p_cfg, tUSERIAL_CBACK *p_cback)
{
    struct sched_param param;
    int policy;
    int flags;
    int index = 0;
#if defined(__FreeBSD__)
    struct sockaddr_hci addr;
    int dir;
    int csock;
    char path[sizeof(userial_cb.devname) + 1];
    struct ngm_connect con;
#else
    struct sockaddr_in addr;
#endif

    /* Save the open parameters */
    userial_cb.port = port;
    userial_cb.cfg = *p_cfg;
    userial_cb.ser_cb = p_cback;

    if (userial_cb.p_device != NULL)
    {
        strncpy(userial_cb.devname, (char*)userial_cb.p_device, sizeof(userial_cb.devname));
        userial_cb.devname[sizeof(userial_cb.devname)-1] = 0;
    }
    else
#if defined(BSA_SERIAL_DEV_NAME)
    {
        strncpy(userial_cb.devname, BSA_SERIAL_DEV_NAME, sizeof(userial_cb.devname));
        userial_cb.devname[sizeof(userial_cb.devname)-1] = 0;
    }
#else /* defined(BSA_SERIAL_DEV_NAME) */
    {
#if defined(__CYGWIN__)
        sprintf(userial_cb.devname, "/dev/com%d", port);
#elif defined(__FreeBSD__)
        sprintf(userial_cb.devname, "ubt%dhci", port);
#else
        sprintf(userial_cb.devname, "/dev/ttyS%d", port);
#endif
    }
#endif /* defined(BSA_SERIAL_DEV_NAME) */

    /* Check the device type */
    if (strstr(userial_cb.devname, "btusb") != NULL)
    {
        userial_cb.devicetype = USERIAL_BTUSB;
    }
    else if (strstr(userial_cb.devname, "ubt") != NULL)
    {
        userial_cb.devicetype = USERIAL_BTSOCK;

#if defined(__FreeBSD__)
        if (NgMkSockNode(NULL, &csock, NULL) < 0)
        {
            APPL_TRACE_DEBUG2("ERROR: NgMkSockNode errno %d(%s)",
                errno, strerror(errno));
            return;
        }
        sprintf(path, "%s:", userial_cb.devname);
        strcpy(con.path, "btsock_hci_raw:");
        strcpy(con.ourhook, "hook");
        strcpy(con.peerhook, "raw");
        if (NgSendMsg(csock, path, NGM_GENERIC_COOKIE, NGM_CONNECT,
            &con, sizeof(con)) < 0)
        {
            if (errno != EEXIST)
            {
                APPL_TRACE_DEBUG2("ERROR: NgSendMsg errno %d(%s)", errno,
                    strerror(errno));
                close(csock);
                return;
            }
        }
        close(csock);
#endif
    }
    else if (strstr(userial_cb.devname, "sock") != NULL)
    {
        userial_cb.devicetype = USERIAL_SOCKET;
    }
    else
    {
        userial_cb.devicetype = USERIAL_UART;
    }

    if (userial_cb.devicetype == USERIAL_BTUSB)
    {
        flags = O_RDWR | O_NOCTTY | O_NONBLOCK;
    }
    else
    {
        flags = O_RDWR;
    }

    APPL_TRACE_DEBUG1("Bluetooth port used:%s", userial_cb.devname);

    index = 0;
    while (1)
    {
        /* Try to open the device depending on the device type */
        if (userial_cb.devicetype == USERIAL_BTSOCK)
        {
#if defined(__FreeBSD__)
            userial_cb.fd = socket(PF_BLUETOOTH, SOCK_RAW, BLUETOOTH_PROTO_HCI);
#else
            userial_cb.fd = -1;
#endif
        }
        else if (userial_cb.devicetype == USERIAL_SOCKET)
        {
#if defined(__FreeBSD__)
            userial_cb.fd = -1;
#else
            userial_cb.fd = socket(PF_INET, SOCK_STREAM, 0);
            if (userial_cb.fd < 0)
            {
                APPL_TRACE_ERROR0("ERROR: socket failed");
            }
            else
            {
                /* Connect to the server */
                addr.sin_family = PF_INET;
                addr.sin_port = htons(54321);
                inet_pton(PF_INET, "127.0.0.1", &addr.sin_addr);

                if (-1 == connect(userial_cb.fd, (const void *)&addr, sizeof(addr)))
                {
                    APPL_TRACE_ERROR0("ERROR: connect failed");
                    close(userial_cb.fd);
                    userial_cb.fd = -1;
                }
            }
#endif
        }
        else
        {
            userial_cb.fd = open(userial_cb.devname, flags);
        }
        if (userial_cb.fd < 0)
        {
            if (index++ == 10)
            {
                APPL_TRACE_ERROR3("ERROR: open(%s) failed(%d), errno=%d", userial_cb.devname, userial_cb.fd, errno);
                index = 0;
            }
            GKI_delay(100);
            continue;
        }

        if (userial_cb.devicetype == USERIAL_BTSOCK)
        {
#if defined(__FreeBSD__)
            memset(&addr, 0, sizeof(addr));
            addr.hci_len = sizeof(addr);
            addr.hci_family = AF_BLUETOOTH;
            strncpy(addr.hci_node, userial_cb.devname, sizeof(addr.hci_node));
            if (bind(userial_cb.fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                APPL_TRACE_ERROR1("ERROR: bind errno %d", errno);
                close(userial_cb.fd);
                GKI_delay(100);
                continue;
            }
            if (connect(userial_cb.fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            {
                APPL_TRACE_ERROR1("ERROR: connect errno %d", errno);
                close(userial_cb.fd);
                GKI_delay(100);
                continue;
            }
            dir = TRUE;
#define SO_HCI_RAW_TUNNEL 0xFF /* turn on/off HCI tunnel mode */
            if (setsockopt(userial_cb.fd, SOL_HCI_RAW, SO_HCI_RAW_TUNNEL,
                   (const void *)&dir, sizeof(dir)) < 0)
            {
                APPL_TRACE_ERROR1("ERROR: setsockopt errno %d", errno);
                close(userial_cb.fd);
                GKI_delay(100);
                continue;
            }
#endif
        }
        /* Completed successfully */
        break;
    }
    if (userial_cb.devicetype == USERIAL_UART)
    {
        if (serial_configure())
        {
            APPL_TRACE_ERROR0("ERROR: serial_configure failed");
            USERIAL_Close(port);
            return;
        }
    }

    /* Create the HCI Read thread */
    if (uipc_thread_create(userial_read_thread, (UINT8 *)"hci_rx",
           NULL, 0, &userial_cb.read_thread, NULL) < 0)
    {
        APPL_TRACE_ERROR0("ERROR: uipc_thread_create failed!");
        return;
    }

    if (pthread_getschedparam(userial_cb.read_thread, &policy, &param) == 0)
    {
        policy = SCHED_FIFO;
        param.sched_priority = 90;
#if defined(BSA_SET_USERIAL_SCHED_PARAM) && (BSA_SET_USERIAL_SCHED_PARAM == TRUE)
        policy = USERIAL_SCHED_POLICY;
        param.sched_priority = USERIAL_SCHED_PRIORITY;
#endif
        pthread_setschedparam(userial_cb.read_thread, policy, &param);
    }

    if (pthread_cond_init(&userial_cb.read_cv, NULL) < 0)
    {
        APPL_TRACE_ERROR0("ERROR: pthread_cond_init failed!");
    }

    if (pthread_mutex_init(&userial_cb.read_protection, NULL) < 0)
    {
        APPL_TRACE_ERROR0("ERROR: pthread_mutex_init failed!");
    }

#if defined(BSA_HW_HOT_RESET) && (BSA_HW_HOT_RESET == TRUE)
    userial_flag_hw_hot_reset = FALSE;
    APPL_TRACE_DEBUG1("Turn off userial_flag_hw_hot_reset %d", userial_flag_hw_hot_reset);
#endif

#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
    APPL_TRACE_DEBUG0("Leaving USERIAL_Open");
#endif
}

/*******************************************************************************
 **
 ** Function           USERIAL_Reopen
 **
 ** Description        Properly close and re-open the current USERIAL interface
 **
 ** Output Parameter   None
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
void USERIAL_Reopen(void)
{
    tUSERIAL_PORT saved_port;
    tUSERIAL_OPEN_CFG saved_cfg;
    tUSERIAL_CBACK *saved_cb;
    APPL_TRACE_DEBUG0("USERIAL_Reopen enter");

    saved_port = userial_cb.port;
    saved_cfg = userial_cb.cfg;
    saved_cb = userial_cb.ser_cb;

    /* Close the port */
    USERIAL_Close(userial_cb.port);

    /* Wait for a platform dependent amount of time */
    APPL_TRACE_DEBUG0("USERIAL_Reopen sleeping");
    GKI_delay(USERIAL_REOPEN_DELAY);

    APPL_TRACE_DEBUG0("USERIAL_Reopen start reopening port");

    /* Open the port */
    USERIAL_Open(saved_port, &saved_cfg, saved_cb);

    APPL_TRACE_DEBUG0("USERIAL_Reopen exit");
}

/*******************************************************************************
 **
 ** Function           USERIAL_Read
 **
 ** Description        Read data from a serial port using byte buffers.
 **
 ** Output Parameter   None
 **
 ** Returns            Number of bytes actually read from the serial port and
 **                    copied into p_data.  This may be less than len.
 **
 *******************************************************************************/
UDRV_API UINT16 USERIAL_Read(tUSERIAL_PORT port, UINT8 *p_data, UINT16 len)
{
    UINT16 cpy_len = 0;
#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
    APPL_TRACE_DEBUG3("USERIAL_Read ++ len: %d, fd %d, rx_len %d",
            len, userial_cb.fd, userial_cb.rx_len);
#endif

    /* we expect (userial_cb.fd == -1) to be false most of the time */
    if (BCM_UNLIKELY(userial_cb.fd == -1))
    {
        APPL_TRACE_WARNING1("USERIAL_Read(%d) -> port close", port);
        return 0;
    }

    if (userial_cb.rx_len)
    {
        /* cpy_len = min(len, userial_cb.rx_len) */
        cpy_len = (len < userial_cb.rx_len) ? len : userial_cb.rx_len;
        memcpy(p_data, &(userial_cb.rx_buf[userial_cb.rx_offset]), cpy_len);
        userial_cb.rx_len -= cpy_len;
        userial_cb.rx_offset += cpy_len;
        if (userial_cb.rx_len == 0)
        {
            pthread_mutex_lock(&userial_cb.read_protection);
            pthread_cond_signal(&userial_cb.read_cv);
            pthread_mutex_unlock(&userial_cb.read_protection);
        }
    }
    else /* (userial_cb.rx_len==0) */
    {
#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
        APPL_TRACE_DEBUG0("USERIAL_Read unlock");
#endif
        pthread_mutex_lock(&userial_cb.read_protection);
        pthread_cond_signal(&userial_cb.read_cv);
        pthread_mutex_unlock(&userial_cb.read_protection);
    }

#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
    APPL_TRACE_DEBUG1("USERIAL_Read %d --", cpy_len);
#endif
    return cpy_len;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Readbuf
 **
 ** Description        Read data from a serial port using GKI buffers.
 **
 ** Output Parameter   Pointer to a GKI buffer which contains the data.
 **
 ** Returns            Nothing
 **
 ** Comments           The caller of this function is responsible for freeing the
 **                    GKI buffer when it is finished with the data.  If there is
 **                    no data to be read, the value of the returned pointer is
 **                    NULL.
 **
 *******************************************************************************/

UDRV_API void USERIAL_ReadBuf(tUSERIAL_PORT port, BT_HDR **p_buf)
{

}

/*******************************************************************************
 **
 ** Function           USERIAL_WriteBuf
 **
 ** Description        Write data to a serial port using a GKI buffer.
 **
 ** Output Parameter   None
 **
 ** Returns            TRUE  if buffer accepted for write.
 **                    FALSE if there is already a buffer being processed.
 **
 ** Comments           The buffer will be freed by the serial driver.  Therefore,
 **                    the application calling this function must not free the
 **                    buffer.
 **
 *******************************************************************************/

UDRV_API BOOLEAN USERIAL_WriteBuf(tUSERIAL_PORT port, BT_HDR *p_buf)
{
    return FALSE;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Write
 **
 ** Description        Write data to a serial port using a byte buffer.
 **
 ** Output Parameter   None
 **
 ** Returns            Number of bytes actually written to the serial port.  This
 **                    may be less than len.
 **
 *******************************************************************************/
UDRV_API UINT16 USERIAL_Write(tUSERIAL_PORT port, UINT8 *p_data, UINT16 len)
{
    int ret, total = 0;

    errno = 0;

#if defined(USERIAL_DEBUG) && (USERIAL_DEBUG == TRUE)
    APPL_TRACE_DEBUG2("USERIAL_Write %d : (%d bytes) - ", userial_cb.fd, len);
#endif
    while (len != 0)
    {
#if defined(__FreeBSD__)
        if (BCM_LIKELY(userial_cb.devicetype == USERIAL_BTSOCK))
#else
        if (BCM_UNLIKELY(userial_cb.devicetype == USERIAL_BTSOCK))
#endif
        {
            ret = send(userial_cb.fd, p_data + total, len, 0);
        }
        else
        {
            ret = write(userial_cb.fd, p_data + total, len);
        }
        if (ret < 0)
        {
            APPL_TRACE_ERROR3("USERIAL_Write len %d, ret %d, errno %d",
                    len, ret, errno);
            break;
        }
        /* If only some bytes have been written */
        if (ret >= 0)
        {
            total += ret;
            len -= ret;
        }
    }
    return ((UINT16) total);
}

/*******************************************************************************
 **
 ** Function           USERIAL_Ioctl
 **
 ** Description        Perform an operation on a serial port.
 **
 ** Output Parameter   The p_data parameter is either an input or output depending
 **                    on the operation.
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
UDRV_API void USERIAL_Ioctl(tUSERIAL_PORT port, tUSERIAL_OP op, tUSERIAL_IOCTL_DATA *p_data)
{
    tUSB_SCO_CONTROL ioctl_data;
    int rv;

    switch (op)
    {
    case USERIAL_OP_FLUSH:
        break;
    case USERIAL_OP_FLUSH_RX:
        break;
    case USERIAL_OP_FLUSH_TX:
        break;
    case USERIAL_OP_BAUD_WR:
        APPL_TRACE_DEBUG0("USERIAL_Ioctl: Received USERIAL_OP_BAUD_WR ioctl");
        /* Command only supported by standard H4 driver */
        if (userial_cb.devicetype == USERIAL_UART)
        {
            userial_cb.cfg.baud = p_data->baud;
            if (serial_configure())
            {
                APPL_TRACE_ERROR0("ERROR: serial_change_rate, serial_configure failed!");
            }
        }
        break;

    case USERIAL_OP_SCO_UP:
        APPL_TRACE_DEBUG0("USERIAL_Ioctl: Received USERIAL_OP_SCO_UP ioctl");
        /* Command only supported by BT USB driver */
        if (userial_cb.devicetype == USERIAL_BTUSB)
        {
            ioctl_data.sco_handle = p_data->sco_handle;
            ioctl_data.burst = 48;
            rv = ioctl(userial_cb.fd, IOCTL_BTWUSB_ADD_VOICE_CHANNEL, &ioctl_data);
            if (rv < 0)
            {
                APPL_TRACE_DEBUG0("USERIAL_Ioctl: USERIAL_OP_SCO_UP failed");
            }
        }
        break;

    case USERIAL_OP_SCO_DOWN:
        APPL_TRACE_DEBUG0("USERIAL_Ioctl: Received USERIAL_OP_SCO_DOWN ioctl");
        /* Command only supported by BT USB driver */
        if (userial_cb.devicetype == USERIAL_BTUSB)
        {
            ioctl_data.sco_handle = p_data->sco_handle;
            ioctl_data.burst = 0;
            rv = ioctl(userial_cb.fd, IOCTL_BTWUSB_REMOVE_VOICE_CHANNEL, &ioctl_data);
            if (rv < 0)
            {
                APPL_TRACE_DEBUG0("USERIAL_Ioctl: USERIAL_OP_SCO_DOWN failed");
            }
        }
        break;

    default:
        break;
    }

    return;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Close
 **
 ** Description        Close a serial port
 **
 ** Output Parameter   None
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
UDRV_API void USERIAL_Close(tUSERIAL_PORT port)
{
    int fd = userial_cb.fd;

    APPL_TRACE_DEBUG2("USERIAL_Close(%d): closing transport fd %d", port, fd);

    /* if not already closed */
    if (fd != -1)
    {
        userial_cb.fd = -1;
        userial_cb.ser_cb = NULL;
        close(fd);
        if (pthread_cond_destroy(&userial_cb.read_cv) < 0)
        {
            APPL_TRACE_ERROR0("ERROR: pthread_cond_destroy read_cv failed!");
        }
        if (pthread_mutex_destroy(&userial_cb.read_protection) < 0)
        {
            APPL_TRACE_ERROR0("ERROR: pthread_mutex_destroy read_protection failed!");
        }
    }
}

/*******************************************************************************
 **
 ** Function           USERIAL_Feature
 **
 ** Description        Check whether a feature of the serial API is supported.
 **
 ** Output Parameter   None
 **
 ** Returns            TRUE  if the feature is supported
 **                    FALSE if the feature is not supported
 **
 *******************************************************************************/
UDRV_API BOOLEAN USERIAL_Feature(tUSERIAL_FEATURE feature)
{
    switch (feature)
    {
    case USERIAL_FEAT_PORT_1:
    case USERIAL_FEAT_PORT_2:
    case USERIAL_FEAT_PORT_3:
    case USERIAL_FEAT_PORT_4:

    case USERIAL_FEAT_BAUD_600:
    case USERIAL_FEAT_BAUD_1200:
    case USERIAL_FEAT_BAUD_9600:
    case USERIAL_FEAT_BAUD_19200:
    case USERIAL_FEAT_BAUD_57600:
    case USERIAL_FEAT_BAUD_115200:

    case USERIAL_FEAT_STOPBITS_1:
    case USERIAL_FEAT_STOPBITS_2:

    case USERIAL_FEAT_PARITY_NONE:
    case USERIAL_FEAT_PARITY_EVEN:
    case USERIAL_FEAT_PARITY_ODD:

    case USERIAL_FEAT_DATABITS_5:
    case USERIAL_FEAT_DATABITS_6:
    case USERIAL_FEAT_DATABITS_7:
    case USERIAL_FEAT_DATABITS_8:

    case USERIAL_FEAT_FC_HW:
    case USERIAL_FEAT_BUF_BYTE:

    case USERIAL_FEAT_OP_FLUSH_RX:
    case USERIAL_FEAT_OP_FLUSH_TX:
        return TRUE;
    default:
        return FALSE;
    }

    return FALSE;
}

#if (BTU_DUAL_STACK_MM_INCLUDED == TRUE)
/*******************************************************************************
 **
 ** Function           userial_get_dev_name
 **
 ** Description        Return device name
 **
 ** Returns            Pointer or device's name string
 **
 *******************************************************************************/
char *userial_get_dev_name(void)
{
    return userial_cb.devname;
}
#endif

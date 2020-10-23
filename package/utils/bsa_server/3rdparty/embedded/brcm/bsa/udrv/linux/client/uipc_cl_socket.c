/*****************************************************************************
**
**  Name:           uipc_cl_socket.c
**
**  Description:    Socket Client for BSA
**
**  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

/* The pthread.h header file must be the first included file of each source file
 * using the threads library, because it defines some important macros that affect other header files
 */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#include "buildcfg.h"
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"
#else
#include "bsa_api.h"
#include "bsa_int.h"
#endif

#include "uipc_thread.h"

#include "uipc_cl_socket.h"


/* #define UIPC_CL_SOCKET_DEBUG */

#define MAX_API_BUFFER_SIZE 1024

/* Definitions for socketpair access */
#define UIPC_CL_SOCKPAIR_READ_DESC    0     /* Offset for Read descriptor */
#define UIPC_CL_SOCKPAIR_WRITE_DESC   1     /* Offset for Write descriptor */
#define UIPC_CL_SOCKPAIR_NB_DESC      2     /*  number of descriptor */

/* Definitions of data(commands) which can be written in the socketpair*/
#define UIPC_CL_SOCKPAIR_STOP        0x01    /* To stop the Thread */
#define UIPC_CL_SOCKPAIR_IND         0x02    /* to indicate force the thread to make a new select */

#define UIPC_CL_SOCKET_MAX(a,b)     ((a)>(b)?(a):(b))

typedef struct
{
    BOOLEAN in_use; /* TRUE if this client is connected */
    int sock; /* The socket file descriptor for our "listening" socket */
    int disconnect_server;
    tTHREAD read_thread; /* Associated Read thread */
    tUIPC_CL_SOCKET_MODE mode;
    tUIPC_RCV_CBACK *p_cback;
    int sockpair[UIPC_CL_SOCKPAIR_NB_DESC];
    BOOLEAN tx_ready_request;
    BOOLEAN rx_ready_request;
} tUIPC_CL_SOCKET_CLIENT;

typedef struct
{
    tUIPC_CL_SOCKET_CLIENT clients[UIPC_CL_SOCKET_DESC_MAX];
    void *p_data;
} tUIPC_CL_SOCKET_CB;

static tUIPC_CL_SOCKET_CB uipc_cl_socket_cb;

/*
 * Local functions
 */
static void uipc_cl_socket_read_task(void *arg);
static void uipc_cl_socket_set_non_blocking(int sock);
static int uipc_cl_socket_handle_socketpair(tUIPC_CL_SOCKET_CLIENT *p_client);
static int uipc_cl_socket_handle_read(tUIPC_CL_SOCKET_CLIENT *p_client);
static int uipc_cl_socket_handle_writable(tUIPC_CL_SOCKET_CLIENT *p_client);


/*******************************************************************************
 **
 ** Function         uipc_cl_socket_init
 **
 ** Description      Initialize UIPC sockets.
 **
 ** Parameters       p_data: underlying configuration opaque pointer
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_socket_init(void *p_data)
{
    memset(&uipc_cl_socket_cb, 0, sizeof(uipc_cl_socket_cb));
    uipc_cl_socket_cb.p_data = p_data;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_socket_connect
 **
 ** Description      Connect to a server socket.
 **
 ** Parameters       socket_name: unique name of the socket to open
 **                  p_cback: callback of the socket
 **                  mode: callback usage mode
 **
 ** Returns          Assigned socket descriptor
 **
 *******************************************************************************/
tUIPC_CL_SOCKET_DESC uipc_cl_socket_connect(UINT8 *socket_name,
        tUIPC_RCV_CBACK *p_cback, tUIPC_CL_SOCKET_MODE mode)
{
    char errorstring[80];
    int status;
    int sock;
    tUIPC_CL_SOCKET_DESC socket_desc;
    tUIPC_CL_SOCKET_CLIENT *p_client;
    /* Local (UNIX) socket information structure */
    struct sockaddr_un server_address;

#ifdef UIPC_CL_SOCKET_DEBUG
    APPL_TRACE_DEBUG2("uipc_cl_socket_connect: socket=%s mode=%d", socket_name, mode);
#endif

    if (p_cback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_connect: NULL callback");
        return UIPC_CL_SOCKET_DESC_MAX;
    }

    if ((socket_name == NULL) || (strlen((char *)socket_name) == 0))
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_start_server fail => bad socket name");
        return UIPC_CL_SOCKET_DESC_MAX;
    }

    /* Look for a free client index */
    for (socket_desc = 0; socket_desc < UIPC_CL_SOCKET_DESC_MAX; socket_desc++)
    {
        p_client = &uipc_cl_socket_cb.clients[socket_desc];
        if (p_client->in_use == FALSE)
        {
#ifdef UIPC_CL_SOCKET_DEBUG
            APPL_TRACE_DEBUG1("uipc_cl_socket_connect: allocated %d", socket_desc);
#endif
            memset(p_client, 0, sizeof(*p_client));
            p_client->in_use = TRUE;
            break;
        }
    }
    if (socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_connect: no available control block");
        return UIPC_CL_SOCKET_DESC_MAX;
    }

    /* Obtain a file descriptor for our "client" socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_cl_socket_connect: socket failed(%s)", errorstring);
        p_client->in_use = FALSE;
        return UIPC_CL_SOCKET_DESC_MAX;
    }

    /* Set socket parameters */
    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    /* Check if there is a name prefix to add in front of the UNIX socket name */
    if (uipc_cl_socket_cb.p_data != NULL)
    {
        strncpy(server_address.sun_path, uipc_cl_socket_cb.p_data, sizeof(server_address.sun_path)-1);
    }
    else
    {
        server_address.sun_path[0] = 0;
    }
    strncat(server_address.sun_path, (char*)socket_name,
            sizeof(server_address.sun_path)-1-strlen(server_address.sun_path));

    /* Connect to the server socket */
    status = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if (status < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR2("uipc_cl_socket_connect: connect(%s) failed(%s)",
                server_address.sun_path, errorstring);
        p_client->in_use = FALSE;
        close(sock);
        return UIPC_CL_SOCKET_DESC_MAX;
    }

#ifdef UIPC_CL_SOCKET_DEBUG
    APPL_TRACE_DEBUG1("uipc_cl_socket_connect socket:%s connected", socket_name);
#endif

    /* If this socket in non blocking mode */
    if ((mode & UIPC_CL_SOCKET_MODE_BLOCK_MASK) == UIPC_CL_SOCKET_MODE_NON_BLOCK)
    {
#ifdef UIPC_CL_SOCKET_DEBUG
        APPL_TRACE_DEBUG0("uipc_cl_socket_connect: set NonBlocking");
#endif
        uipc_cl_socket_set_non_blocking(sock);
    }
    else
    {
#ifdef UIPC_CL_SOCKET_DEBUG
        APPL_TRACE_DEBUG0("uipc_cl_socket_connect: leave Blocking");
#endif
    }

    /* Save client socket information (callback, access mode, IDs) */
    p_client->p_cback = p_cback;
    p_client->mode = mode;
    p_client->sock = sock;

    /* If this socket is in RxDataReady mode, set request event flag now */
    if ((p_client->mode & UIPC_CL_SOCKET_MODE_RX_MASK) == UIPC_CL_SOCKET_MODE_RX_DATA_READY_EVT)
    {
#ifdef UIPC_CL_SOCKET_DEBUG
        APPL_TRACE_DEBUG0("uipc_cl_socket_connect: set rx_ready_request");
#endif
        p_client->rx_ready_request = TRUE;
    }


    /* Create a socketpair to be able to kill the reading task */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, p_client->sockpair) < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_cl_socket_connect: socketpair failed(%s)", errorstring);
        p_client->in_use = FALSE;
        close(sock);
        return UIPC_CL_SOCKET_DESC_MAX;
    }

    /* Create the thread in charge of reading the socket */
    status = uipc_thread_create(uipc_cl_socket_read_task, socket_name, NULL, 0,
            &p_client->read_thread,
            (void *)(UINTPTR)socket_desc);
    if (status < 0)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_connect: uipc_thread_create failed");
        p_client->in_use = FALSE;
        close(sock);
        return UIPC_CL_SOCKET_DESC_MAX;
    }
    return socket_desc;
}


/*******************************************************************************
 **
 ** Function         uipc_cl_socket_disconnect
 **
 ** Description      This function stop the socket client
 **
 ** Parameters       socket_desc: socket descriptor
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_socket_disconnect(tUIPC_CL_SOCKET_DESC socket_desc)
{
    char errorstring[80];
    tUIPC_CL_SOCKET_CLIENT *p_client;
    char byte;

    if (socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_disconnect: bad socket %d", socket_desc);
        return FALSE;
    }
    p_client = &uipc_cl_socket_cb.clients[socket_desc];
    if (p_client->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_disconnect: socket %d not in use", socket_desc);
        return FALSE;
    }

    p_client->in_use = FALSE;
    close(p_client->sock);

    /* Write UIPC_SV_SOCKPAIR_STOP value in the "write" side of the socketpair */
    /* This will trig the select and stop the thread */
    byte = UIPC_CL_SOCKPAIR_STOP;
    if (write(p_client->sockpair[UIPC_CL_SOCKPAIR_WRITE_DESC], &byte, sizeof(byte)) < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_cl_socket_disconnect: write socketpair failed(%s)", errorstring);
    }
    return TRUE;
}


/*******************************************************************************
 **
 ** Function         uipc_control_socket_send
 **
 ** Description      This function send data to a Server process
 **
 ** Parameters       socket_desc: socket descriptor
 **                  p_buf: Pointer to buffer to send
 **                  length: length of buffer
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_socket_send(tUIPC_CL_SOCKET_DESC socket_desc, UINT8 *p_buffer, UINT16 length)
{
    if (uipc_cl_socket_write(socket_desc, p_buffer, length) < 0)
    {
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_socket_send_buf
 **
 ** Description      This function send data buffer to the Server.
 **                  NOTE: THIS FUNCTION DOES NOT FREE THE BUFFER
 **
 ** Parameters       socket_desc: socket descriptor
 **                  p_msg: BT_HDR
 **                     IN: offset indicates where starts the data
 **                     IN: len the data length (without offset)
 **                     OUT: len the data remaining (not sent)
 **                     OUT: layer_specific: 0 if socket is full (flow control)
 **                                        : 1 if error
 **
 ** Returns          TRUE (buffer fully sent) or FALSE (error or buffer not fully sent)
 **
 *******************************************************************************/
BOOLEAN uipc_cl_socket_send_buf(tUIPC_CL_SOCKET_DESC socket_desc, BT_HDR *p_msg)
{
    int rv;

    /* Write the data */
    rv = uipc_cl_socket_write(socket_desc, (UINT8 *)(p_msg + 1) + p_msg->offset, p_msg->len);

    if (BCM_LIKELY(rv == p_msg->len))
    {
        /* Data has been sent successfully, free the buffer */
        GKI_freebuf(p_msg);
        return TRUE;
    }
    else if (rv < 0)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_send_buf: uipc_cl_socket_write failed");
        /* This is an error */
        p_msg->layer_specific = UIPC_LS_TX_FAIL;
    }
    /* Only a part of the buffer has been sent */
    else
    {
        APPL_TRACE_WARNING2("uipc_cl_socket_send_buf: partial write req:%d sent:%d",
                p_msg->len, rv);
        /* Update len (remove number of bytes sent) */
        p_msg->len -= rv;
        /* Update offset (skip bytes sent) */
        p_msg->offset += rv;
        /* This is NOT an error (just a warning) */
        p_msg->layer_specific = UIPC_LS_TX_FLOW_OFF;
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_socket_write
 **
 ** Description      This function write data in a socket
 **
 ** Parameters       socket_desc: socket descriptor
 **                  buffer: buffer containing data to send
 **                  length: length of the data to send
 **
 ** Returns          number of bytes written or < 0 in case of error
 **
 *******************************************************************************/
int uipc_cl_socket_write(tUIPC_CL_SOCKET_DESC socket_desc, UINT8 *buffer, UINT16 length)
{
#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
    char errorstring[80];
#endif
    int ret_code;
    tUIPC_CL_SOCKET_CLIENT *p_client;

#ifdef UIPC_CL_SOCKET_DEBUG
    APPL_TRACE_DEBUG2("uipc_cl_socket_write: socket=%d len=%d", socket_desc, length);
#endif

    if (length == 0)
    {
        APPL_TRACE_DEBUG0("uipc_cl_socket_write: length=0");
        return 0;
    }
    if (buffer == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_write: NULL pointer");
        return 0;
    }
    if (socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_write: bad socket %d", socket_desc);
        return -1;
    }
    p_client = &uipc_cl_socket_cb.clients[socket_desc];
    if (p_client->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_write: socket %d not in use", socket_desc);
        return -1;
    }

    /*  Write Data into the socket */
    ret_code = write(p_client->sock, buffer, length);

    if ((ret_code < 0) && (errno != EAGAIN))
    {
#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR3("uipc_cl_socket_write: CASE1!!!!this should not be printed..  write:%s,errno=%d,ret_code=%d",
                errorstring, errno, ret_code);
#endif
    }
    else if ((ret_code < 0) && (errno == EAGAIN))
    {
#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_DEBUG3("uipc_cl_socket_write: CASE 2!!!! uipc_cl_socket_send write:%s,errno=%d,ret_code=%d",
                errorstring, errno, ret_code);
#endif
        /* socket is full and cannot accept new data */
        ret_code = 0;
    }
    else if (ret_code != length)
    {
        APPL_TRACE_DEBUG2("uipc_cl_socket_write: %d bytes to send but only %d sent",
                length, ret_code);
    }

    /* return nb bytes written (or error) */
    return (ret_code);
}

/*******************************************************************************
 **
 ** Function         uipc_cl_socket_read
 **
 ** Description      Called to read a data from a socket.
 **
 ** Parameters       socket_desc: socket descriptor
 **                  p_buf: buffer to read into
 **                  len: length of the buffer to read into
 **
 ** Returns          number of bytes read
 **
 *******************************************************************************/
UINT32 uipc_cl_socket_read(tUIPC_CL_SOCKET_DESC socket_desc, UINT8 *p_buf, UINT32 length)
{
#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
    char errorstring[80];
#endif
    int ret_code;
    tUIPC_CL_SOCKET_CLIENT *p_client;
    UINT32 read_length;

#ifdef UIPC_CL_SOCKET_DEBUG
    APPL_TRACE_DEBUG1("uipc_cl_socket_read: socket=%d", socket_desc);
#endif

    if (length == 0)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_read: length=0");
        return 0;
    }
    if (p_buf == NULL)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_read: NULL pointer");
        return 0;
    }
    if (socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_read: bad socket %d", socket_desc);
        return 0;
    }
    p_client = &uipc_cl_socket_cb.clients[socket_desc];

    if (p_client->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_read: socket %d not in use", socket_desc);
        return 0;
    }
    if ((p_client->mode & UIPC_CL_SOCKET_MODE_BLOCK_MASK) == UIPC_CL_SOCKET_MODE_BLOCK)
    {
        APPL_TRACE_ERROR0("uipc_cl_socket_read: not allowed (Socket Blocking)");
        return 0;
    }

    /*  Read Data from the socket */
    ret_code = read(p_client->sock, p_buf, length);
    if (ret_code < 0)
    {
        /*
         * Sockets in NonBlocking mode return an error and set errno to
         * EWOULDBLOCK if there is nothing to read
         */
        if (errno != EWOULDBLOCK)
        {
#if (BT_USE_TRACES == TRUE || BT_TRACE_APPL == TRUE)
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_cl_socket_read: write failed(%s)", errorstring);
#endif
        }
        read_length = 0;
    }
    else
    {
#ifdef UIPC_CL_SOCKET_DEBUG
        APPL_TRACE_DEBUG1("uipc_cl_socket_read: read %d bytes", ret_code);
#endif
        read_length = ret_code;
    }

    /* return nb bytes read */
    return (read_length);
}


/*******************************************************************************
 **
 ** Function         uipc_cl_socket_set_non_blocking
 **
 ** Description      Set a socket in Non blocking mode
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_cl_socket_set_non_blocking(int sock)
{
    int opts;

    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

/*******************************************************************************
 **
 ** Function           uipc_cl_socket_read_task
 **
 ** Description        Thread in charge of continuously reading data from socket
 **
 ** Returns            void
 **
 *******************************************************************************/
static void uipc_cl_socket_read_task(void *arg)
{
    tUIPC_CL_SOCKET_DESC socket_desc = (tUIPC_CL_SOCKET_DESC)(long)arg;
    ssize_t length;
    tUIPC_CL_SOCKET_CLIENT *p_client;
    int sock;
    fd_set socks_rd; /* Read Socket file descriptors we want to wake up for, using select() */
    fd_set socks_wr; /* Write Socket file descriptors we want to wake up for, using select() */
    int fd;
    int status;
    tUIPC_CL_SOCKET_MODE mode_tmp;
    char errorstring[80];
    BT_HDR *p_msg;

#ifdef UIPC_CL_SOCKET_DEBUG
    APPL_TRACE_DEBUG1("uipc_cl_socket_read_task: started socket %d", socket_desc);
#endif

    if (socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_read_task: bad socket %d", socket_desc);
        return;
    }
    p_client = &uipc_cl_socket_cb.clients[socket_desc];

    sock = p_client->sock;

    /* Main server loop - until application disconnects from server */
    do
    {
        /*
         * This task will wait for events (using select):
         *    For Read: from either Socket (if requested) or socketpair
         *    For Writable capability on socket (if requested) */
        FD_ZERO(&socks_rd);
        FD_ZERO(&socks_wr);

        /* Update the max FileDescriptor (needed by select API) */
        fd = p_client->sockpair[UIPC_CL_SOCKPAIR_READ_DESC];
        /* Always trig on read from Socketpair */
        FD_SET(fd, &socks_rd);

        /* If the Rx mode is in Rx Data event mode OR */
        /* If the Rx mode is in Rx Data Ready event mode AND event requested */
        mode_tmp = p_client->mode & UIPC_CL_SOCKET_MODE_RX_MASK;
        if ((mode_tmp == UIPC_CL_SOCKET_MODE_RX_DATA_EVT) ||
            ((mode_tmp == UIPC_CL_SOCKET_MODE_RX_DATA_READY_EVT) && p_client->rx_ready_request))
        {
            /* Add the socket in the Read table */
            FD_SET(sock, &socks_rd);
            /* Update the max FileDescriptor (needed by select API) */
            fd = UIPC_CL_SOCKET_MAX(fd, sock);
        }

        /* If the Tx mode is in Tx Data Ready event mode AND event requested */
        mode_tmp = p_client->mode & UIPC_CL_SOCKET_MODE_TX_MASK;
        if ((mode_tmp == UIPC_CL_SOCKET_MODE_TX_DATA_READY_EVT) && p_client->tx_ready_request)
        {
            /* Add the socket in the Read table */
            FD_SET(sock, &socks_wr);
            /* Update the max FileDescriptor (needed by select API) */
            fd = UIPC_CL_SOCKET_MAX(fd, sock);
        }

        /* Wait for read indication on:
         *   Read: both the socket (if requested) and the socketpair
         *   Writable: on the socket (if requested)
         */
        status = select(fd + 1, &socks_rd, &socks_wr, (fd_set *) NULL, NULL);
        if (status < 0)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_cl_socket_read_task: select failed(%s)", errorstring);

            /* Allocate buffer for close event */
            if ((p_msg =  (BT_HDR *) GKI_getbuf(sizeof(BT_HDR))) != NULL)
            {
                p_msg->offset = 0;
                p_msg->len = 0;
                p_msg->event = UIPC_CLOSE_EVT;
                p_msg->layer_specific = 0;
                if (p_client->p_cback != NULL)
                {
                    /* call the Server callback function */
                    p_client->p_cback(p_msg);
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_cl_socket_read_task: no buffer");
                }
            }
            length = -1; /* to exit the read thread */
        }
        else
        {
            length = -1;
            /* If there is something to read from the socket */
            if (FD_ISSET(sock, &socks_rd))
            {
                /* Read it */
                length = uipc_cl_socket_handle_read(p_client);
            }
            /* If the socket is writable */
            if (FD_ISSET(sock, &socks_wr))
            {
                /* Handle it */
                length = uipc_cl_socket_handle_writable(p_client);
            }
            /* If this is a socketpair indication */
            if (FD_ISSET(p_client->sockpair[UIPC_CL_SOCKPAIR_READ_DESC],&socks_rd))
            {
                /* Handle it */
                length = uipc_cl_socket_handle_socketpair(p_client);
            }
        }
    } while (length >= 0);

    APPL_TRACE_DEBUG0("uipc_cl_socket_read_task: exit");
    /* Close the two sides of the socketpair */
    close(p_client->sockpair[UIPC_CL_SOCKPAIR_WRITE_DESC]);
    close(p_client->sockpair[UIPC_CL_SOCKPAIR_READ_DESC]);
    uipc_thread_stop(p_client->read_thread);
}


/*******************************************************************************
 **
 ** Function           uipc_cl_socket_handle_socketpair
 **
 ** Description        Read the socketpair and check read data
 **
 ** Returns            -1 if UIPC_SV_SOCKPAIR_STOP received
 **                     0 Otherwise
 **
 *******************************************************************************/
static int uipc_cl_socket_handle_socketpair(tUIPC_CL_SOCKET_CLIENT *p_client)
{
    UINT8 byte;
    int rv;

#ifdef UIPC_CL_SOCKET_DEBUG
    APPL_TRACE_DEBUG0("uipc_cl_socket_handle_socketpair");
#endif

    /* Read one byte from the socketpair */
    rv = read(p_client->sockpair[UIPC_CL_SOCKPAIR_READ_DESC], &byte, sizeof(byte));
    if (rv <= 0)
    {
        rv = 0; /*  Ignore */
        APPL_TRACE_ERROR0("uipc_cl_socket_handle_socketpair: read socketpair failed");
    }
    else if (byte == UIPC_CL_SOCKPAIR_STOP)
    {
        rv = -1; /* Exit thread */
        APPL_TRACE_DEBUG0("uipc_cl_socket_handle_socketpair: read Stop event");
    }
    else if (byte == UIPC_CL_SOCKPAIR_IND)
    {
        rv = 0; /* Socket indication => we will make a new select*/
        APPL_TRACE_DEBUG0("uipc_cl_socket_handle_socketpair: read Ind event");
    }
    else
    {
        rv = 0; /*  Ignore */
        APPL_TRACE_ERROR1("uipc_cl_socket_handle_socketpair: read wrong data:0x%x", byte);
    }
    return rv;
}

/*******************************************************************************
 **
 ** Function           uipc_cl_socket_handle_read
 **
 ** Description        Read the socket (if if readdata mode) or send RxDataReady
 **
 ** Returns            -1 if read error => readTask will stop
 **                     0 or data read length otherwise
 **
 *******************************************************************************/
static int uipc_cl_socket_handle_read(tUIPC_CL_SOCKET_CLIENT *p_client)
{
    ssize_t length = 0;
    BT_HDR *p_msg;
    struct pollfd poll_fd;
    int rv;

    /* If this client is in RxDataReady mode */
    if ((p_client->mode & UIPC_CL_SOCKET_MODE_RX_MASK) == UIPC_CL_SOCKET_MODE_RX_DATA_READY_EVT)
    {
        /* Non blocking socket, let's check if the socket is still connected */
        poll_fd.fd = p_client->sock;
        poll_fd.events = 0;
        poll_fd.revents = 0;

        /* Check if the socket is still connected */
        rv = poll(&poll_fd, 1, 0);

#ifdef UIPC_SV_SOCKET_DEBUG
        APPL_TRACE_ERROR2("uipc_cl_socket_handle_read: poll=%d revents=x%x", rv, poll_fd.revents);
#endif
        if ((rv == 1) && (poll_fd.revents & POLLHUP))
        {
            APPL_TRACE_DEBUG0("uipc_cl_socket_handle_read: Non Blocking Socket disconnected from Server");
            /* Allocate buffer for Rx data Ready Event*/
            p_msg =  (BT_HDR *) GKI_getbuf(sizeof(BT_HDR));
            if (p_msg != NULL)
            {
                /* Send a close indication */
                p_msg->event = UIPC_CLOSE_EVT;
                p_msg->len = 0;
                p_msg->layer_specific = 0;
                if (p_client->p_cback != NULL)
                {
                    /* call the Server callback function */
                    p_client->p_cback(p_msg);
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_cl_socket_handle_read: READY no Cback");
                }
            }
            length = -1; /* to exit the read thread */
        }
        /* else (the socket is still connected) if application asked for UIPC_RX_DATA_READY_EVT */
        else if (p_client->rx_ready_request)
        {
            p_client->rx_ready_request = FALSE;
            /* Allocate buffer for Rx data Ready Event */
            p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR));
            if (p_msg != NULL)
            {
                p_msg->offset = 0;
                p_msg->len = 0;
                p_msg->layer_specific = 0;
                p_msg->event = UIPC_RX_DATA_READY_EVT;
                if (p_client->p_cback != NULL)
                {
                    /* call the Server callback function */
                    p_client->p_cback(p_msg);
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_cl_socket_handle_read: READY no Cback");
                }
            }
            else
            {
                APPL_TRACE_ERROR0("uipc_cl_socket_handle_read: READY no buffer");
            }
        }
        else
        {
            APPL_TRACE_ERROR0("uipc_cl_socket_handle_read: shouldn't be called");
        }
    }
    /* else this client is in RxData mode */
    else
    {
        /* Allocate buffer for Rx data */
        p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR) + MAX_API_BUFFER_SIZE);
        if (p_msg != NULL)
        {
            p_msg->offset = 0;

            /* Read data from server */
            length = read(p_client->sock, (UINT8 *) (p_msg + 1), MAX_API_BUFFER_SIZE);
            /* If read return a negative value => Server die */
            if (length <= 0)
            {
                APPL_TRACE_DEBUG0("uipc_cl_socket_handle_read: Socket disconnected from Server");
                /* Send a close indication */
                p_msg->event = UIPC_CLOSE_EVT;
                p_msg->len = 0;
                p_msg->layer_specific = 0;
                if (p_client->p_cback != NULL)
                {
                    /* call the Server callback function */
                    p_client->p_cback(p_msg);
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_cl_socket_handle_read: READY no Cback");
                }
                length = -1; /* to exit the read thread */
            }
            else
            {
#ifdef UIPC_SV_SOCKET_DEBUG
                APPL_TRACE_DEBUG2("uipc_cl_socket_handle_read: rx %d from %d ", length, num_client);
                /* scru_dump_hex(p_msg->data, "Data", (length >= 16)?16:length, 0, 0); */
#endif
                p_msg->event = UIPC_RX_DATA_EVT;
                p_msg->len = (UINT16) length;
                p_msg->layer_specific = 0;
                if (p_client->p_cback != NULL)
                {
                    /* call the Server callback function */
                    p_client->p_cback(p_msg);
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_cl_socket_handle_read: READY no Cback");
                }
            }
        }
        else
        {
            APPL_TRACE_ERROR0("uipc_cl_socket_handle_read: DATA no buffer");
        }
    } /* socket mode is ReadData */
    return (int)length;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_socket_handle_writable
 **
 ** Description      Handle writable indication for sockets
 **
 ** Returns          int
 **
 *******************************************************************************/
static int uipc_cl_socket_handle_writable(tUIPC_CL_SOCKET_CLIENT *p_client)
{
    BT_HDR *p_msg;

    /* If this server is in TxDataReady mode */
    if ((p_client->mode & UIPC_CL_SOCKET_MODE_TX_MASK) == UIPC_CL_SOCKET_MODE_TX_DATA_READY_EVT)
    {
        /* if no UIPC_TX_DATA_READY_EVT sent */
        if (p_client->tx_ready_request)
        {
            p_client->tx_ready_request = FALSE;
            /* Allocate buffer for Tx Data Ready Event*/
            p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR));
            if (p_msg != NULL)
            {
                p_msg->offset = 0;
                p_msg->len = 0;
                p_msg->layer_specific = 0;
                p_msg->event = UIPC_TX_DATA_READY_EVT;
                if (p_client->p_cback != NULL)
                {
                    /* call the Server callback function */
                    p_client->p_cback(p_msg);
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_cl_socket_handle_writable: READY no Cback");
                }
            }
            else
            {
                APPL_TRACE_ERROR0("uipc_cl_socket_handle_writable: no buffer");
            }
        }
        else
        {
            APPL_TRACE_ERROR0("uipc_cl_socket_handle_writable: shouldn't be called");
        }
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         uipc_cl_socket_ioctl
 **
 ** Description      Control a socket server.
 **
 ** Parameters       socket_desc: socket descriptor
 **                  request: identifier of the control command
 **                  param: pointer to the parameters of the commands
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_cl_socket_ioctl(tUIPC_CL_SOCKET_DESC socket_desc, UINT32 request, void *param)
{
    tUIPC_CL_SOCKET_CLIENT *p_client;
    UINT8 byte;
    char errorstring[80];

    if (socket_desc >= UIPC_CL_SOCKET_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_ioctl: bad socket %d", socket_desc);
        return FALSE;
    }
    /* Get reference to this server */
    p_client = &uipc_cl_socket_cb.clients[socket_desc];

    if (p_client->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_cl_socket_ioctl: socket %d not in use", socket_desc);
        return FALSE;
    }

    switch(request)
    {
    case UIPC_REQ_TX_READY:
        APPL_TRACE_DEBUG1("uipc_cl_socket_ioctl: socket=%d UIPC_REQ_TX_READY", socket_desc);

        p_client->tx_ready_request = TRUE;
        /* Write UIPC_SV_SOCKPAIR_IND value in the "write" side of the socketpair */
        /* This will trig the select and stop the thread */
        byte = UIPC_CL_SOCKPAIR_IND;
        if (write(p_client->sockpair[UIPC_CL_SOCKPAIR_WRITE_DESC], &byte, 1) < 0)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_sv_socket_ioctl: write socketpair failed(%s)", errorstring);
            return FALSE;
        }
        return TRUE;

    case UIPC_REQ_RX_READY:
        APPL_TRACE_DEBUG1("uipc_cl_socket_ioctl: socket=%d UIPC_REQ_RX_READY", socket_desc);
        p_client->rx_ready_request = TRUE;
        /* Write UIPC_SV_SOCKPAIR_IND value in the "write" side of the socketpair */
        /* This will trig the select and stop the thread */
        byte = UIPC_CL_SOCKPAIR_IND;
        if (write(p_client->sockpair[UIPC_CL_SOCKPAIR_WRITE_DESC], &byte, 1) < 0)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_cl_socket_ioctl: write socketpair failed(%s)", errorstring);
            return FALSE;
        }
        return TRUE;

    default:
        APPL_TRACE_ERROR1("uipc_sv_socket_ioctl: bad request:%d", request);
        return FALSE;
    }
}


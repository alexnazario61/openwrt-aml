/*****************************************************************************
 **
 **  Name:           uipc_sv_socket.c
 **
 **  Description:    Socket Server for BSA
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

#include "buildcfg.h"
#if defined (NSA) && (NSA == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"

#include "nsa_task.h"
#include "nsa_sv_control.h"
#else
#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_task.h"
#include "bsa_sv_control.h"
#endif
#include "uipc_sv_socket.h"
#include "uipc_thread.h"


/* #define UIPC_SV_SOCKET_DEBUG */


#ifndef UIPC_SV_MAX_CLIENT
#define UIPC_SV_MAX_CLIENT 5 /* Maximum number of client per server */
#endif


#define MAX_API_BUFFER_SIZE 1024

#define UIPC_SV_SOCKPAIR_READ_DESC    0
#define UIPC_SV_SOCKPAIR_WRITE_DESC   1
#define UIPC_SV_SOCKPAIR_NB_DESC      2

#define UIPC_SV_SOCKPAIR_STOP        0x01    /* To stop the Thread */
#define UIPC_SV_SOCKPAIR_IND         0x02    /* to indicate force the thread to make a new select */

#define UIPC_SV_SOCKET_MAX(a,b)     ((a)>(b)?(a):(b))

typedef struct
{
    BOOLEAN in_use; /* TRUE if this server is started */
    BOOLEAN disconnect;
    BOOLEAN tx_ready_request;
    BOOLEAN rx_ready_request;
    int sock; /* The socket file descriptor for our "listening" socket */
    int connectlist[UIPC_SV_MAX_CLIENT]; /* Array of connected sockets so we know who we are talking to */
    int max_client; /* Maximum number of client allowed to connect */
    fd_set socks_rd; /* Read Socket file descriptors we want to wake up for, using select() */
    fd_set socks_wr; /* Write Socket file descriptors we want to wake up for, using select() */
    int highsock; /* Highest #'d file descriptor, needed for select() */
    tTHREAD read_thread; /* Associated Read thread */
    tUIPC_SV_SOCKET_MODE mode;
    tUIPC_RCV_CBACK *p_uipc_callback;
    char socket_name[BSA_MGT_UIPC_PATH_MAX];
    int sockpair[UIPC_SV_SOCKPAIR_NB_DESC]; /* sockerpair used to stop the rx thread */
} tUIPC_SV_SOCKET_SERVER;

typedef struct
{
    tUIPC_SV_SOCKET_SERVER servers[UIPC_SV_SOCKET_SERVER_MAX];
    void *p_data;
} tUIPC_SV_SOCKET_CB;

/* Server Socket control block */
tUIPC_SV_SOCKET_CB uipc_sv_socket_cb;

/*
 * Local functions
 */
static void uipc_sv_socket_set_non_blocking(int sock);
static void uipc_sv_socket_build_select_list(tUIPC_SV_SOCKET_SERVER *p_server);
static void uipc_sv_socket_handle_new_connection(tUIPC_SV_SOCKET_SERVER *p_server);
static void uipc_sv_socket_deal_with_data(tUIPC_SV_SOCKET_SERVER *p_server, tBSA_CLIENT_NB num_client);
static void uipc_sv_socket_read_socks(tUIPC_SV_SOCKET_SERVER *p_server);

static tBSA_STATUS uipc_sv_socket_remove(char *p_path, char *p_socket_name);

static int uipc_sv_socket_create(UINT8 *socket_name, UINT32 socket_desc);
static void uipc_sv_socket_read_task(void *arg);

static int uipc_sv_socket_write(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        tBSA_CLIENT_NB num_client, UINT8 *buffer, UINT16 length);

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_init
 **
 ** Description      Initialize UIPC sockets.
 **
 ** Parameters       p_data: underlying configuration opaque pointer
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_init(void *p_data)
{
    memset(&uipc_sv_socket_cb, 0, sizeof(uipc_sv_socket_cb));
    uipc_sv_socket_cb.p_data = p_data;

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_start_server
 **
 ** Description      Start UIPC Server socket.
 **
 ** Parameters
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
tUIPC_SV_SOCKET_SERVER_DESC uipc_sv_socket_start_server(UINT8 *socket_name,
        tUIPC_RCV_CBACK *p_uipc_callback, UINT32 max_client, tUIPC_SV_SOCKET_MODE mode)
{
    int status = 0;
    char errorstring[80];
    tUIPC_SV_SOCKET_SERVER_DESC socket_desc;
    tUIPC_SV_SOCKET_SERVER *p_server;

    APPL_TRACE_DEBUG2("uipc_sv_socket_start_server: socket=%s mode=0x%x", socket_name, mode);

    if ((socket_name == NULL) || (strlen((char *)socket_name) == 0))
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_start_server: bad socket name");
        return UIPC_SV_SOCKET_SERVER_MAX;
    }
    if (p_uipc_callback == NULL)
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_start_server: null callback");
        return UIPC_SV_SOCKET_SERVER_MAX;
    }

    /* First, check if a new server can be created */
    for (socket_desc = 0; socket_desc < UIPC_SV_SOCKET_SERVER_MAX; socket_desc++)
    {
        p_server = &uipc_sv_socket_cb.servers[socket_desc];
        /* If this one is free */
        if (p_server->in_use == FALSE)
        {
            /* Let's use it: Clear it first to be sure to erase all fields */
            memset(p_server, 0, sizeof(*p_server));
            p_server->in_use = TRUE;
            p_server->p_uipc_callback = p_uipc_callback;
            p_server->max_client = max_client;
            p_server->mode = mode;

            /* If this socket is in RxDataReady mode, request event now */
            if ((p_server->mode & UIPC_SV_SOCKET_MODE_RX_MASK) == UIPC_SV_SOCKET_MODE_RX_DATA_READY_EVT)
            {
                p_server->rx_ready_request = TRUE;
            }

            strncpy(p_server->socket_name,
                    (char *) socket_name,
                    BSA_MGT_UIPC_PATH_MAX - 1);
            p_server->socket_name[BSA_MGT_UIPC_PATH_MAX-1] = '\0';
            break;
        }
    }
    if (socket_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_start_server: could not find free socket");
        return UIPC_SV_SOCKET_SERVER_MAX;
    }

    /* Create a socketpair to be able to kill the reading thread */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, p_server->sockpair) < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_sv_socket_start: socketpair failed(%s)", errorstring);
        p_server->in_use = FALSE;
        return UIPC_SV_SOCKET_SERVER_MAX;
    }


    /* Create the socket */
    status = uipc_sv_socket_create(socket_name, socket_desc);
    if (status != 0)
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_start_server: uipc_sv_socket_create failed");
        /* close the socketpair */
        close(p_server->sockpair[UIPC_SV_SOCKPAIR_READ_DESC]);
        close(p_server->sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC]);
        p_server->in_use = FALSE;
        return UIPC_SV_SOCKET_SERVER_MAX;
    }

    /* Create the thread in charge of reading the socket */
    status = uipc_thread_create(uipc_sv_socket_read_task, socket_name, NULL, 0,
            &p_server->read_thread, (void *)(UINTPTR)socket_desc);
    if (status < 0)
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_start_server: uipc_thread_create failed (%d)", status);
        uipc_sv_socket_remove(uipc_sv_socket_cb.p_data, (char *)socket_name);
        close(p_server->sockpair[UIPC_SV_SOCKPAIR_READ_DESC]);
        close(p_server->sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC]);
        p_server->in_use = FALSE;
        return UIPC_SV_SOCKET_SERVER_MAX;
    }
    return socket_desc;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_stop_server
 **
 ** Description      This function stop the socket server
 **
 ** Parameters
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_stop_server(tUIPC_SV_SOCKET_SERVER_DESC socket_desc, UINT8 *socket_name)
{
    int err;
    struct timespec delay;
    char errorstring[80];
    char byte;
    tUIPC_SV_SOCKET_SERVER *p_server;

    APPL_TRACE_DEBUG1("uipc_sv_socket_stop_server: server=%d", socket_desc);
    if (socket_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_stop_server: bad socket %d", socket_desc);
        return FALSE;
    }
    p_server = &uipc_sv_socket_cb.servers[socket_desc];

    /* If this one is in use and the good one */
    if (p_server->in_use)
    {
        /* Let's free it */
        p_server->in_use = FALSE;
        p_server->disconnect = TRUE;

        /* Write UIPC_SV_SOCKPAIR_STOP value in the "write" side of the socketpair */
        /* This will trig the select and stop the thread */
        byte = UIPC_SV_SOCKPAIR_STOP;
        if (write(p_server->sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC], &byte, 1) < 0)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_sv_socket_stop_server: socketpair write failed(%s)", errorstring);
        }
        else
        {
            /* Let's sleep 50 milli-seconds to allow the thread to die */
            delay.tv_sec = 0;
            delay.tv_nsec = 50 * 1000;
            do
            {
                err = nanosleep(&delay, &delay);
            } while (err < 0 && errno == EINTR);
        }

        /* Erase the socket file */
        uipc_sv_socket_remove(uipc_sv_socket_cb.p_data, p_server->socket_name);
        p_server->p_uipc_callback = NULL;
        p_server->disconnect = TRUE;
        p_server->socket_name[0] = 0;
        return TRUE;
    }

    APPL_TRACE_ERROR1("uipc_sv_socket_stop_server: socket %d not used", socket_desc);
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_remove
 **
 ** Description      Delete one UIPC sockets.
 **
 ** Returns          int
 **
 *******************************************************************************/
static tBSA_STATUS uipc_sv_socket_remove(char *p_path, char *p_socket_name)
{
    struct sockaddr_un server_address; /* bind info structure */

    if (p_path != NULL)
    {
        strncpy(server_address.sun_path, p_path, sizeof(server_address.sun_path)-1);
    }
    else
    {
        server_address.sun_path[0] = '\0';
    }

    strncat(server_address.sun_path, p_socket_name,
                    sizeof(server_address.sun_path) - 1 - strlen(server_address.sun_path));
    unlink(server_address.sun_path);

    APPL_TRACE_DEBUG1("uipc_sv_socket_remove: [%s] deleted", (char *)server_address.sun_path);

    return BSA_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_terminate
 **
 ** Description      Delete all UIPC sockets.
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_socket_terminate(void)
{
    tUIPC_SV_SOCKET_SERVER_DESC socket_desc;
    tUIPC_SV_SOCKET_SERVER *p_server;

    for(socket_desc = 0 ; socket_desc < UIPC_SV_SOCKET_SERVER_MAX ; socket_desc++)
    {
        p_server = &uipc_sv_socket_cb.servers[socket_desc];
        if (p_server->in_use != FALSE)
        {
            uipc_sv_socket_remove(uipc_sv_socket_cb.p_data, p_server->socket_name);
        }
    }
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_send
 **
 ** Description      This function send data to a Client application
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_send(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        tBSA_CLIENT_NB num_client, UINT8 *p_buffer, UINT16 length)
{
    if (uipc_sv_socket_write(socket_desc, num_client, p_buffer, length) < 0)
    {
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_send_buf
 **
 ** Description      This function send data buffer to a Client application.
 **                  NOTE: THIS FUNCTION DOES NOT FREE THE BUFFER
 **                  NOTE 2: THIS FUNCTION IS CALLED WHEN A BUFFER IS STORED AFTER
 **                          BTH_HDR
 **
 ** Parameters       BT_HDR
 **                     IN: offset indicates where starts the data
 **                     IN: len the data length (without offset)
 **                     OUT: len the data remaining (not sent)
 **                     OUT: layer_specific: 0 if socket is full (flow control)
 **                                        : 1 if error
 **
 ** Returns          TRUE (buffer fully sent) or FALSE (error or buffer not fully sent)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_send_buf(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        tBSA_CLIENT_NB num_client,  BT_HDR *p_msg)
{
    int rv;

    /* Write the data */
    rv = uipc_sv_socket_write(socket_desc, num_client, (UINT8 *)(p_msg + 1) + p_msg->offset, p_msg->len);

    /* If buffer sent correctly */
    if (BCM_LIKELY(rv == p_msg->len))
    {
        /* Data has been sent successfully, free the buffer */
        GKI_freebuf(p_msg);
        return TRUE;
    }
    else if (rv < 0)
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_send_buf: uipc_sv_socket_write failed");
        p_msg->layer_specific = 1;
    }
    /* Only a part of the buffer has been sent */
    else
    {
        APPL_TRACE_WARNING2("uipc_sv_socket_send_buf: partial req:%d sent:%d", p_msg->len, rv);
        /* Update len (remove number of bytes sent) */
        p_msg->len -= rv;
        /* Update offset (skip bytes sent) */
        p_msg->offset += rv;
        /* This is NOT an error (just a warning) */
        p_msg->layer_specific = 0;
    }
    /* Error or not sent entirely */
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_send_pbuf
 **
 ** Description      This function send data buffer from a pointer to a Client application.
 **                  NOTE 1: THIS FUNCTION DOES NOT FREE THE BUFFER
 **                  NOTE 2: THIS FUNCTION IS CALLED WHEN A BUFFER ADDRESS IS STORED AFTER
 **                          BTH_HDR
 **
 ** Parameters       BT_HDR
 **                     IN: offset indicates where starts the data
 **                     IN: len the data length (without offset)
 **                     OUT: len the data remaining (not sent)
 **                     OUT: layer_specific: 0 if socket is full (flow control)
 **                                        : 1 if error
 **
 ** Returns          TRUE (buffer fully sent) or FALSE (error or buffer not fully sent)
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_send_pbuf(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        tBSA_CLIENT_NB num_client,  BT_HDR *p_msg)
{
    int rv;

    /* Write the (remaining data) */
    rv = uipc_sv_socket_write(socket_desc, num_client, *((UINT8 **)(p_msg + 1)) + p_msg->offset, p_msg->len);

    /* If buffer sent correctly */
    if (BCM_LIKELY(rv == p_msg->len))
    {
        /* BSA HL does not support flow control so free the buffer unconditionally */
        GKI_freebuf(p_msg);
        return TRUE;
    }
    else if (rv < 0)
    {
        /* This is an error */
        APPL_TRACE_ERROR0("uipc_sv_socket_send_buf: uipc_sv_socket_write failed");
        p_msg->layer_specific = 1;
    }
    /* Only a part of the buffer has been sent */
    else
    {
        APPL_TRACE_WARNING2("uipc_sv_socket_send_buf: partial write req:%d sent:%d",
                p_msg->len, rv);
        /* Update len (remove number of bytes sent) */
        p_msg->len -= rv;
        /* Update offset (skip bytes sent) */
        p_msg->offset += rv;
        /* This is NOT an error (just a warning) */
        p_msg->layer_specific = 0;
    }

    return FALSE;
}


/*******************************************************************************
 **
 ** Function         uipc_sv_socket_write
 **
 ** Description      This function write data in a socket
 **
 ** Parameters
 **
 ** Returns          number of bytes written or < 0 in case of error
 **
 *******************************************************************************/
static int uipc_sv_socket_write(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        tBSA_CLIENT_NB num_client, UINT8 *buffer, UINT16 length)
{
    char errorstring[80];
    int rv;
    tUIPC_SV_SOCKET_SERVER *p_server;

#ifdef UIPC_SV_SOCKET_DEBUG
    APPL_TRACE_DEBUG3("uipc_sv_socket_write: desc=%d cl=%d len=%d", socket_desc, num_client, length);
#endif

    if (BCM_UNLIKELY(length == 0))
    {
        APPL_TRACE_DEBUG0("uipc_sv_socket_write: len = 0");
        return 0;
    }
    if (BCM_UNLIKELY(buffer == NULL))
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_write: NULL ptr");
        return 0;
    }
    if (BCM_UNLIKELY(socket_desc >= UIPC_SV_SOCKET_SERVER_MAX))
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_write: bad socket %d", socket_desc);
        return -1;
    }
    if (BCM_UNLIKELY(num_client >= UIPC_SV_MAX_CLIENT))
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_write: bad client %d", num_client);
        return -1;
    }

    p_server = &uipc_sv_socket_cb.servers[socket_desc];
    if (BCM_UNLIKELY(p_server->in_use == FALSE))
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_write: socket %d not running", socket_desc);
        return -1;
    }
    /* If the connection to client is closed */
    if (BCM_UNLIKELY(p_server->connectlist[num_client] == 0))
    {
        APPL_TRACE_ERROR2("uipc_sv_socket_write: socket %d, client %d not connected:%d",
                socket_desc, num_client);
        return -1;
    }

    /*  Write Data into the socket */
    rv = write(p_server->connectlist[num_client], buffer, length);

    if (BCM_LIKELY(rv == length))
    {
#ifdef UIPC_SV_SOCKET_DEBUG
        APPL_TRACE_DEBUG1("uipc_sv_socket_write: written %d bytes", rv);
#endif
    }
    else if (rv < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_sv_socket_write: write failed(%s)", errorstring);
    }
    else if (rv != length)
    {
        APPL_TRACE_ERROR2("uipc_sv_socket_write: wrote %d but only %d succeeded", length, rv);
    }

    return (rv);
}

/*******************************************************************************
**
** Function         uipc_sv_socket_read
**
** Description      Called to read a data from a socket.
**
** Returns          number of bytes read
**
*******************************************************************************/
UINT32 uipc_sv_socket_read(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        tBSA_CLIENT_NB num_client, UINT8 *p_buf, UINT32 length)
{
    char errorstring[80];
    int rv;
    tUIPC_SV_SOCKET_SERVER *p_server;
    UINT32 read_length;

#ifdef UIPC_SV_SOCKET_DEBUG
    APPL_TRACE_DEBUG1("uipc_sv_socket_read: socket=%d", socket_desc);
#endif

    if (BCM_UNLIKELY(length == 0))
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_read: length=0");
        return 0;
    }
    if (BCM_UNLIKELY(p_buf == NULL))
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_read: NULL pointer");
        return 0;
    }
    if (BCM_UNLIKELY(socket_desc >= UIPC_SV_SOCKET_SERVER_MAX))
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_read: bad socket %d", socket_desc);
        return 0;
    }
    if (BCM_UNLIKELY(num_client >= UIPC_SV_MAX_CLIENT))
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_read: bad num_client %d", num_client);
        return 0;
    }

    p_server = &uipc_sv_socket_cb.servers[socket_desc];
    if (BCM_UNLIKELY(p_server->in_use == FALSE))
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_read: socket %d not running", socket_desc);
        return 0;
    }
    if (BCM_UNLIKELY((p_server->mode & UIPC_SV_SOCKET_MODE_BLOCK_MASK) == UIPC_SV_SOCKET_MODE_BLOCK))
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_read: not allowed (Socket Blocking)");
        return 0;
    }
    /* If the connection to client is closed */
    if (BCM_UNLIKELY(p_server->connectlist[num_client] == 0))
    {
        APPL_TRACE_ERROR2("uipc_sv_socket_read: socket %d client %d not connected",
                socket_desc, num_client);
        return 0;
    }

    /*  Read Data from the socket */
    rv = read(p_server->connectlist[num_client], p_buf, length);
    if (BCM_UNLIKELY(rv < 0))
    {
        /*
         * Sockets in NonBlocking mode return an error and set errno to
         * EWOULDBLOCK if there is nothing to read
         */
        if (errno != EWOULDBLOCK)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_sv_socket_read: write failed(%s)", errorstring);
        }
        read_length = 0;
    }
    else
    {
#ifdef UIPC_SV_SOCKET_DEBUG
        APPL_TRACE_DEBUG1("uipc_sv_socket_read: read %d bytes", ret_code);
#endif
        read_length = rv;
    }

    /* return nb bytes read */
    return (read_length);

}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_set_non_blocking
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_socket_set_non_blocking(int sock)
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
    return;
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_build_select_list
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_socket_build_select_list(tUIPC_SV_SOCKET_SERVER *p_server)
{
    int listnum; /* Current item in connectlist for for loops */
    tUIPC_SV_SOCKET_MODE mode_tmp;

    /* First put together fd_set for select(), which will
     consist of the sock variable in case a new connection
     is coming in, plus all the sockets we have already
     accepted. */

    /* FD_ZERO() clears out the fd_set called socks, so that
     it doesn't contain any file descriptors. */

    FD_ZERO(&p_server->socks_rd);
    FD_ZERO(&p_server->socks_wr);

    /* FD_SET() adds the file descriptor "sock" to the fd_set,
     so that select() will return if a connection comes in
     on that socket (which means you have to do accept(), etc. */

    FD_SET(p_server->sock, &p_server->socks_rd);

    /* Add the socketpair (read side) */
    FD_SET(p_server->sockpair[UIPC_SV_SOCKPAIR_READ_DESC], &p_server->socks_rd);

    /* select needs to know the highest file descriptor */
    p_server->highsock = UIPC_SV_SOCKET_MAX(p_server->sock,
            p_server->sockpair[UIPC_SV_SOCKPAIR_READ_DESC]);

    /* If the Rx mode is in Rx Data event mode OR */
    /* If the Rx mode is in Rx Data Ready event mode AND event requested */
    mode_tmp = p_server->mode & UIPC_SV_SOCKET_MODE_RX_MASK;
    if ((mode_tmp == UIPC_SV_SOCKET_MODE_RX_DATA_EVT) ||
        ((mode_tmp == UIPC_SV_SOCKET_MODE_RX_DATA_READY_EVT) && p_server->rx_ready_request))
    {
        /* Loops through all the possible connections and adds
         those sockets to the fd_set */
        for (listnum = 0; listnum < UIPC_SV_MAX_CLIENT; listnum++)
        {
            if (p_server->connectlist[listnum] != 0)
            {
                FD_SET(p_server->connectlist[listnum], &p_server->socks_rd);
                p_server->highsock = UIPC_SV_SOCKET_MAX(p_server->highsock,
                            p_server->connectlist[listnum]);
            }
        }
    }

    /* If the Tx mode is in Tx Data Ready event mode AND event requested */
    mode_tmp = p_server->mode & UIPC_SV_SOCKET_MODE_TX_MASK;
    if ((mode_tmp == UIPC_SV_SOCKET_MODE_TX_DATA_READY_EVT) && p_server->tx_ready_request)
    {
        /* For the moment, only DG and HL uses this feature and only one client is connect
         * per Data socket */
        FD_SET(p_server->connectlist[0], &p_server->socks_wr);
        p_server->highsock = UIPC_SV_SOCKET_MAX(p_server->highsock, p_server->connectlist[0]);
    }

}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_handle_new_connection
 **
 ** Description
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_socket_handle_new_connection(tUIPC_SV_SOCKET_SERVER *p_server)
{
    int connection; /* Socket file descriptor for incoming connections */
    UINT16 client_num;
    UINT16 number_client_connected;
    BT_HDR *p_msg;

    APPL_TRACE_DEBUG1("uipc_sv_socket_handle_new_connection: p_sock_cb:%x", p_server);

    /* We have a new connection coming in! Find a spot for it in connectlist. */
    connection = accept(p_server->sock, NULL, NULL);
    if (connection < 0)
    {
        APPL_TRACE_ERROR0("uipc_sv_socket_handle_new_connection accept fail");
        exit(EXIT_FAILURE);
    }

    APPL_TRACE_DEBUG1("uipc_sv_socket_handle_new_connection: connection %d", connection);

    /* Set this socket in non blocking mode */
    if ((p_server->mode & UIPC_SV_SOCKET_MODE_BLOCK_MASK) == UIPC_SV_SOCKET_MODE_NON_BLOCK)
    {
        APPL_TRACE_DEBUG0("uipc_sv_socket_handle_new_connection: set NonBlocking");
        uipc_sv_socket_set_non_blocking(connection);
    }
    else
    {
        APPL_TRACE_DEBUG0("uipc_sv_socket_handle_new_connection: leave Blocking");
    }

    /* count number of already connected clients */
    number_client_connected = 0;
    for (client_num = 0; client_num < UIPC_SV_MAX_CLIENT ; client_num++)
    {
        if (p_server->connectlist[client_num] != 0)
        {
            number_client_connected++;
        }
    }
    if (number_client_connected >= p_server->max_client)
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_handle_new_connection: maximum client reached %d", number_client_connected);
    }
    else
    {
        /* Look for free room for this client connection */
        for (client_num = 0; client_num < UIPC_SV_MAX_CLIENT; client_num++)
        {
            if (p_server->connectlist[client_num] == 0)
            {
                /* Allocate a message */
                p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR));
                if (p_msg != NULL)
                {
                    APPL_TRACE_DEBUG2("uipc_sv_socket_handle_new_connection: accepted %d client_num=%d",
                            connection, client_num);
                    p_server->connectlist[client_num] = connection;
                    p_msg->event = UIPC_OPEN_EVT;
                    p_msg->len = 0;
                    p_msg->layer_specific = client_num;
                    /* call the UIPC callback function to indicate connection */
                    p_server->p_uipc_callback(p_msg);
                    return;
                }
                else
                {
                    APPL_TRACE_ERROR0("uipc_sv_socket_handle_new_connection: GKI_getbuf failed");
                }
                break;
            }
        }
    }
    APPL_TRACE_ERROR1("uipc_sv_socket_handle_new_connection: %d refused", connection);
    close(connection);
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_deal_with_data
 **
 ** Description      Handle writable indication for sockets
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_socket_handle_writable(tUIPC_SV_SOCKET_SERVER *p_server)
{
    BT_HDR *p_msg;

    /* If this server is in TxDataReady mode */
    if ((p_server->mode & UIPC_SV_SOCKET_MODE_TX_MASK) == UIPC_SV_SOCKET_MODE_TX_DATA_READY_EVT)
    {
        /* if no UIPC_TX_DATA_READY_EVT sent */
        if (p_server->tx_ready_request)
        {
            p_server->tx_ready_request = FALSE;
            /* Allocate buffer for Rx data Ready Event*/
            p_msg = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR));
            if (p_msg != NULL)
            {
                p_msg->offset = 0;
                p_msg->len = 0;
                p_msg->layer_specific = 0;
                p_msg->event = UIPC_TX_DATA_READY_EVT;
                /* call the Server callback function */
                p_server->p_uipc_callback(p_msg);
            }
        }
        else
        {
            APPL_TRACE_ERROR0("uipc_sv_socket_handle_writable: shouldn't be called");
        }
    }
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_deal_with_data
 **
 ** Description      Handle read indication for sockets
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_socket_deal_with_data(tUIPC_SV_SOCKET_SERVER *p_server, tBSA_CLIENT_NB num_client)
{
    ssize_t length;
    BT_HDR *p_msg;

    /* If this server is in RxDataReady mode */
    if ((p_server->mode & UIPC_SV_SOCKET_MODE_RX_MASK) == UIPC_SV_SOCKET_MODE_RX_DATA_READY_EVT)
    {
        /* if no UIPC_RX_DATA_READY_EVT sent */
        if (p_server->rx_ready_request)
        {
            p_server->rx_ready_request = FALSE;
            /* Allocate buffer for Rx data Ready Event*/
            p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR));
            if (p_msg != NULL)
            {
                p_msg->offset = 0;
                p_msg->len = 0;
                p_msg->layer_specific = 0;
                p_msg->event = UIPC_RX_DATA_READY_EVT;
                /* call the Server callback function */
                p_server->p_uipc_callback(p_msg);
            }
        }
        else
        {
            APPL_TRACE_ERROR0("uipc_sv_socket_deal_with_data: shouldn't be called");
        }
    }
    /* else if this server is in RxData mode */
    else if ((p_server->mode & UIPC_SV_SOCKET_MODE_RX_MASK) == UIPC_SV_SOCKET_MODE_RX_DATA_EVT)
    {
        /* Allocate buffer for Rx data */
        p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR) + MAX_API_BUFFER_SIZE);
        if (p_msg != NULL)
        {
            p_msg->offset = 0;

            /* Read data from client */
            length = read(p_server->connectlist[num_client],
                    (UINT8 *) (p_msg + 1), MAX_API_BUFFER_SIZE);
            if (BCM_UNLIKELY(length <= 0))
            {
                p_msg->event = UIPC_CLOSE_EVT;
                p_msg->len = 0;
                p_msg->layer_specific = (UINT16) num_client;
                p_server->connectlist[num_client] = 0;
                /* call the IUPC callback function */
                p_server->p_uipc_callback(p_msg);
            }
            else
            {
#ifdef UIPC_SV_SOCKET_DEBUG
                APPL_TRACE_DEBUG2("uipc_sv_socket_deal_with_data: rx %d from %d", length, num_client);
                /* scru_dump_hex(p_msg->data, "Data", (length >= 16)?16:length, 0, 0); */
#endif
                p_msg->event = UIPC_RX_DATA_EVT;
                p_msg->len = (UINT16) length;
                p_msg->layer_specific = (UINT16) num_client;
                /* call the IUPC callback function */
                p_server->p_uipc_callback(p_msg);
            }
        }
    }
}

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_read_socks
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static void uipc_sv_socket_read_socks(tUIPC_SV_SOCKET_SERVER *p_server)
{
    int client_num;
    char byte;
    int rv;

    /* Check if the select returned because of socketpair write */
    if (FD_ISSET(p_server->sockpair[UIPC_SV_SOCKPAIR_READ_DESC], &p_server->socks_rd))
    {
        /* Read one byte from the socketpair */
        APPL_TRACE_DEBUG0("uipc_sv_socket_read_socks: socketpair");
        rv = read(p_server->sockpair[UIPC_SV_SOCKPAIR_READ_DESC], &byte, sizeof(byte));
        if (rv <= 0)
        {
            APPL_TRACE_ERROR1("uipc_sv_socket_read_socks: socketpair failed(%d)", rv);
        }
        else if (byte == UIPC_SV_SOCKPAIR_STOP)
        {
            APPL_TRACE_DEBUG0("uipc_sv_socket_read_socks: read Stop event");
            return;
        }
        else if (byte == UIPC_SV_SOCKPAIR_IND)
        {
            APPL_TRACE_DEBUG0("uipc_sv_socket_read_socks: read Ind event");
        }
        else
        {
            APPL_TRACE_ERROR1("uipc_sv_socket_read_socks: read wrong data:0x%x", byte);
        }
    }

    /* OK, now socks will be set with whatever socket(s)
     are ready for reading.  Lets first check our
     "listening" socket, and then check the sockets
     in connectlist. */

    /* If a client is trying to connect() to our listening
     socket, select() will consider that as the socket
     being 'readable'. Thus, if the listening socket is
     part of the fd_set, we need to accept a new connection. */

    if (FD_ISSET(p_server->sock, &p_server->socks_rd))
    {
        uipc_sv_socket_handle_new_connection(p_server);
    }
    /* Now check connectlist for available data */

    /* Run through our sockets and check to see if anything
     happened with them, if so 'service' them. */

    for (client_num = 0; client_num < UIPC_SV_MAX_CLIENT; client_num++)
    {
        if (FD_ISSET(p_server->connectlist[client_num], &p_server->socks_rd))
        {
            uipc_sv_socket_deal_with_data(p_server, client_num);
        }
    } /* for (all entries in queue) */

    /* Check if the socket is 'writable' */
    if (FD_ISSET(p_server->connectlist[0], &p_server->socks_wr))
    {
        uipc_sv_socket_handle_writable(p_server);
    }
}

/*******************************************************************************
 **
 ** Function           uipc_sv_socket_create
 **
 ** Description        Create a socket for the server
 **
 ** Output Parameter
 **
 ** Returns            0 if successful, -1 otherwise
 **
 *******************************************************************************/
static int uipc_sv_socket_create(UINT8 *socket_name, UINT32 socket_desc)
{
    char errorstring[80];
    struct sockaddr_un server_address; /* bind info structure */
    int reuse_addr = 1; /* Used so we can re-bind to our port
     while a previous connection is still
     in TIME_WAIT state. */
    int status;
    tUIPC_SV_SOCKET_SERVER *p_server;

#ifdef UIPC_SV_SOCKET_DEBUG
    APPL_TRACE_DEBUG0("uipc_sv_socket_create");
#endif

    if (socket_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_create: bad socket %d", socket_desc);
        return -1;
    }
    p_server = &uipc_sv_socket_cb.servers[socket_desc];

    /* Obtain a file descriptor for our "listening" socket */
    p_server->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (p_server->sock < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_sv_socket_create: socket failed(%s)", errorstring);
        return -1;
    }
    APPL_TRACE_DEBUG1("uipc_sv_socket_create: fd %d created", p_server->sock);

    /* So that we can re-bind to it without TIME_WAIT problems */
    setsockopt(p_server->sock, SOL_SOCKET, SO_REUSEADDR,
            &reuse_addr, sizeof(reuse_addr));

    /* Set this socket in non blocking mode */
    if ((p_server->mode & UIPC_SV_SOCKET_MODE_BLOCK_MASK) == UIPC_SV_SOCKET_MODE_NON_BLOCK)
    {
        APPL_TRACE_DEBUG0("uipc_sv_socket_create: set NonBlocking");
        uipc_sv_socket_set_non_blocking(p_server->sock);
    }
    else
    {
        APPL_TRACE_DEBUG0("uipc_sv_socket_create: leave Blocking");
    }

    /* Set socket parameters */
    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    /* Check if there is a name prefix to add in front of the UNIX socket name */
    if (uipc_sv_socket_cb.p_data != NULL)
    {
        strncpy(server_address.sun_path, uipc_sv_socket_cb.p_data, sizeof(server_address.sun_path)-1);
    }
    else
    {
        server_address.sun_path[0] = 0;
    }
    strncat(server_address.sun_path, (char*)socket_name, sizeof(server_address.sun_path)-1-strlen(server_address.sun_path));

    /* Try to connect to this socket, if successful, it means that there is already a server running */
    status = connect(p_server->sock, (struct sockaddr *) &server_address, sizeof(server_address));
    if (status == 0)
    {
        /* There is already a bsa_server running on this socket, then close and exit */
        close(p_server->sock);
        return -1;
    }

    /* Remove old socket (if previous server closed without removing it) */
    if (unlink(server_address.sun_path) == 0)
    {
        APPL_TRACE_WARNING1("uipc_sv_socket_create: removed old file:%s", socket_name);
    }

    if (bind(p_server->sock, (struct sockaddr *) &server_address,
            sizeof(server_address)) < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_sv_socket_create: bind failed(%s)", errorstring);
        close(p_server->sock);
        return -1;
    }

    /* Set up queue for incoming connections. */
    status = listen(p_server->sock, UIPC_SV_MAX_CLIENT);
    if (status < 0)
    {
        strerror_r(errno, errorstring, sizeof(errorstring));
        APPL_TRACE_ERROR1("uipc_sv_socket_create: listen failed(%s)", errorstring);
        close(p_server->sock);
        return -1;
    }

    /* Since we start with only one socket, the listening socket,
     it is the highest socket so far. */
    p_server->highsock = p_server->sock;

    memset((char *) p_server->connectlist, 0, sizeof(p_server->connectlist));

    return 0;
}

/*******************************************************************************
 **
 ** Function           socket_server
 **
 ** Description        Thread in charge of continuously reading data
 **
 ** Output Parameter
 **
 ** Returns            nothing
 **
 *******************************************************************************/
static void uipc_sv_socket_read_task(void *arg)
{
    int readsocks;
    UINT32 socket_desc = (UINT32)(UINTPTR) arg;
    tUIPC_SV_SOCKET_SERVER *p_server;

    if (socket_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_read_task: bad socket_desc %d", socket_desc);
        return;
    }
    else
    {
        APPL_TRACE_DEBUG1("uipc_sv_socket_read_task: socket_desc:%u", socket_desc);
    }

    p_server = &uipc_sv_socket_cb.servers[socket_desc];

    /* Main server loop of the socket read thread - while socket open */
    while (p_server->disconnect == FALSE)
    {
        uipc_sv_socket_build_select_list(p_server);

        /* The first argument to select is the highest file
         descriptor value plus 1. In most cases, you can
         just pass FD_SETSIZE and you'll be fine. */

        /* The second argument to select() is the address of
         the fd_set that contains sockets we're waiting
         to be readable (including the listening socket). */

        /* The third parameter is an fd_set that you want to
         know if you can write on . The fourth parameter
         is sockets you're waiting for out-of-band data for,
         which usually, you're not. */

        /* The last parameter to select() is a time-out of how
         long select() should block. If you want to wait forever
         until something happens on a socket, you'll probably
         want to pass NULL. */

        readsocks = select(p_server->highsock + 1,
                &p_server->socks_rd, /* Read file descriptors */
                &p_server->socks_wr, /* Write file descriptors */
                (fd_set *) NULL,     /* OOB file descriptors */
                NULL); /* timeout */

        /* select() returns the number of sockets that had
         things going on with them -- i.e. they're readable. */

        /* Once select() returns, the original fd_set has been
         modified so it now reflects the state of why select()
         woke up. i.e. If file descriptor 4 was originally in
         the fd_set, and then it became readable, the fd_set
         contains file descriptor 4 in it. */
        if (BCM_UNLIKELY(readsocks < 0))
        {
            APPL_TRACE_ERROR1("uipc_sv_socket_read_task: select failed (%d)", readsocks);
            return;
        }
        if (BCM_UNLIKELY(readsocks == 0))
        {
            /* Nothing ready to read, just show that we're alive */
        }
        else
        {
            uipc_sv_socket_read_socks(p_server);
        }
    } /* while */

    /* Close every connected socket */
    for (readsocks = 0 ; readsocks < p_server->max_client ; readsocks++)
    {
        if (p_server->connectlist[readsocks])
        {
            APPL_TRACE_DEBUG1("uipc_sv_socket_read_task: closing Client Sock:%d", readsocks);
            close(p_server->connectlist[readsocks]);
        }
    }
    /* Close the listening socket */
    close(p_server->sock);

    /* Close the internal socketpair */
    close(p_server->sockpair[UIPC_SV_SOCKPAIR_READ_DESC]);
    close(p_server->sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC]);
    p_server->in_use = FALSE;

    APPL_TRACE_DEBUG0("uipc_sv_socket_read_task: exit");

    return;
}

/*******************************************************************************
 **
 ** Function           uipc_sv_socket_ioctl
 **
 ** Description        Control the Socket server channel
 **
 ** Parameters         desc: socket server descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_ioctl(tUIPC_SV_SOCKET_SERVER_DESC socket_desc, UINT32 request, void *param)
{
    tUIPC_SV_SOCKET_SERVER *p_server;
    UINT8 byte;
    char errorstring[80];

    if (socket_desc >= UIPC_SV_SOCKET_SERVER_MAX)
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_ioctl: bad socket %d", socket_desc);
        return FALSE;
    }
    p_server = &uipc_sv_socket_cb.servers[socket_desc];

    if (p_server->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_sv_socket_ioctl: socket %d not in use", socket_desc);
        return FALSE;
    }

    switch(request)
    {
    case UIPC_REQ_TX_READY:
        APPL_TRACE_DEBUG1("uipc_sv_socket_ioctl: socket_desc %d UIPC_REQ_TX_READY", socket_desc);

        p_server->tx_ready_request = TRUE;
        /* Write UIPC_SV_SOCKPAIR_IND value in the "write" side of the socketpair */
        /* This will trig the select and stop the thread */
        byte = UIPC_SV_SOCKPAIR_IND;
        if (write(p_server->sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC], &byte, 1) < 0)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_sv_socket_ioctl: write socketpair failed(%s)", errorstring);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
        break;

    case UIPC_REQ_RX_READY:
        APPL_TRACE_DEBUG1("uipc_sv_socket_ioctl: socket %d UIPC_REQ_RX_READY", socket_desc);
        p_server->rx_ready_request = TRUE;
        /* Write UIPC_SV_SOCKPAIR_IND value in the "write" side of the socketpair */
        /* This will trig the select and stop the thread */
        byte = UIPC_SV_SOCKPAIR_IND;
        if (write(p_server->sockpair[UIPC_SV_SOCKPAIR_WRITE_DESC], &byte, 1) < 0)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_sv_socket_ioctl: write socketpair failed(%s)", errorstring);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
        break;

    default:
        APPL_TRACE_ERROR1("uipc_sv_socket_ioctl: bad request %d", request);
        break;
    }

    return FALSE;
}



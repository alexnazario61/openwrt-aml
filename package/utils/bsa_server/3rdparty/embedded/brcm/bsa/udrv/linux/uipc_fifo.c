/*****************************************************************************
 **
 **  Name:           uipc_fifo.c
 **
 **  Description:    UIPC FIFO interface definition
 **
 **  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "buildcfg.h"
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"
#define BSA_MGT_UIPC_PATH_MAX NSA_MGT_UIPC_PATH_MAX
#else
#include "bsa_api.h"
#include "bsa_int.h"
#endif

#include "gki.h"

#include "uipc_thread.h"
#include "uipc_fifo.h"



#ifndef UIPC_FIFO_DESC_MAX
#define UIPC_FIFO_DESC_MAX          10
#endif

#define UIPC_FIFO_BUFFER_SIZE_MAX   1024

#define UIPC_FIFO_SOCK_READ_DESC    0
#define UIPC_FIFO_SOCK_WRITE_DESC   1
#define UIPC_FIFO_SOCK_NB_DESC      2

typedef struct
{
    struct
    {
        BOOLEAN in_use;
        tUIPC_CH_ID channel_id;
        int fd;
        tUIPC_RCV_CBACK *p_uipc_callback;
        tTHREAD fifo_thread;
        UIPC_FIFO_MODE mode;
        char fifo_name[BSA_MGT_UIPC_PATH_MAX];
        int sockpair[UIPC_FIFO_SOCK_NB_DESC];
    } descs[UIPC_FIFO_DESC_MAX];
    void *p_data;
} tUIPC_FIFO;


tUIPC_FIFO uipc_fifo_cb;

/*
 * Local functions
 */
static void uipc_fifo_task(void *arg);

/*******************************************************************************
 **
 ** Function         uipc_fifo_init
 **
 ** Description      Initialize UIPC FIFO.
 **
 ** Parameters       p_data: underlying configuration opaque pointer
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_fifo_init(void *p_data)
{
    memset(&uipc_fifo_cb, 0, sizeof(uipc_fifo_cb));
    uipc_fifo_cb.p_data = p_data;
    return TRUE;
}


/*******************************************************************************
 **
 ** Function         uipc_fifo_open
 **
 ** Description      Open an UIPC FIFO.
 **
 ** Parameters
 **
 ** Returns          Descriptor index, -1 in case of error
 **
 *******************************************************************************/
tUIPC_FIFO_DESC uipc_fifo_open(UINT8 *fifo_name, tUIPC_CH_ID channel_id, tUIPC_RCV_CBACK *uipc_callback,
        UIPC_FIFO_MODE mode)
{
    char errorstring[80];
    char name[BSA_MGT_UIPC_PATH_MAX];
    int status;
    INT32 index;

    APPL_TRACE_DEBUG1("uipc_fifo_open enter FIFO:%s", fifo_name);

    /* Check if there is a name prefix to add */
    if (uipc_fifo_cb.p_data != NULL)
    {
        strncpy(name, uipc_fifo_cb.p_data, sizeof(name)-1);
        name[sizeof(name)-1] = 0;
    }
    else
    {
        name[0] = 0;
    }

    /* Check that the FIFO name and the prefix does not go beyond max size */
    if ((strlen(name) + strlen((char*)fifo_name)) >= sizeof(name))
    {
        APPL_TRACE_ERROR2("uipc_fifo_open FIFO full name exceeds max length ('%s' + '%s')",
                name, fifo_name);
        return UIPC_FIFO_BAD_DESC;
    }

    /* Append the FIFO name to its prefix) */
    strncat(name, (char*)fifo_name, sizeof(name)-1-strlen(name));
    name[sizeof(name)-1] = 0;

    /* Check Read/Write mode */
    if ((mode & UIPC_FIFO_MODE_READ) &&
        (mode & UIPC_FIFO_MODE_WRITE))
    {
        APPL_TRACE_ERROR0("uipc_fifo_open mode cannot be Rx and Tx");
        return UIPC_FIFO_BAD_DESC;
    }

    /* If open in Read, Rx mode must be specified */
    if ((mode & UIPC_FIFO_MODE_READ) &&
        ((mode & UIPC_FIFO_MODE_RX_MASK) == 0))
    {
        APPL_TRACE_ERROR0("uipc_fifo_open bad Rx mode not specified");
        return UIPC_FIFO_BAD_DESC;
    }

    /* If open in Read, and Data Event => callback needed */
    if ((mode & UIPC_FIFO_MODE_READ) &&
        ((mode & UIPC_FIFO_MODE_RX_MASK) == UIPC_FIFO_MODE_RX_DATA_EVT) &&
        (uipc_callback == NULL))
    {
        APPL_TRACE_ERROR1("uipc_fifo_open no callback for read mode", mode);
        return UIPC_FIFO_BAD_DESC;
    }

    /* Look for a free entry in the FIFO descriptor table */
    for(index = 0 ; index < UIPC_FIFO_DESC_MAX ; index++)
    {
        if (uipc_fifo_cb.descs[index].in_use == FALSE)
        {
            uipc_fifo_cb.descs[index].in_use = TRUE;
            uipc_fifo_cb.descs[index].channel_id = channel_id;
            uipc_fifo_cb.descs[index].p_uipc_callback = uipc_callback;
            uipc_fifo_cb.descs[index].fd = -1;
            uipc_fifo_cb.descs[index].mode = mode;
            uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_READ_DESC] = -1;
            uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_WRITE_DESC] = -1;
            strncpy((char *)uipc_fifo_cb.descs[index].fifo_name, name,
                    sizeof(uipc_fifo_cb.descs[index].fifo_name) - 1);
            uipc_fifo_cb.descs[index].fifo_name[sizeof(uipc_fifo_cb.descs[index].fifo_name) - 1] = 0;
            break;
        }
    }
    /* Check if entry found */
    if (index >= UIPC_FIFO_DESC_MAX)
    {
         APPL_TRACE_ERROR0("uipc_fifo_open mode no more FIFO descriptor free");
         return UIPC_FIFO_BAD_DESC;
    }

    /* If this FIFO must be created */
    if (mode & UIPC_FIFO_MODE_CREATE)
    {
        /* Create Named pipe (FIFO) */
        status = mkfifo(name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (status < 0)
        {
            if (errno == EEXIST)
            {
                APPL_TRACE_WARNING0("FIFO already exist => let's work with it");
            }
            else
            {
                strerror_r(errno, errorstring, sizeof(errorstring));
                APPL_TRACE_ERROR3("uipc_fifo_open fails to create FIFO (named pipe) status:%d errno:%d => %s",
                        status, errno, errorstring);
                uipc_fifo_cb.descs[index].in_use = FALSE; /* Free FIFO descriptor */
                return UIPC_FIFO_BAD_DESC;
            }
        }
    }

    /* If open in Read, and Data Event */
    if ((mode & UIPC_FIFO_MODE_READ) &&
        ((mode & UIPC_FIFO_MODE_RX_MASK) == UIPC_FIFO_MODE_RX_DATA_EVT))
    {
        /* Create a socketpair to be able to kill this reading task */
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, uipc_fifo_cb.descs[index].sockpair) < 0)
        {
            strerror_r(errno, errorstring, sizeof(errorstring));
            APPL_TRACE_ERROR1("uipc_fifo_open socketpair fail: %s", errorstring);
            return UIPC_FIFO_BAD_DESC;
        }
    }

    /* Create a thread in charge of peer open and optionally read */
    status = uipc_thread_create(uipc_fifo_task, (UINT8*)name, 0, 0,
            &uipc_fifo_cb.descs[index].fifo_thread, (void *)(UINTPTR)index);
    if (status < 0)
    {
        APPL_TRACE_ERROR0("uipc_fifo_open fails to create server thread");
        if (mode & UIPC_FIFO_MODE_CREATE)
        {
            /* Delete FIFO */
            unlink(name);
        }
        uipc_fifo_cb.descs[index].in_use = FALSE; /* Free FIFO descriptor */
        return UIPC_FIFO_BAD_DESC;
    }
    return index;
}

/*******************************************************************************
 **
 ** Function         uipc_fifo_close
 **
 ** Description      Close an UIPC FIFO.
 **
 ** Parameters       uipc_desc: descriptor returned on open
 **                  delete: TRUE to delete the FIFO
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_fifo_close(tUIPC_FIFO_DESC uipc_desc, BOOLEAN delete)
{
    char errorstring[80];
    int fd;

    APPL_TRACE_API0("uipc_fifo_close");

    if (uipc_desc >= UIPC_FIFO_DESC_MAX)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_close bad FIFO desc:%d", uipc_desc);
        return FALSE;
    }
    if (uipc_fifo_cb.descs[uipc_desc].in_use == FALSE)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_close FIFO desc not in use:%d", uipc_desc);
        return FALSE;
    }
    if (uipc_fifo_cb.descs[uipc_desc].fd < 0)
    {
        /* Just a warning */
        APPL_TRACE_WARNING1("WARNING uipc_fifo_close FIFO desc not open:%d", uipc_desc);
    }
    else
    {
        fd = uipc_fifo_cb.descs[uipc_desc].fd;   /* save fifo file desc */
        uipc_fifo_cb.descs[uipc_desc].fd = -1;   /* Read thread will exit next loop */

        /* If this fifo is in read mode, the reading thread must be killed */
        if (uipc_fifo_cb.descs[uipc_desc].mode & UIPC_FIFO_MODE_READ)
        {
            if (uipc_fifo_cb.descs[uipc_desc].sockpair[UIPC_FIFO_SOCK_WRITE_DESC] != -1)
            {
                APPL_TRACE_DEBUG0("uipc_fifo_close writing to socketpair");
                /* Write something in the "write" side of the socketpair (to wake up the thread) */
                if (write(uipc_fifo_cb.descs[uipc_desc].sockpair[UIPC_FIFO_SOCK_WRITE_DESC], "q", 1) < 0)
                {
                    strerror_r(errno, errorstring, sizeof(errorstring));
                    APPL_TRACE_ERROR1("uipc_fifo_close cannot write in socketpair:%s", errorstring);
                }
                sleep(1); /* Wait 1000 ms to be sure the read thread receives the msg */
            }
            else
            {
                APPL_TRACE_DEBUG0("uipc_fifo_close read thread already dead.");
            }
        }
        /* close the fifo */
        close(fd);

    }
    if (delete != FALSE)
    {
        unlink((char *)uipc_fifo_cb.descs[uipc_desc].fifo_name);
    }
    uipc_fifo_cb.descs[uipc_desc].in_use = FALSE;

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_fifo_terminate
 **
 ** Description      Delete all UIPC FIFOs.
 **
 ** Parameters       void
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_fifo_terminate(void)
{
#if (defined(BSA_SERVER) || defined(NSA_SERVER))
    int i;

    for(i=0; i<UIPC_FIFO_DESC_MAX; i++)
    {
        if(uipc_fifo_cb.descs[i].in_use != FALSE)
        {
            unlink((char *) uipc_fifo_cb.descs[i].fifo_name);
            APPL_TRACE_DEBUG1("FIFO [%s] deleted",(char *)uipc_fifo_cb.descs[i].fifo_name);
        }
    }
#endif
}

/*******************************************************************************
 **
 ** Function         uipc_fifo_send
 **
 ** Description      This function send data to a Server application
 **
 ** Parameters       buffer: Pointer to buffer to send
 **                  Length: length of buffer
 **
 ** Returns          TRUE if successful, FALSE otherwise.
 **
 *******************************************************************************/
BOOLEAN uipc_fifo_send(tUIPC_FIFO_DESC uipc_desc, UINT8 *buffer, UINT16 length)
{
    ssize_t write_length = 0;

    if (uipc_desc >= UIPC_FIFO_DESC_MAX)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send bad fifo desc:%d", uipc_desc);
        return FALSE;
    }
    if (uipc_fifo_cb.descs[uipc_desc].in_use == FALSE)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send fifo desc not in use:%d", uipc_desc);
        return FALSE;
    }
    if (uipc_fifo_cb.descs[uipc_desc].fd < 0)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send fifo desc not open:%d", uipc_desc);
        return FALSE;
    }
    write_length = write(uipc_fifo_cb.descs[uipc_desc].fd, buffer, length);
    if ((write_length < 0) || (write_length != length))
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send write fail ret:%d", write_length);
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function           uipc_fifo_task
 **
 ** Description        Thread in charge of waiting for FIFO open and optionally read
 **
 ** Output Parameter
 **
 ** Returns            nothing
 **
 *******************************************************************************/
static void uipc_fifo_task(void *arg)
{
    char errorstring[80];
    int status;
    int index = (UINTPTR) arg;
    int flags = 0;
    ssize_t length;
    BT_HDR *p_msg;
    fd_set fd_tab; /* File descriptors we want to wake up for, using select */
    int highest_fd;
    int fifo_fd;

    APPL_TRACE_API0("uipc_fifo_task thread started");

    if (uipc_fifo_cb.descs[index].mode & UIPC_FIFO_MODE_READ)
    {
        flags |= O_RDONLY;
        /* If Asynchronous read used => set it as non blocking */
        if ((uipc_fifo_cb.descs[index].mode & UIPC_FIFO_MODE_RX_MASK) == UIPC_FIFO_MODE_RX_READ)
        {
            flags |= O_NONBLOCK;
        }
    }
    if (uipc_fifo_cb.descs[index].mode & UIPC_FIFO_MODE_WRITE)
    {
        flags |= O_WRONLY;
    }

    /* open the FIFO */
    status = open(uipc_fifo_cb.descs[index].fifo_name, flags);

    if (status < 0)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_task fails to open fifo with errorcode %d",errno);
        uipc_fifo_cb.descs[index].in_use = FALSE;
        return;
    }

    uipc_fifo_cb.descs[index].fd = status; /* Save FIFO/pipe File descriptor */

    if ((uipc_fifo_cb.descs[index].mode & UIPC_FIFO_MODE_WRITE) &&
            ((uipc_fifo_cb.descs[index].mode & UIPC_FIFO_MODE_TX_MASK) == 0))
    {
        flags |= O_NONBLOCK;
        /* Set fifo to be non-blocking. */
        status = fcntl(uipc_fifo_cb.descs[index].fd ,F_SETFL, flags);
        if (status < 0)
        {
           APPL_TRACE_ERROR1("uipc_fifo_task fcntl failed with errorcode %d",errno);
           close(uipc_fifo_cb.descs[index].fd);
           return;
        }
    }

    APPL_TRACE_DEBUG0("uipc_fifo_task fifo opened");

    if ((uipc_fifo_cb.descs[index].mode & UIPC_FIFO_MODE_READ) &&
        ((uipc_fifo_cb.descs[index].mode & UIPC_FIFO_MODE_RX_MASK) == UIPC_FIFO_MODE_RX_DATA_EVT))
    {
        APPL_TRACE_DEBUG0("uipc_fifo_task read thread will now read fifo");
    }
    else
    {
        /* If the FIFO has not been open in read mode with data event. */
        /* => exit this task  */
        APPL_TRACE_DEBUG0("uipc_fifo_task stop read thread");
        uipc_thread_stop(uipc_fifo_cb.descs[index].fifo_thread);
        return;
    }

    /* If we reach this point, this means that the FIFO is in read mode */
    /* Read Main loop - until close */
    do
    {
        length = 0;
        /* Alloc GKI Buffer */
        if ((p_msg = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR) + UIPC_FIFO_BUFFER_SIZE_MAX)) != NULL)
        {
            p_msg->offset = 0;
            fifo_fd = uipc_fifo_cb.descs[index].fd;

            /* This task will wait for events (using select) from either Fifo or socketpair */
            FD_ZERO(&fd_tab);
            FD_SET(uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_READ_DESC], &fd_tab);
            if (fifo_fd >= 0)
                FD_SET(fifo_fd, &fd_tab);

            /* select needs to know the highest file descriptor + 1*/
            if (fifo_fd > uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_READ_DESC])
            {
                highest_fd = fifo_fd + 1;
            }
            else
            {
                highest_fd = uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_READ_DESC] + 1;
            }

            status = select(highest_fd, &fd_tab, (fd_set *) NULL, (fd_set *) NULL, NULL);

            if (status < 0)
            {
                strerror_r(errno, errorstring, sizeof(errorstring));
                APPL_TRACE_ERROR1("uipc_fifo_task select fail:%s", errorstring);
                length = -1;
            }
            else
            {
                /* If there is something to read from the fifo */
                if (FD_ISSET(fifo_fd, &fd_tab))
                {
                    /* Read it */
                    length = read(fifo_fd, (UINT8 *) (p_msg+ 1), UIPC_FIFO_BUFFER_SIZE_MAX);
                    if(length <= 0)
                    {
                        APPL_TRACE_WARNING1("uipc_fifo_task FIFO read return:%d", (int)length);
                        length = -1;
                    }
                }
                /* If there is something to read from the socket pair */
                if(FD_ISSET(uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_READ_DESC], &fd_tab))
                {
                    APPL_TRACE_DEBUG0("uipc_fifo_task socketpair rx indication");
                    length = -1;
                }
            }
            if (length < 0)
            {
                APPL_TRACE_DEBUG0("uipc_fifo_task closing FIFO");
                p_msg->event = UIPC_FIFO_CLOSE_EVT;
                p_msg->len = 0;
            }
            else if (length > 0)
            {
                p_msg->event = UIPC_FIFO_DATA_RX_EVT;
                p_msg->len = length;
            }

            if ((uipc_fifo_cb.descs[index].p_uipc_callback != NULL) && (length != 0))
            {
                uipc_fifo_cb.descs[index].p_uipc_callback(p_msg);
            }
            else
            {
                if(length != 0)
                {
                    APPL_TRACE_ERROR0("ERROR uipc_fifo_task no callback");
                }
                GKI_freebuf(p_msg);
            }
        }
        else
        {
            APPL_TRACE_ERROR0("ERROR uipc_fifo_task no more GKI buffer");
        }
    } while ((uipc_fifo_cb.descs[index].fd != -1) && (length >= 0));

    APPL_TRACE_DEBUG0("uipc_fifo_task read exit");

    close(uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_WRITE_DESC]);
    uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_WRITE_DESC] = -1;
    close(uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_READ_DESC]);
    uipc_fifo_cb.descs[index].sockpair[UIPC_FIFO_SOCK_READ_DESC] = -1;

    uipc_thread_stop(uipc_fifo_cb.descs[index].fifo_thread);
}


/*******************************************************************************
 **
 ** Function           uipc_fifo_read
 **
 ** Description        Function in charge of reading a fifo (for asynchronous read)
 **
 ** Output Parameter
 **
 ** Returns            nothing
 **
 *******************************************************************************/
UINT32 uipc_fifo_read(tUIPC_FIFO_DESC uipc_desc, UINT8 *buffer, UINT16 length)
{
    ssize_t read_length;

    if (uipc_desc >= UIPC_FIFO_DESC_MAX)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_read bad FIFO desc:%d", uipc_desc);
        return 0;
    }
    if (uipc_fifo_cb.descs[uipc_desc].in_use == FALSE)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_read FIFO desc not in use:%d", uipc_desc);
        return 0;
    }
    if (uipc_fifo_cb.descs[uipc_desc].fd < 0)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_read FIFO desc not open:%d", uipc_desc);
        return 0;
    }

    read_length = read(uipc_fifo_cb.descs[uipc_desc].fd, buffer, length);
    if (read_length < 0)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_read read fail ret:%d", read_length);
        read_length = 0;
    }
    else if (read_length != length)
    {
        APPL_TRACE_WARNING2("uipc_fifo_read read fail ret:%d expect:%d", read_length, length);
    }
    return (UINT32)read_length;
}

/*******************************************************************************
 **
 ** Function         uipc_fifo_send_pbuf
 **
 ** Description      This function send data buffer from a pointer to a Client application.
 **                  NOTE 1: THIS FUNCTION DOES NOT FREE THE BUFFER
 **                  NOTE 2: THIS FUNCTION IS CALLED WHEN A BUFFER ADDRESS IS STORED AFTER
 **                          BTH_HDR
 **
 ** Parameters
 **             tUIPC_FIFO_DESC: fifo descriptor
 **             UINT16: client number
 **             BT_HDR
 **                     IN: offset indicates where starts the data
 **                     IN: len the data length (without offset)
 **                     OUT: len the data remaining (not sent)
 **                     OUT: offset indicates number of bytes that have been written
 **                     OUT: layer_specific: 0 if 16 bits granularity respected
 **                                        : 1 if 1 byte has to be saved
 **
 ** Returns          TRUE (buffer fully sent or buffer not fully sent) or FALSE (error)
 **
 *******************************************************************************/
BOOLEAN uipc_fifo_send_pbuf(tUIPC_FIFO_DESC uipc_desc, UINT16 num_client, BT_HDR *p_msg)
{
    UINT8* p_data;
    ssize_t write_length = 0;

    if (uipc_desc >= UIPC_FIFO_DESC_MAX)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send_pbuf bad fifo desc:%d", uipc_desc);
        return FALSE;
    }

    if (uipc_fifo_cb.descs[uipc_desc].in_use == FALSE)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send_pbuf fifo desc not in use:%d", uipc_desc);
        return FALSE;
    }

    if (uipc_fifo_cb.descs[uipc_desc].fd < 0)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send_pbuf fifo desc not open:%d", uipc_desc);
        return FALSE;
    }

    if( (p_msg == NULL) || (*((UINT8 **)(p_msg + 1)) == NULL ))
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send_pbuf fifo desc not open:%d", uipc_desc);
        return FALSE;
    }
    else
    {
        p_data = *((UINT8 **)(p_msg + 1)) + (p_msg->offset);
    }

    write_length = write(uipc_fifo_cb.descs[uipc_desc].fd, p_data, p_msg->len);

    if (write_length < 0)
    {
        APPL_TRACE_ERROR1("ERROR uipc_fifo_send_pbuf write fail ret:%d", errno);
        /* Ensure layer_specific is set to 0 in case of error */
        p_msg->layer_specific = 0;
        return FALSE;
    }

    if(write_length != p_msg->len)
    {
        APPL_TRACE_WARNING2("uipc_fifo_send_pbuf partial write req:%d sent:%d",
                        p_msg->len, write_length);

        /* Signal a partial write */
        p_msg->layer_specific = 1;

        /* Update len (remove number of bytes sent) */
        p_msg->len -= write_length;
        /* Update offset (skip bytes sent) */
        p_msg->offset += write_length;
        return FALSE;
    }
    else
    {
        /* Ensure layer_specific is set to 0 in case of error */
        p_msg->layer_specific = 0;
        p_msg->len = 0;
        p_msg->offset = write_length;
    }

    GKI_freebuf(p_msg);

    return TRUE;
}


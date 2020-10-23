/*****************************************************************************
 **
 **  Name:           uipc_rb.c
 **
 **  Description:    UIPC Ring Buffer interface definition
 **
 **  This code is using the SystemV IPC standard shared memory and semaphores.
 **  In order to run on Cygwin, cygserver must be enabled (cygserver-config).
 **
 **  To check if there are any remaining elements that have not been destroyed
 **  and to destroy them, use the commands ipcs and ipcrm.
 **
 **  This implementation is limited to one consumer and one producer.
 **  Writes operations can be configured blocking or non blocking.
 **  Reads operations are non blocking but there is a "select" API to block
 **  a thread until a given amount of data is available for read.
 **
 **  Copyright (c) 2010-2013, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include "bt_target.h"
#include "bt_trace.h"
#include "uipc.h"
#include "uipc_bsa.h"
#include "uipc_rb.h"
#include "uipc_shm.h"
#include "uipc_sema.h"

/* bit 0 -> enable extra sanity check */
/* bit 1 -> extra init traces */
/* bit 2 -> extra flow traces */
#ifndef UIPC_RB_DBGFLAGS
#define UIPC_RB_DBGFLAGS 3
#endif

/* If the config file does not declare a specific number of RingBuffer */
#ifndef UIPC_RB_DESC_MAX
/* Use 2 for SCO (RX/TX) and 1 for AV */
#define UIPC_RB_DESC_MAX    3
#endif

/* Name of the RingBuffer (for debug/tuning purpose) */
#ifndef UIPC_RB_NAME_LEN_MAX
#define UIPC_RB_NAME_LEN_MAX    20
#endif

/* RingBuffer Header (located/mapped in the Shared Memory */
typedef struct
{
    volatile UINT32 flags;
    volatile UINT32 size;
    volatile UINT32 r_idx;
    volatile UINT32 w_idx;
} tUIPC_RB_HDR;

/* Definition of one RingBuffer */
typedef struct
{
    BOOLEAN in_use;
    tUIPC_CH_ID channel_id;
    char rb_name[UIPC_RB_NAME_LEN_MAX];
    int shmid;
    int semid;
    int wr_lockid;
    int rd_lockid;
    UINT32 size;
    UINT32 flags;
    tUIPC_RB_HDR *p_hdr;
    char *shm;
#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 1)
    char *shm_e;
#endif
} tUIPC_RB;

typedef struct
{
    tUIPC_RB rb_desc[UIPC_RB_DESC_MAX];
    void *p_cfg;
    UINT16 rb_errno;
} tUIPC_RB_CB;


tUIPC_RB_CB uipc_rb_cb;


/*******************************************************************************
 **
 ** Function         uipc_rb_init
 **
 ** Description      Initialize UIPC ring buffer
 **
 ** Parameters       p_cfg: underlying configuration opaque pointer
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_rb_init(void *p_cfg)
{
    memset(&uipc_rb_cb, 0, sizeof(uipc_rb_cb));
    uipc_rb_cb.p_cfg = p_cfg;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_rb_open
 **
 ** Description      Open an UIPC ring buffer
 **
 ** Parameters       rb_name: Name if the RB (debug purpose)
 **                  channel_id: unique ring buffer identifier
 **                  size: ring buffer size
 **                  flags: ring buffer open flags
 **
 ** Returns          tUIPC_RB_DESC
 **
 *******************************************************************************/
tUIPC_RB_DESC uipc_rb_open(UINT8 *rb_name, tUIPC_CH_ID channel_id, UINT32 size,
        UINT32 flags)
{
    key_t key;
    tUIPC_RB *p_uipc_rb;
    tUIPC_RB_HDR *p_hdr = NULL;
    UINT32 totalsize;
    int index;

    APPL_TRACE_API4("uipc_rb_open name:%s id:%d size:%d flags:x%x", rb_name, channel_id, size, flags);

    /* Sanity check */
    if ((flags & (UIPC_RB_MODE_RD | UIPC_RB_MODE_WR)) == 0)
    {
        APPL_TRACE_ERROR0("uipc_rb_open mode must be either RD or WR");
        return UIPC_RB_BAD_DESC;
    }

    /* Check Read/Write mode */
    if ((flags & UIPC_RB_MODE_RD) &&
        (flags & UIPC_RB_MODE_WR))
    {
        APPL_TRACE_ERROR0("uipc_rb_open mode cannot be RD and WR");
        return UIPC_RB_BAD_DESC;
    }

    /* Look for a free entry in the RB descriptor table */
    for(index = 0 ; index < UIPC_RB_DESC_MAX ; index++)
    {
        if (!uipc_rb_cb.rb_desc[index].in_use)
        {
            /* Save parameters */
            p_uipc_rb = &uipc_rb_cb.rb_desc[index];
            p_uipc_rb->in_use = TRUE;
            p_uipc_rb->flags = flags;
            p_uipc_rb->size = size;
            p_uipc_rb->channel_id = channel_id;
            p_uipc_rb->semid = -1;
            p_uipc_rb->wr_lockid = -1;
            p_uipc_rb->rd_lockid = -1;
            p_uipc_rb->shmid = -1;
            p_uipc_rb->p_hdr = (tUIPC_RB_HDR *)NULL;
            strncpy((char *)p_uipc_rb->rb_name, (char *)rb_name, sizeof(p_uipc_rb->rb_name) - 1);
            break;
        }
    }

    /* Check if entry found */
    if (index >= UIPC_RB_DESC_MAX)
    {
         APPL_TRACE_ERROR0("uipc_rb_open mode no more RingBuffer descriptor free");
         return UIPC_RB_BAD_DESC;
    }

    /* Generate a key selector using the configuration path */
    key = ftok((char *)uipc_rb_cb.p_cfg, channel_id);
    if (key < 0)
    {
        APPL_TRACE_ERROR0("uipc_rb_open ftok fail");
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }

    /* Create/Get the MUTEX semaphore instance */
#ifdef BSA_SERVER
    p_uipc_rb->semid = uipc_sema_create(key, 1);
#else
    p_uipc_rb->semid = uipc_sema_get(key);
#endif
    if (p_uipc_rb->semid < 0)
    {
        APPL_TRACE_ERROR0("uipc_rb_open uipc_sema_create (semid) fail");
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }

    /* Create/Get the counter semaphore instance for the write blocking mode */
#ifdef BSA_SERVER
    p_uipc_rb->wr_lockid = uipc_sema_create(key + 1, size - 1);
#else
    p_uipc_rb->wr_lockid = uipc_sema_get(key + 1);
#endif
    if (p_uipc_rb->wr_lockid < 0)
    {
        APPL_TRACE_ERROR0("uipc_rb_open uipc_sema_create (wr_lockid) fail");
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }

    /* Create/Get the counter semaphore instance for the uipc_rb_select API */
#ifdef BSA_SERVER
    p_uipc_rb->rd_lockid = uipc_sema_create(key + 2, 0);
#else
    p_uipc_rb->rd_lockid = uipc_sema_get(key + 2);
#endif
    if (p_uipc_rb->rd_lockid < 0)
    {
        APPL_TRACE_ERROR0("uipc_rb_open uipc_sema_create (rd_lockid) fail");
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }

    /* Lock the access to the SHM */
    if (uipc_sema_lock(p_uipc_rb->semid, 1) < 0)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_open: uipc_sema_lock (semid) fail");
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }

    /* Create the SHM segment in read/write mode with a size that
     * integrates the header shared between the consumer and the
     * producer. To compute the header size, we have to use the size
     * of an array element. */
    totalsize = size + (UINTPTR) (((tUIPC_RB_HDR *)0)+1);
#ifdef BSA_SERVER
    p_uipc_rb->shmid = uipc_shm_create(key + 2, totalsize, (void **)&p_hdr);
#else
    p_uipc_rb->shmid = uipc_shm_get(key + 2, totalsize, (void **)&p_hdr);
#endif
    if (p_uipc_rb->shmid < 0)
    {
        APPL_TRACE_ERROR0("uipc_rb_open shm get/create failed");
        uipc_sema_unlock(p_uipc_rb->semid, 1);
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }

    /* Save the header pointer */
    p_uipc_rb->p_hdr = p_hdr;

    /* Save the Shared Memory address */
    p_uipc_rb->shm = (char *)(p_hdr + 1);

#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 1)
    p_uipc_rb->shm_e = (char*)p_hdr + totalsize;
#endif

#ifdef BSA_SERVER
    /* Initialize the header */
    p_hdr->flags = flags;
    p_hdr->size = size;
    p_hdr->r_idx = 0;
    p_hdr->w_idx = 0;

    /* Reinitialize the counter semaphore value (force value) */
    uipc_sema_set_val(p_uipc_rb->wr_lockid, size - 1);

#else /* BSA_SERVER */

    /* Sanity check: it should never occur that the same access type is requested */
    if (p_hdr->flags & flags)
    {
        APPL_TRACE_ERROR2("uipc_rb_open bad flags p_hdr->flags:%x flags:%x",
                p_hdr->flags, flags);
        /* Unlock the access to the SHM */
        uipc_sema_unlock(p_uipc_rb->semid, 1);
        /* Close this ring buffer entry */
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }
    /* Sanity check: both consumer and producer sizes should be identical */
    if (p_hdr->size != size)
    {
        APPL_TRACE_ERROR2("uipc_rb_open bad size p_hdr->size:%x size:%x",
                p_hdr->size, size);
        /* Unlock the access to the SHM */
        uipc_sema_unlock(p_uipc_rb->semid, 1);
        /* Close this ring buffer entry */
        uipc_rb_close(index);
        return UIPC_RB_BAD_DESC;
    }
#endif /* !BSA_SERVER */

    /* Unlock the access to the SHM */
    uipc_sema_unlock(p_uipc_rb->semid, 1);

#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 2)
    APPL_TRACE_DEBUG1("uipc_rb_open: Size = %lu", p_uipc_rb->size);
    APPL_TRACE_DEBUG1("uipc_rb_open: Buffer = %p", (p_hdr + 1));
    APPL_TRACE_DEBUG1("uipc_rb_open: semid = %d", p_uipc_rb->semid);
    APPL_TRACE_DEBUG1("uipc_rb_open: wr_lockid = %d", p_uipc_rb->wr_lockid);
    APPL_TRACE_DEBUG1("uipc_rb_open: wr_lockid.val = %d",
            uipc_sema_get_val(p_uipc_rb->wr_lockid));
#endif

    /* Return the descriptor */
    return index;
}

/*******************************************************************************
 **
 ** Function         uipc_rb_close
 **
 ** Description      Close an UIPC ring buffer
 **
 ** Parameters       desc: RB descriptor returned on open
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_rb_close(tUIPC_RB_DESC desc)
{
    tUIPC_RB *p_uipc_rb;
    tUIPC_RB_HDR *p_hdr;

    APPL_TRACE_API1("uipc_rb_close desc:%d", desc);

    if ((desc >= UIPC_RB_DESC_MAX) || (desc < 0))
    {
        APPL_TRACE_ERROR1("uipc_rb_close bad RB desc:%d", desc);
        return FALSE;
    }
    p_uipc_rb = &uipc_rb_cb.rb_desc[desc];
    if (!p_uipc_rb->in_use)
    {
        APPL_TRACE_ERROR1("uipc_rb_close RB desc not in use:%d", desc);
        return FALSE;
    }

    /* Lock the access to the SHM, we still proceed if there is an error */
    if (uipc_sema_lock(p_uipc_rb->semid, 1))
    {
        APPL_TRACE_ERROR0("uipc_rb_close uipc_sema_lock failed");
    }

    /* Detach the Shared Memory */
    p_hdr = p_uipc_rb->p_hdr;
    if (p_hdr)
    {
        /* Mark detached (before actually detaching to prevent segfaults) */
        p_uipc_rb->p_hdr = NULL;

        /* clear the flags */
        p_hdr->flags &= ~(p_uipc_rb->flags);

        /* Detach the segment */
        uipc_shm_detach(p_uipc_rb->shmid, p_hdr);
    }

#ifdef BSA_SERVER
    /* Destroy the write block SEM */
    uipc_sema_delete(p_uipc_rb->wr_lockid);

    /* Destroy the read block SEM */
    uipc_sema_delete(p_uipc_rb->rd_lockid);

    /* Destroy the SEM */
    uipc_sema_delete(p_uipc_rb->semid);
    p_uipc_rb->semid = -1;

#else /* BSA_SERVER */
    /* Unlock the access to the SHM */
    uipc_sema_unlock(p_uipc_rb->semid, 1);
#endif /* !BSA_SERVER */

    APPL_TRACE_DEBUG1("uipc_rb_close rb:%s closed", p_uipc_rb->rb_name);

    /* Mark control block unused */
    p_uipc_rb->in_use = FALSE;

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_rb_write
 **
 ** Description      Write data into the UIPC ring buffer
 **
 ** Parameters       desc: ring buffer descriptor returned on open
 **                  buf: pointer to buffer containing the data to write
 **                  len: length of data to write
 **
 ** Returns          negative error code or the number of bytes written
 **
 *******************************************************************************/
INT32 uipc_rb_write(tUIPC_RB_DESC desc, UINT8 *buf, UINT32 len)
{
    tUIPC_RB *p_uipc_rb;
    tUIPC_RB_HDR *p_hdr;
    UINT32 remain;
    UINT32 tmp32;
    UINT32 available;

#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 4)
    APPL_TRACE_DEBUG3("uipc_rb_write desc:%d b:%p l:%d", desc, buf, len);
#endif

    /* Sanity check */
    if (len == 0)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_write: len = 0");
        uipc_rb_cb.rb_errno = UIPC_EINVAL;
        return -1;
    }
    if ((desc >= UIPC_RB_DESC_MAX) || (desc < 0))
    {
        APPL_TRACE_ERROR1("uipc_rb_write bad RB desc:%d", desc);
        uipc_rb_cb.rb_errno = UIPC_EINVAL;
        return -1;
    }
    p_uipc_rb = &uipc_rb_cb.rb_desc[desc];
    if (p_uipc_rb->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_rb_write RB desc not in use:%d", desc);
        uipc_rb_cb.rb_errno = UIPC_EINVAL;
        return -1;
    }

#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 1)
    /* Sanity check */
    if ((p_uipc_rb->flags & UIPC_RB_MODE_WR) == 0)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_write: flags wrong");
        uipc_rb_cb.rb_errno = UIPC_EINVAL;
        return -1;
    }
#endif

    if (p_uipc_rb->p_hdr == NULL)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_write: shared memory detached");
        uipc_rb_cb.rb_errno = UIPC_EINVAL;
        return -1;
    }

    /* Get the header : after checking the close was not called */
    p_hdr = p_uipc_rb->p_hdr;

    /* If the write is blocking */
    if (p_hdr->flags & UIPC_RB_MODE_WR_BLOCK)
    {
        /* Check that data will not be thrown away */
        if (len >= p_uipc_rb->size)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_write: wrong size");
            uipc_rb_cb.rb_errno = UIPC_EINVAL;
            return -1;
        }
        /* Wait until enough space available to write our data */
        if (uipc_sema_lock(p_uipc_rb->wr_lockid, len) < 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_write: uipc_sema_lock (wr_lockid) fail");
            uipc_rb_cb.rb_errno = UIPC_EINVAL;
            return -1;
        }
    }
    else
    {
        /* Check that data will not be thrown away */
        if (len > p_uipc_rb->size)
        {
            APPL_TRACE_WARNING1("uipc_rb_write: Too much data added at once (%lu), data will be lost", len);
        }
    }

    remain = len;
    /* Lock the access to the SHM */
    if (uipc_sema_lock(p_uipc_rb->semid, 1) < 0)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_write: uipc_sema_lock (semid) fail");
        uipc_rb_cb.rb_errno = UIPC_EINVAL;
        return -1;
    }

    while (remain)
    {
        /* Compute the size available for write (always keep 1 element free) */
        available = (p_hdr->r_idx + p_uipc_rb->size - p_hdr->w_idx - 1) % p_uipc_rb->size;

        /* check write space available */
        if (available == 0)
        {
            uipc_rb_cb.rb_errno = UIPC_ENOMEM;
            /* If the write is blocking */
            if (p_hdr->flags & UIPC_RB_MODE_WR_BLOCK)
            {
                /* This is an error in blocking mode: This error is due to the Flush */
                APPL_TRACE_ERROR0("uipc_rb_write Write Semaphore/Indexes value mismatch: data lost");
                APPL_TRACE_ERROR4("uipc_rb_write r=%lu w=%lu s=%lu m=%lu",
                        p_hdr->r_idx, p_hdr->w_idx, p_uipc_rb->size, remain);
            }
            else
            {
                /* This is a warning in non blocking mode */
                APPL_TRACE_WARNING0("uipc_rb_write: no more space available, data will be lost");
                APPL_TRACE_WARNING4("uipc_rb_write r=%lu w=%lu s=%lu m=%lu",
                        p_hdr->r_idx, p_hdr->w_idx, p_uipc_rb->size, remain);
            }
            /* For both cases exit the loop */
            break;
        }

        /* Check if the size available is greater than requested len */
        if (available > remain)
        {
            available = remain;
        }

        /* Check if the write is contiguous */
        if ((p_hdr->w_idx + available) <= p_uipc_rb->size)
        {
            /* Copy the contiguous memory */
            memcpy(&p_uipc_rb->shm[p_hdr->w_idx], buf, available);

            /* Move the pointer */
            tmp32 = p_hdr->w_idx + available;
            if (tmp32 == p_uipc_rb->size)
            {
                tmp32 = 0;
            }
            p_hdr->w_idx = tmp32;
        }
        else
        {
            UINT32 partlen;

            /* Copy the first part */
            partlen = p_uipc_rb->size - p_hdr->w_idx;
            memcpy(&p_uipc_rb->shm[p_hdr->w_idx], buf, partlen);

            /* Copy the second part */
            memcpy(p_uipc_rb->shm, buf + partlen, available - partlen);

            /* Move the pointer */
            tmp32 = available - partlen;
            p_hdr->w_idx = tmp32;

        }
        /* Decrement the remaining size */
        remain -= available;

#if (BT_USE_TRACES == TRUE)
        /* Sanity check */
        if (p_hdr->w_idx >= p_uipc_rb->size)
        {
            APPL_TRACE_ERROR2("uipc_rb_write bad write index", p_hdr->w_idx,
                    p_uipc_rb->size);
        }
        if (p_uipc_rb->shm > &p_uipc_rb->shm[p_hdr->w_idx])
        {
            APPL_TRACE_ERROR2("uipc_rb_write bad write address1", p_uipc_rb->shm,
                    &p_uipc_rb->shm[p_hdr->w_idx]);
        }
        if (p_uipc_rb->shm_e <= &p_uipc_rb->shm[p_hdr->w_idx])
        {
            APPL_TRACE_ERROR2("uipc_rb_write bad write address2", p_uipc_rb->shm_e,
                    &p_uipc_rb->shm[p_hdr->w_idx]);
        }
#endif
        buf += available;
    }

    /* If the write is blocking => let's update the wr_lockid semaphore count*/
    if (p_hdr->flags & UIPC_RB_MODE_WR_BLOCK)
    {
        /* Compute the size available for write (always keep 1 element free) */
        available = (p_hdr->r_idx + p_uipc_rb->size - p_hdr->w_idx - 1) % p_uipc_rb->size;
        if (available != (UINT32)uipc_sema_get_val(p_uipc_rb->wr_lockid))
        {
            APPL_TRACE_WARNING2("uipc_rb_write wr_lockid sem was not correct cur:%d expect:%d",
                    uipc_sema_get_val(p_uipc_rb->wr_lockid), available);
            uipc_sema_set_val(p_uipc_rb->wr_lockid, available);
        }
    }

    /* Release the select blocking semaphore and keep the counter in range  */
    uipc_sema_set_val(p_uipc_rb->rd_lockid,
            (p_hdr->w_idx + p_uipc_rb->size - p_hdr->r_idx) % p_uipc_rb->size);

    /* Release the access to the SHM */
    uipc_sema_unlock(p_uipc_rb->semid, 1);

    return len-remain;
}

/*******************************************************************************
 **
 ** Function           uipc_rb_read
 **
 ** Description        Read data from UIPC ring buffer
 **
 ** Parameters         desc: ring buffer descriptor returned on open
 **                    buf: pointer to buffer containing the data to read
 **                    len: maximum length of data to read
 **
 ** Returns            negative error code or the number of bytes read
 **
 *******************************************************************************/
INT32 uipc_rb_read(tUIPC_RB_DESC desc, UINT8 *buf, UINT32 len)
{
    tUIPC_RB *p_uipc_rb;
    tUIPC_RB_HDR *p_hdr;
    UINT32 available;
    UINT32 tmp32;

#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 4)
    APPL_TRACE_DEBUG3("uipc_rb_read desc:%d b:%p l:%d", desc, buf, len);
#endif

    /* Sanity check */
    if (len == 0)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_read: len = 0");
        return -1;
    }
    if ((desc >= UIPC_RB_DESC_MAX) || (desc < 0))
    {
        APPL_TRACE_ERROR1("uipc_rb_read bad RB desc:%d", desc);
        return -1;
    }
    p_uipc_rb = &uipc_rb_cb.rb_desc[desc];
    if (p_uipc_rb->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_rb_read RB desc not in use:%d", desc);
        return -1;
    }

#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 1)
        /* Sanity check */
        if ((p_uipc_rb->flags & UIPC_RB_MODE_RD) == 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_read: flags wrong");
            return -1;
        }
#endif


    /* Check if the shared memory can still be accessed */
    if (p_uipc_rb->p_hdr != NULL)
    {
        /* Get the header : after checking the close was not called */
        p_hdr = p_uipc_rb->p_hdr;

        /* Lock the access to the SHM */
        if (uipc_sema_lock(p_uipc_rb->semid, 1) < 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_read: uipc_sema_lock (semid) fail");
            return -1;
        }

        /* Compute the size available for read */
        available = (p_hdr->w_idx + p_uipc_rb->size - p_hdr->r_idx) % p_uipc_rb->size;
        if (available < len)
        {
            APPL_TRACE_WARNING0("uipc_rb_read: no enough data available");
            APPL_TRACE_WARNING4("uipc_rb_read r=%lu w=%lu s=%lu av=%lu",
                p_hdr->r_idx, p_hdr->w_idx, p_uipc_rb->size, available);
        }

        /* Check if the size available is greater than requested len */
        if (available > len) available = len;

        /* Check if the read is contiguous */
        if ((p_hdr->r_idx + available) <= p_uipc_rb->size)
        {
            /* Copy the contiguous memory */
            memcpy(buf, &p_uipc_rb->shm[p_hdr->r_idx], available);

            /* Move the pointer */
            tmp32 = p_hdr->r_idx + available;
            if (tmp32 == p_uipc_rb->size)
            {
                tmp32 = 0;
            }
            p_hdr->r_idx = tmp32;
        }
        else
        {
            UINT32 partlen;

            /* Copy the first part */
            partlen = p_uipc_rb->size - p_hdr->r_idx;
            memcpy(buf, &p_uipc_rb->shm[p_hdr->r_idx], partlen);

            /* Copy the second part */
            memcpy(buf + partlen, p_uipc_rb->shm, available - partlen);

            /* Move the pointer */
            tmp32 = available - partlen;
            p_hdr->r_idx = tmp32;
        }
#if (BT_USE_TRACES == TRUE)
        /* Sanity check */
        if (p_hdr->r_idx >= p_uipc_rb->size)
        {
            APPL_TRACE_ERROR2("uipc_rb_write bad read index", p_hdr->r_idx, p_uipc_rb->size);
        }
        if (p_uipc_rb->shm > &p_uipc_rb->shm[p_hdr->r_idx])
        {
            APPL_TRACE_ERROR2("uipc_rb_write bad read address1",
                    p_uipc_rb->shm, &p_uipc_rb->shm[p_hdr->r_idx]);
        }
        if (p_uipc_rb->shm_e <= &p_uipc_rb->shm[p_hdr->r_idx])
        {
            APPL_TRACE_ERROR2("uipc_rb_write bad read address2",
                    p_uipc_rb->shm_e, &p_uipc_rb->shm[p_hdr->r_idx]);
        }
#endif
        if (p_hdr->flags & UIPC_RB_MODE_WR_BLOCK)
        {
            if (available != 0)
            {
                uipc_sema_unlock(p_uipc_rb->wr_lockid, available);
            }
        }

        /* Release the access to the SHM */
        uipc_sema_unlock(p_uipc_rb->semid, 1);
    }
    else
    {
        available = -1;
    }

    return available;
}

/*******************************************************************************
 **
 ** Function           uipc_rb_select
 **
 ** Description        select data from UIPC ring buffer.
 **
 ** Parameters         desc: ring buffer descriptor returned on open.
 **                    len: maximum length of data to read
 **
 ** Returns            negative error code or the number of bytes available to read
 **
 *******************************************************************************/
INT32 uipc_rb_select(tUIPC_RB_DESC desc, UINT32 len)
{
    tUIPC_RB *p_uipc_rb;
    tUIPC_RB_HDR *p_hdr;

#if defined(UIPC_RB_DBGFLAGS) && (UIPC_RB_DBGFLAGS & 4)
    APPL_TRACE_DEBUG2("uipc_rb_select desc:%d l=%d ", desc, len);
#endif

    /* Sanity check */
    if (len == 0)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_select: len = 0");
        return -1;
    }
    if ((desc >= UIPC_RB_DESC_MAX) || (desc < 0))
    {
        APPL_TRACE_ERROR1("uipc_rb_select bad RB desc:%d", desc);
        return -1;
    }
    p_uipc_rb = &uipc_rb_cb.rb_desc[desc];
    if (p_uipc_rb->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_rb_select RB desc not in use:%d", desc);
        return -1;
    }

    /* Get the header : after checking the close was not called */
    p_hdr = p_uipc_rb->p_hdr;

    /* Wait until all data is available */
    if (uipc_sema_lock(p_uipc_rb->rd_lockid, len) < 0)
    {
        APPL_TRACE_ERROR0("ERROR uipc_rb_select: uipc_sema_lock (rd_lockid) fail");
        return -1;
    }


    return (p_hdr->w_idx + p_uipc_rb->size - p_hdr->r_idx) % p_uipc_rb->size;
}


/*******************************************************************************
 **
 ** Function           uipc_rb_ioctl
 **
 ** Description        Control the ring buffer
 **
 ** Parameters         desc: ring buffer descriptor returned on open
 **                    request: request type
 **                    param: pointer to the request parameters
 **
 ** Returns            TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_rb_ioctl(tUIPC_RB_DESC desc, UINT32 request, void *param)
{
    tUIPC_RB *p_uipc_rb;

    if (desc >= UIPC_RB_DESC_MAX)
    {
        APPL_TRACE_ERROR1("uipc_rb_ioctl bad RB desc:%d", desc);
        return -1;
    }
    p_uipc_rb = &uipc_rb_cb.rb_desc[desc];

    if (p_uipc_rb->in_use == FALSE)
    {
        APPL_TRACE_ERROR1("uipc_rb_ioctl RB desc not in use:%d", desc);
        return -1;
    }

    switch (request)
    {
    case UIPC_WRITE_BLOCK:
        if ((p_uipc_rb->flags & UIPC_RB_MODE_WR) == 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_ioctl: not WR side");
            return FALSE;
        }
        /* Lock the access to the SHM */
        if (uipc_sema_lock(p_uipc_rb->semid, 1) < 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_ioctl BL: uipc_sema_lock (semid) fail");
            return -1;
        }

        p_uipc_rb->p_hdr->r_idx = 0;
        p_uipc_rb->p_hdr->w_idx = 0;

        /* reset the read/write semaphore */
        uipc_sema_set_val(p_uipc_rb->rd_lockid, 0);
        uipc_sema_set_val(p_uipc_rb->wr_lockid, p_uipc_rb->size - 1);

        /* Set the bit in the local flags */
        p_uipc_rb->flags |= UIPC_RB_MODE_WR_BLOCK;
        /* Set the bit in the common flags */
        p_uipc_rb->p_hdr->flags |= UIPC_RB_MODE_WR_BLOCK;


        /* Release the access to the SHM */
        uipc_sema_unlock(p_uipc_rb->semid, 1);
        break;

    case UIPC_WRITE_NONBLOCK:
        if ((p_uipc_rb->flags & UIPC_RB_MODE_WR) == 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_ioctl: not WR side");
            return FALSE;
        }

        /* Lock the access to the SHM */
        if (uipc_sema_lock(p_uipc_rb->semid, 1) < 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_ioctl NBL: uipc_sema_lock (semid) fail");
            return -1;
        }
        p_uipc_rb->p_hdr->r_idx = 0;
        p_uipc_rb->p_hdr->w_idx = 0;

        /* reset the read/write semaphore */
        uipc_sema_set_val(p_uipc_rb->rd_lockid, 0);
        uipc_sema_set_val(p_uipc_rb->wr_lockid, p_uipc_rb->size - 1);

        /* Clear the bit in the local flags */
        p_uipc_rb->flags &= ~UIPC_RB_MODE_WR_BLOCK;
        /* Clear the bit in the common flags */
        p_uipc_rb->p_hdr->flags &= ~UIPC_RB_MODE_WR_BLOCK;

        /* Release the access to the SHM */
        uipc_sema_unlock(p_uipc_rb->semid, 1);
        break;

    case UIPC_RESET :
        /* to be used in case there is a possibility of dead lock when client or server application died */
        /* Release the access to the SHM */
        uipc_sema_set_val(p_uipc_rb->semid, 1);
        /* falls through */
    case UIPC_REQ_TX_FLUSH :
    case UIPC_REQ_RX_FLUSH :
        /* Lock the access to the SHM */
        if (uipc_sema_lock(p_uipc_rb->semid, 1) < 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_ioctl NBL: uipc_sema_lock (semid) fail");
            return -1;
        }
        p_uipc_rb->p_hdr->r_idx = 0;
        p_uipc_rb->p_hdr->w_idx = 0;

        /* Reset the read/write semaphore */
        uipc_sema_set_val(p_uipc_rb->rd_lockid, 0);
        uipc_sema_set_val(p_uipc_rb->wr_lockid, p_uipc_rb->size - 1);

        /* Release the access to the SHM */
        uipc_sema_unlock(p_uipc_rb->semid, 1);
        break;

    case UIPC_REG_CBACK :
        if ((p_uipc_rb->flags & UIPC_RB_MODE_RD) == 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_ioctl: not read side");
            return FALSE;
        }

        /* if someone is blocked on the rd sema let's release it */
        uipc_sema_unlock(p_uipc_rb->rd_lockid, (long)param);
        break;

    case UIPC_READ_ERROR :
        /* Lock the access to the SHM */
        if (uipc_sema_lock(p_uipc_rb->semid, 1) < 0)
        {
            APPL_TRACE_ERROR0("ERROR uipc_rb_ioctl NBL: uipc_sema_lock (semid) fail");
            return FALSE;
        }
        if(param == NULL)
        {
            return FALSE;
        }

        *(UINT16 *)param = uipc_rb_cb.rb_errno;

        /* Release the access to the SHM */
        uipc_sema_unlock(p_uipc_rb->semid, 1);
        break;

    default:
        APPL_TRACE_ERROR1("ERROR uipc_rb_ioctl: unsupported IOCTL request %d", request);
        return FALSE;
        break;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         uipc_rb_terminate
 **
 ** Description      Close (free all resources) all UIPC RingBuffers.
 **
 ** Parameters       void
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_rb_terminate(void)
{
#if (defined(BSA_SERVER) || defined(NSA_SERVER))
    int i;

    for(i=0; i < UIPC_RB_DESC_MAX; i++)
    {
        if(uipc_rb_cb.rb_desc[i].in_use != FALSE)
        {
            /* Reset the iorb first to force the close */
            uipc_rb_ioctl(i, UIPC_RESET, NULL);
            uipc_rb_close(i);
        }
    }
#endif
}


/*****************************************************************************
 **
 **  Name:           uipc_rb.h
 **
 **  Description:    UIPC Ring Buffer interface declaration
 **
 **  Copyright (c) 2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_RB_H
#define UIPC_RB_H

#include "data_types.h"

#define UIPC_RB_BAD_DESC (-1)
typedef INT32 tUIPC_RB_DESC;

/* Define the open modes (bitfield) */
#define UIPC_RB_MODE_RD         0x01
#define UIPC_RB_MODE_WR         0x02
#define UIPC_RB_MODE_WR_BLOCK   0x04


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
BOOLEAN uipc_rb_init(void *p_cfg);

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
        UINT32 flags);


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
BOOLEAN uipc_rb_close(tUIPC_RB_DESC desc);

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
INT32 uipc_rb_write(tUIPC_RB_DESC desc, UINT8 *buf, UINT32 len);

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
INT32 uipc_rb_read(tUIPC_RB_DESC desc, UINT8 *buf, UINT32 len);

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
INT32 uipc_rb_select(tUIPC_RB_DESC desc, UINT32 len);

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
BOOLEAN uipc_rb_ioctl(tUIPC_RB_DESC desc, UINT32 request, void *param);

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
void uipc_rb_terminate(void);

#endif /* UIPC_RB_H */



/*****************************************************************************
 **
 **  Name:           uipc_fifo.h
 **
 **  Description:    UIPC FIFO interface declaration
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_FIFO_H
#define UIPC_FIFO_H

#define UIPC_FIFO_BAD_DESC (-1)
typedef INT32 tUIPC_FIFO_DESC;


/* The following definitions define the way used to get Rx data
 * An Rx data event can be received when rx data are available or the
 * Application can use the read API (in blocking or non blocking mode)
 */
#define UIPC_FIFO_MODE_CREATE                     0x01    /*  */
#define UIPC_FIFO_MODE_READ                       0x02    /*  */
#define UIPC_FIFO_MODE_WRITE                      0x04    /*  */

#define UIPC_FIFO_MODE_RX_DATA_EVT                 0x08    /* Callback */
#define UIPC_FIFO_MODE_RX_READ                     0x10    /* Async Read */
#define UIPC_FIFO_MODE_RX_MASK                     0x18

#define UIPC_FIFO_MODE_TX_DATA_BLOCK               0x20    /* blocking */
#define UIPC_FIFO_MODE_TX_MASK                     0x20

typedef UINT8 UIPC_FIFO_MODE;


/* Events generated */
#define UIPC_FIFO_OPEN_EVT                       0x01
#define UIPC_FIFO_CLOSE_EVT                      0x02
#define UIPC_FIFO_DATA_RX_EVT                    0x03


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
BOOLEAN uipc_fifo_init(void *p_data);

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
tUIPC_FIFO_DESC uipc_fifo_open(UINT8 *fifo_name, tUIPC_CH_ID channel_id,
        tUIPC_RCV_CBACK *uipc_callback, UIPC_FIFO_MODE mode);

/*******************************************************************************
 **
 ** Function         uipc_fifo_close
 **
 ** Description      This function close a channel
 **
 ** Parameters       uipc_desc: descriptor returned on open
 **                  delete: TRUE to delete the FIFO
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN uipc_fifo_close(tUIPC_FIFO_DESC uipc_desc, BOOLEAN delete);

/*******************************************************************************
 **
 ** Function         uipc_fifo_send
 **
 ** Description      This function send data on a channel
 **
 ** Parameters       buffer: Pointer to buffer to send
 **                  Length: length of buffer
 **
 ** Returns
 **
 *******************************************************************************/
BOOLEAN uipc_fifo_send(tUIPC_FIFO_DESC uipc_desc, UINT8 *buffer, UINT16 length);

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
 *              tUIPC_FIFO_DESC: fifo descriptor
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
BOOLEAN uipc_fifo_send_pbuf(tUIPC_FIFO_DESC uipc_desc, UINT16 num_client, BT_HDR *p_msg);

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
UINT32 uipc_fifo_read(tUIPC_FIFO_DESC uipc_desc, UINT8 *buffer, UINT16 length);

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
void uipc_fifo_terminate(void);
#endif


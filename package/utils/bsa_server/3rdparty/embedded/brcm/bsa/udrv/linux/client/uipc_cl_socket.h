/*****************************************************************************
**
**  Name:           uipc_cl_socket.h
**
**  Description:    Socket Client for BSA
**
**  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#ifndef UIPC_CL_SOCKET_H
#define UIPC_CL_SOCKET_H

/* The following definitions define the way used to get Rx data
 * An Rx data event can be received when rx data are available or the
 * Application can use the read API (in blocking or non blocking mode)
 */
/* The following definitions define the way the socket is used */

/*
 * A socket can be either in Blocking or NonBlocking mode.
 * In Blocking mode, both read and write operations are Blocking
 * In NonBlocking mode, both read and write operations are NonBlocking
 */
#define UIPC_CL_SOCKET_MODE_BLOCK              0x00   /* read/write Blocking */
#define UIPC_CL_SOCKET_MODE_NON_BLOCK          0x01   /* read/write Non blocking */
#define UIPC_CL_SOCKET_MODE_BLOCK_MASK         0x01

/*
 * For Rx path (from server), the socket can be configured to:
 * 1/Read Data as soon as they are available and send an event to the callback or
 * 2/Send an event to the callback to indicate that data are available. In this case,
 *   the higher layer will have to read the data explicitly
 */
#define UIPC_CL_SOCKET_MODE_RX_DATA_EVT        0x00    /* Data Callback */
#define UIPC_CL_SOCKET_MODE_RX_DATA_READY_EVT  0x02    /* Data Rx Ready Callback */
#define UIPC_CL_SOCKET_MODE_RX_MASK            0x02

/*
 * For Tx path (to server), the socket can be configured to:
 * 1/be blocking
 * 2/be NonBlocking. If data cannot be, completely, written in the socket, it will
 *   send an event later (via the callback) to indicate that the socket is ready for
 *   write access. Note that a specific IOCTL must be called for this purpose.
 */
#define UIPC_CL_SOCKET_MODE_TX_DATA_READY_EVT  0x04    /* Data Tx Ready Callback */
#define UIPC_CL_SOCKET_MODE_TX_MASK            0x04

typedef UINT8 tUIPC_CL_SOCKET_MODE;  /* Client Socket Mode */

/* Maximum number of client sockets (default: Control & DataGateway & HDP) */
#ifndef UIPC_CL_SOCKET_DESC_MAX
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
#define UIPC_CL_SOCKET_DESC_MAX 1
#else
//#define UIPC_CL_SOCKET_DESC_MAX (1+UIPC_CH_ID_DG_NB+UIPC_CH_ID_HL_NB)
/*consider also nfc control socket even for bsa standalone (to not duplicate libBsa)*/
#define UIPC_CL_SOCKET_DESC_MAX (1+1+UIPC_CH_ID_DG_NB+UIPC_CH_ID_HL_NB)
#endif
#endif

typedef UINT8 tUIPC_CL_SOCKET_DESC; /* Client Socket Descriptor */


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
BOOLEAN uipc_cl_socket_init(void *p_data);

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
        tUIPC_RCV_CBACK *p_cback, tUIPC_CL_SOCKET_MODE mode);

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
BOOLEAN uipc_cl_socket_disconnect(tUIPC_CL_SOCKET_DESC socket_desc);

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
BOOLEAN uipc_cl_socket_send(tUIPC_CL_SOCKET_DESC socket_desc, UINT8 *p_buf, UINT16 length);

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
BOOLEAN uipc_cl_socket_send_buf(tUIPC_CL_SOCKET_DESC socket_desc, BT_HDR *p_msg);

/*******************************************************************************
 **
 ** Function         uipc_cl_socket_write
 **
 ** Description      This, low level, function write data buffer in the socket.
 **
 ** Parameters       socket_desc: socket descriptor
**                   p_buf: Pointer to buffer to send
 **                  length: length of buffer
 **
 ** Returns          Status: -1 for Error or number of bytes written
 **
 *******************************************************************************/
int uipc_cl_socket_write(tUIPC_CL_SOCKET_DESC socket_desc, UINT8 *buffer, UINT16 length);

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
UINT32 uipc_cl_socket_read(tUIPC_CL_SOCKET_DESC socket_desc, UINT8 *p_buf, UINT32 len);

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
BOOLEAN uipc_cl_socket_ioctl(tUIPC_CL_SOCKET_DESC socket_desc,
        UINT32 request, void *param);

#endif



/*****************************************************************************
 **
 **  Name:           uipc_sv_socket.h
 **
 **  Description:    Socket Server for BSA
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef UIPC_SV_SOCKET_H
#define UIPC_SV_SOCKET_H

/*redefine if we are compiling for NSA server*/
#if defined (NSA) && (NSA == TRUE)
#define BSA_MGT_UIPC_PATH_MAX NSA_MGT_UIPC_PATH_MAX
#define BSA_SUCCESS NSA_SUCCESS
#endif

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
#define UIPC_SV_SOCKET_MODE_BLOCK              0x00   /* read/write Blocking */
#define UIPC_SV_SOCKET_MODE_NON_BLOCK          0x01   /* read/write Non blocking */
#define UIPC_SV_SOCKET_MODE_BLOCK_MASK         0x01

/*
 * For Rx path (from application), the socket can be configured to:
 * 1/Read Data as soon as they are available and send an event to the callback or
 * 2/Send an event to the callback to indicate that data are available. In this case,
 *   the higher layer will have to read the data explicitly
 */
#define UIPC_SV_SOCKET_MODE_RX_DATA_EVT        0x00    /* Data Callback */
#define UIPC_SV_SOCKET_MODE_RX_DATA_READY_EVT  0x02    /* Data Rx Ready Callback */
#define UIPC_SV_SOCKET_MODE_RX_DATA_NO_EVT     0x04    /* No Rx Data Indication */
#define UIPC_SV_SOCKET_MODE_RX_MASK            0x06

/*
 * For Tx path (to application), the socket can be configured to:
 * 1/be blocking
 * 2/be NonBlocking. If data cannot be, completely, written in the socket, it will
 *   send an event later (via the callback) to indicate that the socket is ready for
 *   write access. Note that a specific IOCTL must be called for this purpose.
 */
#define UIPC_SV_SOCKET_MODE_TX_DATA_READY_EVT  0x08    /* Data Tx Ready Callback */
#define UIPC_SV_SOCKET_MODE_TX_MASK            0x08

/* Maximum number of server sockets (default: Control & DataGateway & HDP & NSA) */
#ifndef UIPC_SV_SOCKET_SERVER_MAX
#if defined (NSA) && (NSA == TRUE)
#define UIPC_SV_SOCKET_SERVER_MAX 1
#else
#define UIPC_SV_SOCKET_SERVER_MAX   (1 + UIPC_CH_ID_DG_NB + UIPC_CH_ID_HL_NB + 1)
#endif
#endif

typedef UINT8 tUIPC_SV_SOCKET_SERVER_DESC; /* Socket Server Descriptor */

typedef UINT8 tUIPC_SV_SOCKET_MODE; /* Server Mode */

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
BOOLEAN uipc_sv_socket_init(void *p_data);

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_start_server
 **
 ** Description      Start UIPC Server socket.
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
tUIPC_SV_SOCKET_SERVER_DESC uipc_sv_socket_start_server(UINT8 *socket_name, tUIPC_RCV_CBACK *uipc_callback,
        UINT32 max_client, tUIPC_SV_SOCKET_MODE mode);

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
BOOLEAN uipc_sv_socket_stop_server(tUIPC_SV_SOCKET_SERVER_DESC socket_desc, UINT8 *socket_name);

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_send
 **
 ** Description      This function send data to a Client application
 **
 ** Parameters       listnum: New connection index (0..MAX_CLIENT-1)
 **                  buffer: Pointer to buffer to send
 **                  Length: length of buffer
 **
 ** Returns          0 if ok, -1 if failure
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_send(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        tBSA_CLIENT_NB num_client, UINT8 *buffer, UINT16 length);

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
        tBSA_CLIENT_NB num_client, BT_HDR *p_msg);

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
        tBSA_CLIENT_NB num_client,  BT_HDR *p_msg);

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
        tBSA_CLIENT_NB num_client, UINT8 *p_buf, UINT32 len);

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_terminate
 **
 ** Description      Delete all UIPC sockets.
 **
 ** Returns          void
 **
 *******************************************************************************/
void uipc_sv_socket_terminate(void);

/*******************************************************************************
 **
 ** Function         uipc_sv_socket_ioctl
 **
 ** Description      Control a socket server.
 **
 ** Returns          void
 **
 *******************************************************************************/
BOOLEAN uipc_sv_socket_ioctl(tUIPC_SV_SOCKET_SERVER_DESC socket_desc,
        UINT32 request, void *param);

#endif

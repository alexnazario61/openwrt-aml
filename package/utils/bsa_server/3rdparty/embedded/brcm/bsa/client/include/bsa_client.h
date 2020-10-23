/*****************************************************************************
 **
 **  Name:           bsa_client.h
 **
 **  Description:    Server API for BSA
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_CLIENT_API_H
#define BSA_CLIENT_API_H

#define MAX_API_BUFFER_SIZE 1024

/* Disconnection Callback */
typedef void ( tBSA_CLIENT_DISC_CBACK)(int reason);

/* Rx data Callback */
typedef void ( tBSA_CLIENT_RX_CBACK)(int msg_id, unsigned char *buffer,
        int length);


/*******************************************************************************
 **
 ** Function         bsa_send_message_receive_status
 **
 ** Description      This function send a message and receive the status response from
 **                  Server application
 **
 ** Parameters       tx_buffer: Pointer to buffer to send
 **                  tx_lngth: length of buffer
 **
 ** Returns          BSA_SUCCESS if ok,  client or server error code if failure
 **
 *******************************************************************************/
tBSA_STATUS bsa_send_message_receive_status(int message_id, void *tx_buffer,
        int tx_length);

/*******************************************************************************
 **
 ** Function         bsa_send_receive_message
 **
 ** Description      This function send a message and receive the response to/from
 **                  Server application
 **
 ** Parameters       tx_buffer: Pointer to buffer to send
 **                  tx_lngth: length of buffer
 **                  rx_buffer: Pointer to buffer for reception
 **                  rx_length: length of buffer for reception
 **
 ** Returns          size of received buffer if ok, -1 if failure
 **
 *******************************************************************************/
tBSA_STATUS bsa_send_receive_message(int message_id, void *tx_buffer, int tx_length,
        void *rx_buffer, int rx_length);





#endif

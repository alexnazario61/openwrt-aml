/*****************************************************************************
 **
 **  Name:           bsa_strm_dec.h
 **
 **  Description:    Functions in charge of decoding received stream for BSA
 **
 **  Copyright (c) 2009-2011, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_STRM_DEC_H
#define BSA_STRM_DEC_H

#define BSA_MAX_MSG_SIZE (1024)

typedef enum
{
    RX_STREAM_WAIT_4_LEN_L = 0,
    RX_STREAM_WAIT_4_LEN_H,
    RX_STREAM_WAIT_4_MSG_ID_L,
    RX_STREAM_WAIT_4_MSG_ID_H,
    RX_STREAM_WAIT_4_DATA
} tRX_STREAM_STATE;

typedef struct
{
    unsigned char data[BSA_MAX_MSG_SIZE]; /* Keep this field first to avoid word alignment issues */
    int received_data; /* how many bytes already received */
    int decoded_length; /* Length extracted from the received data */
    int decoded_msg_id;
    tRX_STREAM_STATE state;
    int flush_data;
} tBSA_RX_STREAM;

/*******************************************************************************
 **
 ** Function         bsa_strm_dec_init
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns
 **
 *******************************************************************************/
int bsa_strm_dec_init(tBSA_RX_STREAM *p_rx_stream);

/*******************************************************************************
 **
 ** Function         bsa_strm_dec
 **
 ** Description      .
 **
 ** Parameters
 **
 ** Returns 0 if message received is not complete otherwise 1
 **
 *******************************************************************************/
int bsa_strm_dec(tBSA_RX_STREAM *p_rx_stream, unsigned char **p_buffer,
        int *p_length);

#endif

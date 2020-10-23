/*****************************************************************************
 **
 **  Name:           bsa_strm_dec.c
 **
 **  Description:    Functions in charge of decoding received stream for BSA
 **
 **  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bt_target.h"
#include "bsa_strm_dec.h"

#define MIN(a,b) (((a) < (b))? (a): (b))

#if 0
#define DEBUG_BSA_STREAM_DECODE
#endif

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
int bsa_strm_dec_init(tBSA_RX_STREAM *p_rx_stream)
{
    if (p_rx_stream != NULL)
    {
        memset(p_rx_stream, 0, sizeof(tBSA_RX_STREAM));
    }
    else
    {
        APPL_TRACE_ERROR0("ERROR bsa_strm_dec_init error null pointer");
        return -1;
    }
    return 0;
}

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
        int *p_length)
{
    unsigned char *p_data;
    int copy_len;
    int length;

    p_data = *p_buffer;
    length = *p_length;

    while (length != 0)
    {
        if (p_rx_stream->state == RX_STREAM_WAIT_4_LEN_L)
        {
            p_rx_stream->decoded_length = *p_data++;
            p_rx_stream->state = RX_STREAM_WAIT_4_LEN_H;
            length--;
            continue; /* Next bytes */
        }
        if (p_rx_stream->state == RX_STREAM_WAIT_4_LEN_H)
        {
            p_rx_stream->decoded_length += *p_data++ << 8;
            p_rx_stream->state = RX_STREAM_WAIT_4_MSG_ID_L;
            p_rx_stream->received_data = 0;
#ifdef DEBUG_BSA_STREAM_DECODE
            APPL_TRACE_DEBUG1("bsa_strm_dec LEN:%d", p_rx_stream->decoded_length);
#endif
            length--;
            if (p_rx_stream->decoded_length < 2)
            {
                APPL_TRACE_ERROR1("bsa_strm_dec ERROR bad len:%d",
                        p_rx_stream->decoded_length);
                p_rx_stream->state = RX_STREAM_WAIT_4_LEN_L;
            }
            else if (p_rx_stream->decoded_length > BSA_MAX_MSG_SIZE)
            {
                APPL_TRACE_ERROR1("bsa_strm_dec ERROR bad len:%d",
                        p_rx_stream->decoded_length);
                p_rx_stream->flush_data = 1;
            }
            continue; /* Next bytes */
        }

        if (p_rx_stream->state == RX_STREAM_WAIT_4_MSG_ID_L)
        {
            p_rx_stream->decoded_msg_id = *p_data++;
            p_rx_stream->state = RX_STREAM_WAIT_4_MSG_ID_H;
            length--;
            continue; /* Next bytes */
        }

        if (p_rx_stream->state == RX_STREAM_WAIT_4_MSG_ID_H)
        {
            p_rx_stream->decoded_msg_id += *p_data++ << 8;
            p_rx_stream->state = RX_STREAM_WAIT_4_DATA;
            length--;
#ifdef DEBUG_BSA_STREAM_DECODE
            APPL_TRACE_DEBUG1("bsa_strm_dec MSG_ID:%d", p_rx_stream->decoded_msg_id);
#endif
            /* If decoded_length == 2 => Message ID without parameters */
            if (p_rx_stream->decoded_length == 2)
            {
                /* A complete message has been received, return */
                p_rx_stream->decoded_length -= 2;
                *p_buffer = p_data;
                *p_length = length;
                return 1;
            }
            /* skip msg_ig */
            p_rx_stream->decoded_length -= 2;
            continue; /* Next bytes */
        }

        if (p_rx_stream->state == RX_STREAM_WAIT_4_DATA)
        {
            /* Check min (remaining room available and received buffer and decoded_length) */
            copy_len
                    = MIN(BSA_MAX_MSG_SIZE - p_rx_stream->received_data, length);
            copy_len = MIN(copy_len, (p_rx_stream->decoded_length-p_rx_stream->received_data));
            if (p_rx_stream->flush_data == 0)
            {
                memcpy(&p_rx_stream->data[p_rx_stream->received_data], p_data,
                        copy_len);
            }
            p_data += copy_len;
            length -= copy_len;
            p_rx_stream->received_data += copy_len;
#ifdef DEBUG_BSA_STREAM_DECODE
            APPL_TRACE_DEBUG4("received_data:%d; decoded_length:%d, copy_len %d, len %d", p_rx_stream->received_data, p_rx_stream->decoded_length, copy_len, length);
#endif
            if (p_rx_stream->decoded_length == p_rx_stream->received_data)
            {
#ifdef DEBUG_BSA_STREAM_DECODE
                APPL_TRACE_DEBUG1("p_rx_stream->flush_data:%d", p_rx_stream->flush_data);
#endif
                if (p_rx_stream->flush_data == 0)
                {
                    /* A complete message has been received, return */
                    *p_buffer = p_data;
                    *p_length = length;
                    return 1;
                }
            }
            continue;
        }
        APPL_TRACE_ERROR1("bsa_strm_dec bad state:%d", p_rx_stream->state);
    }

    /* If we reach this point, this means that the message has not been fully received */
    *p_buffer = p_data;
    *p_length = length;

    return 0;
}

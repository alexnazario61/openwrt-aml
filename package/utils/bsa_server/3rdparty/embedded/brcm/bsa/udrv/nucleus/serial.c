/*******************************************************************************
 **  Name:       serial.c
 **
 **  Description:
 **
 **  This file contains the universal driver wrapper for the serial drivers
 **
 **  Copyright (c) 2005-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *******************************************************************************/

#include "bt_target.h"
#include <string.h>
#include "gki.h"
#include "userial.h"

#define READ_LIMIT 2000

extern UINT8 read_buffer[];
extern int read_from;
extern int write_to;
int bt_serial_trigger = 1;

BUFFER_Q Userial_in_q;
static BT_HDR *pRxBuf = NULL;


static UINT32 userial_baud_tbl[] = {
    300,
    600,
    1200,
    2400,
    9600,
    19200,
    57600,
    115200,
    230400,
    460800,
    921600,
    1024000,
    1843200
};

/*******************************************************************************
 **
 ** Function           USERIAL_GetLineSpeed
 **
 ** Description        This function convert USERIAL baud to line speed.
 **
 ** Output Parameter   None
 **
 ** Returns            line speed
 **
 *******************************************************************************/
UDRV_API extern UINT32 USERIAL_GetLineSpeed(UINT8 baud)
{
    if (baud <= USERIAL_BAUD_1_5M)
    {
       APPL_TRACE_DEBUG1("USERIAL_GetLineSpeed speed=%d", 
        (userial_baud_tbl[baud - USERIAL_BAUD_300]));
        return (userial_baud_tbl[baud - USERIAL_BAUD_300]);
    }    
    else
    {
        APPL_TRACE_ERROR0("USERIAL_GetLineSpeed speed=0 !");
        return 0;
    }
}

/*******************************************************************************
 **
 ** Function           USERIAL_GetBaud
 **
 ** Description        This function convert line speed to USERIAL baud.
 **
 ** Output Parameter   None
 **
 ** Returns            line speed
 **
 *******************************************************************************/
UDRV_API extern UINT8 USERIAL_GetBaud(UINT32 line_speed)
{
    UINT8 i;
    
    for (i = USERIAL_BAUD_300; i <= USERIAL_BAUD_921600; i++)
    {
        if (userial_baud_tbl[i - USERIAL_BAUD_300] == line_speed)
        {
           APPL_TRACE_DEBUG1("USERIAL_GetBaud baud=%u", i);
           return i;
        }
    }

    APPL_TRACE_DEBUG1("USERIAL_GetBaud baud=%u", USERIAL_BAUD_AUTO);
    return USERIAL_BAUD_AUTO;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Init
 **
 ** Description        This function initializes the  serial driver.
 **
 ** Output Parameter   None
 **
 ** Returns            Nothing
 **
 *******************************************************************************/

UDRV_API void    USERIAL_Init(void *p_cfg)
{

    BT_UART_INIT_PORT();

    GKI_init_q(&Userial_in_q);

    BT_UART_INIT(1152);    /* Baud rate 115200 */
    BT_UART_START_DMA();
    /* Add a 100 msec delay for USERIAL Init */
    GKI_delay(100);
    APPL_TRACE_DEBUG0("USERIAL_Init done ");

    return;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Open
 **
 ** Description        Open the indicated serial port with the given configuration
 **
 ** Output Parameter   None
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
UDRV_API void USERIAL_Open(tUSERIAL_PORT port, tUSERIAL_OPEN_CFG *p_cfg, tUSERIAL_CBACK *p_cback)
{
    APPL_TRACE_DEBUG0("USERIAL_Open() ");
}

/*******************************************************************************
 **
 ** Function           USERIAL_Read
 **
 ** Description        Read data from a serial port using byte buffers.
 **
 ** Output Parameter   None
 **
 ** Returns            Number of bytes actually read from the serial port and
 **                    copied into p_data.  This may be less than len.
 **
 *******************************************************************************/
UDRV_API UINT16 USERIAL_Read(tUSERIAL_PORT port, UINT8 *p_data, UINT16 len)
{
    UINT16  total_len = 0;
       UINT16  copy_len = 0;
       UINT8    *current_packet = NULL;

    do
    {
        if (pRxBuf != NULL)
        {
            current_packet = ((UINT8 *) (pRxBuf + 1)) + (pRxBuf->offset);

            if ((pRxBuf->len) <=  ( len - total_len))
            {
                copy_len = pRxBuf->len;
            }    
            else
            {
                copy_len =  ( len - total_len);
            }
            memcpy((p_data + total_len), current_packet, copy_len); 

            total_len += copy_len;
            pRxBuf->offset  += copy_len;
            pRxBuf->len  -= copy_len;

            if (pRxBuf->len == 0)
            {
                GKI_freebuf(pRxBuf);
                pRxBuf = NULL;
            }
            }

        if (pRxBuf == NULL)
        {
            pRxBuf = (BT_HDR*) GKI_dequeue (&Userial_in_q);
        }
    } while((pRxBuf != NULL) && (total_len < len));


    if ((!total_len)  || (pRxBuf == NULL))
    {
        bt_serial_trigger = 1;
    }

    return total_len;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Readbuf
 **
 ** Description        Read data from a serial port using GKI buffers.
 **
 ** Output Parameter   Pointer to a GKI buffer which contains the data.
 **
 ** Returns            Nothing
 **
 ** Comments           The caller of this function is responsible for freeing the
 **                    GKI buffer when it is finished with the data.  If there is
 **                    no data to be read, the value of the returned pointer is
 **                    NULL.
 **
 *******************************************************************************/

UDRV_API void USERIAL_ReadBuf(tUSERIAL_PORT port, BT_HDR **p_buf)
{
    APPL_TRACE_DEBUG0("USERIAL_ReadBuf() ");

}

/*******************************************************************************
 **
 ** Function           USERIAL_WriteBuf
 **
 ** Description        Write data to a serial port using a GKI buffer.
 **
 ** Output Parameter   None
 **
 ** Returns            TRUE  if buffer accepted for write.
 **                    FALSE if there is already a buffer being processed.
 **
 ** Comments           The buffer will be freed by the serial driver.  Therefore,
 **                    the application calling this function must not free the
 **                    buffer.
 **
 *******************************************************************************/

UDRV_API BOOLEAN USERIAL_WriteBuf(tUSERIAL_PORT port, BT_HDR *p_buf)
{
    APPL_TRACE_DEBUG0("USERIAL_WriteBuf() ");
    return FALSE;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Write
 **
 ** Description        Write data to a serial port using a byte buffer.
 **
 ** Output Parameter   None
 **
 ** Returns            Number of bytes actually written to the serial port.  This
 **                    may be less than len.
 **
 *******************************************************************************/
UDRV_API UINT16 USERIAL_Write(tUSERIAL_PORT port, UINT8 *p_data, UINT16 len)
{
    BT_UART_SEND_DATA(p_data, len);

    return len;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Ioctl
 **
 ** Description        Perform an operation on a serial port.
 **
 ** Output Parameter   The p_data parameter is either an input or output depending
 **                    on the operation.
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
UDRV_API void USERIAL_Ioctl(tUSERIAL_PORT port, tUSERIAL_OP op, tUSERIAL_IOCTL_DATA *p_data)
{
    APPL_TRACE_DEBUG1("USERIAL_Ioctl() op:%u", op);
    switch (op)
    {
    case USERIAL_OP_FLUSH:
        break;
    case USERIAL_OP_BAUD_WR:
        if(p_data->baud == USERIAL_BAUD_1_5M)
        {
            BT_UART_INIT(18432);    /* Baud rate 1843200 */
            BT_UART_START_DMA();
        }
        else
            {
            BT_UART_INIT(1152);    /* Baud rate 115200 */
            BT_UART_START_DMA();
        }
        GKI_delay(150);            /* BT baseband need delay time more than 150mS */
        break;
    case USERIAL_OP_FLUSH_RX:
    case USERIAL_OP_FLUSH_TX:
    default:
        break;
    }

    return;
}

/*******************************************************************************
 **
 ** Function           USERIAL_Close
 **
 ** Description        Close a serial port
 **
 ** Output Parameter   None
 **
 ** Returns            Nothing
 **
 *******************************************************************************/
UDRV_API void USERIAL_Close(tUSERIAL_PORT port)
{
    APPL_TRACE_DEBUG1("USERIAL_Close() port:%u", port);
}

/*******************************************************************************
 **
 ** Function           USERIAL_Feature
 **
 ** Description        Check whether a feature of the serial API is supported.
 **
 ** Output Parameter   None
 **
 ** Returns            TRUE  if the feature is supported
 **                    FALSE if the feature is not supported
 **
 *******************************************************************************/
UDRV_API BOOLEAN USERIAL_Feature(tUSERIAL_FEATURE feature)
{
    switch (feature)
    {
    case USERIAL_FEAT_PORT_1:
    case USERIAL_FEAT_PORT_2:


    case USERIAL_FEAT_BAUD_600:
    case USERIAL_FEAT_BAUD_1200:
    case USERIAL_FEAT_BAUD_9600:
    case USERIAL_FEAT_BAUD_19200:
    case USERIAL_FEAT_BAUD_57600:
    case USERIAL_FEAT_BAUD_115200:

    case USERIAL_FEAT_STOPBITS_1:
    case USERIAL_FEAT_STOPBITS_2:
    case USERIAL_FEAT_STOPBITS_1_5:

    case USERIAL_FEAT_PARITY_NONE:
    case USERIAL_FEAT_PARITY_EVEN:
    case USERIAL_FEAT_PARITY_ODD:

    case USERIAL_FEAT_DATABITS_5:
    case USERIAL_FEAT_DATABITS_6:
    case USERIAL_FEAT_DATABITS_7:
    case USERIAL_FEAT_DATABITS_8:

    case USERIAL_FEAT_FC_HW:
    case USERIAL_FEAT_BUF_BYTE:

    case USERIAL_FEAT_OP_FLUSH_RX:
    case USERIAL_FEAT_OP_FLUSH_TX:
        return TRUE;
    default:
        return FALSE;
    }

    return FALSE;
}

void Timer_BrdBt_fn(unsigned long temp)
{
    unsigned int rx_length = 0;
    BT_HDR *p_buf = NULL;
    UINT8 *current_packet;    

    rx_length = BT_UART_TX_AVAIL_DATA();

    if (rx_length)
    {
        /*APPL_TRACE_DEBUG1("Timer_BrdBt_fn() rx_length:%u", rx_length);*/
        p_buf = (BT_HDR *) GKI_getbuf(BT_HDR_SIZE + rx_length);

        if (p_buf == NULL)
        {
            /* Check out buffer, not sure this is necessary */
            p_buf = (BT_HDR*) GKI_dequeue (&Userial_in_q);
        }

        if (p_buf != NULL)
        {
            p_buf->offset = 0;
            p_buf->len = rx_length;
            p_buf->layer_specific = 0;
            current_packet = (UINT8 *) (p_buf + 1);
            BT_UART_RECEIVE_DATA(current_packet, rx_length);
            
            GKI_enqueue(&Userial_in_q, p_buf);

            if (bt_serial_trigger)
            {
                bt_serial_trigger = 0;
                GKI_isend_event(HCISU_TASK,HCISU_EVT_MASK);
            }
        }

    }

}


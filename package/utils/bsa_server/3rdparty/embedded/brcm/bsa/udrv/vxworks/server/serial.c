/*******************************************************************************
**  Name:       userial.c
**
**  Description:
**
**  This file contains the universal driver wrapper for the Serial BT USB
**  drivers
**
**  Copyright (c) 2010, Broadcom Corporation, All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
*******************************************************************************/
#include "vxWorks.h"
#include "taskLib.h"
#include "stdio.h"
#include "bt_target.h"
/*#include <string.h>*/
#include "gki.h"
#include "usb.h"
#include "bt_trace.h"
#include "hcidefs.h"

void btUsbClientInit(void);
void btUsb_CtlOut_send(UINT8 *, UINT16);
void btUsb_BulkOut_send(UINT8 *, UINT16);

//#define USERIAL_DUMP
#ifdef USERIAL_DUMP
#define USERIAL_DUMP_BYTES 30
//char rx_log_buff[USERIAL_DUMP_BYTES*3+16];
char tx_log_buff[USERIAL_DUMP_BYTES*3+16];
#endif


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
        return 0;
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
    return USB_BAUD_AUTO;
}

/*******************************************************************************
**
** Function           USERIAL_Init
**
** Description        This function initializes the  USB driver. 
**
** Output Parameter   None
**
** Returns            Nothing
**
*******************************************************************************/
UDRV_API void    USERIAL_Init(void *p_cfg)
{
    btUsbClientInit();
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
UDRV_API void    USERIAL_Open(tUSB_PORT port, tUSB_OPEN_CFG *p_cfg, tUSB_CBACK *p_cback)
{
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
UDRV_API UINT16  USERIAL_Read(tUSB_PORT port, UINT8 *p_data, UINT16 len)
{
#if 0
#ifdef USERIAL_DUMP
	UINT16 xx;
#endif
	UINT32 byte_read=0;
	
#if 0
	if (port == USERIAL_PORT_1)
		port = UART_PORT_1;
	else if (port == USERIAL_PORT_2)
		port = UART_PORT_2;
	else
		return 0;
#endif

#ifdef USERIAL_DUMP
	APPL_TRACE_DEBUG1("Ask %d", len);
	rx_log_buff[0] = 0;
#endif

#if 0
	// Block till received "len" number of bytes
	UartRead(port, (INT8 *) p_data, len, &byte_read, 0, 1);
#endif

#ifdef USERIAL_DUMP
	for (xx=0; (xx < byte_read) && (xx < USERIAL_DUMP_BYTES); xx++)
	{
		sprintf((rx_log_buff+xx*3), "%02x ", (int) *(p_data+xx));
	}

	rx_log_buff[xx*3] = 0;

	if (xx < byte_read)
	{
		APPL_TRACE_DEBUG1("Got[ %s ... ]", rx_log_buff);
	}
	else
	{
		APPL_TRACE_DEBUG1("Got[ %s]", rx_log_buff);
	}
#endif

	return ((UINT16) byte_read);
#else
    APPL_TRACE_ERROR0("USERIAL_Read empty");
	return (0);
#endif
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
UDRV_API void USERIAL_ReadBuf(tUSB_PORT port, BT_HDR **p_buf)
{
    APPL_TRACE_ERROR0("USERIAL_ReadBuf empty");
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
UDRV_API BOOLEAN USERIAL_WriteBuf(tUSB_PORT port, BT_HDR *p_buf)
{
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
UDRV_API UINT16  USERIAL_Write(UINT8 type, UINT8 *p_data, UINT16 len)
{
#ifdef USERIAL_DUMP
	UINT16 xx;

	for (xx=0; (xx < len) && (xx < USERIAL_DUMP_BYTES) ; xx++)
	{
		sprintf((tx_log_buff+xx*3), "%02x ", (int) *(p_data+xx));
	}

	tx_log_buff[xx*3] = 0;

	if (xx < len)
	{
		APPL_TRACE_DEBUG1("Write[ %s ... ]", tx_log_buff);
	}
	else
	{
		APPL_TRACE_DEBUG1("Write[ %s]", tx_log_buff);
	}
#endif

    type = *p_data++;   /* Get Packet type from buffer and skip this byte */
    len--;              /* reduce len (type byte skipped */
//    APPL_TRACE_DEBUG2("USERIAL_Write 2 PacketType:%d len:%d", type, len);

	if (type == HCIT_TYPE_COMMAND)
		btUsb_CtlOut_send(p_data, len);
	else if (type == HCIT_TYPE_ACL_DATA)
		btUsb_BulkOut_send(p_data, len);
	else if (type == HCIT_TYPE_SCO_DATA)
	{
		APPL_TRACE_DEBUG0("SCO over Transport is not supported yet");
	}
	else
    {
        APPL_TRACE_ERROR1("Bad PacketType:%d", type);
    }
	return (len);
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
UDRV_API void    USERIAL_Ioctl(tUSB_PORT port, tUSB_OP op, tUSB_IOCTL_DATA *p_data)
{
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
UDRV_API void    USERIAL_Close(tUSB_PORT port)
{
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
UDRV_API BOOLEAN USERIAL_Feature(tUSB_FEATURE feature)
{
    return FALSE;
}


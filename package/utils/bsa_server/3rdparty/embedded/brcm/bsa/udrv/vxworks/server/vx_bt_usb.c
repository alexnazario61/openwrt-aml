/*******************************************************************************
**  Name:       vx_bt_usb.c
**
**  Description:
**
**  This file contains the Bluetooth USB client for vxWorks USBD
**
**  Copyright (c) 2008, Broadcom Corporation, All Rights Reserved.
**  WIDCOMM Bluetooth Core. Proprietary and confidential.
*******************************************************************************/
#include <vxWorks.h>
//#include <ioLib.h>
#include <stdio.h>
#include <string.h>
#include <sysLib.h>
#include <taskLib.h>
#include <usb/usbPlatform.h>
#include <usb/ossLib.h>
#include <usb/usb.h>
#include <usb/usbdLib.h>
#include <usb/usbLib.h>

#include "bt_target.h"
#include "gki.h"
#include "hcidefs.h"
#include "bt_trace.h"
#include "btu.h"

#define BTUSB_INT_PRIORITY /*3*/ 128
#define BTUSB_INT_STACK_SIZE /*2048*/(16*1024)
#define BTUSB_BLKIN_PRIORITY /*2*/ 128
#define BTUSB_BLKIN_STACK_SIZE /*2048*/(16*1024)

#define USB_BT_BULK_IRP_TIME_OUT 5000	/* in millisecs */

/* USB_BT_DEV Structure - used to describe USB Bluetooth device */
typedef struct usbBTDev
{
    USBD_NODE_ID      btDevId;      /* USBD node ID of the device     */     
    UINT16            configuration;  /* Configuration value            */    
    UINT16            interface;      /* Interface number               */
    UINT16            altSetting;     /* Alternate setting of interface */ 
    UINT16            inIntEpAddress;    /* Interrupt in EP address             */
    UINT16            outBkEpAddress;   /* Bulk out EP address            */   
    UINT16            inBkEpAddress;    /* Bulk in EP address             */
    USBD_PIPE_HANDLE  inIntPipeHandle;   /* Pipe handle for Interrupt in EP     */
    USBD_PIPE_HANDLE  outBkPipeHandle;  /* Pipe handle for Bulk out EP    */
    USBD_PIPE_HANDLE  inBkPipeHandle;   /* Pipe handle for Bulk in EP     */
    USB_IRP           inIntIrp;          /* IRP used for Interrupt-in data      */
    USB_IRP           inBkIrp;          /* IRP used for bulk-in data      */
    USB_IRP           outBkIrp;         /* IRP used for bulk-out data     */
    UINT8 *           bulkInData;     /* Pointer for bulk-in data       */
    UINT8 *           bulkOutData;    /* Pointer for bulk-out data      */   
    UINT16            lockCount;      /* Count of times structure locked*/
    BOOL              connected;      /* TRUE if USB_BT device connected */    
    SEM_HANDLE        btIrpSem;     /* Semaphore for IRP Synchronisation */
    MUTEX_HANDLE      btDevMutex;   /* Mutex used to protect driver unit access */
}USB_BT_DEV, *pUSB_BT_DEV;    

static USBD_CLIENT_HANDLE usbdBtHandle;
static MUTEX_HANDLE usbBtMutex;      /* mutex used to protect internal structs */
static USB_BT_DEV usbBtDev;
static INT32  btusb_int_taskID;
static INT32  btusb_blkin_taskID;

static unsigned char intInIrpCallbackInvoked = FALSE;
static unsigned char outBkIrpCallbackInvoked = FALSE;
static unsigned char inBkIrpCallbackInvoked = FALSE;

extern BT_HDR   *l2cap_link_chk_pkt_start(BT_HDR *); /* Called at start of rcv to check L2CAP segmentation */
extern BOOLEAN   l2cap_link_chk_pkt_end (void);       /* Called at end   of rcv to check L2CAP segmentation */
extern BT_HDR   *l2c_link_get_decompressed_pkt(BT_HDR *); /* Called at end of rcv to check L2CAP decompression */

/* Trasnprt receive states */
enum
{
    RX_USB_MSGTYPE_ST,
    RX_USB_LEN_ST,
    RX_USB_NOBUF_LEN_ST,
    RX_USB_DATA_ST,
    RX_USB_IGNORE_ST
};

/* Control block for RX_USB_CB */
typedef struct
{
    BT_HDR	*p_rcv_msg;		/* Buffer for holding current incoming HCI message */
    UINT16	rcv_len;				/* Size of current incoming message */
    UINT8		rcv_state;			/* Receive state of current incoming message */
    UINT8		previous_rcv_byte;	/* Used to accumulate message length from stream */
} tRX_USB_CB;

static tRX_USB_CB btEvt, btAcl;

//#define USERIAL_DUMP
#ifdef USERIAL_DUMP
#define USERIAL_DUMP_BYTES 30
char rx_log_buff[USERIAL_DUMP_BYTES*3+16];
#endif

/*******************************************************************************
 **
 ** Function           btUsb_CtlOut_send
 **
 ** Description        Send HCI commands via Control end-point
 **
 ** Input Parameter
 **				pBf - buffer to send
 **				len - length of buffer
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
void btUsb_CtlOut_send(UINT8 *pBf, UINT16 len)
{
	UINT16 frame_len;

//    APPL_TRACE_DEBUG1("btUsb_CtlOut_send %d bytes", len);

	while (len > 0)
	{
		frame_len = (len > 64) ? 64 : len;
		
		if ((usbdVendorSpecific (usbdBtHandle, 
							usbBtDev.btDevId,
							USB_RT_HOST_TO_DEV | USB_RT_CLASS,  /* RequestType */
							0,  /* request */
							0,  /* value */
							0,   /* index */
							frame_len,  /* len */ 
							pBf,   /*data ptr */
							NULL)) != OK ) /* len of in */
		{
			printf ("bt_usb: Failed to send HCI command\n");  
			break;
		}

		len -= frame_len;
		pBf += frame_len;
	}
}


/*******************************************************************************
 **
 ** Function           btUsb_BulkOut_send
 **
 ** Description        Send ACL data via Bulk Out end-point
 **
 ** Input Parameter
 **				pBf - buffer to send
 **				len - length of buffer
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
void btUsb_BulkOut_send(UINT8 *pBf, UINT16 len)
{
	UINT16 s, frame_len;

//    APPL_TRACE_DEBUG1("btUsb_BulkOut_send %d bytes", len);
	
	while (len > 0)
	{
		frame_len = (len > 64) ? 64 : len;
		
		usbBtDev.outBkIrp.result			= 0;
		usbBtDev.outBkIrp.transferLen		= frame_len;
		usbBtDev.outBkIrp.bfrList[0].pBfr	= pBf;
		usbBtDev.outBkIrp.bfrList[0].bfrLen	= frame_len;

		outBkIrpCallbackInvoked = FALSE;

		if ((s = usbdTransfer (usbdBtHandle, usbBtDev.outBkPipeHandle, &(usbBtDev.outBkIrp))) != OK)
		{
			printf ("btUsb_BulkOut_send: usbdTransfer() returned 0x%02x \n", s);
		}
		else
		{
			while (!outBkIrpCallbackInvoked)
				taskDelay(1);
		}

		len -= frame_len;
		pBf += frame_len;
	}
}


/*******************************************************************************
 **
 ** Function           usbBt_BlkIn_IrpCback
 **
 ** Description        The callback of USB Bulk In IRP transfer
 **
 ** Input Parameter	p -pointer to the IRP submitted
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
static void usbBt_BlkIn_IrpCback (pVOID p)
{
	pUSB_IRP      pIrp     = (pUSB_IRP) p;
	pUSB_BT_DEV pBtDev = pIrp->userPtr;
	
	if (pIrp->result != OK)
	{
		printf ("BulkIn : Irp failed on Bulk In ,%x\n", pIrp->result);

		/* Clear HALT Feature on Bulk In Endpoint */
		if ((usbdFeatureClear (usbdBtHandle, 
							pBtDev->btDevId, 
							USB_RT_ENDPOINT, 
							USB_FSEL_DEV_ENDPOINT_HALT, 
							(pBtDev->inBkEpAddress & 0xFF))) != OK)
		{
			printf ("BulkIn: Failed to clear HALT feauture on Bulk In Endpoint\n");
		}
	}	
	else
	{
		UINT8		byte;
		UINT16		msg_len;
		BOOLEAN		msg_received;
		UINT16		rx_counts = pIrp->bfrList[0].actLen;
		UINT8		*rx_buff = pIrp->bfrList[0].pBfr;		

// If H4 port is not opened, then exit
//    if (p_cb->h4_state != HCISU_H4_OPENED_ST)
//        return (0);

		while (rx_counts)
		{
			byte = *rx_buff++;
			rx_counts--;

			msg_received = FALSE;
    
			switch (btAcl.rcv_state)
			{
				case RX_USB_MSGTYPE_ST:
					/* Start of new message. Allocate a buffer for message. */
					btAcl.p_rcv_msg = (BT_HDR *) GKI_getpoolbuf (HCI_ACL_POOL_ID);

					/* Check for problem allocating message buffer */
					if (btAcl.p_rcv_msg)
					{
						/* Initialize BT_HDR */
						btAcl.p_rcv_msg->len = 0;
						btAcl.p_rcv_msg->event = BT_EVT_TO_BTU_HCI_ACL;
						btAcl.p_rcv_msg->offset = 0;

						btAcl.rcv_state = RX_USB_LEN_ST;              /* Next, wait for length to come in */
						*((UINT8 *)(btAcl.p_rcv_msg + 1) + btAcl.p_rcv_msg->len++) = byte;
					}
					else
					{
						/* Unable to allocate message buffer. */
						printf("BulkIn: Unable to allocate buffer for incoming HCI message.\n");

						btAcl.rcv_state = RX_USB_NOBUF_LEN_ST;        /* Next, wait for length to come in */
					}

					btAcl.rcv_len = HCI_DATA_PREAMBLE_SIZE-1;   /* Get number preamble bytes for this msg type */
					break;

				case RX_USB_LEN_ST:
					/* Receiving preamble */
					*((UINT8 *)(btAcl.p_rcv_msg + 1) + btAcl.p_rcv_msg->len++) = byte;
					btAcl.rcv_len--;

					/* Check if we received entire preamble yet */
					if (btAcl.rcv_len == 0)
					{
						/* Received entire preamble. ACL data lengths are 16-bits (lobyte of msg length was the previous byte received) */
						msg_len = byte;
						msg_len = (msg_len << 8) + btAcl.previous_rcv_byte;

						/* Check for segmented packets. If this is a continuation packet, then   */
						/* current rcv buffer will be freed, and we will continue appending data */
						/* to the original rcv buffer.                                           */
						if ((btAcl.p_rcv_msg = l2cap_link_chk_pkt_start (btAcl.p_rcv_msg)) == NULL)
						{
							/* If a NULL is returned, then we have a problem. Ignore remaining data in this packet */
							btAcl.rcv_len = msg_len;
							if (msg_len == 0)
							{
								btAcl.rcv_state = RX_USB_MSGTYPE_ST;  /* Wait for next message */
							}
							else
							{
								btAcl.rcv_state = RX_USB_IGNORE_ST;   /* Ignore rest of the packet */
							}

							break;
						}
						
						btAcl.rcv_len = msg_len;

						/* Verify that buffer is big enough to fit message */
						if ((sizeof(BT_HDR) + HCI_DATA_PREAMBLE_SIZE + msg_len) > GKI_get_buf_size(btAcl.p_rcv_msg))
						{
							/* Message cannot fit into buffer */
							GKI_freebuf(btAcl.p_rcv_msg);
							btAcl.p_rcv_msg = NULL;
							btAcl.rcv_state = RX_USB_IGNORE_ST;   /* Ignore rest of the packet */

							printf("BulkIn: Invalid length for incoming HCI message.\n");
						}
						else
						{
							/* Message length is valid */
							if (msg_len)
							{
								/* Read rest of message */
								btAcl.rcv_state = RX_USB_DATA_ST;
							}
							else
							{
								/* Message has no additional parameters. (Entire message has been received) */
								msg_received = TRUE;
								btAcl.rcv_state = RX_USB_MSGTYPE_ST;  /* Next, wait for next message */
							}
						}
					}
					else
					{
						/* Did not receive entire message length from stream yet. Retain the byte we just received */
						btAcl.previous_rcv_byte = byte;
					}
					break;

				case RX_USB_NOBUF_LEN_ST:
					/* Unable to allocate buffer for incoming message. Get length of message so that */
					/* the rest of the message can be ignored.                                       */
					/* Receiving preamble */
					btAcl.rcv_len--;

					/* Check if we received entire preamble yet */
					if (btAcl.rcv_len == 0)
					{
						/* Received entire preamble. ACL data lengths are 16-bits (the lobyte of length was the previous byte received)  */
						msg_len = byte;
						msg_len = (msg_len << 8) + btAcl.previous_rcv_byte;
						btAcl.rcv_len = msg_len;

						btAcl.rcv_state = RX_USB_IGNORE_ST;   /* Ignore rest of the packet */
					}
					else
					{
						/* Did not receive entire message length from stream yet. Retain the byte we just received */
						btAcl.previous_rcv_byte = byte;
					}
					break;

				case RX_USB_DATA_ST:
					*((UINT8 *)(btAcl.p_rcv_msg + 1) + btAcl.p_rcv_msg->len++) = byte;
					btAcl.rcv_len--;

					/* Check if we read in entire message yet */
					if (btAcl.rcv_len == 0)
					{
						/* Received entire packet. */
						/* Check for segmented l2cap packets */
						if (!l2cap_link_chk_pkt_end ())
						{
							/* Not the end of packet yet. */
							btAcl.rcv_state = RX_USB_MSGTYPE_ST;      /* Next, wait for next message */
						}
						else
						{
							/* Received entire message */
#if (L2CAP_ENHANCED_FEATURES & L2CAP_COMPRESSION)
							btAcl.p_rcv_msg = l2c_link_get_decompressed_pkt(btAcl.p_rcv_msg);
#endif
							msg_received = TRUE;
							btAcl.rcv_state = RX_USB_MSGTYPE_ST;      /* Next, wait for next message */
						}
					}
					break;

				case RX_USB_IGNORE_ST:
					/* Ignore reset of packet */
					btAcl.rcv_len--;

					/* Check if we read in entire message yet */
					if (btAcl.rcv_len == 0)
					{
						btAcl.rcv_state = RX_USB_MSGTYPE_ST;      /* Next, wait for next message */
					}
					break;
					
				default:
					printf("BulkIn: Unknown btAcl.rcv_state=%d\n", btAcl.rcv_state);
					break;
			}

			/* If we received entire message, then send it to the BTU task */
			if (msg_received)
			{
#ifdef USERIAL_DUMP
				UINT16 xx;
				UINT8   *p = (UINT8 *)(btAcl.p_rcv_msg + 1) + btAcl.p_rcv_msg->offset;

				for (xx=0; (xx < btAcl.p_rcv_msg->len) && (xx < USERIAL_DUMP_BYTES); xx++)
				{
					sprintf((rx_log_buff+xx*3), "%02x ", (int) *(p+xx));
				}

				rx_log_buff[xx*3] = 0;

				if (xx < btAcl.p_rcv_msg->len)
				{
					printf("BulkIn<%d> = %s ...\n", btAcl.p_rcv_msg->len, rx_log_buff);
				}
				else
				{
					printf("BulkIn<%d> = %s\n", btAcl.p_rcv_msg->len, rx_log_buff);
				}
#endif				
				/* Display protocol trace message */
				GKI_send_msg (BTU_TASK, BTU_HCI_RCV_MBOX, btAcl.p_rcv_msg);
				
				btAcl.p_rcv_msg = NULL;
			}
		}
	}
	inBkIrpCallbackInvoked = TRUE;
}


/*******************************************************************************
 **
 ** Function           btusb_BlkIn_thread
 **
 ** Description        The task main body for BTUSBBLK thread
 **
 ** Input Parameter	None
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
char usbBlkInBfr[64];
void btusb_BlkIn_thread(void)
{
	UINT16 s;

	memset (&btAcl, 0, sizeof (tRX_USB_CB));

	for(;;)
	{
		usbBtDev.inBkIrp.result			= 0;
		usbBtDev.inBkIrp.transferLen		= 64;
		usbBtDev.inBkIrp.bfrList[0].pBfr		= (UINT8 *) usbBlkInBfr;
		usbBtDev.inBkIrp.bfrList[0].bfrLen	= 64;

		inBkIrpCallbackInvoked = FALSE;
		
		if ((s = usbdTransfer (usbdBtHandle, usbBtDev.inBkPipeHandle, &(usbBtDev.inBkIrp))) != OK)
		{
			printf ("btusb_BlkIn_thread: usbdTransfer() returned 0x%02x \n", s);
			break;
		}
		else
		{
			while (!inBkIrpCallbackInvoked)
			{
				taskDelay(1);
			}
		}
	}
}

/*******************************************************************************
 **
 ** Function           usbBt_BlkOut_IrpCback
 **
 ** Description        The callback of USB Bulk Out IRP transfer
 **
 ** Input Parameter	p -pointer to the IRP submitted
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
static void usbBt_BlkOut_IrpCback (pVOID p)
{
	pUSB_IRP      pIrp     = (pUSB_IRP) p;
	pUSB_BT_DEV pBtDev = pIrp->userPtr;

	if (pIrp->result != OK)     /* check the result of IRP */
	{
		printf ("BulkOut: Irp failed on Bulk Out %x \n", pIrp->result); 

		/* Clear HALT Feature on Bulk Out Endpoint */ 
		if ((usbdFeatureClear (usbdBtHandle, 
							pBtDev->btDevId, 
							USB_RT_ENDPOINT, 
							USB_FSEL_DEV_ENDPOINT_HALT, 
							(pBtDev->outBkEpAddress & 0xFF))) != OK)
		{
			printf ("BulkOut: Failed to clear HALT feauture on Bulk Out Endpoint\n");
		}
	}
#if 0	
	else
	{
		printf ("BulkOut: Num of Bytes transferred on Bulk Out pipe %d\n", pIrp->bfrList[0].actLen); 
	}
#endif
	outBkIrpCallbackInvoked = TRUE;
}


/*******************************************************************************
 **
 ** Function           usbBt_Int_IrpCback
 **
 ** Description        The callback of USB Interrupt In IRP transfer
 **
 ** Input Parameter	p -pointer to the IRP submitted
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
static void usbBt_Int_IrpCback (pVOID p)
{
	pUSB_IRP      pIrp     = (pUSB_IRP) p;
	pUSB_BT_DEV pBtDev = pIrp->userPtr;

	if (pIrp->result != OK)  /* IRP on Interrupt IN failed */
	{
		printf ("Int : Irp failed on Interrupt In ,%x\n", pIrp->result);

		/* Clear HALT Feature on Bulk In Endpoint */
		if ((usbdFeatureClear (usbdBtHandle, 
							pBtDev->btDevId, 
							USB_RT_ENDPOINT, 
							USB_FSEL_DEV_ENDPOINT_HALT, 
							(pBtDev->inIntEpAddress & 0xFF))) != OK)
		{
			printf ("Int: Failed to clear HALT feauture on Interrupt In Endpoint\n");
		}
	}
	else
	{
		UINT8		byte;
		UINT16		msg_len;
		BOOLEAN		msg_received;
		UINT16		rx_counts = pIrp->bfrList[0].actLen;
		UINT8		*rx_buff = pIrp->bfrList[0].pBfr;

//		printf ("Int : Received %d bytes\n", rx_counts);
		
// If USB port is not opened, then exit
//if (p_cb->usb_state != HCISU_USB_OPENED_ST)
//			return (0);

		while (rx_counts)
		{
			byte = *rx_buff++;
			rx_counts--;

			msg_received = FALSE;

			switch (btEvt.rcv_state)
			{
				case RX_USB_MSGTYPE_ST:
					/* Start of new message. Allocate a buffer for message */
					btEvt.p_rcv_msg = (BT_HDR *) GKI_getpoolbuf (HCI_CMD_POOL_ID);

					/* Check for problem allocating message buffer */
					if (btEvt.p_rcv_msg)
					{
						/* Initialize BT_HDR */
						btEvt.p_rcv_msg->len = 0;
						btEvt.p_rcv_msg->event = BT_EVT_TO_BTU_HCI_EVT;
						btEvt.p_rcv_msg->offset = 0;

						btEvt.rcv_state = RX_USB_LEN_ST;              /* Next, wait for length to come in */
						
						*((UINT8 *)(btEvt.p_rcv_msg + 1) + btEvt.p_rcv_msg->len++) = byte;					
					}
					else
					{
						/* Unable to allocate message buffer. */
						printf("RX-Evt: Unable to allocate buffer for incoming HCI message\n");

						btEvt.rcv_state = RX_USB_NOBUF_LEN_ST;        /* Next, wait for length to come in */
					}

					btEvt.rcv_len = HCIE_PREAMBLE_SIZE-1;   /* Get number preamble bytes for this msg type */
					break;

				case RX_USB_LEN_ST:
					/* Receiving preamble */
					*((UINT8 *)(btEvt.p_rcv_msg + 1) + btEvt.p_rcv_msg->len++) = byte;
					btEvt.rcv_len--;

					/* Check if we received entire preamble yet */
					if (btEvt.rcv_len == 0)
					{
						/* Received entire preamble. Length is in the last byte(s) of the preamble */
						msg_len = byte;
						btEvt.rcv_len = msg_len;

						/* Verify that buffer is big enough to fit message */
						if ((sizeof(BT_HDR) + HCIE_PREAMBLE_SIZE + msg_len) > GKI_get_buf_size(btEvt.p_rcv_msg))
						{
							/* Message cannot fit into buffer */
							GKI_freebuf(btEvt.p_rcv_msg);
							btEvt.p_rcv_msg = NULL;
							btEvt.rcv_state = RX_USB_IGNORE_ST;   /* Ignore rest of the packet */

							printf("RX-Evt: Invalid length for incoming HCI message\n");
						}
						else
						{
							/* Message length is valid */
							if (msg_len)
							{
								/* Read rest of message */
								btEvt.rcv_state = RX_USB_DATA_ST;
							}
							else
							{
								/* Message has no additional parameters. (Entire message has been received) */
								msg_received = TRUE;
								btEvt.rcv_state = RX_USB_MSGTYPE_ST;  /* Next, wait for next message */
							}
						}
					}
					break;

				case RX_USB_NOBUF_LEN_ST:
					/* Unable to allocate buffer for incoming message. Get length of message so that */
					/* the rest of the message can be ignored.                                       */

					/* Receiving preamble */
					btEvt.rcv_len--;

					/* Check if we received entire preamble yet */
					if (btEvt.rcv_len == 0)
					{
						/* Received entire preamble. Length is in the last byte(s) of the preamble */
						msg_len = byte;
						btEvt.rcv_len = msg_len;

						btEvt.rcv_state = RX_USB_IGNORE_ST;   /* Ignore rest of the packet */
					}
					break;


				case RX_USB_DATA_ST:
					*((UINT8 *)(btEvt.p_rcv_msg + 1) + btEvt.p_rcv_msg->len++) = byte;
					btEvt.rcv_len--;

					/* Check if we read in entire message yet */
					if (btEvt.rcv_len == 0)
					{
						/* Received entire packet. */
						msg_received = TRUE;
						btEvt.rcv_state = RX_USB_MSGTYPE_ST;      /* Next, wait for next message */
					}
					break;


				case RX_USB_IGNORE_ST:
					/* Ignore reset of packet */
					btEvt.rcv_len--;

					/* Check if we read in entire message yet */
					if (btEvt.rcv_len == 0)
					{
						btEvt.rcv_state = RX_USB_MSGTYPE_ST;      /* Next, wait for next message */
					}
					break;

				default:
					printf("RX-Evt: Unknown btEvt.rcv_state=%d\n", btEvt.rcv_state);					
					break;
			}

			/* If we received entire message, then send it to the BTU task */
			if (msg_received)
			{
			
#ifdef USERIAL_DUMP
				UINT16 xx;
				UINT8   *p = (UINT8 *)(btEvt.p_rcv_msg + 1) + btEvt.p_rcv_msg->offset;

				for (xx=0; (xx < btEvt.p_rcv_msg->len) && (xx < USERIAL_DUMP_BYTES); xx++)
				{
					sprintf((rx_log_buff+xx*3), "%02X ", (int) *(p+xx));
				}

				rx_log_buff[xx*3] = 0;

				if (xx < btEvt.p_rcv_msg->len)
				{
					printf("IrpIn<%d> = %s ...\n", btEvt.p_rcv_msg->len, rx_log_buff);
				}
				else
				{
					printf("IrpIn<%d> = %s\n", btEvt.p_rcv_msg->len, rx_log_buff);
				}
				
#endif
				GKI_send_msg (BTU_TASK, BTU_HCI_RCV_MBOX, btEvt.p_rcv_msg);

				btEvt.p_rcv_msg = NULL;
			}
		}
	}
//    printf ("Int : returns\n");

	intInIrpCallbackInvoked = TRUE;
}


/*******************************************************************************
 **
 ** Function           btusb_int_thread
 **
 ** Description        The task main body for BTUSBINT thread
 **
 ** Input Parameter	None
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
char usbIntBfr[32];
void btusb_int_thread(void)
{
	UINT16 s;

	memset (&btEvt, 0, sizeof (tRX_USB_CB));

	for(;;)
	{
		usbBtDev.inIntIrp.result			= 0;
		usbBtDev.inIntIrp.transferLen		= 16;
		usbBtDev.inIntIrp.bfrList[0].pBfr		= (UINT8 *) usbIntBfr;
		usbBtDev.inIntIrp.bfrList[0].bfrLen	= 16;

		intInIrpCallbackInvoked = FALSE;

		if ((s = usbdTransfer (usbdBtHandle, usbBtDev.inIntPipeHandle, &(usbBtDev.inIntIrp))) != OK)
		{
			printf ("btusb_int_thread: usbdTransfer() returned 0x%02x \n", s);
			break;
		}
		else
		{
			while (!intInIrpCallbackInvoked)
				taskDelay(1);
		}
	}
}


/*******************************************************************************
 **
 ** Function           findBtEndpoint
 **
 ** Description        Get end-point descriptor
 **
 ** Input Parameter
 **				pBfr - descriptor
 **				bfrLen - length of descriptor
 **				direction - IN/OUT direction
 **				endpoint_type - INTERRUPT/BULK/ISO
 **
 ** Output Parameter   None
 **
 ** Returns            Pointer to the end-point descriptor
 **
 *******************************************************************************/
static pUSB_ENDPOINT_DESCR findBtEndpoint(pUINT8 pBfr, UINT16 bfrLen, UINT16 direction, UINT8 endpoint_type)
{
	pUSB_ENDPOINT_DESCR pEp;

	while ((pEp = usbDescrParseSkip (&pBfr, &bfrLen, USB_DESCR_ENDPOINT)) != NULL)
	{
		if (((pEp->attributes & USB_ATTR_EPTYPE_MASK) == endpoint_type) &&
			((pEp->endpointAddress & USB_ENDPOINT_DIR_MASK) == direction))
			break;
	}

	return pEp;
}


/*******************************************************************************
 **
 ** Function           usbBtPhysDevCreate
 **
 ** Description        The function create USB logical block
 **
 ** Input Parameter
 **				nodeId - USBD Node ID of the device
 **				devConfiguration - Configuratio value
 **				devInterface - Interface number
 **
 ** Output Parameter   None
 **
 ** Returns            OK or ERROR
 **
 *******************************************************************************/
static int usbBtPhysDevCreate (USBD_NODE_ID nodeId, UINT16 devConfiguration, UINT16 devInterface)
{
	UINT16   actLength;
	UINT8  *pBfr;           /* pointer to descriptor store */ 
	UINT8  *pScratchBfr;       /* another pointer to the above store */ 
	UINT     ifNo;
	UINT16   maxPacketSize;  
	USB_CONFIG_DESCR * pCfgDescr;
	USB_INTERFACE_DESCR * pIfDescr;
	USB_ENDPOINT_DESCR * pOutEp;
	USB_ENDPOINT_DESCR * pInEp;

	/* Allocate a non-cacheable buffer for the descriptor's */
	if ((pBfr = OSS_MALLOC (USB_MAX_DESCR_LEN)) == NULL)
	{        
		printf ("usbBtPhysDevCreate: Unable to allocate memory:pBfr\n");
		return ERROR;
	}

	usbBtDev.btDevId       = nodeId; 
	usbBtDev.configuration = devConfiguration;
	usbBtDev.interface     = devInterface;
	usbBtDev.connected     = TRUE;

    /* Initialization for usbBtDev.inIntIrp */
	usbBtDev.inIntIrp.irpLen            = sizeof(USB_IRP);
	usbBtDev.inIntIrp.userCallback      = usbBt_Int_IrpCback;
	usbBtDev.inIntIrp.bfrCount          = 0x01;
	usbBtDev.inIntIrp.bfrList[0].pid    = USB_PID_IN;
	usbBtDev.inIntIrp.userPtr           = &usbBtDev;
    
	/* Initialization for usbBtDev.outBkIrp */
	usbBtDev.outBkIrp.irpLen            = sizeof (USB_IRP);
	usbBtDev.outBkIrp.userCallback      = usbBt_BlkOut_IrpCback;
	usbBtDev.outBkIrp.timeout           = USB_BT_BULK_IRP_TIME_OUT;
	usbBtDev.outBkIrp.bfrCount          = 0x01;
	usbBtDev.outBkIrp.bfrList[0].pid    = USB_PID_OUT;
	usbBtDev.outBkIrp.userPtr           = &usbBtDev;

    /* Initialization for usbBtDev.inBkIrp */
	usbBtDev.inBkIrp.irpLen            = sizeof(USB_IRP);
	usbBtDev.inBkIrp.userCallback      = usbBt_BlkIn_IrpCback;
//	usbBtDev.inBkIrp.timeout           = USB_BT_BULK_IRP_TIME_OUT;
	usbBtDev.inBkIrp.bfrCount          = 0x01;
	usbBtDev.inBkIrp.bfrList[0].pid    = USB_PID_IN;
	usbBtDev.inBkIrp.userPtr           = &usbBtDev;

	if (OSS_SEM_CREATE( 1, 1, &usbBtDev.btIrpSem) != OK)
	{
		printf ("usbBtPhysDevCreate: Unable to create Semaphore for btIrpSem\n");
		return ERROR;
	}

	if (OSS_MUTEX_CREATE(&usbBtDev.btDevMutex)!= OK)
	{
		printf ("usbBtPhysDevCreate: Unable to create Semaphore for btDevMutex\n");
		return ERROR;
	}

	/* Check out the device configuration */

	/* Configuration index is assumed to be one less than config'n value */
	if (usbdDescriptorGet (usbdBtHandle, 
						usbBtDev.btDevId, 
						USB_RT_STANDARD | USB_RT_DEVICE, 
						USB_DESCR_CONFIGURATION, 
						devConfiguration,  
						0, 
						USB_MAX_DESCR_LEN, 
						pBfr, 
						&actLength) != OK)  
        {
		printf ("usbBtPhysDevCreate: Unable to read configuration descriptor\n");
		goto errorExit;
        }

	if ((pCfgDescr = usbDescrParse (pBfr, actLength, USB_DESCR_CONFIGURATION)) == NULL)
	{
		printf ("usbBtPhysDevCreate: Unable to find configuration descriptor\n");
		goto errorExit;
	}
	else
	{
		printf ("usbBtPhysDevCreate: configurationValue=0x%02x\n", pCfgDescr->configurationValue);			
	}
	
	usbBtDev.configuration = pCfgDescr->configurationValue;

	/* Look for the interface 0 */
	ifNo = 0;

	/*
	* usbDescrParseSkip() modifies the value of the pointer it recieves
	* so we pass it a copy of our buffer pointer
	*/
	pScratchBfr = pBfr;

	while ((pIfDescr = usbDescrParseSkip (&pScratchBfr, &actLength, USB_DESCR_INTERFACE))  != NULL)
	{
		if (ifNo == usbBtDev.interface)
			break;
		ifNo++;
	}

	if (pIfDescr == NULL)
		goto errorExit;

	/* 
	* check whether the subclass is supported by the bluetooth driver, if 
	* not report saying that the subclass is not supported
	*/
	if (pIfDescr->interfaceSubClass !=  0x01)
	{
		printf ("Sub-Class 0x%x is not supported by the Bluetooth Driver \n", pIfDescr->interfaceSubClass);
		goto errorExit;
	}

	printf ("usbBtPhysDevCreate: alternateSetting=0x%04x\n", pIfDescr->alternateSetting);
	
	usbBtDev.altSetting = pIfDescr->alternateSetting;


	/* Set the device configuration */
	if ((usbdConfigurationSet (usbdBtHandle, 
							usbBtDev.btDevId, 
							usbBtDev.configuration, 
							pCfgDescr->maxPower * USB_POWER_MA_PER_UNIT)) != OK)
	{
		printf ("usbBtPhysDevCreate: Unable to set device configuration \n");
		goto errorExit;
	}
	else
	{
		printf ("usbBtPhysDevCreate: Configuration set to 0x%x \n", usbBtDev.configuration);
	}
    
	/* Select interface 
	* 
	* NOTE: Some devices may reject this command, and this does not represent
	* a fatal error.  Therefore, we ignore the return status.
	*/
	usbdInterfaceSet (usbdBtHandle, usbBtDev.btDevId, usbBtDev.interface, usbBtDev.altSetting);

	/* 
	* Retrieve the endpoint descriptor(s) following the identified interface
	* descriptor.
	*/
	if ((pInEp = findBtEndpoint (pScratchBfr, actLength, USB_ENDPOINT_IN, USB_ATTR_INTERRUPT)) == NULL)
	{
		goto errorExit;
	}
	
	usbBtDev.inIntEpAddress  = pInEp->endpointAddress;
	printf ("usbBtPhysDevCreate: inIntEpAddress=0x%02x, MaxPktSize=0x%04x(BE)\n", usbBtDev.inIntEpAddress, pInEp->maxPacketSize);

	maxPacketSize = *((pUINT8) &pInEp->maxPacketSize) | (*(((pUINT8) &pInEp->maxPacketSize) + 1) << 8);

	/* Create a Interrupt-in pipe */
	if (usbdPipeCreate (usbdBtHandle, 
					usbBtDev.btDevId, 
					usbBtDev.inIntEpAddress, 
					usbBtDev.configuration, 
					usbBtDev.interface, 
					USB_XFRTYPE_INTERRUPT, 
					USB_DIR_IN, 
					maxPacketSize, 
					0, 
					1, 
					&(usbBtDev.inIntPipeHandle)) != OK)
	{
		printf ("usbBtPhysDevCreate: Error creating Interrupt In pipe\n");
		goto errorExit;
	} 
	else
	{
		printf ("usbBtPhysDevCreate: Interrupt In pipe handle 0x%08x\n", (unsigned int) usbBtDev.inIntPipeHandle);
	} 

	if ((pOutEp = findBtEndpoint (pScratchBfr, actLength, USB_ENDPOINT_OUT, USB_ATTR_BULK)) == NULL)
	{
		goto errorExit;
	}
	
	usbBtDev.outBkEpAddress = pOutEp->endpointAddress;
	printf ("usbBtPhysDevCreate: outBkEpAddress=0x%02x, MaxPktSize=0x%04x(BE)\n", usbBtDev.outBkEpAddress, pOutEp->maxPacketSize);

	maxPacketSize = *((pUINT8) &pOutEp->maxPacketSize) | (*(((pUINT8) &pOutEp->maxPacketSize) + 1) << 8);

	/* Create a Bulk-out pipe */
	if (usbdPipeCreate (usbdBtHandle, 
					usbBtDev.btDevId, 
					usbBtDev.outBkEpAddress, 
					usbBtDev.configuration, 
					usbBtDev.interface, 
					USB_XFRTYPE_BULK, 
					USB_DIR_OUT, 
					maxPacketSize, 
					0, 
					0, 
					&(usbBtDev.outBkPipeHandle)) != OK)
	{
		printf ("usbBtPhysDevCreate: Error creating Bulk Out pipe\n");
		goto errorExit;
	} 
	else
	{
		printf ("usbBtPhysDevCreate: Bulk Out pipe handle 0x%08x\n", (unsigned int) usbBtDev.outBkPipeHandle);
	} 


	if ((pInEp = findBtEndpoint (pScratchBfr, actLength, USB_ENDPOINT_IN, USB_ATTR_BULK)) == NULL)
	{
		goto errorExit;
	}

	usbBtDev.inBkEpAddress  = pInEp->endpointAddress;
	printf ("usbBtPhysDevCreate: inBkEpAddress=0x%02x, MaxPktSize=0x%04x(BE)\n", usbBtDev.inBkEpAddress, pInEp->maxPacketSize);

	maxPacketSize = *((pUINT8) &pInEp->maxPacketSize) | (*(((pUINT8) &pInEp->maxPacketSize) + 1) << 8);

	/* Create a Bulk-in pipe */
	if (usbdPipeCreate (usbdBtHandle, 
					usbBtDev.btDevId, 
					usbBtDev.inBkEpAddress, 
					usbBtDev.configuration, 
					usbBtDev.interface, 
					USB_XFRTYPE_BULK, 
					USB_DIR_IN, 
					maxPacketSize, 
					0, 
					0, 
					&(usbBtDev.inBkPipeHandle)) != OK)
	{
		printf ("usbBtPhysDevCreate: Error creating Bulk In pipe\n");
		goto errorExit;
	} 
	else
	{
		printf ("usbBtPhysDevCreate: Bulk In pipe handle 0x%08x\n", (unsigned int) usbBtDev.inBkPipeHandle);
	} 

	/* Clear HALT feauture on the endpoints */
#ifdef DOUBLE_FEATURE_CLEAR_REQUIRED
_HACK_
	if ((usbdFeatureClear (usbdHandle, 
						usbBtDev.btDevId, 
						USB_RT_ENDPOINT, 
						USB_FSEL_DEV_ENDPOINT_HALT, 
						(pOutEp->endpointAddress & 0xFF))) != OK)
	{
		printf ("usbBtPhysDevCreate: Failed to clear HALT feauture on Bulk Out Endpoint %x\n");
	}  

	if ((usbdFeatureClear (usbdHandle, 
						usbBtDev.btDevId, 
						USB_RT_ENDPOINT, 
						USB_FSEL_DEV_ENDPOINT_HALT, 
						(pOutEp->endpointAddress & 0xFF))) != OK)
	{
		printf ("usbBtPhysDevCreate: Failed to clear HALT feauture on Bulk In Endpoint %x\n");
	} 
#endif

	OSS_FREE (pBfr);
	return (OK);

errorExit:
	/* Error in creating a bulk device ..destroy */
	OSS_FREE (pBfr);
	return (ERROR);
}


/*******************************************************************************
 **
 ** Function           usbBtDevAttachCallback
 **
 ** Description        The callback function handles USB hot plug
 **
 ** Input Parameter
 **				nodeId - USBD Node ID of the device attached
 **				attachAction - Whether device attached / detached
 **				devConfiguration - Configur'n value
 **				devInterface - Interface number
 **				deviceClass - Interface class
 **				deviceSubClass - Device sub-class
 **				deviceProtocol - Interfaceprotocol
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
static void usbBtDevAttachCallback(USBD_NODE_ID nodeId, UINT16 attachAction, UINT16 devConfiguration, UINT16 devInterface,
									UINT16 deviceClass, UINT16 deviceSubClass, UINT16 deviceProtocol)
{
//	OSS_MUTEX_TAKE (usbBtMutex, OSS_BLOCK);
    deviceClass = deviceClass;
    deviceSubClass = deviceSubClass;
    deviceProtocol = deviceProtocol;
    APPL_TRACE_DEBUG0("usbBtDevAttachCallback");

	switch (attachAction)
	{ 
		case USBD_DYNA_ATTACH: 
			printf ("usbBtDevAttachCallback : New Bluetooth USB device attached\n");
			printf ("usbBtDevAttachCallback : Configuration = %d, Interface = %d, Node Id = %d \n", devConfiguration, devInterface, (UINT)nodeId); 
//			printf ("usbBtDevAttachCallback : Class = 0x%02x, SubClass = 0x%02x, Protocol = 0x%02x \n", deviceClass, deviceSubClass, deviceProtocol); 

			if (usbBtPhysDevCreate (nodeId, devConfiguration, devInterface) != OK )
			{			
				printf ("usbBtDevAttachCallback : Error creating BT device\n");
			} 
			else
			{
				btusb_int_taskID = taskSpawn("BTUSBINT", BTUSB_INT_PRIORITY, 0, BTUSB_INT_STACK_SIZE, (FUNCPTR)btusb_int_thread, 0,0,0,0,0,0,0,0,0,0);
				printf("create task : BTUSBINT priority=3, taskID=%08x\n", btusb_int_taskID);
				btusb_blkin_taskID = taskSpawn("BTUSBBLK", BTUSB_BLKIN_PRIORITY, 0, BTUSB_BLKIN_STACK_SIZE, (FUNCPTR)btusb_BlkIn_thread, 0,0,0,0,0,0,0,0,0,0);
				printf("create task : BTUSBBLK priority=2 taskID=%08x\n", btusb_blkin_taskID);
			}
			break;

		case USBD_DYNA_REMOVE:
			usbBtDev.connected = FALSE;
 			printf ("usbBtDevAttachCallback : Bluetooth device detached\n");
			taskDelete(btusb_int_taskID);
			taskDelete(btusb_blkin_taskID);
			break;

		default :
			break; 
        }

//	OSS_MUTEX_RELEASE (usbBtMutex);  
}


/*******************************************************************************
 **
 ** Function           btUsbClientInit
 **
 ** Description        This function register BT USB client application
 **
 ** Output Parameter   None
 **
 ** Returns            None
 **
 *******************************************************************************/
void btUsbClientInit(void)
{
	memset (&usbBtDev, 0, sizeof (usbBtDev));

	usbBtMutex = NULL;
	usbdBtHandle = NULL;

    APPL_TRACE_DEBUG0("btUsbClientInit Enter");
    
	if (usbdClientRegister ("BLUETOOTH", &usbdBtHandle) != OK ||
		usbdDynamicAttachRegister (usbdBtHandle, 
								0xE0 /* Devclass: Wireless Controller */, 
								0x01 /* DevSubclass: RF Controller */,
								0x01 /* DevProtocol: Bluetooth Programming */, 
								usbBtDevAttachCallback) != OK)            
	{
		printf ("btUsbClientInit: USB Client Registration Failed \n");
		return;
	}
    APPL_TRACE_DEBUG0("btUsbClientInit Exit");
}







#if 0
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
unsigned char hci_reset_cmd[] = {0x03, 0x0c, 0x00};

void btusb_ctl(void)
{
	USERIAL_Init();
	btUsbRead = 0;
	printf("Registered BT USB driver and entering btusb task main body...\n");
	for(;;)
	{
		taskDelay(sysClkRateGet()/100);

		if (btUsbRead == 1)
		{
			btUsbRead = 0;
			if ((usbdVendorSpecific (usbdBtHandle, 
								usbBtDev.btDevId,
								USB_RT_HOST_TO_DEV | USB_RT_CLASS,
								0, 
								0, 
								0, 
								3, 
								hci_reset_cmd, 
								NULL)) != OK )
			{
				printf ("bt_usb: Failed to send HCI RESET\n");  
			}
			else 
			{ 
				printf ("bt_usb: HCI RESET...sent\n");
			}
		}
	}
}

void bt_usb(void)
{
    INT32  taskID;

    taskID = taskSpawn("BTUSBCTL", 4, 0, 2048, (FUNCPTR)btusb_ctl, 0,0,0,0,0,0,0,0,0,0);
    printf("create task : BTUSBCTL priority=4 taskID=%08x\n", taskID);
}
#endif

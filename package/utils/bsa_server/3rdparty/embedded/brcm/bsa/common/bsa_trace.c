/*****************************************************************************
 **
 **  Name:          bsa_trace.c
 **
 **  Description: Contains the debug trace routines for BSA.
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "bsa_api.h"
#include "bsa_int.h"

#include "bsa_trace.h"
#include "gki_int.h"

#undef MAX_RFC_PORTS

#if !(defined(BT_USE_TRACES) || (BT_USE_TRACES == TRUE))
void LogMsg(UINT32 trace_set_mask, const char *fmt_str, ...)
{
}

#else
#undef RPC_INCLUDED
#define RPC_INCLUDED TRUE

#define MAX_LOG_BUFFER_SIZE 300
int log_msg_count=0;

#define ScrLog LogMsg

#ifndef TRACE_TASK_INCLUDED
#define TRACE_TASK_INCLUDED TRUE
#endif

#ifndef BT_USE_TRACES_TIMESTAMP
#define BT_USE_TRACES_TIMESTAMP TRUE
#endif

/* Globally disable the traces */
BOOLEAN global_trace_disable = FALSE;


/********************************************************************************
 **
 **    Function Name:   LogMsg
 **
 **    Purpose:
 **
 **    Input Parameters:
 **
 **    Returns:
 **
 *********************************************************************************/
void LogMsg(UINT32 trace_set_mask, const char *fmt_str, ...)
{
    va_list ap;
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
    char buffer_time[GKI_MAX_TIMESTAMP_BUF_SIZE];
#endif

    if (global_trace_disable)
    {
        return;
    }

#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
    GKI_get_time_stamp((INT8 *)buffer_time);
#endif

    va_start(ap, fmt_str);

    /* do not use printf(buffer) here to avoid symbol interpretation and seg fault*/
#if defined (BSA_COLORED_TRACES) && (BSA_COLORED_TRACES == TRUE)
    if(TRACE_GET_TYPE(trace_set_mask) == TRACE_TYPE_ERROR)
    {
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
        printf("\033[31;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
        printf("\033[31;47mBSA_trace %d: ", ++log_msg_count);
#endif
        vprintf(fmt_str,ap);
        printf("\033[30;47m\r\n");
    }
    else
    {
        switch (trace_set_mask & TRACE_LAYER_MASK)
        {
            case TRACE_LAYER_HCI:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[32;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[32;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\033[30;47m\r\n");
                break;

            case TRACE_LAYER_L2CAP:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[34;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[34;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\033[30;47m\r\n");
                break;

            case TRACE_LAYER_RFCOMM:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[35;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[35;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\033[30;47m\r\n");
                break;

            case TRACE_LAYER_SDP:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[36;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[36;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\033[30;47m\r\n");
                break;

            case TRACE_LAYER_OBEX:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[33;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[33;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\033[30;47m\r\n");
                break;

            case TRACE_LAYER_AVP:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[35;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[35;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\033[30;47m\r\n");
                break;

            case TRACE_LAYER_MCA:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[35;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[35;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\033[30;47m\r\n");
                break;


            default:
#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("\033[30;47mBSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("\033[30;47mBSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\r\n");
                break;
        }
    }
#else

#if defined(BT_USE_TRACES_TIMESTAMP) && (BT_USE_TRACES_TIMESTAMP == TRUE)
                printf("BSA_trace %d@%s: ",++log_msg_count, buffer_time);
#else
                printf("BSA_trace %d: ", ++log_msg_count);
#endif
                vprintf(fmt_str,ap);
                printf("\r\n");
#endif

    va_end(ap);
}


/********************************************************************************
 **
 **    Function Name:   scru_dump_hex
 **
 **    Purpose:
 **
 **    Input Parameters:
 **
 **    Returns:
 **
 *********************************************************************************/
UINT8 *scru_dump_hex (UINT8 *p_data, char *p_title, UINT16 len, UINT32 trace_layer, UINT32 trace_type)
{
#if defined (BT_USE_TRACES) && (BT_USE_TRACES == TRUE)
    UINT16  xx, yy;
    char buff1[60];
    char buff2[20];

    if (p_title)
    {
        ScrLog (TRACE_CTRL_GENERAL | trace_layer | TRACE_ORG_PROTO_DISP | trace_type,
            "%s:", p_title);
    }


    memset (buff2, ' ', 16);
    buff2[16] = 0;

    yy = snprintf (buff1, sizeof(buff1), "%04x: ", 0);
    for (xx = 0; xx < len; xx++)
    {
        if ( (xx) && ((xx & 15) == 0) )
        {
            ScrLog (TRACE_CTRL_GENERAL | trace_layer | TRACE_ORG_PROTO_DISP | trace_type,
                    "    %s  %s", buff1, buff2);
            yy = snprintf(buff1, sizeof(buff1), "%04x: ", xx);
            memset (buff2, ' ', 16);
        }
        if(sizeof(buff1)>yy)
        {
            yy += snprintf (&buff1[yy], sizeof(buff1)-yy, "%02x ", *p_data);
        }
        else
        {
            ScrLog (TRACE_CTRL_GENERAL | trace_layer | TRACE_ORG_PROTO_DISP | trace_type,
                    "scru_dump_hex ERROR");

        }

        if ((*p_data >= ' ') && (*p_data <= 'z'))
            buff2[xx & 15] = *p_data;
        else
            buff2[xx & 15] = '.';

        p_data++;
    }

    /* Pad out the remainder */
    for ( ; ; xx++)
    {
        if ((xx & 15) == 0)
        {
            ScrLog (TRACE_CTRL_GENERAL | trace_layer | TRACE_ORG_PROTO_DISP | trace_type,
                    "    %s  %s", buff1, buff2);
            break;
        }
        if(sizeof(buff1)>yy)
        {
            yy += snprintf (&buff1[yy], sizeof(buff1)-yy, "   ");
        }
        else
        {
            ScrLog (TRACE_CTRL_GENERAL | trace_layer | TRACE_ORG_PROTO_DISP | trace_type,
                    "scru_dump_hex ERROR");

        }
    }
#endif
    return (p_data);
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_0
 **
 **    Purpose:  Encodes a trace message that has no parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_0 (UINT32 trace_set_mask, const char *fmt_str)
{
#if (defined(TRACE_TASK_INCLUDED) && TRACE_TASK_INCLUDED == TRUE)
    LogMsg(trace_set_mask, fmt_str);
#endif
}


/********************************************************************************
 **
 **    Function Name:   LogMsg_1
 **
 **    Purpose:  Encodes a trace message that has one parameter argument
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_1 (UINT32 trace_set_mask, const char *fmt_str, UINTPTR p1)
{
#if (defined(TRACE_TASK_INCLUDED) && TRACE_TASK_INCLUDED == TRUE)
    LogMsg (trace_set_mask, fmt_str, p1);
#endif
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_2
 **
 **    Purpose:  Encodes a trace message that has two parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_2 (UINT32 trace_set_mask, const char *fmt_str, UINTPTR p1, UINTPTR p2)
{
#if (defined(TRACE_TASK_INCLUDED) && TRACE_TASK_INCLUDED == TRUE)
    LogMsg (trace_set_mask, fmt_str, p1, p2);
#endif
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_3
 **
 **    Purpose:  Encodes a trace message that has three parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_3 (UINT32 trace_set_mask, const char *fmt_str, UINTPTR p1, UINTPTR p2, UINTPTR p3)
{
#if (defined(TRACE_TASK_INCLUDED) && TRACE_TASK_INCLUDED == TRUE)
    LogMsg (trace_set_mask, fmt_str, p1, p2, p3);
#endif
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_4
 **
 **    Purpose:  Encodes a trace message that has four parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_4 (UINT32 trace_set_mask, const char *fmt_str, UINTPTR p1, UINTPTR p2,
               UINTPTR p3, UINTPTR p4)
{
#if (defined(TRACE_TASK_INCLUDED) && TRACE_TASK_INCLUDED == TRUE)
    LogMsg (trace_set_mask, fmt_str, p1, p2, p3, p4);
#endif
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_5
 **
 **    Purpose:  Encodes a trace message that has five parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_5 (UINT32 trace_set_mask, const char *fmt_str, UINTPTR p1, UINTPTR p2,
               UINTPTR p3, UINTPTR p4, UINTPTR p5)
{
#if (defined(TRACE_TASK_INCLUDED) && TRACE_TASK_INCLUDED == TRUE)
    LogMsg (trace_set_mask, fmt_str, p1, p2, p3, p4, p5);
#endif
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_6
 **
 **    Purpose:  Encodes a trace message that has six parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_6 (UINT32 trace_set_mask, const char *fmt_str, UINTPTR p1, UINTPTR p2,
               UINTPTR p3, UINTPTR p4, UINTPTR p5, UINTPTR p6)
{
#if (defined(TRACE_TASK_INCLUDED) && TRACE_TASK_INCLUDED == TRUE)
    LogMsg (trace_set_mask, fmt_str, p1, p2, p3, p4, p5, p6);
#endif
}

#endif /* BT_USE_TRACES */


#ifdef _DEBUG
/********************************************************************************
 **
 **    Function Name:   wc_assert
 **
 **    Purpose:  Assert Handler
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void wc_assert(char *message, char *file, UINT32 line)
{
}
#endif




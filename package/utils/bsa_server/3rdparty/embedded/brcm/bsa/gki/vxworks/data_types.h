/****************************************************************************/
/*                                                                          */
/*  Name        Data_Types.h                                                */
/*                                                                          */
/*  Function    this file contains common data type definitions used        */
/*              throughout the WIDCOMM Bluetooth code                       */
/*                                                                          */
/*  Date       Modification                                                 */
/*  -----------------------                                                 */
/*  3/12/99    Create                                                       */
/*  07/27/00   Added nettohs macro for Little Endian                        */
/*                                                                          */
/*  Copyright (c) 1999-2004, WIDCOMM Inc., All Rights Reserved.             */
/*  Proprietary and confidential.                                           */
/*                                                                          */
/****************************************************************************/

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include "vxWorks.h"
#include "msgQLib.h"
#include "msgQSmLib.h"
#include "taskLib.h"
#include "tickLib.h"
#include "intLib.h"


#ifndef NULL
#define NULL     0
#endif

#ifndef FALSE
#define FALSE  0
#endif

/* These are needed by the NAP */
#define WAITING  2
#define SUCCESS  1
#define FAILURE  0


#if 0
/* These data types are already declared by VxWorks */
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned long   UINT32;
typedef signed   long   INT32;
typedef signed   char   INT8;
typedef signed   short  INT16;
#endif

typedef unsigned int    *PUINT32;
typedef   int               *PINT32;
typedef unsigned char   WIDECHAR;
#define WIDE_NULL_CHAR  '\0'
typedef UINT32          TIME_STAMP;
typedef unsigned char   BOOLEAN;
typedef unsigned char   UBYTE;


#define PACKED
#define INLINE

#endif


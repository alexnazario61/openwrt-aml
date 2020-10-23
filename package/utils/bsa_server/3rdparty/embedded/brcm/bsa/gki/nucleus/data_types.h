/****************************************************************************
**
**  Name        data_types.h
**
**  Function    this file contains common data type definitions used
**              throughout the WIDCOMM Bluetooth code
**
**  Date       Modification
**  -----------------------
**  3/12/99    Create
**  07/27/00   Added nettohs macro for Little Endian
**
**  Copyright (c) 1999-2004, WIDCOMM Inc., All Rights Reserved.
**  Copyright (c) 2013, Broadcom Corp., All Rights Reserved.
**  Proprietary and confidential.
**
*****************************************************************************/

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include "string.h"

/*Platform-specific includes */
#include "nucleus.h"

#ifndef NULL
#define NULL     0
#endif

#ifndef FALSE
#define FALSE  0
#endif

#if 1

#ifndef UINT8
typedef unsigned char   UINT8;
#endif
#ifndef UINT16
typedef unsigned short  UINT16;
#endif
#ifndef UINT32
typedef unsigned long   UINT32;
#endif
#ifndef INT32
typedef signed   long   INT32;
#endif
#ifndef UINT64
typedef unsigned long long  UINT64;
#endif
#ifndef INT8
typedef signed   char   INT8;
#endif
#ifndef INT16
typedef signed   short  INT16;
#endif

#endif

#ifndef TIME_STAMP
typedef UINT32          TIME_STAMP;
#endif
#ifdef BOOLEAN
#undef BOOLEAN
typedef unsigned char   BOOLEAN;
#endif
#ifndef UINTPTR
typedef unsigned long   UINTPTR;
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif

typedef unsigned char   UBYTE;

#ifdef __arm
#define PACKED  __packed
#define INLINE  __inline
#else
#define PACKED
#define INLINE
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN FALSE
#endif

#if (!defined(__GNUC__)) || (__GNUC__ < 3)
#define __builtin_expect(x, n) (x)
#endif

#define BCM_LIKELY(x) (__builtin_expect(!!(x), 1))
#define BCM_UNLIKELY(x) (__builtin_expect(!!(x), 0))

#define UINT16_LOW_BYTE(x)      ((x) & 0xff)
#define UINT16_HI_BYTE(x)       ((x) >> 8)

#define BCM_STRCAT_S(x1,x2,x3)      strcat((x1),(x3))
#define BCM_STRNCAT_S(x1,x2,x3,x4)  strncat((x1),(x3),(x4))
#define BCM_STRCPY_S(x1,x2,x3)      strcpy((x1),(x3))
#define BCM_STRNCPY_S(x1,x2,x3,x4)  strncpy((x1),(x3),(x4))

#ifndef TIMER_PARAM_TYPE
#define TIMER_PARAM_TYPE    UINTPTR
#endif

struct timeval
{
    UINT32 tv_sec;
    UINT32 tv_usec;
} ;

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

#define GET_HOUR (NU_Retrieve_Clock()*5)/1000
#define GET_MIN ((NU_Retrieve_Clock()*5)/1000)/60
#define GET_SEC ((NU_Retrieve_Clock()*5)/1000)%60
#define GET_MSEC (NU_Retrieve_Clock()*5)%1000

#ifndef getpid
#define getpid() GKI_get_taskid()
#endif

extern int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif

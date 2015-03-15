/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: GPUTypes.h,v $
 * $Revision: 1.10 $
 * $Author: christip $
 * $Date: 2008-02-25 22:32:08 $
 *
 * GPU Types definition file.
 *
 */

#ifndef __GPUTYPES__
#define __GPUTYPES__

#include <stddef.h>

#include <limits.h>

//#define IS_64BITS   (LONG_MAX>2147483647)

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

/* Big constants */
/* _UL - Unsigned long, _SL - Signed long */
#ifdef ENVIRONMENT64  /* 64bit machines */
#define _UL(N)      N##UL
#define _SL(N)      N##L
#else          /* 32bit machines */
#define _UL(N)      N##ULL
#define _SL(N)      N##LL
#endif

typedef float               f32bit;     /*  float point 32 bit.  */
typedef double              f64bit;     /*  float point 64 bit.  */
typedef unsigned char       u8bit;      /* unsigned byte: 0..255 */
typedef signed char         s8bit;      /* signed byte: -128..127 */
typedef unsigned short      u16bit;     /* unsigned halfword: 0..65535 */
typedef signed short        s16bit;     /* signed halfword: -32768..32767*/
typedef unsigned int        u32bit;     /* unsigned word: 0..2^32-1 */
typedef signed int          s32bit;     /* signed word: -2^31..2^31-1 */

//#if IS_64BITS       /* 64bit machines */
typedef unsigned long long  u64bit;     /* unsigned longword: 0..2^64-1 */
typedef signed long long    s64bit;     /* signed longword: -2^63..2^63-1 */
//#else               /* 32bit machines */
//typedef unsigned long long  u64bit;     /* unsigned longword: 0..2^32-1 */
//typedef signed long long    s64bit;     /* signed longword: -2^31..2^31-1 */
//#endif

#ifdef ENVIRONMENT64
typedef unsigned long long  pointer;    /* unsigned integer with the size of a pointer in the machine.  */
#else
typedef unsigned long  pointer;         /* unsigned integer with the size of a pointer in the machine.  */
#endif

typedef u16bit                f16bit;     //  Storage for float 16 values.  No operations implemented.

#include "QuadFloat.h"
#include "QuadInt.h"


/**
 * We also need to define macros for the PRINTF format statements,
 * since on 32bit machines, you typically need a "%lld" to print
 * a 64 bit value, wheareas on 64 bit machines, you simply use "%ld"
 */
#if IS_64BITS
#define FMTU64     "%lu"
#define FMTS64     "%ld"
#define FMTX64     "%lx"
#else
#define FMTU64     "%llu"
#define FMTS64     "%lld"
#define FMTX64     "%llx"
#endif


/**
 * Generic pointer. Be careful with pointer size
 */

typedef char*   genPtr;

#define GPTRSZ  sizeof(genPtr)


/**
 * Register type
 * 64 bits size and contents, with ease to access individual portions
 */
/*
typedef union
{
    u8bit   u8[8];
    u16bit  u16[4];
    u32bit  u32[2];
    //u64bit  u64;
} reg_t;

*/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/**
 * flag
 *
 * To contain a boolean or very range-limited value. Useful to parse
 * arguments and the like.
 */

typedef char    flag;


/**
 * Other structures
 */

typedef enum {
    BIG_END = 1,
    LITTLE_END
} endian_t;

#ifdef WIN32
   #define U64FMT "%I64d"
#else
   #define U64FMT "%lld"
#endif


#endif

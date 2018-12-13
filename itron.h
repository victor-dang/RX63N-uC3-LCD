/***************************************************************************
    MICRO C CUBE / COMPACT, KERNEL
    ITRON macro definitions for eForce operating system (Compact)
    RX Core version
    Copyright (c)  2011-2012, eForce Co., Ltd. All rights reserved.

    Version Information
            2011.09.16: Created.
            2012.07.06: Corresponded to the kernel version 2.
            2012.08.06: Supported for EWRX Compiler
 ***************************************************************************/

#ifndef _ITRON_H_
#define _ITRON_H_

#if 0
#ifndef NULL
#ifdef __cplusplus
#define NULL    (0)
#else
#define NULL    ((void *)0)
#endif
#endif
#endif

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

#define E_OK    0

#define _kernel_INT_SIZE    4
#define _kernel_ALIGN_SIZE  4
#define _kernel_INT_BIT     (_kernel_INT_SIZE*8)

#define TKERNEL_PRID    0x0161u
#define TKERNEL_PRVER   0x0202u


/************************************
    Byte Order Type
 ************************************/

#if defined (__RENESAS__)           /* for Renesas Compiler */
#if defined (__BIG)
#define _UC3_ENDIAN_BIG
#undef _UC3_ENDIAN_LITTLE
#else
#define _UC3_ENDIAN_LITTLE
#undef _UC3_ENDIAN_BIG
#endif
#elif defined (__IAR_SYSTEMS_ICC__) /* for IAR Compiler */
#if __BIG_ENDIAN__ == 1
#define _UC3_ENDIAN_BIG
#undef _UC3_ENDIAN_LITTLE
#else
#define _UC3_ENDIAN_LITTLE
#undef _UC3_ENDIAN_BIG
#endif
#endif


/************************************
    Data Types
 ************************************/

typedef signed char B;
typedef signed short H;
typedef signed long W;
typedef unsigned char UB;
typedef unsigned short UH;
typedef unsigned long UW;
typedef char VB;
typedef short VH;
typedef long VW;
typedef void *VP;
typedef void (*FP)(void);

typedef unsigned long SIZE;
typedef unsigned long ADDR;

typedef int INT;
typedef unsigned int UINT;

typedef VP VP_INT;

typedef INT BOOL;
typedef INT FN;
typedef INT ER;
typedef INT ID;
typedef INT PRI;
typedef INT BOOL_ID;
typedef INT ER_ID;
typedef INT ER_UINT;
typedef UINT FLGPTN;
typedef UINT INTNO;
typedef UINT IMASK;

typedef UINT ATR;
typedef UINT STAT;
typedef UINT MODE;

#ifdef _kernel_LARGE
typedef UH TID;
#else
typedef UB TID;
#endif

typedef struct t_systim {
    UW utime;
    UW ltime;
} SYSTIM;

typedef W TMO;

typedef UW RELTIM;

typedef UW T_REG;

typedef struct t_par {
    ER_UINT ercd;
    FLGPTN  flgptn;
    union {
        VP      ptr;
        VP_INT  dtq;
    } p2;
} T_PAR;

typedef struct t_cnsdep {
    UB  knlipl;
    UB  rserved[3];
} T_CNSDEP;

#define CNSTBL_DEPENDEND T_CNSDEP cnsdep;
#define TCB_DEPENDEND


extern UW vget_ctx(void);

#endif /* _ITRON_H_ */

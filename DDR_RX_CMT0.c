/***********************************************************************
    Micro C Cube Compact, DEVICE DRIVER
    Interval Timer code for RX CMT

    Copyright (c)  2012, eForce Co., Ltd. All rights reserved.

    Version Information
            2012.07.10: Created.
            2012.11.28: Applicated for CMT1-3.
 ***********************************************************************/

#include "kernel.h"
#include <machine.h>

#include "DDR_RX_CMT0_cfg.h"

#if CH_TIM==0
  #define _CMTn         IPR(CMT0,CMI0)
  #define _IEn          IEN(CMT0,CMI0)
  #define CMCORn        CMT0.CMCOR
  #define CMCNTn        CMT0.CMCNT
  #define STRn          CMT.CMSTR0.BIT.STR0
  #define CMIEn         CMT0.CMCR.BIT.CMIE
  #define CKSn          CMT0.CMCR.BIT.CKS
  #define MSTP_CMTn     MSTP_CMT0
#elif CH_TIM==1
  #define _CMTn         IPR(CMT1,CMI1)
  #define _IEn          IEN(CMT1,CMI1)
  #define CMCORn        CMT1.CMCOR
  #define CMCNTn        CMT1.CMCNT
  #define STRn          CMT.CMSTR0.BIT.STR1
  #define CMIEn         CMT1.CMCR.BIT.CMIE
  #define CKSn          CMT1.CMCR.BIT.CKS
  #define MSTP_CMTn     MSTP_CMT1
#elif CH_TIM==2
  #define _CMTn         IPR(CMT2,CMI2)
  #define _IEn          IEN(CMT2,CMI2)
  #define CMCORn        CMT2.CMCOR
  #define CMCNTn        CMT2.CMCNT
  #define STRn          CMT.CMSTR1.BIT.STR2
  #define CMIEn         CMT2.CMCR.BIT.CMIE
  #define CKSn          CMT2.CMCR.BIT.CKS
  #define MSTP_CMTn     MSTP_CMT2
#elif CH_TIM==3
  #define _CMTn         IPR(CMT3,CMI3)
  #define _IEn          IEN(CMT3,CMI3)
  #define CMCORn        CMT3.CMCOR
  #define CMCNTn        CMT3.CMCNT
  #define STRn          CMT.CMSTR1.BIT.STR3
  #define CMIEn         CMT3.CMCR.BIT.CMIE
  #define CKSn          CMT3.CMCR.BIT.CKS
  #define MSTP_CMTn     MSTP_CMT3
#else
  #error invalid chanel number!
#endif


/*******************************
      周期タイマの起動
 *******************************/

void _ddr_rx_cmt0_isr(void)
{
    isig_tim();
}


/*******************************
        初期化ルーチン
 *******************************/

void _ddr_rx_cmt0_init(void)
{
    BOOL locsts;
    UW tcnt;

    tcnt = (PCLK * TICK) / 8000;

    locsts = sns_loc();
    if (locsts == FALSE)
        loc_cpu();

#if RX_SRS == 630 || RX_SRS == 631 || RX_SRS == 635
    SYSTEM.PRCR.WORD = 0xa502;
#endif
    MSTP_CMTn = 0;
#if RX_SRS == 630 || RX_SRS == 631 || RX_SRS == 635
    SYSTEM.PRCR.WORD = 0xa500;
#endif
    CMCORn = tcnt-1;
    CMCNTn = 0;
    CKSn = 0;
    _CMTn = IPL_TIM;
    _IEn = 1;
    CMIEn = 1;
    STRn = 1;

    if (locsts == FALSE)
        unl_cpu();
}

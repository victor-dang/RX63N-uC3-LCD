/***********************************************************************
    Kernel configuration

 ***********************************************************************/

#include "kernel.h"

/*******************************
    システム スタック サイズ
 *******************************/

#pragma stacksize su = 0x400      /* Time Event Handler */
#pragma stacksize si = 0x400      /* Isr Service Routine */

extern void _ddr_init(void);

/*******************************
  Inner initialize function
 *******************************/
void _kernel_initial(void)
{
    _ddr_init();
}

/*******************************
  Various management table
 *******************************/

UB const _kernel_atrtbl[] = {
0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x42, 
0x42, 0x60, 0x60, };

extern void MainTask(VP_INT exinf);
int _kernel_task1_stk[0x200/sizeof(int)];
T_CTSK const _kernel_task1 = {0,
            (FP)MainTask, 0x200, (VP)_kernel_task1_stk, 0, 4};

extern void LedTask(VP_INT exinf);
int _kernel_task2_stk[0x100/sizeof(int)];
T_CTSK const _kernel_task2 = {0,
            (FP)LedTask, 0x100, (VP)_kernel_task2_stk, 0, 8};

T_CFLG const _kernel_flg1 = {0x0};

T_CFLG const _kernel_flg2 = {0x0};

void const * const _kernel_inftbl[] = {
(void const *)&_kernel_task1,
(void const *)&_kernel_task2,
(void const *)&_kernel_flg1,
(void const *)&_kernel_flg2,
};

T_TCB _kernel_tcb[2];
FLGPTN _kernel_flg[2];
T_WTID _kernel_waique[13];

VB const * const _kernel_objname[] = {
(VB const *)0,
(VB const *)0,
(VB const *)0,
(VB const *)0,
(VB const *)0,
(VB const *)0,
(VB const *)0,
(VB const *)0,
(VB const *)0,
"ID_TaskMain",
"ID_TaskLed",
"ID_FLG_SCI_2",
"ID_FLG_DDR_TPL",
};

VP const _kernel_ctrtbl[] = {
(VP)&_kernel_tcb[0],
(VP)&_kernel_tcb[1],
(VP)&_kernel_flg[0],
(VP)&_kernel_flg[1],
};

T_CNSTBL const _kernel_cnstbl = {
_kernel_atrtbl,
_kernel_inftbl-9,
0,
0,
_kernel_waique,
_kernel_ctrtbl-9,
_kernel_objname,
TKERNEL_PRID,
TKERNEL_PRVER,
1,
8,
12,
15         /* Kernel Level */
};

/* end */

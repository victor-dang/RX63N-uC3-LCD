/***********************************************************************
    Initialization of the hardware-dependent
 ***********************************************************************/

#include "kernel.h"

extern void _ddr_rx_cmt0_init(void);


/***********************************************
        Initialize RX peripherals
 ***********************************************/
void init_peripheral(void)
{

}

/***********************************************
  Device driver Initialize entry
 ***********************************************/
void _ddr_init(void)
{
    _ddr_rx_cmt0_init();
}

/* end */

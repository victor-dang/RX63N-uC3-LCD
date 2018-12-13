/***********************************************************************
  Sample program
 ***********************************************************************/

#include "kernel.h"
#include "kernel_id.h"
#include "hw_dep.h"
#include "DDR_RX_SCI_I2C.h"
#include "ddr_lcd.h"
#include "ddr_tpl.h"

extern void init_peripheral(void);

/*******************************
    Default interrupt handler
 *******************************/
void int_abort(void)
{
    while (1);
}

/*******************************
        Main Task
 *******************************/
void MainTask(VP_INT exinf)
{
    ER ercd;
    T_TPL_DAT tpl_dat;

    /* I2C */
    i2c_ini();

    /* LCD */
    ercd = lcd_ini();
    if (ercd != E_OK) {
        return;
    }

    /* Touch panel */
    ercd = tpl_ini();
    if (ercd != E_OK) {
        return;
    }

    while (1) {
        ercd = tpl_get_dat(&tpl_dat, 800);
        if (ercd == E_OK) {
            /* Touch panel */
            if ((tpl_dat.sts & TPL_STS_TPL) != 0x00) {
                lcd_put_img(tpl_dat.x, tpl_dat.y);    /* Draw image */
            }
            /* Switch 1 */
            if ((tpl_dat.sts & TPL_STS_SW1) != 0x00) {
                lcd_adj_blight(-10);     /* Turn down the backlight */
            }
            /* Switch 2 */
            if ((tpl_dat.sts & TPL_STS_SW2) != 0x00) {
                lcd_put_img(120, 68);    /* Middle position */
            }
            /* Switch 3 */
            if ((tpl_dat.sts & TPL_STS_SW3) != 0x00) {
                lcd_adj_blight(10);     /* Turn up the backlight */
            }
        }
    }
}

/*******************************
        LED Task
 *******************************/
void LedTask(VP_INT exinf)
{
    while (1) {
        PORTJ.PODR.BIT.B5 = 0;      /* ON */
        dly_tsk(200);
        PORTJ.PODR.BIT.B5 = 1;      /* OFF */
        dly_tsk(200); 
    }
}

/*******************************
        Main entry
 *******************************/
int main(void)
{
    ER ret;

    /* Initialize hardware */
    init_peripheral();

    /* LED(LD2) */
    PORTJ.PMR.BIT.B5 = 0;
    PORTJ.PDR.BIT.B5 = 1;
    PORTJ.PODR.BIT.B5 = 1;    /* OFF */

    /* Generate system and start */
    ret = start_uC3();

    return ret;
}


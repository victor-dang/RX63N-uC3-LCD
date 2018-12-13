/***************************************************************************
    Micro C Cube Compact
    Touch Panel
    
    Copyright (c) 2013, eForce Co., Ltd. All rights reserved.
    
    Version Information
        2013.04.08: Created
 ***************************************************************************/

#include "kernel.h"
#include "kernel_id.h"
#include "rx63n.h"
#include "DDR_RX_SCI_I2C.h"
#define DDR_TPL_C
#include "ddr_tpl.h"

extern T_RX_SCI_I2C_MGR _ddr_rx_sci_i2c_mgr2;

/* Configuration */
#define TPL_FLG         ID_FLG_DDR_TPL     /* Event flag ID */
#define TPL_INTR_PRI    12                 /* Interrupt priority level */

/* Touch panel initial commands */
typedef struct t_tpl_cmd_ini {
    UB dat[4];    /* Type, Address, Value1, Value2 */
} T_TPL_CMD_INI;

#define TPL_INI_CNT    59   /* Number of initial command */
#define TPL_INI_TYP    0    /* Index for type */
#define TPL_INI_CMD    1    /* Index for command */
#define TPL_WR         0    /* Write 1-byte */
#define TPL_WR2        1    /* Write 2-byte */
#define TPL_DLY        2    /* Delay */

static const T_TPL_CMD_INI ddr_tpl_cmd_ini[TPL_INI_CNT] = {
    {TPL_WR,  0x23, 0x00, 0x00},
    {TPL_DLY,    2, 0x00, 0x00},
    {TPL_WR,  0x2b, 0x03, 0x00},
    {TPL_WR,  0xd4, 0x00, 0x00},
    {TPL_WR,  0x06, 0x0d, 0x00},
    {TPL_WR,  0x07, 0x05, 0x00},
    {TPL_WR,  0x08, 0x00, 0x00},
    {TPL_WR,  0x09, 0x01, 0x00},
    {TPL_WR,  0x0a, 0x02, 0x00},
    {TPL_WR,  0x0b, 0x03, 0x00},
    {TPL_WR,  0x0c, 0x04, 0x00},
    {TPL_WR,  0x0d, 0x05, 0x00},
    {TPL_WR,  0x0e, 0x06, 0x00},
    {TPL_WR,  0x0f, 0x07, 0x00},
    {TPL_WR,  0x10, 0x08, 0x00},
    {TPL_WR,  0x11, 0x09, 0x00},
    {TPL_WR,  0x12, 0x0a, 0x00},
    {TPL_WR,  0x13, 0x0b, 0x00},
    {TPL_WR,  0x14, 0x0c, 0x00},
    {TPL_WR,  0x15, 0x0d, 0x00},
    {TPL_WR,  0x16, 0x0e, 0x00},
    {TPL_WR,  0x17, 0x0f, 0x00},
    {TPL_WR,  0x18, 0x10, 0x00},
    {TPL_WR,  0x19, 0x11, 0x00},
    {TPL_WR,  0x1a, 0x12, 0x00},
    {TPL_WR,  0x1b, 0x13, 0x00},
    {TPL_WR,  0x1c, 0x14, 0x00},
    {TPL_WR,  0x2a, 0x03, 0x00},
    {TPL_WR,  0x8d, 0x01, 0x00},
    {TPL_DLY,   10, 0x00, 0x00},
    {TPL_WR,  0x8d, 0x00, 0x00},
    {TPL_WR,  0x25, 0x0c, 0x00},
    {TPL_DLY,   10, 0x00, 0x00},
    {TPL_WR,  0xc1, 0x02, 0x00},
    {TPL_WR,  0xd5, 0x0c, 0x00},
    {TPL_DLY,   30, 0x00, 0x00},
    {TPL_WR,  0x38, 0x00, 0x00},
    {TPL_WR,  0x33, 0x01, 0x00},
    {TPL_WR,  0x34, 0x3a, 0x00},
    {TPL_WR2, 0x35, 0x00, 0x40},
    {TPL_WR,  0x36, 0x1e, 0x00},
    {TPL_WR,  0x37, 0x03, 0x00},
    {TPL_WR,  0x39, 0x01, 0x00},
    {TPL_WR,  0x56, 0x01, 0x00},
    {TPL_WR2, 0x51, 0x00, 0xff},
    {TPL_WR2, 0x52, 0x00, 0xff},
    {TPL_WR,  0x53, 0x20, 0x00},
    {TPL_WR,  0x54, 0x40, 0x00},
    {TPL_WR,  0x55, 0x40, 0x00},
    {TPL_WR,  0xd9, 0x01, 0x00},
    {TPL_WR,  0xd8, 0x03, 0x00},
    {TPL_WR,  0xd7, 0x04, 0x00},
    {TPL_WR,  0x2c, 0x02, 0x00},
    {TPL_WR,  0x3d, 0x01, 0x00},
    {TPL_WR,  0xd6, 0x01, 0x00},
    {TPL_WR,  0xa2, 0x00, 0x00},
    {TPL_WR,  0x2c, 0x02, 0x00},
    {TPL_WR,  0x66, 0x35, 0x00},
    {TPL_WR,  0x67, 0x36, 0x00}
};

/* Event flag for touch panel */
#define FLG_TPL_ON     0x01

/* I2C */
#define I2C_SLV_ADR_CTL     0x84    /* Controller slave address */
#define I2C_SLV_ADR_TPL     0xb8    /* Touch panel slave address */
#define I2C_TMO             400
#define I2C_BUF_LEN         8
#define i2c_mgr             (&_ddr_rx_sci_i2c_mgr2)

/* Touch panel controller */
#define LCD_WIDTH           480
#define LCD_HEIGHT          272
#define CTL_CMD_STS         0x01    /* Command : Status */
#define CTL_STS_TPL         0x01    /* Status  : Touch panel */
#define CTL_STS_SW          0x1c    /* Status  : Switch */

/* Variables */
static UW ddr_tpl_i2c_buf[I2C_BUF_LEN / sizeof(UW)];

void ddr_tpl_intr(void)
{
    /* Interrupt routine IRQ5 */

    iset_flg(TPL_FLG, FLG_TPL_ON);

    return;
}

ER ddr_tpl_ini(void)
{
    ER ercd;
    INT i;
    const UB* cmd;
    UH len;
    UB* buf;

    /* Initialization */

    /* Port */
    loc_cpu();
    MPC.PWPR.BYTE = 0x00;
    MPC.PWPR.BYTE = 0x40;
    MPC.P15PFS.BYTE = 0x40;     /* P15:IRQ5 */
    MPC.PWPR.BYTE = 0x80;
    PORT1.PMR.BYTE &= ~0x20;
    ICU.IRQCR[5].BYTE = 0x04;   /* Negative edge trigger */
    IPR(ICU, IRQ5) = TPL_INTR_PRI;
    IEN(ICU, IRQ5) = 1;
    unl_cpu();

    /* Touch panel */
    i2c_set_adr(I2C_SLV_ADR_TPL, i2c_mgr);
    for(i = 0; i < TPL_INI_CNT; i++) {
        if (ddr_tpl_cmd_ini[i].dat[TPL_INI_TYP] == TPL_DLY) {
            /* Delay */
            dly_tsk((RELTIM)ddr_tpl_cmd_ini[i].dat[TPL_INI_CMD] * 10);
        } else {
            /* Command */
            cmd = &ddr_tpl_cmd_ini[i].dat[TPL_INI_CMD];
            len = (ddr_tpl_cmd_ini[i].dat[TPL_INI_TYP] == TPL_WR) ? 2 : 3;
            ercd = i2c_snd(cmd, len, I2C_TMO, i2c_mgr);
            if (ercd != E_OK) {
                return ercd;
            }
        }
    }
    /* Clear event */
    buf = (UB*)&ddr_tpl_i2c_buf[0];
    buf[0] = 0x81;
    ercd = i2c_snd(buf, 1, I2C_TMO, i2c_mgr);
    if (ercd != E_OK) {
        return ercd;
    }

    return E_OK;
}

ER ddr_tpl_get_dat(T_TPL_DAT* dat, TMO tmo)
{
    ER ercd;
    FLGPTN flg;
    I2C_DAT i2c_dat;
    UB* cmd;
    UB* res;

    /* Retrieves information */

    if (dat == 0) {
        return E_PAR;
    }
    dat->sts = TPL_STS_INV;

    cmd = (UB*)&ddr_tpl_i2c_buf[0];
    res = (UB*)&ddr_tpl_i2c_buf[1];
    i2c_dat.cmd = cmd;
    i2c_dat.res = res;
    i2c_dat.cmd_len = 1;
    i2c_dat.res_len = 1;

    while (1) {
        /* Touch panel event status */
        i2c_set_adr(I2C_SLV_ADR_TPL, i2c_mgr);
        cmd[0] = 0x79;
        ercd = i2c_cmd(&i2c_dat, I2C_TMO, i2c_mgr); 
        if (ercd != E_OK) {
            return ercd;
        }
        
        if (res[0] == 0x00) {
            /* Wait for interrupt IRQ5 */
            ercd = twai_flg(TPL_FLG, FLG_TPL_ON, TWF_ORW, &flg, tmo);
            if (ercd != E_OK) {
                return ercd;
            }
            
            /* Status polling */
            i2c_set_adr(I2C_SLV_ADR_CTL, i2c_mgr);
            cmd[0] = CTL_CMD_STS;
            ercd = i2c_cmd(&i2c_dat, I2C_TMO, i2c_mgr); 
            if (ercd != E_OK) {
                return ercd;
            }
            
            /* Switch */
            dat->sts = res[0] & CTL_STS_SW;
            
            /* Touch panel event status */
            if ((res[0] & CTL_STS_TPL) != 0x00) {
                i2c_set_adr(I2C_SLV_ADR_TPL, i2c_mgr);
                cmd[0] = 0x79;
                ercd = i2c_cmd(&i2c_dat, I2C_TMO, i2c_mgr); 
                if (ercd != E_OK) {
                    return ercd;
                }
            } else {
                res[0] = 0x00;
            }
        }
        
        /* Coordinate */
        if ((res[0] & 0x01) != 0x00) {
            cmd[0] = 0x7c;
            i2c_dat.res_len = 4;
            ercd = i2c_cmd(&i2c_dat, I2C_TMO, i2c_mgr); 
            dat->x = (((UH)res[2] << 8) & 0x0f00) | res[1];
            dat->y = (((UH)res[2] << 4) & 0x0f00) | res[0];
            dat->x = (dat->x > LCD_WIDTH) ? LCD_WIDTH : dat->x;
            dat->y = (dat->y > LCD_HEIGHT) ? 0 : LCD_HEIGHT - dat->y;
            dat->sts |= TPL_STS_TPL;
        }
        
        /* Clear event */
        if ((res[0] & 0x10) != 0x00) {
            cmd[0] = 0x81;
            ercd = i2c_snd(cmd, 1, I2C_TMO, i2c_mgr);
            if (ercd != E_OK) {
                return ercd;
            }
        }
        
        if (dat->sts != 0x00) {
            break;
        }
    }

    return E_OK;
}


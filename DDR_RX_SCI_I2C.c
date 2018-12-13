/***************************************************************************
    Micro C Cube Compact, DEVICE DRIVER
    SCI I2C
    
    Copyright (c) 2013, eForce Co., Ltd. All rights reserved.
    
    Version Information
        2013.04.08: Created
 ***************************************************************************/

#include "kernel.h"
#include "kernel_id.h"
#include "DDR_RX_SCI_I2C.h"
#include "DDR_RX_SCI_I2C_cfg.h"

/* Configuration */
#ifdef CFG_SCI2
T_RX_SCI_I2C_MGR _ddr_rx_sci_i2c_mgr2 = {
    (volatile struct st_sci0 *)&SCI2, CFG_SCI2_ID_FLG, 0
};
#endif

/* Event flags */
#define FLG_INTR_RXI    0x0001      /* Interrupt rxi */
#define FLG_INTR_TXI    0x0002      /* Interrupt txi */
#define FLG_INTR_TEI    0x0004      /* Interrupt tei */

void _ddr_rx_sci_i2c_intr_rxi(T_RX_SCI_I2C_MGR const* mgr)
{
    /* Interrupt service routine rxi */

    iset_flg(mgr->flg, FLG_INTR_RXI);

    return;
}

void _ddr_rx_sci_i2c_intr_txi(T_RX_SCI_I2C_MGR const* mgr)
{
    /* Interrupt service routine txi */

    iset_flg(mgr->flg, FLG_INTR_TXI);

    return;
}

void _ddr_rx_sci_i2c_intr_tei(T_RX_SCI_I2C_MGR const* mgr)
{
    /* Interrupt service routine tei */

    mgr->reg->SIMR3.BIT.IICSTIF = 0;
    iset_flg(mgr->flg, FLG_INTR_TEI);

    return;
}

static ER _ddr_rx_sci_i2c_snd_dat(const void* buf, UH len, TMO tmo, T_RX_SCI_I2C_MGR* mgr)
{
    ER ercd;
    FLGPTN flg;
    UB* dat;
    UH snd_len;

    /* Send */

    dat = (UB*)buf;
    snd_len = 0;

    /* Start */
    clr_flg(mgr->flg, 0x0000);
    mgr->reg->SIMR3.BYTE = 0x51;
    ercd = twai_flg(mgr->flg, FLG_INTR_TEI, TWF_ORW, &flg, tmo);
    if (ercd != E_OK) {
        return ercd;
    }

    /* Slave address */
    mgr->reg->SIMR3.BIT.IICSCLS = 0;
    mgr->reg->SIMR3.BIT.IICSDAS = 0;
    clr_flg(mgr->flg, 0x0000);
    mgr->reg->TDR = mgr->adr;
    ercd = twai_flg(mgr->flg, FLG_INTR_TXI, TWF_ORW, &flg, tmo);
    if (ercd != E_OK) {
        return ercd;
    }

    /* Send */
    if (mgr->reg->SISR.BIT.IICACKR == 0) {
        for (; snd_len < len; snd_len++) {
            clr_flg(mgr->flg, 0x0000);
            mgr->reg->TDR = dat[snd_len];
            ercd = twai_flg(mgr->flg, FLG_INTR_TXI, TWF_ORW, &flg, tmo);
            if (ercd != E_OK) {
                break;
            }
        }
    }

    if (snd_len != len) {
        ercd = E_OBJ;
    }

    return ercd;
}

static ER _ddr_rx_sci_i2c_rcv_dat(VP buf, UH len, TMO tmo, T_RX_SCI_I2C_MGR* mgr) 
{
    ER ercd;
    FLGPTN flg;
    UB* dat;
    UH i;
    UH rcv_len;

    /* Receive */

    dat = (UB*)buf;
    rcv_len = 0;

    /* Start */
    clr_flg(mgr->flg, 0x0000);
    mgr->reg->SIMR3.BYTE = 0x51;
    ercd = twai_flg(mgr->flg, FLG_INTR_TEI, TWF_ORW, &flg, tmo);
    if (ercd != E_OK) {
        return ercd;
    }

    /* Slave address */
    mgr->reg->SIMR3.BIT.IICSCLS = 0;
    mgr->reg->SIMR3.BIT.IICSDAS = 0;
    clr_flg(mgr->flg, 0x0000);
    mgr->reg->TDR = mgr->adr | 0x01;
    ercd = twai_flg(mgr->flg, FLG_INTR_TXI, TWF_ORW, &flg, tmo);
    if (ercd != E_OK) {
        return ercd;
    }

    /* Receive */
    if (mgr->reg->SISR.BIT.IICACKR == 0) {
        mgr->reg->SIMR2.BIT.IICACKT = 0;
        mgr->reg->SCR.BIT.RIE = 1;
        for (i = 0; i < len - 1; i++) {
            clr_flg(mgr->flg, 0x0000);
            mgr->reg->TDR = 0xff;
            ercd = twai_flg(mgr->flg, FLG_INTR_RXI, TWF_ORW, &flg, tmo);
            if (ercd != E_OK) {
                break;
            }
            dat[i] = mgr->reg->RDR;
            rcv_len++;
            ercd = twai_flg(mgr->flg, FLG_INTR_TXI, TWF_ORW, &flg, tmo);
            if (ercd != E_OK) {
                break;
            }
        }
        
        if (ercd == E_OK) {
            mgr->reg->SIMR2.BIT.IICACKT = 1;
            clr_flg(mgr->flg, 0x0000);
            mgr->reg->TDR = 0xff;
            ercd = twai_flg(mgr->flg, FLG_INTR_RXI, TWF_ORW, &flg, tmo);
            if (ercd == E_OK) {
                dat[i] = mgr->reg->RDR;
                rcv_len++;
                ercd = twai_flg(mgr->flg, FLG_INTR_TXI, TWF_ORW, &flg, tmo);
            }
        }
    }

    if (rcv_len != len) {
        ercd = E_OBJ;
    }

    return ercd;
}

void _ddr_rx_sci_i2c_ini(void)
{
    /* Initialization */

    /* SCI2 */
    #ifdef CFG_SCI2
    loc_cpu();
    /* Power */
    SYSTEM.PRCR.WORD = 0xa502;
    MSTP_SCI2 = 0;
    SYSTEM.PRCR.WORD = 0xa500;

    /* Disabled */
    SCI2.SCR.BYTE = 0x00;

    /* Pin */
    #ifdef CFG_SCI2_PIN
    #if (CFG_SCI2_PIN == 0)
    PORT1.ODR0.BYTE = 0x50;         /* Open drain */
    MPC.PWPR.BYTE = 0x00;
    MPC.PWPR.BYTE = 0x40;
    MPC.P12PFS.BIT.PSEL = 0x0a;     /* P12 */
    MPC.P13PFS.BIT.PSEL = 0x0a;     /* P13 */
    MPC.PWPR.BYTE = 0x80;
    PORT1.PMR.BYTE |= 0x0c;
    #elif (CFG_SCI2_PIN == 1)
    PORT5.ODR0.BYTE = 0x50;         /* Open drain */
    MPC.PWPR.BYTE = 0x00;
    MPC.PWPR.BYTE = 0x40;
    MPC.P50PFS.BIT.PSEL = 0x0a;     /* P50 */
    MPC.P52PFS.BIT.PSEL = 0x0a;     /* P52 */
    MPC.PWPR.BYTE = 0x80;
    PORT5.PMR.BYTE |= 0x05;
    #endif
    #endif
    unl_cpu();

    /* Mode */
    SCI2.SIMR3.BYTE = 0xf0;
    SCI2.SMR.BYTE = 0x00;           /* CLK = PCLK */
    SCI2.SCMR.BYTE = 0xfa;
    SCI2.BRR = 0x04;                /* 約300K */
    SCI2.SEMR.BYTE = 0x20;          /* ノイズ除去機能有効 */
    SCI2.SNFR.BYTE = 0x01;          /* 1分周でノイズ除去 */
    SCI2.SIMR1.BYTE = 0xf9;         /* 簡易I2Cモード */
    SCI2.SIMR2.BYTE = 0x23;
    SCI2.SPMR.BYTE = 0x00;

    /* Interrupt enable */
    loc_cpu();
    IPR(SCI2, RXI2) = CFG_SCI2_INTR_PRI;
    if (IEN(ICU,GROUP12) == 0) {
        IPR(ICU, GROUP12) = CFG_SCI2_INTR_PRI;
        IEN(ICU,GROUP12) = 1;
    } else {
        if (IPR(ICU, GROUP12) < CFG_SCI2_INTR_PRI) {
            IPR(ICU, GROUP12) = CFG_SCI2_INTR_PRI;
        }
    }
    ICU.GEN[GEN_SCI2_ERI2].BIT.EN2 = 1;
    IEN(SCI2, RXI2) = 1;
    IEN(SCI2, TXI2) = 1;
    IEN(SCI2, TEI2) = 1;
    unl_cpu();

    /* Transfer enabled */
    SCI2.SCR.BYTE = 0x30;
    #endif

    return;
}

ER _ddr_rx_sci_i2c_set_adr(UB adr, T_RX_SCI_I2C_MGR* mgr)
{
    /* Set a slave address */

    if (mgr == 0) {
        return E_PAR;
    }

    mgr->adr = adr & 0xfe;

    return E_OK;
}

ER _ddr_rx_sci_i2c_snd(const void* buf, UH len, TMO tmo, T_RX_SCI_I2C_MGR* mgr)
{
    ER ercd;
    FLGPTN flg;

    /* Send */

    if (buf == 0) {
        return E_PAR;
    }
    if (len == 0) {
        return E_OK;
    }

    mgr->reg->SCR.BIT.TIE = 1;
    mgr->reg->SCR.BIT.TEIE = 1;
    ercd = _ddr_rx_sci_i2c_snd_dat(buf, len, tmo, mgr);

    /* End */
    clr_flg(mgr->flg, 0x0000);
    mgr->reg->SIMR3.BYTE = 0x54;
    twai_flg(mgr->flg, FLG_INTR_TEI, TWF_ORW, &flg, tmo);
    mgr->reg->SIMR3.BIT.IICSCLS = 3;
    mgr->reg->SIMR3.BIT.IICSDAS = 3;
    mgr->reg->SCR.BIT.TIE = 0;
    mgr->reg->SCR.BIT.TEIE = 0;

    return ercd;
}

ER _ddr_rx_sci_i2c_rcv(VP buf, UH len, TMO tmo, T_RX_SCI_I2C_MGR* mgr) 
{
    ER ercd;
    FLGPTN flg;

    /* Receive */

    if (buf == 0) {
        return E_PAR;
    }
    if (len == 0) {
        return E_OK;
    }

    mgr->reg->SCR.BIT.TIE = 1;
    mgr->reg->SCR.BIT.TEIE = 1;
    ercd = _ddr_rx_sci_i2c_rcv_dat(buf, len, tmo, mgr);

    /* End */
    clr_flg(mgr->flg, 0x0000);
    mgr->reg->SIMR3.BYTE = 0x54;
    twai_flg(mgr->flg, FLG_INTR_TEI, TWF_ORW, &flg, tmo);
    mgr->reg->SIMR3.BIT.IICSCLS = 3;
    mgr->reg->SIMR3.BIT.IICSDAS = 3;
    mgr->reg->SCR.BIT.RIE = 0;
    mgr->reg->SCR.BIT.TIE = 0;
    mgr->reg->SCR.BIT.TEIE = 0;

    return ercd;
}

ER _ddr_rx_sci_i2c_cmd(T_RX_SCI_I2C_DAT* dat, TMO tmo, T_RX_SCI_I2C_MGR* mgr)
{
    ER ercd;
    FLGPTN flg;

    /* Command */

    if (dat == 0) {
        return E_PAR;
    }
    if (dat->cmd_len == 0 || dat->cmd == 0) {
        return E_PAR;
    }

    mgr->reg->SCR.BIT.TIE = 1;
    mgr->reg->SCR.BIT.TEIE = 1;
    ercd = _ddr_rx_sci_i2c_snd_dat(dat->cmd, dat->cmd_len, tmo, mgr);
    if (ercd == E_OK && dat->res_len != 0) {
        ercd = _ddr_rx_sci_i2c_rcv_dat(dat->res, dat->res_len, tmo, mgr);
    }

    /* End */
    clr_flg(mgr->flg, 0x0000);
    mgr->reg->SIMR3.BYTE = 0x54;
    twai_flg(mgr->flg, FLG_INTR_TEI, TWF_ORW, &flg, tmo);
    mgr->reg->SIMR3.BIT.IICSCLS = 3;
    mgr->reg->SIMR3.BIT.IICSDAS = 3;
    mgr->reg->SCR.BIT.RIE = 0;
    mgr->reg->SCR.BIT.TIE = 0;
    mgr->reg->SCR.BIT.TEIE = 0;

    return ercd;
}


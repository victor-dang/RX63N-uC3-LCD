/***************************************************************************
    Micro C Cube Compact
    LCD Direct Drive
    
    Copyright (c) 2013, eForce Co., Ltd. All rights reserved.
    
    Version Information
        2013.04.08: Created
 ***************************************************************************/

#include "kernel.h"
#include "kernel_id.h"
#include "rx63n.h"
#include "DDR_RX_SCI_I2C.h"
#include <string.h>

extern T_RX_SCI_I2C_MGR _ddr_rx_sci_i2c_mgr2;
extern const UB ddr_lcd_dat_img1[];

/* Configuration */
#define MTU_INTR_PRI    12      /* Interrupt priority level for MTU */

/* LCD */
#define LCD_WIDTH           480
#define LCD_HEIGHT          272
#define LCD_MARGIN          8
#define LCD_CLKNUM          4
#define LCD_DE_START        43
#define LCD_DE_LENGTH       480
#define LCD_HSYNC_CYCLE     575 
#define LCD_HSYNC_LENGTH    41
#define LCD_VSYNC_LENGTH    10
#define LCD_VSYNC_BP        (10 + 2)
#define LCD_VSYNC_DP        (10 + 2 + 272)
#define LCD_VSYNC_FP        (10 + 2 + 272 + 2)
#define LCD_VSYNC_CYCLE     286
#define LCD_BUF_LEN         (LCD_WIDTH * LCD_HEIGHT * 2)
#define LCD_CMD_BACK_LIGHT  0x03
#define LCD_DAT_BACK_LIGHT  0x20

/* I2C */
#define I2C_SLV_ADR_LCD     0x84    /* LCD slave address */
#define I2C_TMO             400
#define I2C_BUF_LEN         4
#define i2c_mgr             (&_ddr_rx_sci_i2c_mgr2)

/* Image data */
#define IMG_1_HEIGHT    136
#define IMG_1_WIDTH     240

/* Variables */
static INT ddr_lcd_blight;
static UW ddr_lcd_buf_ptr;
static UH ddr_lcd_cnt;
static UW ddr_lcd_img_back[(LCD_WIDTH * 2) / sizeof(UW)];
static UW ddr_lcd_i2c_buf[I2C_BUF_LEN / sizeof(UW)];
#pragma section B SRAM_1
static UW ddr_lcd_buf_1[LCD_BUF_LEN / sizeof(UW)];
static UW ddr_lcd_buf_2[LCD_BUF_LEN / sizeof(UW)];
#pragma section

/* Function prototypes */
ER ddr_lcd_adj_blight(INT);
void ddr_lcd_put_img(UH, UH);

void ddr_lcd_intr(void)
{
    /* Interrupt routine MTU3 */

    ddr_lcd_cnt++;

    switch (ddr_lcd_cnt) {
        case LCD_VSYNC_LENGTH:          /* �����������ԏI�� */
            PORT7.PODR.BYTE |= 0x08;    /* VSYNC High */
            break;
        case LCD_VSYNC_BP:              /* �o�b�N�|�[�`���ԏI�� */
            ICU.IR[0x6B].BYTE = 0x00;
            MTU1.TIER.BYTE = 0x03;      /* DMA�N�������� */
            EXDMAC0.EDMCNT.BYTE = 0x01; /* DMA�]������ */
            MTU3.TIORH.BYTE = 0x25;     /* DE�o�͂����� */
            break;
        case LCD_VSYNC_DP:              /* �\�����ԏI�� */
            MTU1.TIER.BYTE = 0x00;      /* DMA�N�����~ */
            MTU3.TIORH.BYTE = 0x11;     /* DE�o�͂��֎~(Low�Œ�) */
            break;
        case LCD_VSYNC_FP:              /* �t�����g�|�[�`���ԏI�� */
            EXDMAC0.EDMSAR = (void*)ddr_lcd_buf_ptr;
            EXDMAC0.EDMCRB = LCD_HEIGHT;
            PORT7.PODR.BYTE &= ~0x08;   /* VSYNC Low */
            ddr_lcd_cnt = 0;            /* �J�E���g������ */
            break;
        default:
            break;
    }
}

ER ddr_lcd_ini(void)
{
    ER ercd;
    UH* dat;
    INT i;
    UH x, y;

    /* Initialization */

    ddr_lcd_cnt = 0;
    ddr_lcd_blight = LCD_DAT_BACK_LIGHT;

    /* External bus */
    BSC.CS1MOD.WORD = 0x0001;
    BSC.CS1WCR1.LONG = 0x02020000;  /* LCD�o��(12MHz)�ɂ��킹�Ē��� */
    BSC.CS1WCR2.LONG = 0x00110111;  /* LCD�o��(12MHz)�ɂ��킹�Ē��� */
    BSC.CS1CR.WORD = 0x0001;        /* ���싖�� */
    BSC.CS1REC.WORD = 0x0000;       /* �f�t�H���g�l */
    BSC.CSRECEN.WORD = 0x3e3e;
    BSC.BEREN.BYTE = 0x00;
    BSC.BUSPRI.WORD = 0x0000;

    loc_cpu();
    MPC.PWPR.BYTE = 0x00;
    MPC.PWPR.BYTE = 0x40;
    MPC.PFCSE.BYTE = 0x02;          /* CS1�o�͋��� */
    MPC.PFCSS0.BYTE = 0x00;         /* P61��CS1�o�͒[�q */
    MPC.PFCSS1.BYTE = 0x00;         /* ���g�p �f�t�H���g�ݒ� */
    MPC.PFAOE0.BYTE = 0xff;         /* A8~15�o�͋��� */
    MPC.PFAOE1.BYTE = 0x0f;         /* A16~19�o�͋��� A20~23�o�͋֎~ */
    MPC.PFBCR0.BYTE = 0x51;         /* P51��WR1/BC1 PE0~PE7��D8~15 PA0~7��A0~A7 */
    MPC.PFBCR1.BYTE = 0x00;         /* �f�t�H���g�l */
    MPC.PWPR.BYTE = 0x80;

    SYSTEM.PRCR.WORD = 0xa502;
    SYSTEM.SYSCR0.WORD = 0x5a03;
    SYSTEM.PRCR.WORD = 0xa500;
    unl_cpu();

    /* Background image */
    dat = (UH*)ddr_lcd_img_back;
    for (i = 0; i < LCD_WIDTH; i++) {
        dat[i] = 0xffff;    /* White */
    }

    /* Foreground image */
    x = (LCD_WIDTH - IMG_1_WIDTH) / 2;
    y = (LCD_HEIGHT - IMG_1_HEIGHT) / 2;
    ddr_lcd_put_img(x, y);

    /* EXDMA */
    loc_cpu();
    SYSTEM.PRCR.WORD = 0xa502;
    MSTP_EXDMAC = 0;
    SYSTEM.PRCR.WORD = 0xa500;
    unl_cpu();
    EXDMAC0.EDMCNT.BYTE = 0x00;                     /* DMA�]���֎~ */
    EXDMAC.EDMAST.BYTE = 0x00;                      /* EXDMAC�N���֎~ */
    EXDMAC0.EDMSAR = (void*)ddr_lcd_buf_ptr;        /* �]�����A�h���X�ݒ� */
    EXDMAC0.EDMCRA = (LCD_WIDTH << 16) + LCD_WIDTH; /* �u���b�N�T�C�Y�ݒ� */
    EXDMAC0.EDMCRB = LCD_HEIGHT;                    /* �u���b�N�]���J�E���g�ݒ� */
    EXDMAC0.EDMTMD.WORD = 0x8103;                   /* EXDMA�ݒ� */
    EXDMAC0.EDMAMD.LONG = 0x00028000;               /* �A�h���X���[�h�ݒ� */
    EXDMAC0.EDMCNT.BYTE = 0x01;                     /* DMA�]������ */
    EXDMAC.EDMAST.BYTE = 0x01;                      /* EXDMAC�N������ */

    /* MTU */
    loc_cpu();
    MPC.PWPR.BYTE = 0x00;
    MPC.PWPR.BYTE = 0x40;
    MPC.P17PFS.BYTE = 0x01;     /* P17:MTIOC3A */
    MPC.P56PFS.BYTE = 0x01;     /* P56:MTIOC3C */
    MPC.P32PFS.BYTE = 0x01;     /* P32:MTIOC0C */
    MPC.PWPR.BYTE = 0x80;
    PORT1.PMR.BYTE |= 0x80;
    PORT3.PMR.BYTE |= 0x04;
    PORT5.PMR.BYTE |= 0x40;
    PORT7.PMR.BYTE &= ~0x08;    /* P73 �ėp���o�̓|�[�g */
    PORT7.PDR.BYTE |= 0x08;     /* P73 �o�̓|�[�g*/
    PORT7.PODR.BYTE &= ~0x08;   /* VSYNC Low */

    SYSTEM.PRCR.WORD = 0xa502;
    MSTP_MTU = 0;
    SYSTEM.PRCR.WORD = 0xa500;
    unl_cpu();
    MTU.TSTR.BYTE &= ~0x01;             /* MTU0�J�E���g�����~ */
    MTU0.TCR.BYTE = 0xa0;               /* TGRC�̃R���y�A�}�b�` �����オ��G�b�W ICLK�^1 */
    MTU0.TMDR.BYTE = 0x02;              /* PWM1���� */
    MTU0.TIORH.BYTE = 0x00;             /* MTU0.TGRA TGRB�̓���:�o�͋֎~ */
    MTU0.TIORL.BYTE = 0x25;             /* MTU0.TGRC�̓���:�����o��1�A�R���y�A�}�b�`��0�o�� */
                                        /* MTU0.TGRD�̓���:�����o��0�A�R���y�A�}�b�`��1�o�� */
    MTU0.TGRC = LCD_CLKNUM - 1;         /* DCLK */
    MTU0.TGRD = (LCD_CLKNUM / 2) - 1;   /* DCLK/2 */

    MTU.TSTR.BYTE &= ~0x02;             /* MTU1�J�E���g�����~ */
    MTU1.TCR.BYTE = 0x20;               /* TGRA�̃R���y�A�}�b�` �����オ��G�b�W ICLK�^1 */
    MTU1.TMDR.BYTE = 0x00;              /* �m�[�}�����[�h���� */
    MTU1.TGRA = (LCD_HSYNC_CYCLE * LCD_CLKNUM) - 1;                         /* HSYNC���� */
    MTU1.TGRB = (LCD_HSYNC_CYCLE * LCD_CLKNUM) - 4;                         /* DCLK/2 */
    MTU1.TCNT = ((LCD_HSYNC_CYCLE - LCD_DE_START + 6) * LCD_CLKNUM) - 1;    /* DCLK/2 */
    MTU1.TIER.BYTE = 0x00;              /* ���荞�ݗv�������� */
    loc_cpu();
    ICU.IER[0x12].BYTE |= 0x20;         /* MTU1 TGIB1�����ݗv������ */
    ICU.IPR[0x95].BYTE = MTU_INTR_PRI;  /* MTU1 TGIB1���荞�ݗD�惌�x���ݒ� */
    unl_cpu();

    MTU.TSTR.BYTE &= ~0x40;             /* MTU3�J�E���g�����~ */
    MTU3.TCR.BYTE = 0xa0;               /* TGRC�̃R���y�A�}�b�` �����オ��G�b�W ICLK�^1 */
    MTU3.TMDR.BYTE = 0x02;              /* PWM1���� */
    MTU3.TIORH.BYTE = 0x11;             /* MTU3.TGRA�̓���:�����o��0�A�R���y�A�}�b�`��0�o�� */
                                        /* MTU3.TGRB�̓���:�����o��0�A�R���y�A�}�b�`��0�o�� */
    MTU3.TIORL.BYTE = 0x25;             /* MTU3.TGRC�̓���:�����o��1�A�R���y�A�}�b�`��0�o�� */
                                        /* MTU3.TGRD�̓���:�����o��0�A�R���y�A�}�b�`��1�o�� */
    MTU3.TGRA = ((LCD_DE_START + LCD_DE_LENGTH) * LCD_CLKNUM) - 1;          /* DE��Low�o�� */
    MTU3.TGRB = (LCD_DE_START * LCD_CLKNUM) - 1;                            /* DE��High�o�� */
    MTU3.TGRC = (LCD_HSYNC_CYCLE * LCD_CLKNUM) - 1;                         /* HSYNC���� */
    MTU3.TGRD = (LCD_HSYNC_LENGTH * LCD_CLKNUM) - 1;                        /* HSYNC �p���X�� */
    MTU3.TIER.BYTE = 0x00;              /* ���荞�ݗv�������� */
    MTU3.TCNT = 0;                      /* �^�C�}�J�E���^������ */
    loc_cpu();
    ICU.IER[0x13].BYTE |= 0x01;         /* MTU3 TGIA3�����ݗv������ */
    ICU.IPR[0x98].BYTE = MTU_INTR_PRI;  /* MTU3 TGIA3���荞�ݗD�惌�x���ݒ� */
    unl_cpu();
    MTU3.TIER.BYTE = 0x01;              /* TGIA���荞�ݗv������ */

    MTU.TSTR.BYTE |= 0x43;              /* MTU0,MTU1,MTU3 �J�E���g����J�n */

    /* LCD backlight */
    ercd = ddr_lcd_adj_blight(0);
    if (ercd != E_OK) {
        return ercd;
    }

    return E_OK;
}

ER ddr_lcd_adj_blight(INT vol)
{
    ER ercd;
    UB* cmd;

    /* Adjust a backlight */

    ddr_lcd_blight += vol;
    if (ddr_lcd_blight < 0) {
        ddr_lcd_blight = 0;
    } else if (ddr_lcd_blight > 0x00ff) {
        ddr_lcd_blight = 0x00ff;
    }

    i2c_set_adr(I2C_SLV_ADR_LCD, i2c_mgr);
    cmd = (UB*)&ddr_lcd_i2c_buf[0];
    cmd[0] = LCD_CMD_BACK_LIGHT;
    cmd[1] = (UB)ddr_lcd_blight;
    ercd = i2c_snd(cmd, 2, I2C_TMO, i2c_mgr);

    return ercd;
}

void ddr_lcd_put_img(UH x, UH y)
{
    static INT cnt = 0;
    UH* dat;
    INT i;
    INT pos_x, pos_y;
    INT len_x, len_y;

    /* Draw image */

    cnt++;
    if ((cnt % 2) == 0) {
        dat = (UH*)ddr_lcd_buf_1;
    } else {
        dat = (UH*)ddr_lcd_buf_2;
    }

    /* Background */
    for(i = 0; i < LCD_HEIGHT; i++) {
        memcpy(&dat[LCD_WIDTH * i], &ddr_lcd_img_back[0], (LCD_WIDTH * 2));
    }

    /* Image 1 */
    pos_x = LCD_WIDTH - IMG_1_WIDTH - LCD_MARGIN;
    pos_x = (x > pos_x) ? pos_x : x;
    len_x = LCD_WIDTH - pos_x;
    len_x = (len_x < IMG_1_WIDTH) ? len_x : IMG_1_WIDTH;

    pos_y = LCD_HEIGHT - IMG_1_HEIGHT - LCD_MARGIN;
    y = (y > pos_y) ? pos_y : y;
    pos_y = LCD_HEIGHT - IMG_1_HEIGHT - y;
    len_y = LCD_HEIGHT - pos_y;
    len_y = (len_y < IMG_1_HEIGHT) ? len_y : IMG_1_HEIGHT;

    for(i = 0; i < len_y; i++) {
        memcpy(&dat[LCD_WIDTH * (i + pos_y) + pos_x], &ddr_lcd_dat_img1[IMG_1_WIDTH * 2 * i], len_x * 2);
    }

    /* Flip */
    loc_cpu();
    if ((cnt % 2) == 0) {
        ddr_lcd_buf_ptr = (UW)&ddr_lcd_buf_1[0];
    } else {
        ddr_lcd_buf_ptr = (UW)&ddr_lcd_buf_2[0];
    }
    unl_cpu();

    return;

}


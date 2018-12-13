/***************************************************************************
    Micro C Cube Compact
    Touch Panel
    
    Copyright (c) 2013, eForce Co., Ltd. All rights reserved.
    
    Version Information
        2013.04.08: Created
 ***************************************************************************/

#ifndef DDR_TPL_H
#define DDR_TPL_H

/* Status */
#define TPL_STS_INV     0x00    /* Invalid */
#define TPL_STS_TPL     0x01    /* Touch panel */
#define TPL_STS_SW1     0x04    /* Switch 1 */
#define TPL_STS_SW2     0x08    /* Switch 2 */
#define TPL_STS_SW3     0x10    /* Switch 3 */

/* Data */
typedef struct t_tpl_dat {
    UH sts;     /* Status */
    UH x;       /* x-coordinate */
    UH y;       /* y-coordinate */
} T_TPL_DAT;

/* Function prototypes */
ER ddr_tpl_ini(void);                    /* Initialization */
ER ddr_tpl_get_dat(T_TPL_DAT*, TMO);     /* Retrieves information */

/* Conversion macros */
#ifndef DDR_TPL_C
#define tpl_ini         ddr_tpl_ini
#define tpl_get_dat     ddr_tpl_get_dat
#endif

#endif


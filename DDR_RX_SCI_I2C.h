/***************************************************************************
    Micro C Cube Compact, DEVICE DRIVER
    SCI I2C Header
    
    Copyright (c) 2013, eForce Co., Ltd. All rights reserved.
    
    Version Information
        2013.04.08: Created
 ***************************************************************************/

#ifndef DDR_RX_SCI_I2C_H
#define DDR_RX_SCI_I2C_H

/* Manager */
typedef struct t_rx_sci_i2c_mgr {
    volatile struct st_sci0* reg;   /* Register */
    ID flg;                         /* ID event flag */
    UB adr;                         /* Slave address */
} T_RX_SCI_I2C_MGR;

/* Command data */
typedef struct t_rx_sci_i2c_dat {
    const void* cmd;     /* Command data */
    VP res;              /* Response data */
    UH  cmd_len;         /* Length of command */
    UH res_len;          /* Length of response */
} T_RX_SCI_I2C_DAT;

/* Function prototypes */
void _ddr_rx_sci_i2c_ini(void);                                             /* Initialization */
ER _ddr_rx_sci_i2c_set_adr(UB, T_RX_SCI_I2C_MGR*);                          /* Set a slave address */
ER _ddr_rx_sci_i2c_snd(const void*, UH, TMO, T_RX_SCI_I2C_MGR*);            /* Send */
ER _ddr_rx_sci_i2c_rcv(VP, UH, TMO, T_RX_SCI_I2C_MGR*);                     /* Receive */
ER _ddr_rx_sci_i2c_cmd(T_RX_SCI_I2C_DAT* dat, TMO, T_RX_SCI_I2C_MGR*);      /* Command */

/* Conversion macros */
#define I2C_DAT         T_RX_SCI_I2C_DAT
#define i2c_ini         _ddr_rx_sci_i2c_ini
#define i2c_set_adr     _ddr_rx_sci_i2c_set_adr
#define i2c_snd         _ddr_rx_sci_i2c_snd
#define i2c_rcv         _ddr_rx_sci_i2c_rcv
#define i2c_cmd         _ddr_rx_sci_i2c_cmd

#endif


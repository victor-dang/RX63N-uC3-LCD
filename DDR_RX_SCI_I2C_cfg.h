/***************************************************************************
    Micro C Cube Compact, DEVICE DRIVER
    SCI I2C Configuration
    
    Copyright (c) 2013, eForce Co., Ltd. All rights reserved.
    
    Version Information
        2013.04.08: Created
 ***************************************************************************/

#ifndef DDR_RX_SCI_I2C_CFG_H
#define DDR_RX_SCI_I2C_CFG_H

/* Configuration */
#define CFG_SCI2                            /* SCI 2 */
#define CFG_SCI2_PIN        0               /* Pin P12, P13 */
#define CFG_SCI2_ID_FLG     ID_FLG_SCI_2    /* ID event flag */
#define CFG_SCI2_INTR_PRI   1               /* Interrupt priority level */

#include "rx63n.h"

#endif


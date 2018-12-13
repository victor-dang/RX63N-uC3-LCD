/***************************************************************************
    Micro C Cube Compact
    LCD Direct Drive
    
    Copyright (c) 2013, eForce Co., Ltd. All rights reserved.
    
    Version Information
        2013.04.08: Created
 ***************************************************************************/

#ifndef DDR_LCD_H
#define DDR_LCD_H

/* Function prototypes */
ER ddr_lcd_ini(void);            /* Initialization */
ER ddr_lcd_adj_blight(INT);      /* Adjust a backlight */
void ddr_lcd_put_img(UH, UH);    /* Draw image */

/* Conversion macros */
#define lcd_ini            ddr_lcd_ini
#define lcd_adj_blight     ddr_lcd_adj_blight
#define lcd_put_img        ddr_lcd_put_img

#endif


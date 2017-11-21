/*********************************************************************
* FileName:        LCD.h
********************************************************************/

#ifndef LCD_H
#define LCD_H

#include "HardwareProfile.h"

#define LCD_OFF	0
#define LCD_ON	1

extern unsigned char line1_buffer[17];
extern unsigned char line2_buffer[17];
extern unsigned char version[17];
extern unsigned char g_lcd;

extern void delay(int count);
extern void LCD_PWM_Init(void);
extern void LCD_Init(void);
extern void lcd_printf(unsigned char line, unsigned char pos, unsigned char *disp);
extern void update_lcd(void);
extern void version_info(void);
extern void LCD_Backlight_Onoff(unsigned char onoff);

#endif


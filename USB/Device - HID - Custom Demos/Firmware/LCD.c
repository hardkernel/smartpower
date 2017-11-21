/*********************************************************************
* FileName:        LCD.c
*********************************************************************/

#include "p18cxxx.h"
#include "HardwareProfile.h"
#include "LCD.h"

unsigned char line1_buffer[17]={"     ODROID     "};
unsigned char line2_buffer[17]={"SMART POWER V3.0"};

unsigned char version[17]={"SMART POWER V3.0"};

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void delay(int count)
{
    int i;
    for(i=0; i<count; i++)    {   i++;    i--;    };
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void LCD_E(void)
{
    delay(50);
    mLCD_E = 1;
    delay(50);
    mLCD_E = 0;
    delay(50);
}

void CommandWrite(unsigned char cmd)
{
    mLCD_DATA(0);
    mLCD_RS = 0;
    mLCD_RW = 0;
    mLCD_DATA(cmd);
    LCD_E();
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void CommandWriteByte(unsigned char cmd)
{
    mLCD_DATA(0);
    mLCD_RS = 0;
    mLCD_RW = 0;
    mLCD_DATA(cmd);
    LCD_E();
    mLCD_DATA((cmd << 4));
    LCD_E();
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void CommandWrite4Bits(unsigned char cmd)
{
    mLCD_DATA(0);
    mLCD_RS = 0;
    mLCD_RW = 0;
    mLCD_DATA(cmd & 0xF0);
    LCD_E();
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void DataWrite(unsigned char dat)
{
    mLCD_DATA(0);
    mLCD_RS = 1;
    mLCD_RW = 0;
    mLCD_DATA(dat);
    LCD_E();
    mLCD_DATA((dat << 4));
    LCD_E();
}

void lcd_printf(unsigned char line, unsigned char pos, unsigned char *disp)
{
    unsigned char   Data, count=0;
    
    if(line)    CommandWriteByte(0xC0+pos);
    else        CommandWriteByte(0x80+pos);

    while(*disp != '\0')    {
    	if(count > 15) break;
        Data = *disp++;
        DataWrite(Data);
        count++;
    }
}

void update_lcd(void)
{
	lcd_printf(0,0, line1_buffer);
	lcd_printf(1,0, line2_buffer);
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void version_info(void)
{
	lcd_printf(0,0, line1_buffer);
	lcd_printf(1,0, version);
}
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void LCD_PWM_Init(void)
{
//	TRISC  |= 0x06; //- RC2 input
	
	//- PWM Period = [PR2 + 1] * 4 * TOSC * TMR2 Prescale Value
	//- 3,750 Hz   = (199 + 1) * 4 * 1/48,000,000 * 16
	//		PR2 = 199;
	PR2 = 12;
	CCP1CON = 0x0C; //- PWM mode
	
	//- Duty Cycle Ratio = CCPR1L:CCP1CON<5:4> / 4 * (PR2 + 1)
	//- 50 %   =        400          * 1/48,000,000 * 16
	CCPR1L = 0x64; //- 50%
	//- CCPR1L = 0x0;
	
	CCP2CON = 0x0C; //- PWM mode
	
	//- Duty Cycle Ratio = CCPR1L:CCP1CON<5:4> / 4 * (PR2 + 1)
	//- 50 %   =        400          * 1/48,000,000 * 16
	CCPR2L = 0x1; //- 50%
	//		CCP2CON |= 0x10; //- PWM mode
	
	T2CON |= 0x06;
	
	TRISC  &= 0xF9; //- RC2 output
	
	VREFCON1 = 0xE0;
	VREFCON2 = 0x10;
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void LCD_Init(void)
{
    mLCD_DATA(0);
    mLCD_RS = 0;
    mLCD_RW = 0;
    mLCD_E = 0;
    delay(2000);    // 4ms
    delay(2000);    // 4ms
    delay(2000);    // 4ms
    delay(2000);    // 4ms
    delay(2000);    // 4ms
    delay(2000);    // 4ms
    CommandWrite4Bits(0x30);
    delay(2000);    // 4ms
    CommandWrite4Bits(0x30);
    delay(1000);    // 4ms
    CommandWrite4Bits(0x30);
    delay(1000);    // 4ms
    CommandWrite4Bits(0x20);
    delay(1000);    // 4ms
    CommandWriteByte(0x28);
    delay(100);    // 4ms
    CommandWriteByte(0x08);
    delay(100);    // 4ms
    CommandWriteByte(0x01);
    delay(100);    // 4ms
    CommandWriteByte(0x02);
    delay(100);    // 4ms
    CommandWriteByte(0x0F);
    delay(100);    // 4ms
}

void LCD_Backlight_Onoff(unsigned char onoff)
{
	if(onoff) {
		TRISAbits.TRISA1=0; 
		LATAbits.LATA1=1;
	}
	else {
		TRISAbits.TRISA1=0; 
		LATAbits.LATA1=0;
	}
	g_lcd = onoff;
}
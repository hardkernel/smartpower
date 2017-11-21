/*********************************************************************
* FileName:        timer.c
*********************************************************************/
#define USE_OR_MASKS
#include <p18cxxx.h>
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "timers.h"
#include "stdio.h"

#include "timer.h"
#include "LCD.h"
#include "i2c_ina231.h"
#include "uart.h"

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
unsigned long count =0;
unsigned char g_state = MAESURE_STOP;
unsigned char g_onoff = OUTPUT_OFF;
unsigned char g_lcd = LCD_ON;
unsigned char update_voltage=0;
unsigned long pre_vol=0;
float voltage=0, current=0, power=0, watt=0;

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
unsigned short read_ADC(void);
float find_voltage(void);

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void timer0_start(void)
{
	//Fosc=16Mhz, internal source = Fosc/4, 
	OpenTimer0(TIMER_INT_ON | T0_16BIT | T0_SOURCE_INT | T0_PS_1_16);
	T0CONbits.T0CS = 0; //Timer 1Clock internal source
	INTCONbits.TMR0IF = 0; //clear interrupt flag
	INTCONbits.GIEH = 1; /* Enable global Interupt */
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void timer0_remove(void)
{
	CloseTimer0(); 
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void toggle_LED(void)
{
	if(TRISCbits.TRISC2) 	TRISCbits.TRISC2 = 0;
	else 					TRISCbits.TRISC2 = 1;
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void maesure_onoff(unsigned char onoff)
{
	if(g_lcd==LCD_OFF) {
		TRISBbits.TRISB2 = 0;
		LATBbits.LATB2 = 0;
		g_state = MAESURE_STOP;
		g_onoff = OUTPUT_OFF;
	}
	// onoff : RB2
	TRISBbits.TRISB2 = 0;
	
	if(onoff)	{
		if(PORTCbits.RC0)  return;
		LATBbits.LATB2 = 1;
	}
	else 		{
		LATBbits.LATB2 = 0;
		g_state = MAESURE_STOP;
	}

	g_onoff = onoff;
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void ftoa(float value, char *string)
{ 
	if (value < 0) { 
		*string++ = '-'; 
		value = -value; 
	}

	if(value > 10)
		sprintf(string,(const far rom char *)"%02lu.%03u",(long)value, (int)((value-(long)value)*1000));
	else 
		sprintf(string,(const far rom char *)"%01lu.%03u",(long)value, (int)((value-(long)value)*1000)); 
}
 
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void maesure_update(void)
{
	char c_vol[10],c_curr[10], c_pow[10], c_watt[10];

	voltage = find_voltage();
	if(update_voltage) {
		set_digital_pot(pre_vol);
		update_voltage=0;
	}
	current = ina231_read_current();
	power = ina231_read_power();
	
	switch(g_state) {
		case MAESURE_STOP:
			watt = 0;
		case MAESURE_PAUSE:
			break;
		case MAESURE_PLAY:
			watt += (power*0.1)/3600;
			break;
		case MAESURE_START:
			watt = 0;
			g_state = MAESURE_PLAY;
			break;
		default: 
			break;
	}

	ftoa(voltage, c_vol);
	ftoa(current, c_curr);
	ftoa(power, c_pow);
	ftoa(watt, c_watt);

	if(g_lcd) {
		if(g_onoff) {
			sprintf((char*)line1_buffer,(const far rom char *)" %sV  %s A",(char*)c_vol,(char*)c_curr);
			
			if((power > 10)&&(watt >= 100))
				sprintf((char*)line2_buffer,(const far rom char *)"%sW%sWh",(char*)c_pow,(char*)c_watt);
			else if((power > 10)&&(watt >= 10))
				sprintf((char*)line2_buffer,(const far rom char *)"%sW %sWh",(char*)c_pow,(char*)c_watt);
			else if((power > 10)&&(watt < 10))
				sprintf((char*)line2_buffer,(const far rom char *)"%sW  %sWh",(char*)c_pow,(char*)c_watt);
			else if((power < 10)&&(watt >= 100))
				sprintf((char*)line2_buffer,(const far rom char *)" %sW%sWh",(char*)c_pow,(char*)c_watt);
			else if((power < 10)&&(watt >= 10))
				sprintf((char*)line2_buffer,(const far rom char *)" %sW %sWh",(char*)c_pow,(char*)c_watt);
			else 
				sprintf((char*)line2_buffer,(const far rom char *)" %sW  %sWh",(char*)c_pow, (char*)c_watt);
		}
		else {
			sprintf((char*)line1_buffer,(const far rom char *)" %sV  -.--- A", (char*)c_vol);
			sprintf((char*)line2_buffer,(const far rom char *)" -.---W  -.---Wh");
		}
	}
	else {
		sprintf((char*)line1_buffer,(const far rom char *)"                ");
		sprintf((char*)line2_buffer,(const far rom char *)"                ");
	}
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void scan_key_input(void)
{
	unsigned int count=0;
	if(PORTCbits.RC0) {
    	if(g_onoff) maesure_onoff(0);
		return;
	}

	if(!PORTDbits.RD3){
		while(!PORTDbits.RD3) {
			delay(2000);
			count++;
			if(count > 300){
				if(g_lcd) 	LCD_Backlight_Onoff(0);
				else 		LCD_Backlight_Onoff(1);
				maesure_onoff(0);
				TRISCbits.TRISC2 = 1;
				maesure_update();
				update_lcd();
				while(!PORTDbits.RD3);
				goto exit;
			}
		}
		if(g_lcd) {
	    	if(g_onoff) maesure_onoff(0);
	    	else 		maesure_onoff(1);
		}
	}

	if(!PORTDbits.RD2){
		while(!PORTDbits.RD2);
		if(g_onoff) {
			if(g_state == MAESURE_PLAY) 	g_state=MAESURE_PAUSE;
			else if((g_state == MAESURE_STOP)||(g_state == MAESURE_PAUSE))	g_state = MAESURE_START;
		}
	}
exit :
	return;
	
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void timer0_ISR(void)
{
	if(count < 35) goto skip;

#if defined(BOARD_USES_BT)
	if(g_bt_init==0) {
		bt_config();
		g_bt_init=1;
	}
#endif

	if((count%2)==0) {
		scan_key_input();
		maesure_update();
		if(g_state == MAESURE_PLAY) line1_buffer[8]='*';
	}
	else {
		if(g_state == MAESURE_PAUSE)
			if((count%5)==0)
				sprintf((char*)&line2_buffer[8],(const far rom char *)"      Wh");
		update_lcd();
	}
skip:
	if(((count%15)==0)&&g_lcd)
		toggle_LED();

	count++;
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
unsigned short read_ADC(void)
{
	unsigned short ret=0;
	unsigned char adc_l=0;
	unsigned char adc_h=0;

	ADCON0bits.GO = 1;        // Start AD conversion
	while(ADCON0bits.GO);     // Wait for conversion
	adc_l = ADRESL;
	adc_h = ADRESH;
	
	ret = ((unsigned short)adc_l|((unsigned short)adc_h<<8));
	
	return ret;
}
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
float find_voltage(void)
{
	unsigned short adc_val =0;
	unsigned short v_word=0;
	float max_out=5.26;
	float min_out=3.00;
	float digit = 0.0;
	float v_float = 0.0;
	
	adc_val = read_ADC();

	digit = (max_out-min_out)/1024;
	v_float = min_out +(adc_val * digit);
	v_float = v_float*100;
	v_word = (unsigned short)v_float;
	
	if(pre_vol != v_word) {
		pre_vol = v_word;
		update_voltage=1;
	}
	return (float)v_word/100;
}

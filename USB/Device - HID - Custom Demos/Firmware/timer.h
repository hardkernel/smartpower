/*********************************************************************
* FileName:        LCD.h
********************************************************************/

#ifndef TIMER_H
#define TIMER_H

#define MAESURE_STOP	0
#define MAESURE_PLAY	1
#define MAESURE_PAUSE	2
#define MAESURE_START	3

#define OUTPUT_OFF	0
#define OUTPUT_ON	1

extern unsigned char g_state;
extern unsigned char g_onoff;
extern unsigned char g_lcd;

extern void timer0_start(void);
extern void timer0_remove(void);
extern void timer0_ISR(void);
extern void maesure_onoff(unsigned char ocoff);
extern void scan_key_input(void);

#endif


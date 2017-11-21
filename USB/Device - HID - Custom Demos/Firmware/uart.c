/*********************************************************************
* FileName:        uart.c
*********************************************************************/
#include "p18cxxx.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "stdio.h"
#include "usart.h"
#include "uart.h"

#define BAUDRATE	115200
#define FOSC		48000000
#define SPBRG_VAL	(((FOSC/BAUDRATE)+2)/4)-1        // BRG16=1, BGH=1 ! 

void uart_init(void)
{
	INTCONbits.PEIE = 0; 
	INTCONbits.GIE = 0; 
	
	IPR1bits.RCIP = 1; //Make receive interrupt high priority */ 
	
	baud1USART(BAUD_IDLE_RX_PIN_STATE_HIGH & 
	          BAUD_IDLE_TX_PIN_STATE_HIGH & 
	          BAUD_16_BIT_RATE & 
	          BAUD_WAKEUP_OFF & 
	          BAUD_AUTO_OFF); 
	
	Open1USART( USART_TX_INT_OFF & USART_RX_INT_ON & 
	          USART_ASYNCH_MODE & USART_EIGHT_BIT & 
	          USART_CONT_RX & USART_BRGH_HIGH & USART_ADDEN_OFF, 
	          SPBRG_VAL );


	// ### setup global interrupt system ### 
	INTCONbits.PEIE = 1; 
	INTCONbits.GIEL = 1;	//   enable low priority/peripheral interrupts 
	INTCONbits.GIEH = 1;	//   enable high priority/global interrupts 
	
	return; 
}

void bt_config(void)
{
	putc1USART('A');
	putc1USART('T');
	return;
}

void uart_put_str(char* tx_buff)
{
	puts1USART(tx_buff);
	return;
}

/**********************************************************************
* FileName:        i2c_ina231.c
**********************************************************************/

#include "p18cxxx.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "stdio.h"

#include "i2c.h"
#include "i2c_ina231.h"
#include "timer.h"

extern float voltage;
extern unsigned char g_onoff;
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
void I2C1_init(void)
{
	TRISDbits.TRISD0 = 0; // input ALERT line  
	LATDbits.LATD0 = 0;
	
	TRISBbits.TRISB0 = 1; // input SDA line  
	TRISBbits.TRISB1 = 1; // input SCK line  
	WPUBbits.WPUB0 = 1;   // Pull-Up
	WPUBbits.WPUB1 = 1;   // Pull-Up
	
	SSP1STATbits.SMP = 1;
	SSP1STATbits.CKE = 0;  
	SSPADD = 0xBF; // 200 Kz for slave
	SSP1CON1 = 0b00101000; // mode master
	SSP1CON2 = 0b00000000;  

	IdleI2C1();
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
void I2C1_deinit(void)
{
	SSP1CON1 = 0b00001000; // SSP1EN  disable
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
char ina231_write(unsigned char pointer_addr, unsigned short value)
{
	signed char ret=0;
	unsigned char wdata[2];

    wdata[0]=(unsigned char)((value >> 8) & 0x00ff);
    wdata[1]=(unsigned char)(value & 0x00ff);
	
	IdleI2C1();                      // ensure module is idle
	StartI2C1();                     // initiate START condition

	ret = WriteI2C1((unsigned char)INA231_I2C_ADDR);
	if(ret<0) return ret;

	ret = WriteI2C1(pointer_addr);
	if(ret<0) return ret;

	ret = WriteI2C1(wdata[0]);
	if(ret<0) return ret;

	ret = WriteI2C1(wdata[1]);
	if(ret<0) return ret;

	StopI2C1();
	
	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
char ina231_read(unsigned char pointer_addr, unsigned char* pbuff)
{
	unsigned char rdata[2] = {0,0};
	signed char ret=0;

	IdleI2C1();                      // ensure module is idle
	StartI2C1();                     // initiate START condition

	ret = WriteI2C1(INA231_I2C_ADDR);
	if(ret<0) return ret;
        
	ret = WriteI2C1(pointer_addr);
	if(ret<0) return ret;

	RestartI2C1();

	ret = WriteI2C1((0x01)|INA231_I2C_ADDR);
	if(ret<0) return ret;

	rdata[1] = ReadI2C1();
	AckI2C1();

	rdata[0] = ReadI2C1();
	NotAckI2C1();

	StopI2C1();
	
	*pbuff = rdata[0];
	*pbuff++;
	*pbuff = rdata[1];

	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
char ina231_configure(void)
{
	signed char ret=0;
	unsigned short config;

	config = 0x45FF;
	ret = ina231_write(INA231_REG_CONFIG, config);

	config = 0x08BD;
	ret = ina231_write(INA231_REG_CALIBRATION, config);

	config = 0x0408;
	ret = ina231_write(INA231_REG_MASK_ENABLE, config);

	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
float ina231_read_voltage(void)
{
	unsigned short bus_vol;
	float LSB_V = 0.00125;

	ina231_read(INA231_REG_BUS_VOL,     (unsigned char*)&bus_vol);
	
	return (float) (bus_vol * LSB_V);
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
float ina231_read_current(void)
{
	unsigned short curr;
	float LSB_I = 0.0001526;

	ina231_read(INA231_REG_CURRENT,     (unsigned char*)&curr);
	
	if(curr<59) curr =0;
	else if (curr>32768) {
    	if(g_onoff) maesure_onoff(0);
		return 0;
	}
	
	return (float) (curr * LSB_I);
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
float ina231_read_power(void)
{
	unsigned short power;
	float LSB_P = 0.00381687;

	ina231_read(INA231_REG_POWER,     (unsigned char*)&power);
	
	if(power<10) power =0;

	return (float) (power * LSB_P);
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
char mcp4652_write(unsigned char addr, unsigned char value)
{
	char ret=0;
	unsigned char cmd_byte=0;

	cmd_byte |= (addr | CMD_WRITE);
	
	IdleI2C1();                      // ensure module is idle
	StartI2C1();                     // initiate START condition

	ret = WriteI2C1((unsigned char)MCP4652_I2C_ADDR);
	if(ret<0) return ret;

	ret = WriteI2C1(cmd_byte);
	if(ret<0) return ret;

	ret = WriteI2C1(value);
	if(ret<0) return ret;

	StopI2C1();
	
	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
char mcp4652_init(void)
{
	char ret=0;
	unsigned char data=0;

	data = 0xBB;
	ret = mcp4652_write(WRITE_TCON, data);
	
	data = 0x80;
	ret = mcp4652_write(WRITE_WIPER0, data);

	data = 0x80;
	ret = mcp4652_write(WRITE_WIPER1, data);

	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
char set_digital_pot(unsigned short Vout)
{
	char ret=0;
	unsigned short Rw=100, Rs=20, Rwb1=0, Rwb2=0, Rpot=0;
	unsigned short Rbase=1211, Rout=0;
	unsigned char N1=255,N2=255;
	unsigned short temp=0;
	float R_float=0;
	float Rwb1_float=0;
	float Rwb2_float=0;
	float Rpot_float=0;
	float Vreal_float=0;
	int i=10;
	
	R_float = 5.910/(voltage-0.591);
	R_float = R_float * 1000;
	Rout = (unsigned short)R_float;
	Rpot = Rout - Rbase;

	Rwb1 = (((525-Vout)/5)*40)+100;
	N1 = (Rwb1-Rw)/Rs;
	
	Rwb1_float = (float)Rwb1/1000;
	Rpot_float = (float)Rpot/1000;

	Rwb2_float = (Rpot_float*Rwb1_float)/(Rwb1_float-Rpot_float);
	Rwb2_float = Rwb2_float *1000;
	Rwb2 = (unsigned short) Rwb2_float;
	N2 = (Rwb2-Rw)/Rs;

	ret = mcp4652_write(WRITE_WIPER0, N1);
	ret = mcp4652_write(WRITE_WIPER1, N2);
	
	return ret;
}
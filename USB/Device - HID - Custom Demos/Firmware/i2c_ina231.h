/**********************************************************************
* FileName:        i2c_ina231.h
**********************************************************************/
#ifndef I2C_INA231_H
#define I2C_INA231_H
#include <p18cxxx.h>

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
#define INA231_I2C_ADDR 		0x80

#define INA231_REG_CONFIG		0x00
#define INA231_REG_SHUNT_VOL	0x01
#define INA231_REG_BUS_VOL		0x02
#define INA231_REG_POWER		0x03
#define INA231_REG_CURRENT		0x04
#define INA231_REG_CALIBRATION	0x05
#define INA231_REG_MASK_ENABLE	0x06
#define INA231_REG_ALERT_LIMIT	0x07

extern void I2C1_init(void);
extern void I2C1_deinit(void);
extern char ina231_write(unsigned char pointer_addr, unsigned short value);
extern char ina231_read(unsigned char pointer_addr, unsigned char* pbuff);
extern char ina231_configure(void);

extern float ina231_read_voltage(void);
extern float ina231_read_current(void);
extern float ina231_read_power(void);


#define MCP4652_I2C_ADDR 		0x58
#define WRITE_WIPER0 	(0b0000<<4)
#define WRITE_WIPER1 	(0b0001<<4)
#define WRITE_TCON		(0b0100<<4)

#define CMD_WRITE		(0b00<<2)
#define CMD_INC			(0b01<<2)
#define CMD_DEC			(0b10<<2)
#define CMD_READ		(0b11<<2)

extern char mcp4652_init(void);
extern char set_digital_pot(unsigned short Vout);

#endif
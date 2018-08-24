/*
 * i2c.h
 *
 *  Created on: Feb 10, 2018
 *      Author: Preshit
 */

#ifndef I2C_H_
#define I2C_H_

//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_i2c.h"
#include "em_gpio.h"
#include "main.h"
#include "heartrate.h"
//#include "sleepmode.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define I2C0_SDA_PORT		gpioPortC
#define I2C0_SDA_PIN		11
#define I2C0_SCL_PORT		gpioPortC
#define I2C0_SCL_PIN		10


#define TEMP_SENSOR_ADDRESS			(0x40)
#define SENSOR_TEMP_MEASURE_CMD		(0xE3)
#define SENSOR_RESET_CMD			(0xFE)

/************************************************************************************/
/************************************************************************************/
#define write_bit 	0
#define read_bit  	1
uint8_t temp1[4],temp2[4],temp3[4];
//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_init();
void i2c_enable();
int i2c_read_burst(I2C_TypeDef *i2c,uint16_t reg, int samples_to_read);
int i2c_write(I2C_TypeDef *i2c, uint16_t address, uint8_t data);
int i2c_read_byte(I2C_TypeDef *i2c, uint16_t reg, uint8_t *data);
void i2c_reset();
//void I2C0_IRQHandler();


#endif /* I2C_H_ */

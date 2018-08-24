/*
 * i2c.c
 *
 *  Created on: Feb 10, 2018
 *      Author: Preshit
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>



//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_init(){
	I2C_Init_TypeDef i2c_init =
	{
			.enable 	= true,
			.master 	= true,
			.refFreq 	= 0,
			.freq       = 400000,
//			.freq		= I2C_FREQ_STANDARD_MAX,
//			.freq 		= I2C_FREQ_FAST_MAX,
//			.freq 		= I2C_FREQ_FASTPLUS_MAX,
			.clhr 		= i2cClockHLRStandard
	};
	I2C_Init(I2C0, &i2c_init);
}

int i2c_write(I2C_TypeDef *i2c, uint16_t reg, uint8_t data){
	i2c->CMD = I2C_CMD_START;						//start command
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|write_bit;	//send slave address

	while((I2C0->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->TXDATA = (reg);							//send register address
	while((i2c->IF & I2C_IF_ACK)==0);  				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;

	i2c->TXDATA = (data);

	while((i2c->IF & I2C_IF_ACK)==0);  				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit
	i2c->CMD = I2C_CMD_STOP;						//stop command

	return 0;
}

int i2c_read_byte(I2C_TypeDef *i2c, uint16_t reg, uint8_t *data){
	//i2c->IFC = I2C_IFC_ACK;							//clear ACK bit
	i2c->CMD = I2C_CMD_START;						//start command
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|write_bit;	//send slave address

	while((I2C0->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->TXDATA = (reg);							//send register address
	while((i2c->IF & I2C_IF_ACK)==0);  				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	i2c->CMD =	I2C_CMD_START;						//Repeated start so send Start command first
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|read_bit;	//send slave address

	while((i2c->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
	(*data) = ((i2c->RXDATA));

	i2c->CMD = I2C_CMD_NACK;						//NACK command
	i2c->CMD = I2C_CMD_STOP;						//stop command

	return 0;
}

int i2c_read_burst(I2C_TypeDef *i2c, uint16_t reg, int samples_to_read){
	int i;
	temp1[4]=0;
	temp2[4]=0;
	temp3[4]=0;

	i2c->CMD = I2C_CMD_START;						//start command
	i2c->TXDATA = (MAX30105_ADDRESS << 1)& ~(read_bit);	//send slave address

	while((I2C0->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->TXDATA = (reg);				//send register address
	while((i2c->IF & I2C_IF_ACK)==0);  				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	i2c->CMD =	I2C_CMD_START;						//Repeated start so send Start command first
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|read_bit;	//send slave address

	while((i2c->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	for(i=0;i<samples_to_read;i++){
        sense.head++; //Advance the head of the storage struct
        sense.head %= 4; //Wrap condition

        temp1[3]=0;

		while((i2c->IF & I2C_IF_RXDATAV)==0);		//wait for data to be recevied in receive buffer
		i2c->CMD = I2C_CMD_ACK;						//ACK command
		(temp1[2]) = ((i2c->RXDATA));


		while((i2c->IF & I2C_IF_RXDATAV)==0);		//wait for data to be recevied in receive buffer
		i2c->CMD = I2C_CMD_ACK;						//ACK command
		(temp1[1]) = ((i2c->RXDATA));


		while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
		i2c->CMD = I2C_CMD_ACK;						//ACK command
		(temp1[0]) = ((i2c->RXDATA));

		memcpy(&tempLong, temp2, sizeof(tempLong));
		tempLong &= 0x3FFFF; //Zero out all but 18 bits
		sense.red[sense.head] = tempLong; //Store this reading into the sense array

		temp2[3]=0;

		while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
		i2c->CMD = I2C_CMD_ACK;						//ACK command
		(temp2[2]) = ((i2c->RXDATA));


		while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
		i2c->CMD = I2C_CMD_ACK;						//ACK command
		(temp2[1]) = ((i2c->RXDATA));


		while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
		i2c->CMD = I2C_CMD_ACK;						//ACK command
		(temp2[0]) = ((i2c->RXDATA));

		memcpy(&tempLong, temp2, sizeof(tempLong));
		tempLong &= 0x3FFFF; //Zero out all but 18 bits
		sense.IR[sense.head] = tempLong; //Store this reading into the sense array
	}

	i2c->CMD = I2C_CMD_NACK;						//NACK command
	i2c->CMD = I2C_CMD_STOP;						//stop command

	return (0);
}


void i2c_enable(){
	GPIO_PinModeSet(I2C0_SCL_PORT,I2C0_SCL_PIN,gpioModeWiredAnd,1);
	GPIO_PinModeSet(I2C0_SDA_PORT,I2C0_SDA_PIN,gpioModeWiredAnd,1);

	i2c_reset();

	//Route I2C pins to I2C0 peripherals
	I2C0->ROUTELOC0 = I2C_ROUTELOC0_SCLLOC_LOC14 | I2C_ROUTELOC0_SDALOC_LOC16;
	I2C0->ROUTEPEN = I2C_ROUTEPEN_SCLPEN | I2C_ROUTEPEN_SDAPEN;

	if(I2C0->STATE & I2C_STATE_BUSY){
		I2C0->CMD = I2C_CMD_ABORT;
	}

	i2c_init();

	I2C0->IFC = _I2C_IFC_MASK;		//clear interrupt flags
}

void i2c_reset(){
	//Reset the I2C bus
	for (int i = 0; i < 9; i++)
	{
		GPIO_PinOutClear(I2C0_SCL_PORT, I2C0_SCL_PIN);
		GPIO_PinOutSet(I2C0_SCL_PORT, I2C0_SCL_PIN);
	}
}

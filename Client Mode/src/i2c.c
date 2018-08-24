
//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"

#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"

//***********************************************************************************
// global variables
//***********************************************************************************
I2C_TypeDef * i2c= (I2C_TypeDef *)I2C0;
//uint8_t amplitude = 0x1F;
#define SENSOR_ADDR 0x40
#define MEASURE_TEMP 0xE3
#define WRITE_BIT 0
#define READ_BIT 1


//***********************************************************************************
// functions
//***********************************************************************************

void i2c_init() {
	//Set up the clock tree for HFPER and I2C
	CMU_ClockEnable(cmuClock_HFPER,true);
	CMU_ClockEnable(cmuClock_I2C0,true);

	//Set up SDA (PD6)and SCL(PD7) pins
	GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeWiredAnd, SDA_default);
	GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeWiredAnd, SCL_default);

	//Toggle SCL 9 times for any I2C slave which requires this to be done
	for (int i=0; i<9; i++) {
		GPIO_PinOutClear(SCL_port,SCL_pin);
		GPIO_PinOutSet(SCL_port,SCL_pin);
	}

	//Route the I2C pins to the I2C peripheral
	I2C0->ROUTELOC0 |= I2C_ROUTELOC0_SCLLOC_LOC14 | I2C_ROUTELOC0_SDALOC_LOC16;
	I2C0->ROUTEPEN |= I2C_ROUTEPEN_SCLPEN | I2C_ROUTEPEN_SDAPEN;

	i2c_init_struct();

	//I2C_Init(I2C0, (const I2C_Init_TypeDef *) I2C_INIT_DEFAULT);

//	I2C_IntClear(i2c, _I2C_IFC_MASK);
//	I2C_IntEnable(i2c, _I2C_IEN_MASK);
//	NVIC_ClearPendingIRQ(I2C0_IRQn);
//	NVIC_EnableIRQ(I2C0_IRQn);

	//Enable the pin which gives power to the Si7021 sensor
	//Commenting the below 2 lines for HeartBeat code
	//GPIO_DriveStrengthSet(SI7021_port, gpioDriveStrengthWeakAlternateWeak);
	//GPIO_PinModeSet(SI7021_port, SI7021_pin, gpioModePushPull, SI7021_default);
}


//Call the I2C Initialization function
void i2c_init_struct(){
    I2C_Init_TypeDef i2c_init =
    {
            .enable     = true,
            .master     = true,
            .refFreq     = 0,
            //.freq		= I2C_FREQ_FASTPLUS_MAX,
			//.freq         = I2C_FREQ_STANDARD_MAX,
			.freq         = I2C_FREQ_FAST_MAX,
            .clhr         = i2cClockHLRStandard
    };

    I2C_Init(I2C0, &i2c_init);
}

//Fetch Temperature from the Si7021 sensor via I2C protocol
int getTemp() {
	//Exit from the busy state
	if (I2C0->STATE & I2C_STATE_BUSY) {
		I2C0->CMD= I2C_CMD_ABORT;
	}

	//Send a STOP condition (for trying a bus recovery)
	//I2C0->CMD= I2C_CMD_STOP;

	int tempVal;
	//Send the start command followed by the Device Address
	I2C0->CMD= I2C_CMD_START;
	I2C0->TXDATA= (SENSOR_ADDR << 1) | WRITE_BIT;

	wait_for_ack();

	//Send the command for measuring temp
	I2C0->TXDATA=MEASURE_TEMP;

	wait_for_ack();

	I2C0->CMD= I2C_CMD_START;
	I2C0->TXDATA=(SENSOR_ADDR <<1) | READ_BIT;

	wait_for_ack();

	//Wait for the Rx data to arrive
	while ((I2C0->IF & I2C_IF_RXDATAV)==0);
	tempVal=(I2C0->RXDATA)<<8;

	//Send an acknowledgement
	I2C0->CMD= I2C_CMD_ACK;

	while ((I2C0->IF & I2C_IF_RXDATAV)==0);
	tempVal|= I2C0->RXDATA;

	//Send a NACK and STOP once both bytes are received
	I2C0->CMD= I2C_CMD_NACK;
	I2C0->CMD= I2C_CMD_STOP;

	return tempVal;
}


void wait_for_ack() {
	while (!(I2C0->IF & I2C_IF_ACK));
	I2C0->IFC= I2C_IF_ACK;
	return;
}

int i2c_particle_sensor_write(I2C_TypeDef *i2c, uint16_t reg, uint8_t data){
	if(I2C0->STATE & I2C_STATE_BUSY){
		I2C0->CMD = I2C_CMD_ABORT;
	}

	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->CMD = I2C_CMD_START;						//start command
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|WRITE_BIT;	//send slave address

	while((I2C0->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->TXDATA = (reg);							//send register address
	while((i2c->IF & I2C_IF_ACK)==0);  				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;

	i2c->TXDATA = (data);

	i2c->CMD = I2C_CMD_NACK;						//NACK command
	i2c->CMD = I2C_CMD_STOP;						//stop command

	return 0;
}

int i2c_particle_sensor_read_byte(I2C_TypeDef *i2c, uint16_t reg, uint8_t *data){
	if(i2c->STATE & I2C_STATE_BUSY){
		i2c->CMD = I2C_CMD_ABORT;
	}

	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->CMD = I2C_CMD_START;						//start command
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|WRITE_BIT;	//send slave address

	while((I2C0->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->TXDATA = (reg);				//send register address
	while((i2c->IF & I2C_IF_ACK)==0);  				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	i2c->CMD =	I2C_CMD_START;						//Repeated start so send Start command first
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|READ_BIT;	//send slave address

	while((i2c->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
	(*data) = ((i2c->RXDATA));

	i2c->CMD = I2C_CMD_NACK;						//NACK command
	i2c->CMD = I2C_CMD_STOP;						//stop command

	return 0;
}

int i2c_particle_sensor_read(I2C_TypeDef *i2c, uint16_t reg, int *data){
	if(i2c->STATE & I2C_STATE_BUSY){
		i2c->CMD = I2C_CMD_ABORT;
	}

	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->CMD = I2C_CMD_START;						//start command
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|WRITE_BIT;	//send slave address

	while((I2C0->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;							//clear ACK bit

	i2c->TXDATA = (reg);				//send register address
	while((i2c->IF & I2C_IF_ACK)==0);  				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	i2c->CMD =	I2C_CMD_START;						//Repeated start so send Start command first
	i2c->TXDATA = (MAX30105_ADDRESS << 1)|READ_BIT;	//send slave address

	while((i2c->IF & I2C_IF_ACK)==0);				//wait for slave to respond
	i2c->IFC = I2C_IFC_ACK;			   				//clear ACK bit

	while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
	(*data) = ((i2c->RXDATA)<<8);

	i2c->CMD = I2C_CMD_ACK;							//send ACK

	while((i2c->IF & I2C_IF_RXDATAV)==0);			//wait for data to be recevied in receive buffer
	(*data) |= (i2c->RXDATA);

	i2c->CMD = I2C_CMD_NACK;						//NACK command
	i2c->CMD = I2C_CMD_STOP;						//stop command

	return 0;
}


int bitMask(uint8_t reg, uint8_t mask, uint8_t thing){
	uint8_t originalContents;
	i2c_particle_sensor_read_byte(I2C0,reg,&originalContents);
	originalContents = (originalContents & mask)|thing;
	i2c_particle_sensor_write(I2C0,reg,originalContents);
	return 0;
}

int i2c_particle_sensor_setup(){
	//Reset all configuration, threshold, and data registers to POR values
	bitMask(MAX30105_MODECONFIG, MAX30105_RESET_MASK, MAX30105_RESET);

	// Sample Average = 4
	bitMask(MAX30105_FIFOCONFIG,MAX30105_SAMPLEAVG_MASK,MAX30105_SAMPLEAVG_4);

	//Enable roll over if FIFO over flows
	bitMask(MAX30105_FIFOCONFIG,MAX30105_ROLLOVER_MASK,MAX30105_ROLLOVER_ENABLE);

	//Mode Configuration
	bitMask(MAX30105_MODECONFIG, MAX30105_MODE_MASK, MAX30105_MODE_MULTILED);

	// adcRange: one of MAX30105_ADCRANGE_2048, _4096, _8192, _16384
	bitMask(MAX30105_PARTICLECONFIG, MAX30105_ADCRANGE_MASK, MAX30105_ADCRANGE_2048);

	// sampleRate: one of MAX30105_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200
	bitMask(MAX30105_PARTICLECONFIG, MAX30105_SAMPLERATE_MASK, MAX30105_SAMPLERATE_50);

	// pulseWidth: one of MAX30105_PULSEWIDTH_69, _188, _215, _411
	bitMask(MAX30105_PARTICLECONFIG, MAX30105_PULSEWIDTH_MASK, MAX30105_PULSEWIDTH_69);

	i2c_particle_sensor_write(I2C0, MAX30105_LED1_PULSEAMP, amplitudeLoaded);
	i2c_particle_sensor_write(I2C0, MAX30105_LED2_PULSEAMP, amplitudeLoaded);
	i2c_particle_sensor_write(I2C0, MAX30105_LED3_PULSEAMP, amplitudeLoaded);
	i2c_particle_sensor_write(I2C0, MAX30105_LED_PROX_AMP, amplitudeLoaded);

	bitMask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT1_MASK, SLOT_RED_LED);
	bitMask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT2_MASK, SLOT_IR_LED << 4);
	bitMask(MAX30105_MULTILEDCONFIG2, MAX30105_SLOT3_MASK, SLOT_GREEN_LED);

	i2c_particle_sensor_write(I2C0, MAX30105_FIFOWRITEPTR, 0);
	i2c_particle_sensor_write(I2C0, MAX30105_FIFOOVERFLOW, 0);
	i2c_particle_sensor_write(I2C0, MAX30105_FIFOREADPTR, 0);

	return 0;
}

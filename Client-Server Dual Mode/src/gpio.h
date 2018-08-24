//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// LED0 pin is
#define	LED0_port		gpioPortF
#define LED0_pin		4
#define LED0_default	false 	// off
// LED1 pin is
#define LED1_port		gpioPortF
#define LED1_pin		5
#define LED1_default	false	// off

//Button PB0
#define BUTTON0_port	gpioPortF
#define BUTTON0_pin		6

//Button PB1
#define BUTTON1_port	gpioPortF
#define BUTTON1_pin		7

#define SENSOR_ENABLE_PORT	gpioPortD
#define SENSOR_ENABLE_PIN	12
//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_init(void);
void gpio_int_enable();
void GPIO_EVEN_IRQHandler(void);

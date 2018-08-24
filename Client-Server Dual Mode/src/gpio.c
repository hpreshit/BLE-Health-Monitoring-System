//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"
#include "native_gecko.h"
/* GATT database */
#include "gatt_db.h"
#include "infrastructure.h"
#include "em_core.h"
#include "main.h"
//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************
void gpio_init(void){

	// Set LED ports to be standard output drive with default off (cleared)
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, LED0_default);

	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);

	GPIO_DriveStrengthSet(SENSOR_ENABLE_PORT, gpioDriveStrengthStrongAlternateStrong);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, LED1_default);
	GPIO_PinModeSet(BUTTON0_port,BUTTON0_pin, gpioModeInputPull, 1);
	GPIO_PinModeSet(BUTTON1_port,BUTTON1_pin, gpioModeInputPull, 1);
	GPIO_PinModeSet(SENSOR_ENABLE_PORT,SENSOR_ENABLE_PIN, gpioModeInputPull, 1);
	GPIO_PinOutClear(SENSOR_ENABLE_PORT,SENSOR_ENABLE_PIN);
}

void gpio_int_enable(){
	  GPIO_ExtIntConfig(BUTTON0_port,BUTTON0_pin,4,false,true,true);
	  GPIO_ExtIntConfig(BUTTON1_port,BUTTON1_pin,6,false,true,true);
	  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

void GPIO_EVEN_IRQHandler(void){
	CORE_AtomicDisableIrq();
	gpioINT=GPIO->IFC;
	EXTERNAL_SIGNAL=BUTTON_SIGNAL;
	gecko_external_signal(0x01);
	CORE_AtomicEnableIrq();
}

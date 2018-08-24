//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"
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
void cmu_init(void){
	//CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_CORELE,true);

	// Peripheral clocks enabled
	CMU_ClockEnable(cmuClock_GPIO, true);
}

void cmu_init_letimer0(void){
	if (letimer_minimumMode==4){
		CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);
		CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_ULFRCO);
	}
	else{
		CMU_OscillatorEnable(cmuOsc_LFXO,true,true);
		CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);
	}

	CMU_ClockEnable(cmuClock_CORELE,true);
	// Peripheral clocks enabled
	CMU_ClockEnable(cmuClock_LETIMER0,true);
	//CMU_ClockEnable(cmuClock_GPIO, true);
}

void cmu_init_i2c(void){
	CMU_ClockEnable(cmuClock_HFPER, true);

	// Peripheral clocks enabled
	CMU_ClockEnable(cmuClock_I2C0, true);
	CMU_ClockEnable(cmuClock_GPIO, true);
}

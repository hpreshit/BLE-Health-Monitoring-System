//***********************************************************************************
// Include files
//***********************************************************************************
#include "letimer.h"
#include "native_gecko.h"
#include "graphics.h"


//***********************************************************************************
// global variables
//***********************************************************************************

#define LETIMER_MAX_CNT 65535

LETIMER_TypeDef * timer= (LETIMER_TypeDef *)LETIMER0;
int comp0, comp1, freq, prescalar=1;
int intflags;
char buffer[7];
char line[40];


//***********************************************************************************
// functions
//***********************************************************************************

//Calculate the prescalar based on the required duty cycle
void calc_prescalar(){
	while( ((LETIMER_MAX_CNT * prescalar)/freq) < (TOTAL_CYCLE/1000)){
		prescalar= prescalar<<1;
	}
}


void letimer_init(void){
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);

	//Enable the LFA clock tree to CORELE and LETIMER0
	CMU_ClockEnable(cmuClock_CORELE, true);
	CMU_ClockEnable(cmuClock_LETIMER0, true);

	freq=CMU_ClockFreqGet(cmuClock_LETIMER0);

	LETIMER_Init_TypeDef Init = LETIMER_INIT_DEFAULT;
	Init.enable=false;	//LETIMER0 currently disabled
	Init.comp0Top=true;	//Set the top of the counter to Comp0
	LETIMER_Init(timer, (const LETIMER_Init_TypeDef *)&Init);

	calc_prescalar();

	//Comp0 corresponds to count for the complete duty cycle and Comp1 corresponds to On-duty cycle
	comp0= (TOTAL_CYCLE*freq)/ (prescalar*1000);
	comp1= comp0- ((STABILIZATION_TIME*comp0)/TOTAL_CYCLE);
	LETIMER_CompareSet(timer, 0, comp0);
	LETIMER_CompareSet(timer, 1, comp1);
	CMU_ClockDivSet(cmuClock_LETIMER0,prescalar);
//	CMU->LFAPRESC0= prescalar;

	//Clear any pending interrupts and enable the desired interrupts
	LETIMER_IntClear(timer,LETIMER_IFC_COMP0 | LETIMER_IFC_COMP1 | LETIMER_IFC_UF | LETIMER_IFC_REP0 | LETIMER_IFC_REP1);
	LETIMER_IntEnable(timer, LETIMER_IEN_UF | LETIMER_IEN_COMP1);
	NVIC_EnableIRQ(LETIMER0_IRQn);

	LETIMER_Enable(timer,true);
}


//Function delegated to service the letimer interrupt
void service_letimer(void) {
	int tempVal=0;
	float celsius;
	uint32_t tempSend;
	uint8_t tempBuf[5];
	uint8_t *ptr = tempBuf;
	uint8_t flags = 0x00;   /* HTM flags set as 0 for Celsius, no time stamp and no temperature type. */

	//On completion of 4 seconds
	if (intflags & LETIMER_IF_UF){
//		blockSleepMode(EM2);
		//Set the sensor-enable pin, and enable the SDA and SCL pins
//		GPIO_PinOutSet(SI7021_port,SI7021_pin);
		GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeWiredAnd, SDA_default);
		GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeWiredAnd, SCL_default);
//		unBlockSleepMode(EM2);
//		SI7021_init();

		//GPIO_PinOutToggle(LED0_port, LED0_pin);

		flashLoad= gecko_cmd_flash_ps_load((uint16_t) PS_KEY_START+1);
		thresholdLoaded=*(flashLoad->value.data);
		thresholdSaved= thresholdLoaded+1;
		gecko_cmd_flash_ps_save((uint16_t) PS_KEY_START+1, 1, ( uint8 *)&thresholdSaved);

		itoa(thresholdLoaded,buffer,10);
		GRAPHICS_Clear();
		sprintf(line,"Heart rate \nthreshold:\n%s", buffer );
		GRAPHICS_AppendString(line);
		GRAPHICS_Update();

		flashLoad= gecko_cmd_flash_ps_load((uint16_t) PS_KEY_START+2);
		amplitudeLoaded=*(flashLoad->value.data);
		amplitudeSaved= amplitudeLoaded+1;
		gecko_cmd_flash_ps_save((uint16_t) PS_KEY_START+2, 1, ( uint8 *)&amplitudeSaved);

		itoa(amplitudeLoaded,buffer,16);
		sprintf(line,"\n\n\n\nIR Amplitude: \n0x%s", buffer );
		GRAPHICS_AppendString(line);
		GRAPHICS_Update();

	}
	//On completion of 80 ms stabilization period
	else if(intflags & LETIMER_IF_COMP1) {
//		blockSleepMode(EM2);
		//Call the function to read the temperature via I2C
		celsius= (175.72*tempVal)/65536 - 46.85;
		//Turn ON LED1 if temperature is below threshold
		if (celsius<thresholdLoaded){
			//GPIO_PinOutSet(LED1_port, LED1_pin);
		}
		else {
			//GPIO_PinOutClear(LED1_port, LED1_pin);
		}
		//Turn off the sensor-enable pin, and disable the SDA and SCL pins
		GPIO_PinOutClear(SI7021_port,SI7021_pin);
		GPIO_PinModeSet(SDA_port, SDA_pin, gpioModeDisabled, 0);
		GPIO_PinModeSet(SCL_port, SCL_pin, gpioModeDisabled, 0);
//		unBlockSleepMode(EM2);
	}
}


void LETIMER0_IRQHandler(void){
	CORE_ATOMIC_IRQ_DISABLE();
	intflags= LETIMER_IntGet(timer);
	LETIMER_IntClear(timer, LETIMER_IFC_UF | LETIMER_IFC_COMP1);
//	schedule_event|=event_letimer;
	CORE_ATOMIC_IRQ_ENABLE();
	gecko_external_signal(event_letimer);
}

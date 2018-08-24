/*
 * letimer.c
 *
 *  Created on: Jan 27, 2018
 *      Author: Preshit
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "letimer.h"
#include "em_letimer.h"
#include "main.h"
#include "native_gecko.h"
/* GATT database */
#include "gatt_db.h"
#include "infrastructure.h"
#include "i2c.h"
//***********************************************************************************
// global variables
//***********************************************************************************
uint16_t prescalar = 1;
uint32_t clockFreq = 0;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void letimer_init(){
	LETIMER_Enable(LETIMER0,false);								//Disable LETIMER0

	const LETIMER_Init_TypeDef letimer0Init =
	{
	.enable = false,
	.debugRun = false,
	.comp0Top = true,
	.bufTop = false,
	.out0Pol = 0,
	.out1Pol = 0,
	.ufoa0 = letimerUFOANone,
	.ufoa1 = letimerUFOANone,
	.repMode = letimerRepeatFree
	};

	LETIMER_Init(LETIMER0,&letimer0Init);						//Initialize LETIMER0
}

void letimer_set_prescalar(){
	uint16_t countermax = 0xFFFF;

	clockFreq = CMU_ClockFreqGet(cmuClock_LETIMER0);			//Get Clock frequency for LETIMER0
	while(LETIMER0_Period>(countermax*prescalar/clockFreq)){	//calculate prescalar
		prescalar<<=1;
	}
	//CMU->LFAPRESC0 = prescalar;
	CMU_ClockDivSet(cmuClock_LETIMER0, prescalar);				//Set prescalar
}

void letimer_set_compvalue(){
	uint32_t ondutycount = 0;
	uint32_t periodcount = 0;

	periodcount =((LETIMER0_Period*clockFreq)/(prescalar));			//calculate count for period
	ondutycount =((LETIMER0_On_Time*clockFreq)/(prescalar*1000));	//calculate count for on-duty cycle

	LETIMER_CompareSet(LETIMER0,0,periodcount);						//Set COMP0 register value
	LETIMER_CompareSet(LETIMER0,1,(periodcount-ondutycount)); 		//Set COMP1 register value
}

void letimer_enable(){
	//Clear Interrupt flags
	LETIMER0->IFC |= LETIMER_IFC_REP1|LETIMER_IFC_REP1|LETIMER_IFC_UF|LETIMER_IFC_COMP1|LETIMER_IFC_COMP0;

	//Enable COMP0 and COMP1 interrupts
	LETIMER0->IEN = LETIMER_IEN_COMP1|LETIMER_IEN_UF;

	NVIC_EnableIRQ(LETIMER0_IRQn);	 	//Enable LETIMER0 Interrupt

	LETIMER_Enable(LETIMER0,true);	    //Enable LETIMER0
}

void LETIMER0_IRQHandler(void){
	CORE_AtomicDisableIrq();
	flagInterrupt = LETIMER_IntGet(LETIMER0);
	LETIMER_IntClear(LETIMER0,LETIMER_IF_COMP1);	//clear comp1 interrupt flag
	LETIMER_IntClear(LETIMER0,LETIMER_IF_UF);		//clear underflow interrupt flag
	EXTERNAL_SIGNAL=LETIMER_SIGNAL;
	CORE_AtomicEnableIrq();
	if(SENSOR_ENABLE==1){
		gecko_external_signal(0x01);
	}
}

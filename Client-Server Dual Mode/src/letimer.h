/*
 * letimer.h
 *
 *  Created on: Jan 27, 2018
 *      Author: Preshit
 */


//***********************************************************************************
// Include files
//***********************************************************************************

#include "em_cmu.h"
#include "em_core.h"
#include "gpio.h"
//#include "sleepmode.h"
#include "i2c.h"
#include "sleep.h"

//***********************************************************************************
// function prototypes
//***********************************************************************************
void letimer_init();
void letimer_set_prescalar();
void letimer_set_compvalue();
void letimer_enable();
void LETIMER0_IRQHandler(void);

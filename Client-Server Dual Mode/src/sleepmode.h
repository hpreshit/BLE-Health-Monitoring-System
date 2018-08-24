/*
 * sleep.h
 *
 *  Created on: Jan 31, 2018
 *      Author: Preshit
 */

#ifndef SLEEPMODE_H_
#define SLEEPMODE_H_

//***********************************************************************************
// Include files
//***********************************************************************************
#include "main.h"
#include "em_core.h"

//***********************************************************************************
// enumeration
//***********************************************************************************
typedef enum{
	Em0=0,
	Em1,
	Em2,
	Em3,
	Em4
}sleepstate_enum;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void sleep(void);
void blocksleepmode(sleepstate_enum minmode);
void unblocksleepmode(sleepstate_enum minmode);

#endif /* SLEEPMODE_H_ */

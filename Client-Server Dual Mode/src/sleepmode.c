/***************************************************************************//**
 * @file sleepmode.c
 * @brief blocksleepmode and unblocksleepmode functions.
 * @version 1.0
 *******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleepmode.h"
#include "em_emu.h"

//***********************************************************************************
// global variables
//***********************************************************************************
int sleep_block_counter[5]={0};

//***********************************************************************************
// function prototypes
//***********************************************************************************
void blocksleepmode(sleepstate_enum minmode){
	CORE_AtomicDisableIrq();
	sleep_block_counter[minmode]++;
	CORE_AtomicEnableIrq();
}

void unblocksleepmode(sleepstate_enum minmode){
	CORE_AtomicDisableIrq();
	if(sleep_block_counter[minmode]>0){
		sleep_block_counter[minmode]--;
	}
	CORE_AtomicEnableIrq();
}

void sleep(void){
	if(sleep_block_counter[0]>0){
		return;
	}else if(sleep_block_counter[1]>0){
		return;
	}else if(sleep_block_counter[2]>0){
		EMU_EnterEM1();
	}else if(sleep_block_counter[3]>0){
		EMU_EnterEM2(true);
	}else{
		EMU_EnterEM3(true);
	}
}

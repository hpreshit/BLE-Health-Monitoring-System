#ifndef MAIN_H_
#define MAIN_H_

//***********************************************************************************
// Include files
//***********************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "infrastructure.h"


//***********************************************************************************
// defined files
//***********************************************************************************
//Values in milliseconds for LETIMER
#define STABILIZATION_TIME 80
#define TOTAL_CYCLE 4000

#define event_letimer 0x01

//Threshold for the temperature below which LED will turn on
#define THRESHOLD 40

//Threshold for the heart rate above which the alert notification would be sent
uint8_t thresholdLoaded;
uint8_t thresholdSaved;

//Amplitude which needs to be configured for the IR sensor
uint8_t amplitudeLoaded;
uint8_t amplitudeSaved;

//Persistent Storage Keys
#define PS_KEY_START 0x4000
#define PS_KEY_END 0x407F

//Values associated to BLE connection
#define CONNECTION_ID 0x01
#define SEND_TO_ALL_CONNECTIONS 0xFF
#define DATA_LEN 5

//Minimum and Maximum transmit power for BGM121
#define BGM121_TX_MIN -260
#define BGM121_TX_MAX 80

//***********************************************************************************
// global variables
//***********************************************************************************
int schedule_event;
struct gecko_msg_flash_ps_load_rsp_t *flashLoad;

typedef enum{
	EM0,
	EM1,
	EM2,
	EM3,
	EM4
} em;

int sleep_block_counter[EM4];


/* Event pointer for handling events */
struct gecko_cmd_packet* evt;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void sleep(void);

void blockSleepMode(em);

void unBlockSleepMode(em);

#endif /*MAIN_H_ */

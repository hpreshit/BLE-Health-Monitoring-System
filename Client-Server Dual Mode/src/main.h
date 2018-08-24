//***********************************************************************************
// Include files
//***********************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

//***********************************************************************************
// defined files
//***********************************************************************************

#define LETIMER0_Period 		2			//LETIMER0 Period in seconds
#define LETIMER0_On_Time 		200		//LETIMER0 On Time in milliseconds
#define letimer_minimumMode		4			//Mode Limit
#define i2c_minimumMode			2			//Mode Limit

#define PS_KEY					(0x4001)

#define BLE_ADVERTISING_MIN_MS		(337)
#define BLE_ADVERTISING_MAX_MS		(337)
#define BLE_ADVERTISING_MIN_COUNT	(539)	//count = BLE_ADVERTISING_MIN_MS * 1.6
#define BLE_ADVERTISING_MAX_COUNT	(539)	//count = BLE_ADVERTISING_MAX_MS * 1.6
#define BLE_CONNECTION_MIN_MS		(75)
#define BLE_CONNECTION_MAX_MS		(75)
#define BLE_CONNECTION_MIN_COUNT	(60)	//Count = BLE_CONNECTION_MIN_MS/1.25
#define BLE_CONNECTION_MAX_COUNT	(60)	//Count = BLE_CONNECTION_MAX_MS/1.25
#define BLE_SLAVE_LATENCY_MS		(450)
#define BLE_SLAVE_LATENCY			(5)		//Latency = (BLE_SLAVE_LATENCY_MS/BLE_CONNECTION_MAX_MS)-1
#define BLE_CONNECTION_TIMEOUT_MS	(60)
#define BLE_CONNECTION_TIMEOUT_COUNT (900)
#define BLE_TX_MAX					(80)
#define BLE_TX_MIN					(-260)


#define LETIMER_SIGNAL			1
#define BUTTON_SIGNAL			2
#define HEARTRATE_DEFAULT		72
#define SPO2_DEFAULT			99
//***********************************************************************************
// global variables
//***********************************************************************************
int flagInterrupt;
uint32_t gpioINT;
uint8_t EXTERNAL_SIGNAL;
uint8_t CONNECTED;
volatile uint8_t SENSOR_ENABLE;

//***********************************************************************************
//Heart Rate and SpO2 calculation Variables
//***********************************************************************************
int32_t heartRateAvg=HEARTRATE_DEFAULT;
int32_t spo2Avg=SPO2_DEFAULT;
int32_t abnormal_heartrate;
int32_t abnormal_spo2;
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
uint8_t spo2_threshold1;
uint8_t heartrate_threshold1;

//***********************************************************************************
// SERVER variables
//***********************************************************************************
uint8_t conn_handle_server1;
uint8_t conn_handle_server2;
uint8_t activeConnectionId;

//***********************************************************************************
// CLIENT variables
//***********************************************************************************
#define UUIDLEN_ALERT 16
#define UUIDLEN_FINDME 16

//***********************************************************************************
//Gatt notification variables
//***********************************************************************************
char data[15];
uint8_t serviceuuid[16];
uint8_t characuuid[16];
uint8_t uuidlen;
uint8_t conn_handle;
volatile uint32_t service;
volatile uint16_t characteristic;


typedef enum {
	IDLE =0,
	SERVICE,
	CHARACTERISTIC,
	NOTIFICATION
} state;
state current_state= IDLE;

typedef enum{
	HEARTRATE=0,
	SPO2,
	BUTTON,
	NONE
} var;
var abnormality;

/* Event pointer for handling events */
struct gecko_cmd_packet* evt;
struct gecko_msg_flash_ps_load_rsp_t *flashLoad;
//***********************************************************************************
// function prototypes
//***********************************************************************************
void letimer_service();
void button_service();

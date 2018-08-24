/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko C application
 * that allows Over-the-Air Device Firmware Upgrading (OTA DFU). The application
 * starts advertising after boot and restarts advertising after a connection is closed.
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include "graphics.h"
#include "infrastructure.h"
/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_letimer.h"
//#include "ustimer.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

//***********************************************************************************
// Include files
//***********************************************************************************
#include <stdlib.h>
#include "main.h"
#include "gpio.h"
#include "cmu.h"
#include "letimer.h"
#include "i2c.h"
#include "sleep.h"
#include "heartrate.h"

//***********************************************************************************
// global variables
//***********************************************************************************
int16_t rssi;
uint8_t result;
uint8_t value = 15;
char buffer[25];
uint8_t i;

//**********************************************************************************
//Particle Sensor Variables
//**********************************************************************************
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
uint16_t redAvg=0;
uint16_t irAvg=0;
uint32_t redAvgNew=0;
uint32_t irAvgNew=0;

//***********************************************************************************
//Heart Rate and SpO2 calculation Variables
//***********************************************************************************
uint8_t HRcount=0;
uint8_t SPO2count=0;
uint8_t count=0;
uint8_t heartrate_saved=90;
uint8_t spo2_saved=80;
uint8_t heartrate_threshold=90;
uint8_t spo2_threshold=80;

//***********************************************************************************
//Gatt notification variables
//***********************************************************************************
uint8_t alert1[15]="ABNORMAL HR";
uint8_t alert2[15]="ABNORMAL SPO2";
uint8_t alert3[15]="\nFINDME ALERT";
uint8_t flags1 = 0x01;
uint8_t immediate_alert;
uint8 alert_flag=1;
uint8 findme_flag=1;

uint8_t serviceuuid_alert[UUIDLEN_ALERT]= {0x98,0xbe,0x8f,0xc7,0x33,\
		0x49,0x48,0x8a,0x01,0x4e,0x14,0xc7,0xdb,0x24,0x78,0x06};
uint8_t characuuid_alert[UUIDLEN_ALERT] = {0xcf,0xd2,0x36,0x82,0x3f,\
		0x00,0xde,0x9f,0xea,0x49,0x92,0x45,0xf0,0xf7,0x05,0xac};
uint8_t serviceuuid_findme[UUIDLEN_FINDME]={0x1c,0x4f,0x76,0xb2,0xec,\
		0x1d,0xd1,0x99,0x7f,0x40,0xd5,0xc3,0xeb,0x32,0x82,0xae};   //ae8232eb-c3d5-407f-99d1-1decb2764f1c
uint8_t characuuid_findme[UUIDLEN_FINDME]={0xfc,0xe8,0xd3,0x96,0x0c,\
		0xa8,0x39,0xaa,0x5c,0x44,0x5e,0x7e,0x85,0xbe,0xbf,0x0b};	//0bbfbe85-7e5e-445c-aa39-a80c96d3e8fc

//uint8 peer_addr[]= {0x81, 0x88, 0x15, 0x57, 0x0B, 0x00};	//MAC ADDR of GECKO 2

uint8 peer_addr[]= {0x8B, 0x38, 0xA9, 0x57, 0x0B, 0x00};	//MAC ADDR of GECKO 1

uint8 scanned_addr[6] ={0};

uint8 central_addr[]= {0x66, 0x88, 0x15, 0x57, 0x0B, 0x00};	//MAC ADDR of CENTRALIZED GECKO 3

uint8_t STOP_SCANNING=0;

uint8_t opened_count=0;
volatile uint8_t closed_count=0;

uint8_t adv_data[2]={1,2};


//***********************************************************************************
// functions
//***********************************************************************************
void sensor_init(){
	//GPIO_PinOutSet(SENSOR_ENABLE_PORT,SENSOR_ENABLE_PIN);	//Enable Sensor
	GRAPHICS_Clear();
	GRAPHICS_AppendString("\n\nSENSOR \nINITIALIZED?");
	GRAPHICS_Update();
	//Configure Sensor
	sensor_setup();

	//Take 100 samples from sensor
	for (uint8_t i = 0 ; i < 100 ; i++){
		count=0;
		while ((available() == 0) && (count<5)){
		sensebeat();
		count++;
		}
		redBuffer[i] = sense.red[sense.head];
		irBuffer[i] = sense.IR[sense.head];
		nextSample();
	}

	GRAPHICS_AppendString("\n\n\n\nDONE!");
	GRAPHICS_Update();
	abnormality=NONE;
	SENSOR_ENABLE=1;
}

void letimer_service(){
	if(flagInterrupt & LETIMER_IF_UF){
		  uint16_t temp1;
		  uint8_t temp2;
		  uint8_t htmBuffer1[8];
		  uint8_t *p1 = htmBuffer1;

		  uint8_t htmBuffer2[8];
		  uint8_t *p2 = htmBuffer2;

		  uint8_t htmBuffer3[8];
		  uint8_t *p3 = htmBuffer3;

		  if(CONNECTED){
			  if(abnormality==HEARTRATE){
				  GRAPHICS_AppendString("\n\n\n\nABNORMAL\nHEARTRATE");
			  	  GRAPHICS_Update();
			  	  gecko_cmd_gatt_server_send_characteristic_notification(conn_handle_server1, \
						  gattdb_alert_message, 15, alert1);
			  }
			  if(abnormality==SPO2){
				  GRAPHICS_AppendString("\n\n\n\nABNORMAL\nSPO2");
				  GRAPHICS_Update();
			  	  gecko_cmd_gatt_server_send_characteristic_notification(conn_handle_server1, \
						  gattdb_alert_message, 15, alert2);
			  }
			  if(abnormality==BUTTON){
			  	  gecko_cmd_gatt_server_send_characteristic_notification(conn_handle_server1, \
						  gattdb_alert_message, 15, alert3);

			  	  temp2=1;
				  UINT8_TO_BITSTREAM(p3, temp2);
			  	  gecko_cmd_gatt_server_send_characteristic_notification(conn_handle_server1, \
						  gattdb_immediate_alert, 1, htmBuffer3);
			  }
			  if(abnormality==HEARTRATE || abnormality==SPO2){
				  UINT8_TO_BITSTREAM(p1, flags1);
				  temp1=(uint16_t)abnormal_heartrate;
				  UINT16_TO_BITSTREAM(p1, temp1);
				  gecko_cmd_gatt_server_send_characteristic_notification(conn_handle_server2, \
						  gattdb_heart_rate_measurement, 2, htmBuffer1);

				  temp2=(uint8_t)abnormal_spo2;
				  UINT8_TO_BITSTREAM(p2, temp2);
				  gecko_cmd_gatt_server_send_characteristic_notification(conn_handle_server2, \
						  gattdb_spo2_measurement, 1, htmBuffer2);
			  }
		  }

		  //Delete first 25 samples and shift the remaining 75 samples
		  for (uint8_t i = 25; i < 100; i++){
			  redBuffer[i - 25] = redBuffer[i];
			  irBuffer[i - 25] = irBuffer[i];
		  }

		  //take 25 new sets of samples before calculating the heart rate.
		  for (uint8_t i = 75; i < 100; i++){
			  count=0;
			  while ((available() == 0) && (count<5)){
			  sensebeat();
			  count++;
			  }
			  redBuffer[i] = sense.red[sense.head];
			  irBuffer[i] = sense.IR[sense.head];
			  nextSample();
		  }

		  //GPIO_PinOutClear(SENSOR_ENABLE_PORT,SENSOR_ENABLE_PIN);	//Load Power Management OFF

		  //After gathering 25 new samples recalculate HR and SP02
		  maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, \
				  &validSPO2, &heartRate, &validHeartRate);

		  for (uint8_t i=0;i<100;i++){
			  redAvg=((uint16_t)redBuffer[i]+(i*redAvg))/(i+1);
			  irAvg=((uint16_t)irBuffer[i]+(i*irAvg))/(i+1);
		  }

		  //Take heart rate and SpO2 average for avery new calculated value
		  if((validHeartRate==1) && (heartRate<160) && (heartRate>25)){
			  heartRateAvg=(heartRate+(HRcount*heartRateAvg))/(HRcount+1);
			  HRcount++;
		  }
		  if((validSPO2==1) && (spo2>65)){
			  spo2Avg=spo2;
		  }

		  //Check if Finger is placed on the sensor
		  if(redAvg<10000){
			  GRAPHICS_Clear();
			  GRAPHICS_AppendString("\n\nFINGER \nREMOVED");
			  GRAPHICS_Update();
			  heartRateAvg=HEARTRATE_DEFAULT;
		  }
		  else{
			  GRAPHICS_Clear();
			  snprintf(buffer,25,"\n\nHR=%d",heartRateAvg);
			  GRAPHICS_AppendString(buffer);
			  snprintf(buffer,25,"\n\n\n\nSPO2=%d",spo2Avg);
			  GRAPHICS_AppendString(buffer);
			  GRAPHICS_Update();

			  if(heartRateAvg>heartrate_threshold){
		  		  /* Set advertising parameters. 100ms advertisement interval. All channels used.
		  		   * The first two parameters are minimum and maximum advertising interval, both in
		  		   * units of (milliseconds * 1.6). The third parameter '7' sets advertising on all channels. */
		  		  gecko_cmd_le_gap_set_adv_parameters(BLE_ADVERTISING_MIN_COUNT,BLE_ADVERTISING_MAX_COUNT,7);	//Server
//		  		  gecko_cmd_le_gap_set_adv_data(0,1,(const uint8 *)&adv_data[1]);
		  		  /* Start general advertising and enable connections. */
		  		  gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);	//Server
		  		  abnormal_heartrate=heartRateAvg;
		  		  abnormal_spo2=spo2Avg;
		  		  heartRateAvg=HEARTRATE_DEFAULT;
		  		  spo2Avg=SPO2_DEFAULT;
		  		  abnormality=HEARTRATE;
			  }

			  else if(spo2Avg<spo2_threshold){
		  		  /* Set advertising parameters. 100ms advertisement interval. All channels used.
		  		   * The first two parameters are minimum and maximum advertising interval, both in
		  		   * units of (milliseconds * 1.6). The third parameter '7' sets advertising on all channels. */
		  		  gecko_cmd_le_gap_set_adv_parameters(BLE_ADVERTISING_MIN_COUNT,BLE_ADVERTISING_MAX_COUNT,7);	//Server
//		  		  gecko_cmd_le_gap_set_adv_data(0,1,(const uint8 *)&adv_data[1]);
		  		  /* Start general advertising and enable connections. */
		  		  gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);	//Server
		  		  abnormal_spo2=spo2Avg;
		  		  abnormal_heartrate=heartRateAvg;
		  		  heartRateAvg=HEARTRATE_DEFAULT;
		  		  spo2Avg=SPO2_DEFAULT;
		  		  abnormality=SPO2;
			  }
		  }
		  sensor_shutdown();
	      GPIO_PinModeSet(I2C0_SCL_PORT,I2C0_SCL_PIN,gpioModeDisabled,0);
		  GPIO_PinModeSet(I2C0_SDA_PORT,I2C0_SDA_PIN,gpioModeDisabled,0);
	}

	if(flagInterrupt & LETIMER_IF_COMP1){
		GPIO_PinModeSet(I2C0_SCL_PORT,I2C0_SCL_PIN,gpioModeWiredAnd,1);
		GPIO_PinModeSet(I2C0_SDA_PORT,I2C0_SDA_PIN,gpioModeWiredAnd,1);
	    sensor_wakeup();
	}
}

void button_service(){
	if(gpioINT==0x0010){
		gecko_cmd_le_gap_set_adv_parameters(BLE_ADVERTISING_MIN_COUNT,BLE_ADVERTISING_MAX_COUNT,7);	//Server
		gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);	//Server
		abnormality=BUTTON;
	}
	else{
		if(CONNECTED==0){
			if(SENSOR_ENABLE==1){
				sensor_shutdown();
				SENSOR_ENABLE=0;
			}
			else if(SENSOR_ENABLE==0){
				sensor_wakeup();
				SENSOR_ENABLE=1;
			}
		}
	}
}
//***********************************************************************************
// main
//***********************************************************************************

/**
 * @brief  Main function
 */
int main(void)
{
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  //USTIMER_Init();
  /* Enable atomic read-clear operation on reading IFC register */
  MSC->CTRL |= MSC_CTRL_IFCREADCLEAR;
  // Initialize GPIO
  gpio_init();
  //GPIO Interrupt ENABLE
  gpio_int_enable();
  // Initialize clocks for letimer0
  cmu_init_letimer0();
  // Initialize clocks for i2c
  cmu_init_i2c();
  // Enable I2C
  i2c_enable();
  // Initialize LCD
  GRAPHICS_Init();
  // Initialize stack
  gecko_init(&config);

  GRAPHICS_Clear();
  GRAPHICS_AppendString("\n\nPEER NODE");
  GRAPHICS_Update();

  while (1) {
	  /* Check for stack event. */
	  evt = gecko_wait_event();

      /* Handle events */
	  switch (BGLIB_MSG_ID(evt->header)) {

/*****************************************************************************************************/
//DUAL-MODE
/*****************************************************************************************************/
	  	  /* This boot event is generated when the system boots up after reset.
	       * Do not call any stack commands before receiving the boot event.
	       * Here the system is set to start advertising immediately after boot procedure. */
	  	  case gecko_evt_system_boot_id:
	  		  /********************MITM*******************************/
	          /* delete all bondings to force the pairing process */
	          gecko_cmd_sm_delete_bondings();
	          gecko_cmd_sm_configure(0x05, sm_io_capability_displayyesno); /* Numeric comparison */
	          /* enable bondable to accommodate certain mobile OS */
	          gecko_cmd_sm_set_bondable_mode(1);
	          //gecko_cmd_sm_list_all_bondings();
	  		  /*******************************************************/

	          /***************Persistent Memory***********************/
//	          gecko_cmd_flash_ps_erase((uint16_t) PS_KEY+1);
//	          gecko_cmd_flash_ps_erase((uint16_t) PS_KEY+2);

	          flashLoad= gecko_cmd_flash_ps_load((uint16_t) PS_KEY+1);
	          if(flashLoad->result==0){
				  heartrate_threshold=*(flashLoad->value.data);
			  }
			  else {
				  gecko_cmd_flash_ps_save((uint16_t) PS_KEY+1, 1, (const uint8 *)&heartrate_saved);
			  }

	          flashLoad= gecko_cmd_flash_ps_load((uint16_t) PS_KEY+2);
	          if(flashLoad->result==0){
				  spo2_threshold=*(flashLoad->value.data);
			  }
			  else {
				  gecko_cmd_flash_ps_save((uint16_t) PS_KEY+2, 1, (const uint8 *)&spo2_saved);
			  }
	          /*******************************************************/
	  		  gecko_cmd_le_gap_discover(le_gap_discover_observation);  	//Client
	  		  gecko_cmd_le_gap_set_scan_parameters(160, 32, 0);
	  		  break;

	  	  case gecko_evt_le_connection_opened_id:
	  		  /***********************SERVER***********************************/
	  		  if(evt->data.evt_le_connection_opened.master == 0){
	  			  if(memcmp(peer_addr,evt->data.evt_le_connection_opened.address.addr,6)==0){
	  				  conn_handle_server1=evt->data.evt_le_connection_opened.connection;
	  				  CONNECTED=1;
	  			  }
	  			  if(memcmp(central_addr,evt->data.evt_le_connection_opened.address.addr,6)==0){
	  				  CONNECTED=1;
	  				  if(abnormality==BUTTON){
	  					  gecko_cmd_endpoint_close(evt->data.evt_le_connection_opened.connection);
	  				  }
	  				  conn_handle_server2=evt->data.evt_le_connection_opened.connection;
					  /*******************MITM********************/
			  		  /* Store the connection ID */
			  		  activeConnectionId = evt->data.evt_le_connection_opened.connection;
			  		  gecko_cmd_sm_increase_security(activeConnectionId);
					  /*******************************************/
	  			  }
		  		  //Stop advertising
				  gecko_cmd_le_gap_set_mode(le_gap_non_discoverable, le_gap_non_connectable);
				  //Stop scanning
				  STOP_SCANNING=1;
	  		  }
	  		  /**********************CLIENT********************/
	  		  else{
			  		  GRAPHICS_Clear();
			  		  GRAPHICS_AppendString("\nOPENED \nCONNECTION");
			  		  GRAPHICS_Update();
	  			  	  SENSOR_ENABLE=0;
		  			  gecko_cmd_gatt_discover_primary_services_by_uuid(conn_handle,\
		  					  UUIDLEN_ALERT,serviceuuid_alert);
	  		  }
	  		  break;

	      case gecko_evt_le_connection_closed_id:
	    	  STOP_SCANNING=0;		//restart Scanning

	    	  if(conn_handle==evt->data.evt_le_connection_closed.connection){		//Client
	    		  SENSOR_ENABLE=1;
	    	  }
	    	  if(conn_handle_server1==evt->data.evt_le_connection_closed.connection){	//Server
	    		  CONNECTED=0;			//FLAG for gatt notification
	    	  }
	  		  //Stop advertising
			  gecko_cmd_le_gap_set_mode(le_gap_non_discoverable, le_gap_non_connectable);
	    	  gecko_cmd_le_gap_discover(le_gap_discover_observation);  	//Client
	    	  gecko_cmd_le_gap_set_scan_parameters(160, 32, 0);

	    	  /* Check if need to boot to dfu mode */
	    	  if (boot_to_dfu) {
	    		  /* Enter to DFU OTA mode */
	    		  gecko_cmd_system_reset(2);
	    	  	  } else {
	    	  		  /* Restart advertising after client has disconnected */
	    	  		  gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);
	    	  	  }
	    	  break;
/********************************************************************************************/
//SERVER
/********************************************************************************************/
	      case gecko_evt_system_external_signal_id:
	    	  switch(EXTERNAL_SIGNAL){
	    	  	  case LETIMER_SIGNAL:
	    	  		  letimer_service();
	    	  		  break;
	    	  	  case BUTTON_SIGNAL:
	    	  		  button_service();
	    	  		  break;
	    	  	  default:
	    	  		  break;
	    	  }
	    	  break;


/********************************************************************************************/
//CLIENT
/********************************************************************************************/
	      case gecko_evt_le_gap_scan_response_id:
	    	  if(STOP_SCANNING){
	    		  gecko_cmd_le_gap_end_procedure();
	    		  break;
	    	  }
	    	  memmove(scanned_addr,evt->data.evt_le_gap_scan_response.address.addr,6);
	    	  //Check whether the scan response is from the desired address
	    	  for (i=0; i<6; i++) {
	    		  if(scanned_addr[i]!=peer_addr[i])
	    			  break;
	    	  }
	    	  if(i==6){
				  struct gecko_msg_le_gap_open_rsp_t *pResp= \
						  gecko_cmd_le_gap_open(evt->data.evt_le_gap_scan_response.address, \
								  evt->data.evt_le_gap_scan_response.address_type);
				  conn_handle= pResp->connection;
				  //advertise as non-connectable
				  gecko_cmd_le_gap_set_mode(le_gap_non_discoverable, le_gap_non_connectable);
				  gecko_cmd_le_gap_end_procedure();
    		  }
	    	  break;

	      case gecko_evt_gatt_service_id:
	    	  service= evt->data.evt_gatt_service.service;
	    	  if (memcmp(serviceuuid_alert, evt->data.evt_gatt_service.uuid.data, UUIDLEN_ALERT)==0 && alert_flag) {
	    		  uuidlen= UUIDLEN_ALERT;
	    		  memmove(serviceuuid,serviceuuid_alert,uuidlen);
	    		  memmove(characuuid,characuuid_alert,uuidlen);
	    	  }
	    	  else if (memcmp(serviceuuid_findme, evt->data.evt_gatt_service.uuid.data, UUIDLEN_FINDME)==0 && findme_flag) {
	    		  uuidlen= UUIDLEN_FINDME;
	    		  memmove(serviceuuid,serviceuuid_findme,uuidlen);
	    		  memmove(characuuid,characuuid_findme,uuidlen);
	    	  }
	    	  break;

	      case gecko_evt_gatt_characteristic_id:
			  if (memcmp(characuuid_alert,evt->data.evt_gatt_characteristic.uuid.data,UUIDLEN_ALERT)==0) {
				  characteristic= evt->data.evt_gatt_characteristic.characteristic;
			  }
			  else if (memcmp(characuuid_findme,evt->data.evt_gatt_characteristic.uuid.data,UUIDLEN_FINDME)==0) {
				  characteristic= evt->data.evt_gatt_characteristic.characteristic;
			  }
	    	  break;

	      case gecko_evt_gatt_characteristic_value_id:
	    	  characteristic= evt->data.evt_gatt_characteristic_value.characteristic;
	    	  if (characteristic==gattdb_alert_message) {
	    		  if(memcmp(alert1,evt->data.evt_gatt_characteristic_value.value.data,15)==0){
			    	  GRAPHICS_Clear();
			    	  memmove(buffer,evt->data.evt_gatt_characteristic_value.value.data,15);
			    	  GRAPHICS_AppendString(buffer);
			    	  GRAPHICS_Update();
	    		  }
	    		  else if(memcmp(alert2,evt->data.evt_gatt_characteristic_value.value.data,15)==0){
			    	  GRAPHICS_Clear();
			    	  memmove(buffer,evt->data.evt_gatt_characteristic_value.value.data,15);
			    	  GRAPHICS_AppendString(buffer);
			    	  GRAPHICS_Update();
	    		  }
	    		  else if(memcmp(alert3,evt->data.evt_gatt_characteristic_value.value.data,15)==0){
			    	  GRAPHICS_Clear();
			    	  memmove(buffer,evt->data.evt_gatt_characteristic_value.value.data,15);
			    	  GRAPHICS_AppendString(buffer);
			    	  GRAPHICS_Update();
			    	  GRAPHICS_AppendString("\n\nNEEDS \nATTENTION");
					  GRAPHICS_Update();
					  GPIO_PinOutToggle(LED1_port,LED1_pin);
	    		  }
	    		  gecko_cmd_endpoint_close(evt->data.evt_gatt_characteristic_value.connection);
	    		  gecko_cmd_le_gap_end_procedure();
	    	  }
	    	  else if (characteristic==gattdb_immediate_alert) {
		    	  GRAPHICS_AppendString("\n\nIMMEDIATE \nALERT");
		    	  GRAPHICS_Update();
	    		  gecko_cmd_endpoint_close(evt->data.evt_gatt_characteristic_value.connection);
	    		  gecko_cmd_le_gap_end_procedure();
	    	  }
	    	  break;

	      case gecko_evt_gatt_procedure_completed_id:
	    	  if (current_state==IDLE){
	    		  current_state=SERVICE;
		    	  if(alert_flag || findme_flag)
		    		  gecko_cmd_gatt_discover_characteristics_by_uuid(conn_handle, \
		    				  service, uuidlen, characuuid_alert );
	    	  }
	    	  else if (current_state==SERVICE) {
	    		  current_state= CHARACTERISTIC;
	    		  if(alert_flag || findme_flag){
	    			  gecko_cmd_gatt_set_characteristic_notification(conn_handle, characteristic, gatt_notification);
	    			  if(alert_flag)	alert_flag=0;
	    			  else if(findme_flag)	findme_flag=0;
	    		  }

	    	  }
	    	  else if (current_state==CHARACTERISTIC) {
	    		  current_state= IDLE;
	    		  if(findme_flag){
		  			  gecko_cmd_gatt_discover_primary_services_by_uuid(conn_handle,\
		  					  UUIDLEN_FINDME,serviceuuid_findme);
	    		  }
	    	  }
	    	  break;

/***************************************************************************************/
//MITM
/***************************************************************************************/
	  	  case gecko_evt_sm_confirm_passkey_id:
			  GRAPHICS_Clear();
			  GRAPHICS_AppendString("\nPASSKEY");
			  snprintf(buffer,25,"\n\n%ld",evt->data.evt_sm_confirm_passkey.passkey);
			  GRAPHICS_AppendString(buffer);
			  GRAPHICS_AppendString("\n\n\n\nTO CONFIRM \nPRESS PB1");
			  GRAPHICS_Update();
	  		  while(!(GPIO_PinInGet(BUTTON1_port,BUTTON1_pin)== 0));
	  		  gecko_cmd_sm_passkey_confirm(activeConnectionId,1);
	  		  break;

/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/


	      /* Events related to OTA upgrading
	         ----------------------------------------------------------------------------- */

	      /* Check if the user-type OTA Control Characteristic was written.
	       * If ota_control was written, boot the device into Device Firmware Upgrade (DFU) mode. */
	      case gecko_evt_gatt_server_user_write_request_id:
	    	  if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
	    		  /* Set flag to enter to OTA mode */
	    		  boot_to_dfu = 1;
	    		  /* Send response to Write Request */
	    		  gecko_cmd_gatt_server_send_user_write_response(
	    				  evt->data.evt_gatt_server_user_write_request.connection,
						  gattdb_ota_control,
						  bg_err_success);
	    		  /* Close connection to enter to DFU OTA mode */
	    		  gecko_cmd_endpoint_close(evt->data.evt_gatt_server_user_write_request.connection);
	    	  }

	    	  if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_user_control_point) {
	    		  heartrate_threshold=evt->data.evt_gatt_server_attribute_value.value.data[0];
				  GRAPHICS_Clear();
				  GRAPHICS_AppendString("\nTHRESHOLD: ");
				  snprintf(buffer,25,"\n\n%d",heartrate_threshold);
				  GRAPHICS_AppendString(buffer);
				  GRAPHICS_Update();
	    	  }
	    	  break;

	      default:
	    	  break;
   	  }
  }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */

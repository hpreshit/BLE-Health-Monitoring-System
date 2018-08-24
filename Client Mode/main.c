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

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_ldma.h"

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
#include "src/main.h"
#include "src/gpio.h"
#include "src/letimer.h"
#include "src/cmu.h"
#include "src/i2c.h"
#include "graphics.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define ADVERTISEMENT_INTERVAL (337*1.6)
#define ADV_ON_ALL_CHANNELS 7
#define CONNECTION_INTERVAL (75/1.25)
#define SLAVE_LATENCY ((450/75) -1)
#define TIMEOUT 1100

#define CHARACTERISTIC_HEARTRATE	22
#define CHARACTERISTIC_PULSEOXI		29
#define UUIDLEN_HEARTRATE 2
#define UUIDLEN_PULSEOXI 16

//***********************************************************************************
// functions
//***********************************************************************************
void sleep(void){
	if (sleep_block_counter[EM1]>0)
		return;
	else if (sleep_block_counter[EM2]>0)
		EMU_EnterEM1();
	else if (sleep_block_counter[EM3]>0)
		EMU_EnterEM2(true);
	else
		EMU_EnterEM3(true);
	return;

}


void blockSleepMode(em mode){
	CORE_ATOMIC_IRQ_DISABLE();
	sleep_block_counter[mode]++;
	CORE_ATOMIC_IRQ_ENABLE();
}


void unBlockSleepMode(em mode){
	CORE_ATOMIC_IRQ_DISABLE();
	sleep_block_counter[mode]--;
	CORE_ATOMIC_IRQ_ENABLE();
}

//*******************************************************************************
// Variables
//*******************************************************************************
char buffer[7];
char line[40];
uint8 conn_handle;
char buff[25] = {0};
uint8 heartrate_flag=1;
uint8 pulseoxi_flag=1;
uint8 uuidlen,i;

uint8 serviceuuid[16];
uint8 serviceuuid_thermo[2]= {0x09, 0x18};
uint8 serviceuuid_heartrate[UUIDLEN_HEARTRATE]= {0x0D, 0x18};
uint8 serviceuuid_pulseoxi[UUIDLEN_PULSEOXI]= {0x6B, 0xC2, 0x60, 0x71, \
		0x34, 0x88, 0x77, 0x94, 0xA0, 0x47, 0x9D, 0xB1, 0x5B, 0x1A, 0x15, 0xDB};
uint8 characuuid[16];
uint8 characuuid_thermo[2]={0x1C, 0x2A};
uint8 characuuid_heartrate[UUIDLEN_HEARTRATE]= {0x37, 0x2A};
uint8 characuuid_pulseoxi[UUIDLEN_PULSEOXI]= {0x90, 0x6F, 0x0F, 0xD9, \
		0xC3, 0x90, 0x0B, 0xA4, 0x1A, 0x4F, 0x41, 0x34, 0xDC, 0xA6, 0xFF, 0x8E};
uint8 node1_addr[]= {0x8B, 0x38, 0xA9, 0x57, 0x0B, 0x00};
uint8 node2_addr[]= {0x81, 0x88, 0x15, 0x57, 0x0B, 0x00};
uint8 scanned_addr[6] ={0};

uint8 data[2];
uint32_t dest= 0x20000000;
uint32_t length;

static const LDMA_TransferCfg_t transferCfg = LDMA_TRANSFER_CFG_MEMORY();
static const LDMA_Descriptor_t desc;
volatile uint32_t service;
volatile uint16_t characteristic;

typedef enum {
	NONE=0,
	NODE1,
	NODE2
} node;
node scanned_node;

typedef enum {
	IDLE =0,
	SERVICE,
	CHARACTERISTIC,
	NOTIFICATION
} state;
state current_state= IDLE;

/*
 * @brief  Initialization routine for LDMA
 */
void ldma_initialize() {
	LDMA_Init_t init = LDMA_INIT_DEFAULT;
	LDMA_Init(&init);
	LDMA->CH[0].CTRL |= LDMA_CH_CTRL_REQMODE_ALL;
	LDMA->CH[0].DST = dest;
	LDMA->IFC |= 1;
	LDMA->IEN |= 1;
	LDMA->CHEN |= 1;
}

/*
 * @brief  ISR for LDMA
 */
void LDMA_IRQHandler() {
	if (LDMA->IF && 0x01) {
		LDMA->IFC= 1;
		GRAPHICS_AppendString("\n\nDMA Transfer");
		snprintf(buff,25,"\n\n0x%x: %d", dest, (uint8_t)*(uint8_t *)(dest));
		GRAPHICS_AppendString(buff);
		snprintf(buff,25,"\n\n0x%x: %d", dest+1, (uint8_t)*(uint8_t *)(dest+1));
		GRAPHICS_AppendString(buff);
		GRAPHICS_Update();
		dest+=2;
		LDMA->CH[0].DST = dest;
		LDMA->CHEN |= 1;
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
  schedule_event=0;
  thresholdSaved=80;
  amplitudeSaved= 0x1F;

  // Initialize Device
  initMcu();

  // Initialize board
  initBoard();

  /* Initialize GPIO */
  gpio_init();

  // Initialize clocks
  cmu_init();

  // Initialize stack
  gecko_init(&config);

  //Initialize LDMA
  ldma_initialize();

  //Initialize LCD
  GRAPHICS_Init();
  GRAPHICS_Clear();
  GRAPHICS_AppendString("CLIENT GECKO \n");
  GRAPHICS_Update();

  while (1) {

	    /* Check for stack event. */
	    evt = gecko_wait_event();

	    /* Handle events */
	    switch (BGLIB_MSG_ID(evt->header)) {
	      /* This boot event is generated when the system boots up after reset.
	       * Here the system is set to start advertising immediately after boot procedure. */
	      case gecko_evt_system_boot_id:
	    	  gecko_cmd_le_gap_discover(le_gap_discover_observation);
	    	  gecko_cmd_le_gap_set_scan_parameters(160, 112, 0);

	    	  //MITM
	    	  gecko_cmd_sm_delete_bondings();
	    	  gecko_cmd_sm_configure(0x05, sm_io_capability_displayyesno);
	    	  gecko_cmd_sm_set_bondable_mode(1);

	        break;


	      case gecko_evt_sm_confirm_passkey_id:
	    	  GRAPHICS_Clear();
	    	  GRAPHICS_AppendString("\nPASSKEY");
	    	  snprintf(buff,25,"\n\n%ld",evt->data.evt_sm_confirm_passkey.passkey);
	    	  GRAPHICS_AppendString(buff);
	    	  GRAPHICS_AppendString("\n\n\n\nTO CONFIRM \nPRESS PB1");
	    	  GRAPHICS_Update();
	    	  while(!(GPIO_PinInGet(BUTTON1_port,BUTTON1_pin)== 0));
	    	  gecko_cmd_sm_passkey_confirm(conn_handle,1);
	    	  gecko_cmd_gatt_discover_primary_services_by_uuid(conn_handle,\
	    			  UUIDLEN_HEARTRATE, serviceuuid_heartrate);
	    	  break;


	      case gecko_evt_le_gap_scan_response_id:
	    	  memmove(scanned_addr,evt->data.evt_le_gap_scan_response.address.addr,6);
	    	  //Check whether the scan response is from the desired address
			  if (memcmp(node1_addr,evt->data.evt_le_gap_scan_response.address.addr,6)==0) {
				  struct gecko_msg_le_gap_open_rsp_t *pResp= gecko_cmd_le_gap_open(evt->data.evt_le_gap_scan_response.address, evt->data.evt_le_gap_scan_response.address_type);
				  conn_handle= pResp->connection;
				  gecko_cmd_le_gap_end_procedure();
				  scanned_node= NODE1;
			  }
			  else if (memcmp(node2_addr,evt->data.evt_le_gap_scan_response.address.addr,6)==0) {
				  struct gecko_msg_le_gap_open_rsp_t *pResp= gecko_cmd_le_gap_open(evt->data.evt_le_gap_scan_response.address, evt->data.evt_le_gap_scan_response.address_type);
				  conn_handle= pResp->connection;
				  gecko_cmd_le_gap_end_procedure();
				  scanned_node= NODE2;
			  }
			  else
				  scanned_node= NONE;

	    	  break;


	      case gecko_evt_system_external_signal_id:
	    	  //Call the LETIMER function on receipt of the external signal
	    	  service_letimer();
	    	  break;

	      case gecko_evt_le_connection_opened_id:
	    	  //Bonding already exists!
	    	  if (evt->data.evt_le_connection_opened.bonding != 0xFF) {
		    	  gecko_cmd_gatt_discover_primary_services_by_uuid(conn_handle,\
		    			  UUIDLEN_HEARTRATE, serviceuuid_heartrate);
	    	  }
	    	  break;


	      case gecko_evt_gatt_service_id:
	    	  service= evt->data.evt_gatt_service.service;
	    	  if (memcmp(serviceuuid_heartrate, evt->data.evt_gatt_service.uuid.data, UUIDLEN_HEARTRATE)==0 && heartrate_flag) {
	    		  uuidlen= UUIDLEN_HEARTRATE;
	    		  memmove(serviceuuid,serviceuuid_heartrate,uuidlen);
	    		  memmove(characuuid,characuuid_heartrate,uuidlen);
	    	  }
	    	  else if (memcmp(serviceuuid_pulseoxi, evt->data.evt_gatt_service.uuid.data, UUIDLEN_PULSEOXI)==0 && pulseoxi_flag) {
	    		  uuidlen= UUIDLEN_PULSEOXI;
	    		  memmove(serviceuuid,serviceuuid_pulseoxi,uuidlen);
	    		  memmove(characuuid,characuuid_pulseoxi,uuidlen);
	    	  }
	    	  break;


	      case gecko_evt_gatt_characteristic_id:
			  if (memcmp(characuuid_heartrate,evt->data.evt_gatt_characteristic.uuid.data,UUIDLEN_HEARTRATE)==0) {
				  characteristic= evt->data.evt_gatt_characteristic.characteristic;
			  }
			  else if (memcmp(characuuid_pulseoxi,evt->data.evt_gatt_characteristic.uuid.data,UUIDLEN_PULSEOXI)==0) {
				  characteristic= evt->data.evt_gatt_characteristic.characteristic;
				  gecko_cmd_le_gap_end_procedure();
			  }
	    	  break;


	      case gecko_evt_gatt_characteristic_value_id:
	    	  characteristic= evt->data.evt_gatt_characteristic_value.characteristic;
	    	  if (characteristic==CHARACTERISTIC_HEARTRATE) {
	    		  data[0]= evt->data.evt_gatt_characteristic_value.value.data[1];
	    	  }
	    	  else if (characteristic==CHARACTERISTIC_PULSEOXI) {
	    		  data[1]= evt->data.evt_gatt_characteristic_value.value.data[0];
	    		  GRAPHICS_Clear();
	    		  snprintf(buff,25,"\nDev %d abnormal",scanned_node);
	    		  GRAPHICS_AppendString(buff);
	    		  snprintf(buff,25,"\nHR: %d BPM",data[0]);
	    		  GRAPHICS_AppendString(buff);
	    		  snprintf(buff,25,"\nSpO2: %d",data[1]);
	    		  GRAPHICS_AppendString(buff);
		    	  GRAPHICS_Update();
	    		  LDMA->CH[0].CTRL &= ~_LDMA_CH_CTRL_XFERCNT_MASK;
	    		  LDMA->CH[0].CTRL |= 2<< _LDMA_CH_CTRL_XFERCNT_SHIFT;
	    		  LDMA->CH[0].SRC= (uint32_t) &data;
	    		  LDMA->SWREQ |= 1;
	    		  gecko_cmd_endpoint_close(evt->data.evt_gatt_characteristic_value.connection);
	    		  gecko_cmd_le_gap_end_procedure();
	    	  }
	    	  break;


	      case gecko_evt_gatt_procedure_completed_id:
	    	  if (current_state==IDLE){
	    		  current_state=SERVICE;
	    		  if (heartrate_flag || pulseoxi_flag)
	    			  gecko_cmd_gatt_discover_characteristics_by_uuid(conn_handle, \
	    		  	    	    			  service, uuidlen, characuuid );
	    	  }
	    	  else if (current_state==SERVICE) {
	    		  current_state= CHARACTERISTIC;
//		    	  struct gecko_msg_gatt_set_characteristic_notification_rsp_t * var=
	    		  if (heartrate_flag || pulseoxi_flag) {
	    			  gecko_cmd_gatt_set_characteristic_notification(conn_handle, characteristic, gatt_notification);
	    			  if (heartrate_flag)
	    				  heartrate_flag=0;
	    			  else if (pulseoxi_flag)
	    				  pulseoxi_flag=0;
	    		  }
	    	  }
	    	  else if (current_state==CHARACTERISTIC) {
	    		  current_state= IDLE;
	    		  if (pulseoxi_flag) {
		    		  gecko_cmd_gatt_discover_primary_services_by_uuid(conn_handle,\
		    				  UUIDLEN_PULSEOXI, serviceuuid_pulseoxi);
	    		  }
	    	  }

	    	  break;


	      case gecko_evt_le_connection_closed_id:
	    	  /* Check if need to boot to dfu mode */
	        if (boot_to_dfu) {
	          /* Enter to DFU OTA mode */
	          gecko_cmd_system_reset(2);
	        } else {
	        	gecko_cmd_system_set_tx_power(0);
	          /* Restart advertising after client has disconnected */
	          gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);
	        }
	        //Start the discovery again
	        gecko_cmd_le_gap_discover(le_gap_discover_observation);
	        gecko_cmd_le_gap_set_scan_parameters(160, 112, 0);
	        heartrate_flag=1;
	        pulseoxi_flag=1;
	        break;


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
	        break;

	      default:
	        break;
	    }

  }
}
/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */

/*
 * Copyright (c) 2018, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO)
 * All rights reserved.
 */

/* Includes -------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h> 
#include <string.h> 

#include "board.h"
#include "gpio.h"
#include "leds.h"
#include "log.h"
#include "rtc.h"
#include "watchdog.h"
#include "uart.h"
#include "xenon.h"
#include "application.h"
#include "device_nvm.h"
#include "adc.h"
#include "spi.h"

#include "unified_comms_bluetooth.h"
#include "unified_comms_gatt.h"
#include "unified_comms_serial.h"

#include "bluetooth.h"
#include "FreeRTOS_CLI.h"
#include "s4431800_spi.h"
#include "s4431800_state.h"
#include "s4431800_uart.h"
#include "s4431800_adc.h"
#include "s4431800_cli.h"
#include "s4431800_os.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Private Defines ------------------------------------------*/
// clang-format off
// clang-format on
/* Type Definitions -----------------------------------------*/

/* Function Declarations ------------------------------------*/

void vCustomSerialHandler(xCommsInterface_t *pxComms,
						  xUnifiedCommsIncomingRoute_t *pxCurrentRoute,
						  xUnifiedCommsMessage_t *pxMessage);

/* Private Variables ----------------------------------------*/
/*-----------------------------------------------------------*/

void vApplicationSetLogLevels( void )
{
	eLogSetLogLevel(LOG_RESULT, LOG_INFO);
	eLogSetLogLevel(LOG_APPLICATION, LOG_INFO);
	
}

/*-----------------------------------------------------------*/

void vApplicationStartupCallback( void )
{
	WatchdogReboot_t *  pxRebootData;
	tdf_watchdog_info_t xWatchdogInfo;
	vLedsOn( LEDS_ALL );
	/* Get Reboot reasons and clear */
	pxRebootData = xWatchdogRebootReason();
	if ( pxRebootData != NULL ) {
		vWatchdogPrintRebootReason( LOG_APPLICATION, LOG_INFO, pxRebootData );
		vWatchdogPopulateTdf( pxRebootData, &xWatchdogInfo );
	}

	s4431800_state_init();
	s4431800_cli_init();

	xSerialComms.fnReceiveHandler = vCustomSerialHandler;
	vUnifiedCommsListen(&xSerialComms, COMMS_LISTEN_ON_FOREVER); 

	vLedsOff( LEDS_ALL );

}

/*-----------------------------------------------------------*/

void vApplicationTickCallback( uint32_t ulUptime )
{

	UNUSED( ulUptime );

}

/*-----------------------------------------------------------*/

void vCustomSerialHandler(xCommsInterface_t *pxComms,
						  xUnifiedCommsIncomingRoute_t *pxCurrentRoute,
						  xUnifiedCommsMessage_t *pxMessage)
{

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	char pcLocalString[60] = {0};
	UNUSED(pxCurrentRoute);
	UNUSED(pxComms);
	/* 
	 * Copy the string to a local buffer so it can be NULL terminated properly
	 * The %s format specifier does not respect provided lengths
	 */
	pvMemcpy(pcLocalString, pxMessage->pucPayload, pxMessage->usPayloadLen);

	vLogResetLogLevels();

	eLog(LOG_APPLICATION, LOG_ERROR, "\r\nString: %s\r\n\r\n", pcLocalString);

	xQueueSendToBackFromISR( s4431800_QueueProccess, &pcLocalString, &xHigherPriorityTaskWoken );

}

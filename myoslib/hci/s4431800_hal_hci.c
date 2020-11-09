/********************************************************************************
 * @file    mylib/hci/s4431800_hal_hci.c
 * @author  Harry Stokes 44318008
 * @date    15/04/2020
 * @brief   hal hci driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4431800_hal_hci_init()
 * s4431800_hci_send()
 * s4431800_hci_x_value()
 * s4431800_hci_y_value()
 * s4431800_hci_z_value()
 * s4431800_hci_port_value()
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "s4431800_hal_hci.h"
#include "s4431800_os_hci.h"
#include "uart.h"
#include "unified_comms_serial.h"
#include "gpio.h"
#include "leds.h"
#include "log.h"
#include "tdf.h"
#include "rtc.h"
#include "watchdog.h"
#include "test_reporting.h"
#include "tiny_printf.h"
#include "device_nvm.h"
#include "unified_comms_bluetooth.h"
#include "bluetooth.h"
#include "xenon.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/* Utils includes. */
#include "FreeRTOS_CLI.h"

/* Task Priorities ------------------------------------------------------------*/
#define mainRADIOTASK_PRIORITY					( tskIDLE_PRIORITY + 5 )
#define mainFSMTASK_PRIORITY					( tskIDLE_PRIORITY + 3 )

/* Task Stack Allocations -----------------------------------------------------*/
#define mainRADIOTASK_STACK_SIZE		        ( configMINIMAL_STACK_SIZE * 5)
#define mainFSMTASK_STACK_SIZE	        	    ( configMINIMAL_STACK_SIZE * 1)

UART_MODULE_CREATE( SERIAL_HANDLER, NRF_UARTE1, UARTE1_IRQHandler, UNUSED_IRQ, 4, SERIAL_INTERFACE_DEFAULT_SIZE, 64 );
xUartModule_t * pxUartSend = &UART_MODULE_GET( SERIAL_HANDLER );

char* pcBufferArray;
uint32_t bufferLength;
uint32_t dataLength; 
uint32_t s4431800_buffLen;
char s4431800_buffer[20];
xTdfTime_t xTime; 
EventGroupHandle_t xCreatedEventGroup;
tdf_lsm6dsl_t xLSM6DSL;
tdf_lps22hb_all_t xLPS22HB_ALL;
tdf_3d_pose_t x3D_POSE;
tdf_height_msl_t xheight_msl;
tdf_range_mm_t xrange_mm;
float ultraMeters;
int toggle = 0;


/**
  * @brief  Initialise hci uart.
  * @param  None
  * @retval None
  */
extern void s4431800_hal_hci_init(void) {

	pxUartSend->xPlatform.pxTimer = NRF_TIMER1; 
	pxUartSend->ulBaud = 115200; 
	pxUartSend->xPlatform.xRx = UART1_RX_PIN;
	pxUartSend->xPlatform.xTx = UART1_TX_PIN;	
	pxUartSend->xPlatform.xRts = UART1_RTS_PIN; 
	pxUartSend->xPlatform.xCts = UNUSED_GPIO; 
	eUartInit( pxUartSend, true );	

	static xSerialReceiveArgs_t xArgs;
	xArgs.pxUart    = pxUartSend;
	xArgs.fnHandler = (fnSerialByteHandler_t) s4431800_hci_handler;   
	configASSERT( xTaskCreate( vSerialReceiveTask, "Ser Recv", configMINIMAL_STACK_SIZE, &xArgs, tskIDLE_PRIORITY + 1, NULL ) == pdPASS );
	vUartOn(pxUartSend);

	s4431800_buffLen = 0;
	dataLength = 0;

	s4431800_os_hci_init();

	xTaskCreate( (void *) &s4431800_hal_task, "Radio", mainRADIOTASK_STACK_SIZE, NULL, mainRADIOTASK_PRIORITY, NULL);
	xCreatedEventGroup = xEventGroupCreate();
   

}



/**
  * @brief  Called when a byte (char) is recieved.
  * @param  recvChar - byte recieved
  * @retval None
  */
void s4431800_hci_handler(char recvChar) {

	// eLog(LOG_APPLICATION, LOG_ERROR, "Char Value: %02x\r\n", recvChar);

	if (s4431800_buffLen == 2) {
		dataLength = (int) recvChar;
	}

	if (recvChar == 0xAA) {
		s4431800_buffLen = 0;
		dataLength = 0;
		memset(s4431800_buffer, 0, 20);
	} 

	s4431800_buffer[s4431800_buffLen] = recvChar;
	s4431800_buffLen++;

	if (s4431800_buffLen >= 7 && (6 + ((uint32_t) s4431800_buffer[2])) == s4431800_buffLen) {

		if (s4431800_SemaphorePacket != NULL) {
			xSemaphoreGive(s4431800_SemaphorePacket);
		}

	}
}


/**
  * @brief  Sends a message over uart connection.
  * @param  message - String to be sent.
  * @param 	count - Length of string to be sent.
  * @retval None
  */
void s4431800_hci_send(char * message, int count) {

    pcBufferArray = (char *) xUartBackend.fnClaimBuffer(pxUartSend, &bufferLength);
    pvMemcpy(pcBufferArray, message, count);
    xUartBackend.fnSendBuffer(pxUartSend, pcBufferArray, bufferLength);
    
}

/**
 * @brief Proccess the buffer
 * @param None
 * @retval None
 */
void s4431800_hal_task(void) {

	int16_t val;
	float xMag, yMag, zMag, x, y, z, xDegrees, yDegrees, altitude;
    
    for(;;) {

		 if ( s4431800_SemaphorePacket != NULL) {

            if ( xSemaphoreTake(s4431800_SemaphorePacket, 10) == pdTRUE) {

				switch(s4431800_buffer[3]) {
					case 0x00:
						// eLog(LOG_APPLICATION, LOG_ERROR, "Testing\r\n");
						xEventGroupSetBits(xCreatedEventGroup, BIT_0);
						break;
					case 0x01:
						switch(s4431800_buffer[2]){
							case 0x01:
								eLog(LOG_APPLICATION, LOG_ERROR, "Port Value: %d\r\n", s4431800_buffer[6]);
								break;
							case 0x02:
								switch(s4431800_buffer[5]) {
									case 0x28:
										val = (uint8_t) s4431800_buffer[6] + ((uint8_t) s4431800_buffer[7] * 256);
										eLog(LOG_APPLICATION, LOG_ERROR, "X Value: %d\r\n", val);
										break;
									case 0x2a:
										val = (uint8_t) s4431800_buffer[6] + ((uint8_t) s4431800_buffer[7] * 256);
										eLog(LOG_APPLICATION, LOG_ERROR, "Y Value: %d\r\n", val);
										break;
									case 0x2c:
										val = (uint8_t) s4431800_buffer[6] + ((uint8_t) s4431800_buffer[7] * 256);
										eLog(LOG_APPLICATION, LOG_ERROR, "Z Value: %d\r\n", val);
										break;
								} break;

							case 0x06:
								val =  (uint8_t) s4431800_buffer[6] + (256 * (uint8_t) s4431800_buffer[7]);
								xMag = (float) val / 16384;
								val = (uint8_t) s4431800_buffer[8] + (256 * (uint8_t) s4431800_buffer[9]);
								yMag = (float) val / 16384;
								val = (uint8_t) s4431800_buffer[10] + (256 * (uint8_t) s4431800_buffer[11]);
								zMag = (float) val / 16384;

								x = pow(xMag, 2);
								y = pow(yMag, 2);
								z = pow(zMag, 2);

								xDegrees = 180 * atan(xMag/(float)sqrt(y+z)) / M_PI * 100;
                                yDegrees = 180 * atan(yMag/(float)sqrt(x+z)) / M_PI * 100;

                                x3D_POSE.pitch = xDegrees;
                                x3D_POSE.roll = yDegrees;

								eLog(LOG_APPLICATION, LOG_ERROR, "Pitch: %d, Roll: %d\r\n", x3D_POSE.pitch / 100, x3D_POSE.roll/ 100);
								// eLog(LOG_APPLICATION, LOG_ERROR, "Mag Values: x = %f, y = %f, z = %f\r\n", (double) xMag, (double) yMag, (double) zMag);

                                bRtcGetTdfTime(&xTime);
                                eTdfAddMulti(BLE_LOG, TDF_3D_POSE, TDF_TIMESTAMP_GLOBAL, &xTime, &x3D_POSE);
                                eTdfFlushMulti(BLE_LOG); 
								break;

							case 0x0c:
								xLSM6DSL.gyro_x = (int) s4431800_buffer[6] + (256 * (int) s4431800_buffer[7]);
								xLSM6DSL.gyro_y = (int) s4431800_buffer[8] + (256 * (int) s4431800_buffer[9]);
								xLSM6DSL.gyro_z = (int) s4431800_buffer[10] + (256 * (int) s4431800_buffer[11]);
								xLSM6DSL.acc_x =  ((float) s4431800_buffer[12] + (256 * (float) s4431800_buffer[13]));
								xLSM6DSL.acc_y =  ((float) s4431800_buffer[14] + (256 * (float) s4431800_buffer[15]));
								xLSM6DSL.acc_z =  ((float) s4431800_buffer[16] + (256 * (float) s4431800_buffer[17]));

								eLog(LOG_APPLICATION, LOG_ERROR, "Acc Values: x = %d, y = %d, z = %d\r\n", xLSM6DSL.acc_x, xLSM6DSL.acc_y, xLSM6DSL.acc_z);
								eLog(LOG_APPLICATION, LOG_ERROR, "Gyr Values: x = %d, y = %d, z = %d\r\n", xLSM6DSL.gyro_x, xLSM6DSL.gyro_y, xLSM6DSL.gyro_z);

								bRtcGetTdfTime(&xTime);
								eTdfAddMulti(BLE_LOG, TDF_LSM6DSL, TDF_TIMESTAMP_GLOBAL, &xTime, &xLSM6DSL);
								eTdfFlushMulti(BLE_LOG); break;
						} break;
					case 0x02:
						switch(s4431800_buffer[2]) {
							case 0x01:
								eLog(LOG_APPLICATION, LOG_ERROR, "Port Value: %d\r\n", s4431800_buffer[6]);
								break;
						} break;
					case 0x03:
						switch(s4431800_buffer[2]) {
							case 0x01:
								eLog(LOG_APPLICATION, LOG_ERROR, "Port Value: %d\r\n", s4431800_buffer[6]);
								break;
							case 0x03:
								xLPS22HB_ALL.pressure = (int) s4431800_buffer[6] + (256 * (int) s4431800_buffer[7]) + (65536 * (int) s4431800_buffer[8]);
								xLPS22HB_ALL.temperature = 0;
								eLog(LOG_APPLICATION, LOG_ERROR, "Pressure Value: %d\r\n", xLPS22HB_ALL.pressure);

								bRtcGetTdfTime(&xTime);
								eTdfAddMulti(BLE_LOG, TDF_LPS22HB_ALL, TDF_TIMESTAMP_GLOBAL, &xTime, &xLPS22HB_ALL);
								eTdfFlushMulti(BLE_LOG); 
								
								altitude = ((pow((1025.0/ ((double) xLPS22HB_ALL.pressure/4096) ), (1.0/5.257)) - 1) * (20.0 + 273.15)) / 0.0065;
								xheight_msl.height = altitude;
								eTdfAddMulti(BLE_LOG, TDF_HEIGHT_MSL, TDF_TIMESTAMP_GLOBAL, &xTime, &xheight_msl);
								eTdfFlushMulti(BLE_LOG); 
								
								break;

						} break;
					case 0x04:
						switch(s4431800_buffer[2]) {
							case 0x01:
								eLog(LOG_APPLICATION, LOG_ERROR, "Port Value: %d\r\n", s4431800_buffer[6]);
								break;
						} break;
					case 0x05:
						switch(s4431800_buffer[2]) {
							case 0x01:
								eLog(LOG_APPLICATION, LOG_ERROR, "Port Value: %d\r\n", s4431800_buffer[6]);
								break;
						} break;
					case 0x06:
						switch(s4431800_buffer[2]) {
							case 0x02:
								val = (int) s4431800_buffer[7] + (256 * (int) s4431800_buffer[6]);
								ultraMeters = (float) val * 17150 / 100000000;
								eLog(LOG_APPLICATION, LOG_ERROR, "Ultra value %f\r\n", (double) ultraMeters);
								xrange_mm.range = ultraMeters * 1000;

								bRtcGetTdfTime(&xTime);
                                eTdfAddMulti(BLE_LOG, TDF_RANGE_MM, TDF_TIMESTAMP_GLOBAL, &xTime, &xrange_mm);
                                eTdfFlushMulti(BLE_LOG); 
								break;
						} break;
				}

			}
		 }

        vTaskDelay(10);

    }
}

void buttonPress() {

	BaseType_t xHigherPriorityTaskWoken, xResult;

	xHigherPriorityTaskWoken = pdFALSE;

	if (toggle == 0) {
		xResult = xEventGroupSetBitsFromISR(xCreatedEventGroup, 0x01, &xHigherPriorityTaskWoken);
		toggle = 1;
	} else {
		xResult = xEventGroupSetBitsFromISR(xCreatedEventGroup, 0x08, &xHigherPriorityTaskWoken);
		toggle = 0;
	}


	if( xResult != pdFAIL ) {
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}

}

float ultra_value(void) {
	return ultraMeters;
}

float pitch_value(void) {
	return x3D_POSE.pitch;
}

float roll_value(void) {
	return x3D_POSE.roll;
}

uint32_t preassure_value(void) {
	return xLPS22HB_ALL.pressure;
}
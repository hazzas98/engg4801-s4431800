/********************************************************************************
 * @file    mylib/cli/s4431800_uart.c
 * @author  Harry Stokes 44328008
 * @date    05/07/2020
 * @brief   UART communications file
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * 
 ******************************************************************************
 */

/* Standard includes. */
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Utils includes. */
#include "FreeRTOS_CLI.h"
#include "s4431800_uart.h"

#include "application.h"
#include "nrf52840.h"

#include "gpio.h"
#include "leds.h"
#include "log.h"
#include "spi.h"
#include "spi_arch.h"
#include "adc.h"
#include "xenon.h"
#include "uart.h"
#include "unified_comms_serial.h"
#include "tdf.h"
#include "rtc.h"
#include "watchdog.h"
#include "test_reporting.h"
#include "tiny_printf.h"
#include "device_nvm.h"
#include "unified_comms_bluetooth.h"
#include "bluetooth.h"
#include "board.h"
#include "nrf_spim.h"


uint8_t ucConnectCommand[3] = "AT\r";
uint8_t ucSendCommand[9] = "AT+SBDIX\r";
uint8_t ucResult[9] = "+SBDIX: 0";


UART_MODULE_CREATE( SERIAL_HANDLER, NRF_UARTE1, UARTE1_IRQHandler, UNUSED_IRQ, 4, SERIAL_INTERFACE_DEFAULT_SIZE, 64 );
xUartModule_t * pxUartSend = &UART_MODULE_GET( SERIAL_HANDLER );

xUartModule_t * pxUartSend;


char* pcBufferArray;
uint32_t bufferLength;
int received = 0;
int receivedToggle = 0;
int sent = 0;
int sentResult = 0;
int timeout = 0;

/**
  * @brief  Initialise hci uart.
  * @param  None
  * @retval None
  */
extern void s4431800_uart_init(void) {

    pxUartSend->xPlatform.pxTimer = NRF_TIMER1; 
    pxUartSend->ulBaud = 19200; 
    pxUartSend->xPlatform.xRx = UART1_RX_PIN;
    pxUartSend->xPlatform.xTx = UART1_TX_PIN;	
    pxUartSend->xPlatform.xRts = UNUSED_GPIO; 
    pxUartSend->xPlatform.xCts = UNUSED_GPIO; 
    eUartInit( pxUartSend, false );	

    static xSerialReceiveArgs_t xArgs;
    xArgs.pxUart    = pxUartSend;
    xArgs.fnHandler = (fnSerialByteHandler_t) s4431800_sat_handler;   
    configASSERT( xTaskCreate( vSerialReceiveTask, "Ser Recv", configMINIMAL_STACK_SIZE, &xArgs, tskIDLE_PRIORITY + 1, NULL ) == pdPASS );
    vGpioSetup( ROCKBLOCK_PIN, GPIO_PUSHPULL, GPIO_PUSHPULL_HIGH );


    xTaskCreate( (void *) &s4431800_uart_task, "Radio", mainRADIOTASK_STACK_SIZE, NULL, mainRADIOTASK_PRIORITY, NULL);

}

/**
  * @brief  Called when a byte (char) is recieved.
  * @param  recvChar - byte recieved
  * @retval None
  */
void s4431800_sat_handler(char recvChar) {

    switch (received) {
        case 0:
            if (recvChar == 'O') {received = 1;} break; 
        case 1:
            if (recvChar == 'K') {receivedToggle = 1;} 
            received = 0; break;
        default:
            received = 0; break;
    }

    if (sent == 8) {
        if (ucResult[sent] == recvChar) {
            sentResult = 1;
        } else {
            sentResult = 2;
        }
        sent = 0;
    }

    if (ucResult[sent] == recvChar) {
        sent++;
        sentResult = 0;
    } else {
        sent = 0;
    }

}

/**
  * @brief  Sends a message over uart connection.
  * @param  message - String to be sent.
  * @param 	count - Length of string to be sent.
  * @retval None
  */
extern void s4431800_hci_send(char * message, int count) {

    pcBufferArray = (char *) xUartBackend.fnClaimBuffer(pxUartSend, &bufferLength);
    pvMemcpy(pcBufferArray, message, count);
    xUartBackend.fnSendBuffer(pxUartSend, pcBufferArray, bufferLength);
    
}



void s4431800_send_data(uint8_t ucCommand) {

    vUartOn(pxUartSend);
    s4431800_hci_send(ucConnectCommand, 3);

    while (receivedToggle == 0) {
        vTaskDelay(50);
    }

    vTaskDelay(500); // 1 second

    receivedToggle = 0;
    s4431800_hci_send(ucCommand, 45);

    while (receivedToggle == 0) {
        vTaskDelay(50); 
    }


    vTaskDelay(500); // 1 second
    receivedToggle = 0;
    timeout = 0;

    while(timeout < 5) {

        s4431800_hci_send(ucSendCommand, 9);

        while (sentResult == 0) {
            vTaskDelay(50);
        }

        if (sentResult == 2) {
            timeout++;
            sentResult = 0;
        } else if (sentResult == 1) {
            sentResult = 0;
            break;
        }

        vTaskDelay(100);
    }

}

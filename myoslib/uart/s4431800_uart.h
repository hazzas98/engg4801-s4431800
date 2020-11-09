/********************************************************************************
 * @file    mylib/cli/s4431800_uart.h
 * @author  Harry Stokes 44318008
 * @date    5/07/2020
 * @brief   UART Communications file
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * 
 ******************************************************************************
 */

#ifndef S4431800_UART_H
#define S4431800_UART_H

/* Scheduler includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS_CLI.h"
#include "gpio.h"

/* Private typedef -----------------------------------------------------------*/

/* Task Priorities ------------------------------------------------------------*/
#define mainRADIOTASK_PRIORITY					( tskIDLE_PRIORITY + 5 )
#define mainFSMTASK_PRIORITY					( tskIDLE_PRIORITY + 3 )

/* Task Stack Allocations -----------------------------------------------------*/
#define mainRADIOTASK_STACK_SIZE		        ( configMINIMAL_STACK_SIZE * 5)
#define mainFSMTASK_STACK_SIZE	        	    ( configMINIMAL_STACK_SIZE * 1)


#define UART1_RX_PIN        (xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 0, 8 )}    // Pin RX
#define UART1_TX_PIN   		(xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 0, 6 )}    // Pin TX
#define UART1_RTS_PIN   	(xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 1, 1 )}    // Pin D2
#define UART1_CTS_PIN   	(xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 1, 2 )}    // Pin D3
#define ROCKBLOCK_PIN   	(xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 1, 8 )}    // Pin D4

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


extern void s4431800_uart_init(void);
void s4431800_sat_handler(char recvChar);
extern void s4431800_hci_send(char * message, int count);
void s4431800_send_data(uint8_t ucCommand);

#endif
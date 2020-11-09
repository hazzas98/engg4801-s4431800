/********************************************************************************
 * @file    mylib/cli/s4431800_state.c
 * @author  Harry Stokes 44318008
 * @date    05/07/2020
 * @brief   State machine file
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
#include "s4431800_spi.h"
#include "s4431800_state.h"
#include "s4431800_uart.h"
#include "s4431800_adc.h"

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

#define MEASUREMENT_STATE       0
#define COMMUNICATION_STATE     1
#define IDLE_STATE              2

SPI_MODULE_CREATE(SPI_HANDLER, 2, SPIM2_IRQHandler);
xSpiModule_t * pxModule = &SPI_MODULE_GET(SPI_HANDLER);


/**
  * @brief  Initialise hci uart.
  * @param  None
  * @retval None
  */
extern void s4431800_state_init(void) {

    s4431800_spi_init(pxModule);
    s4431800_uart_init();

    xTaskCreate( (void *) &s4431800_state_task, "Radio", mainRADIOTASK_STACK_SIZE, NULL, mainRADIOTASK_PRIORITY, NULL);

}


/**
 * @brief Proccess the buffer
 * @param None
 * @retval None
 */
void s4431800_state_task(void) {

    int state = MEASUREMENT_STATE;
    float NTU;
    uint16_t tempValue;
    uint8_t ucCommand[45];

    for(;;) {

        switch(state) {

            case MEASUREMENT_STATE:

                tempValue = s4431800_spi_record_data(pxModule);
                NTU = s4431800_adc_record();
                snprintf(ucCommand, 45, "AT+SBDWT= Temperature: %d, NTU Value: %d\r", tempValue, (int) NTU);
                vGpioWrite(ROCKBLOCK_PIN, true);
                state = COMMUNICATION_STATE;
                vTaskDelay(1000); // 2 seconds
                break;

            case COMMUNICATION_STATE:

                s4431800_send_data(ucCommand);
                vTaskDelay(1000); // 2 seconds
                state = IDLE_STATE;
                break;

            case IDLE_STATE:

                vUartOff(pxUartSend);
                vLedsToggle(LEDS_BLUE);
                vGpioWrite(ROCKBLOCK_PIN, false);
                state = MEASUREMENT_STATE;
                // 1800000 == 1hr, 180000 == 6min, 18000 == 36sec, 1800 = 3.6sec
                vTaskDelay(1800000);
                break;

            default: 

                state = MEASUREMENT_STATE;
                break;

        }

    }
}

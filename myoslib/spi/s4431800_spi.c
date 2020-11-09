/********************************************************************************
 * @file    mylib/cli/s4431800_spi.c
 * @author  Harry Stokes 44328008
 * @date    05/07/2020
 * @brief   Spi communications file
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


/**
  * @brief  Initialise the spi protocol.
  * @param  pxModule pointer to spi instance
  * @retval None
  */
extern void s4431800_spi_init(xSpiModule_t * pxModule ) {

    pxModule->xPlatform.xMosi = MOSI_PIN;
    pxModule->xPlatform.xMiso = MISO_PIN;
    pxModule->xPlatform.xSclk = SCLK_PIN;
    eSpiInit(pxModule);

}

/**
  * @brief  Communicate though SPI
  * @param  pxModule pointer to spi instance
  * @retval Temperature
  */
uint16_t s4431800_spi_record_data(xSpiModule_t * pxModule ) {

    eSpiBusStart( pxModule, &pxCurrentConfig, portMAX_DELAY );
    vSpiCsAssert( pxModule );
    vSpiReceive( pxModule, pucData, 5 );
    uint16_t tempValue = (pucData[0] << 4) + ((pucData[1] & 0xF0) >> 4);
    vSpiCsRelease(pxModule);
    vSpiBusEnd( pxModule );
    vTaskDelay(500); // 1 second

    return tempValue;
}

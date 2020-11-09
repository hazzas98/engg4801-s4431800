/********************************************************************************
 * @file    mylib/cli/s4431800_spi.h
 * @author  Harry Stokes 44328008
 * @date    5/07/2020
 * @brief   Spi Communications file
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * 
 ******************************************************************************
 */

#ifndef S4431800_SPI_H
#define S4431800_SPI_H

/* Scheduler includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS_CLI.h"
#include "gpio.h"

#define MISO_PIN            (xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 1, 14 )}   // Pin MI
#define MOSI_PIN   		    (xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 1, 13 )}   // Pin MO
#define SCLK_PIN   		    (xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 1, 15 )}   // Pin SCK
#define CS_PIN     		    (xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 0, 31 )}   // Pin A5

/* Private typedef -----------------------------------------------------------*/
uint8_t pucData[5];

const xSpiConfig_t pxCurrentConfig = {
	.ulMaxBitrate   = 40000000,
	.ucDummyTx      = 0xFF,
	.ucMsbFirst     = true,
	.xCsGpio	    = CS_PIN,
	.eClockMode     = eSpiClockMode3
};

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


extern void s4431800_spi_init(xSpiModule_t * pxModule );
extern uint16_t s4431800_spi_record_data(xSpiModule_t * pxModule );

#endif
/********************************************************************************
 * @file    mylib/cli/s4431800_adc.c
 * @author  Harry Stokes 44318008
 * @date    05/07/2020
 * @brief   ADC pin read file
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * 
 ******************************************************************************
 */

#ifndef S4431800_ADC_H
#define S4431800_ADC_H

/* Scheduler includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS_CLI.h"
#include "gpio.h"

/* Private typedef -----------------------------------------------------------*/

#define A0_PIN			(xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 0, 3 )}

extern float s4431800_adc_record(void);

#endif
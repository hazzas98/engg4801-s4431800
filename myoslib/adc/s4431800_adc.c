/********************************************************************************
 * @file    mylib/cli/s4431800_adc.c
 * @author  Harry Stokes 44318008
 * @date    05/07/2020
 * @brief   ADC pin read file
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4431800_adc_record()
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

/**
 * @brief Returns the pin voltage of A0_pin
 * @param None
 * @retval Voltage level
 */ 
float s4431800_adc_record(void) {

    float x = ((float) ulBoardAdcSample(A0_PIN, ADC_RESOLUTION_14BIT, ADC_REFERENCE_VOLTAGE_3V6) / 16383) * (float) 5;
    float NTU = -1321*x + (float) 5342.6;

    return NTU;

}

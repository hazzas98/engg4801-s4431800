/********************************************************************************
 * @file    mylib/cli/s4431800_state.h
 * @author  Harry Stokes 44318008
 * @date    5/07/2020
 * @brief   State machine file
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * 
 ******************************************************************************
 */

#ifndef S4431800_STATE_H
#define S4431800_STATE_H

/* Scheduler includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS_CLI.h"
#include "gpio.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern void s4431800_state_init(void);
void s4431800_state_task(void);


#endif
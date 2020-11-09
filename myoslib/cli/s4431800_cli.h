#ifndef S_CLI_HEADER_H
#define S_CLI_HEADER_H

/* Scheduler includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS_CLI.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//command definitions
extern CLI_Command_Definition_t xGetTime;
extern CLI_Command_Definition_t xSetLED;
extern CLI_Command_Definition_t xSend;
// extern CLI_Command_Definition_t xAcc;
extern CLI_Command_Definition_t xUARTTest;

/* External function prototypes -----------------------------------------------*/

//hal driver to initialise system monitor
extern void s4431800_cli_init(void);



#endif
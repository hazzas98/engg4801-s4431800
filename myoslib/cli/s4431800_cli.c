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

#include "s4431800_cli.h"
#include "s4431800_os.h"

#include "gpio.h"
#include "leds.h"
#include "log.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//define all the commands
static BaseType_t prvSetLEDCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvGetTimeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvSendCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
// static BaseType_t prvlsm6dslCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvUARTTestCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

//command to change LED lights
CLI_Command_Definition_t xSetLED = {
	"led",
	"led <type> <colour>: set the LED on board.\r\n",
	prvSetLEDCommand,
	2
};

//command to read current time
CLI_Command_Definition_t xGetTime = {
	"time",
	"time <type>: Print the current time.\r\n",
	prvGetTimeCommand,
	1
};

//command to rsend
CLI_Command_Definition_t xSend = {
	"i2creg",
	"i2creg <r/w> <sid> <regaddr> <regVal>: Send or recieve over serial.\r\n",
	prvSendCommand,
	4
};

// //command to rsend
// CLI_Command_Definition_t xlsm6dsl = {
// 	"lsm6dsl",
// 	"lsm6dsl <r/w> <x/y/z/a>: Send or recieve accelerometer values.\r\n",
// 	prvlsm6dslCommand,
// 	2
// };

//command to read current time
CLI_Command_Definition_t xUARTTest = {
	"test",
	"test <type>: Send a UART package.\r\n",
	prvUARTTestCommand,
	1
};

/**
 * @brief Initalize commands
 * @param None
 * @retval None
 */
extern void s4431800_cli_init(void) {

    /* Register CLI commands */
	FreeRTOS_CLIRegisterCommand(&xSetLED);
	FreeRTOS_CLIRegisterCommand(&xGetTime);
	FreeRTOS_CLIRegisterCommand(&xSend);
	// FreeRTOS_CLIRegisterCommand(&xlsm6dsl);
	FreeRTOS_CLIRegisterCommand(&xUARTTest);

    //initialise os driver 
    s4431800_os_init();

}

//command to set the LED lights on board
static BaseType_t prvSetLEDCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	vLedsToggle( LEDS_GREEN );

	long lParam_len;
	const char *cCmd_string;
	size_t dummy = 0;
	struct LEDMessage LEDsRecv;

	/* Get parameters from command string */
	cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

	LEDsRecv.typeAction = cCmd_string[0];
	LEDsRecv.color = cCmd_string[2];


	//send the message to inform the radio task of this change
	if (s4431800_QueueSetLED != NULL) {

		xQueueSendToFront(s4431800_QueueSetLED, ( void * ) &LEDsRecv, ( portTickType ) 100 );

	}

	/* Write command echo output string to write buffer. */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "\n\r%s\n\r", cCmd_string);
	dummy = xWriteBufferLen - dummy;

	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	return pdFALSE;

}


//command to get time
static BaseType_t prvGetTimeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {
	

	long lParam_len;
	const char *cCmd_string;
    struct TimeMessage Message;
	size_t dummy = 0;

	/* Get parameters from command string */
	cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

    Message.typeAction = cCmd_string[0];


	//send the message to inform the radio task of this change
	if (s4431800_QueueGetTime != NULL) {

		xQueueSendToFront(s4431800_QueueGetTime, ( void * ) &Message, ( portTickType ) 10 );

	}

	/* Write command echo output string to write buffer. */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "\n\r%s\n\r", cCmd_string);
	dummy = xWriteBufferLen - dummy;

	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	return pdFALSE;

}


//command to set the LED lights on board
static BaseType_t prvSendCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	size_t dummy = 0;
	struct SendMessage messageSend;
	int8_t *pcParameter1, *pcParameter2, *pcParameter3, *pcParameter4;
	BaseType_t xParameter1StringLength, xParameter2StringLength, xParameter3StringLength, xParameter4StringLength;

	/* Get parameters from command string */
	pcParameter4 = (int8_t*) FreeRTOS_CLIGetParameter(pcCommandString, 4, &xParameter4StringLength);
	pcParameter4[xParameter4StringLength] = '\0';
	pcParameter3 = (int8_t*) FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameter3StringLength);
	pcParameter3[xParameter3StringLength] = '\0';
	pcParameter2 = (int8_t*) FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameter2StringLength);
	pcParameter2[xParameter2StringLength] = '\0';
	pcParameter1 = (int8_t*) FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameter1StringLength);
	pcParameter1[xParameter1StringLength] = '\0';


	messageSend.readWrite = pcParameter1[0];
	messageSend.sid = atoi((const char*)pcParameter2); 
	messageSend.regaddr = atoi((const char*)pcParameter3); 
	messageSend.regVal = atoi((const char*)pcParameter4); 

	// eLog(LOG_APPLICATION, LOG_ERROR, "\r\n SID Value: %d\r\n", messageSend.sid );


	//send the message to inform the radio task of this change
	if (s4431800_QueueSend != NULL) {

		xQueueSendToFront(s4431800_QueueSend, ( void * ) &messageSend, ( portTickType ) 100 );

	}

	/* Write command echo output string to write buffer. */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "\n\r%s\n\r", pcParameter1);
	dummy = xWriteBufferLen - dummy;

	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	return pdFALSE;

}

// //command to set the LED lights on board
// static BaseType_t prvlsm6dslCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

// 	size_t dummy = 0;
// 	struct accRead messageSend;
// 	int8_t *pcParameter1, *pcParameter2;
// 	BaseType_t xParameter1StringLength, xParameter2StringLength;

// 	/* Get parameters from command string */
// 	pcParameter2 = (int8_t*) FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameter2StringLength);
// 	pcParameter2[xParameter2StringLength] = '\0';
// 	pcParameter1 = (int8_t*) FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameter1StringLength);
// 	pcParameter1[xParameter1StringLength] = '\0';


// 	messageSend.readWrite = pcParameter1[0];
// 	messageSend.axis = pcParameter2[0];


// 	//send the message to inform the radio task of this change
// 	if (s4431800_QueueGetAcc != NULL) {

// 		xQueueSendToFront(s4431800_QueueGetAcc, ( void * ) &messageSend, ( portTickType ) 100 );

// 	}

// 	/* Write command echo output string to write buffer. */
// 	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "\n\r%s\n\r", pcParameter1);
// 	dummy = xWriteBufferLen - dummy;

// 	/* Return pdFALSE, as there are no more strings to return */
// 	/* Only return pdTRUE, if more strings need to be printed */
// 	return pdFALSE;

// }


//command to set the LED lights on board
static BaseType_t prvUARTTestCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long lParam_len;
	const char *cCmd_string;
	size_t dummy = 0;
	struct SendTest SendTest;

	/* Get parameters from command string */
	cCmd_string = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParam_len);

	SendTest.messageType = atoi((const char*) cCmd_string); 

	//send the message to inform the radio task of this change
	if (s4431800_QueueTestSend != NULL) {

		xQueueSendToFront(s4431800_QueueTestSend, ( void * ) &SendTest, ( portTickType ) 100 );

	}

	/* Write command echo output string to write buffer. */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "\n\r%s\n\r", cCmd_string);
	dummy = xWriteBufferLen - dummy;

	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	return pdFALSE;

}
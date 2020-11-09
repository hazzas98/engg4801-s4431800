/********************************************************************************
 * @file    mylib/cli/s4431800_os.c
 * @author  Harry Stokes 44328008
 * @date    04/2020
 * @brief   CLI OS file
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * 
 ******************************************************************************
 */



/* Includes ------------------------------------------------------------------*/
#include "s4431800_os.h"
#include "s4431800_cli.h"
#include "s4431800_spi.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "board.h"

#include "gpio.h"
#include "leds.h"
#include "log.h"
#include "rtc.h"
#include "watchdog.h"

#include "device_nvm.h"

#include "unified_comms_bluetooth.h"
#include "unified_comms_gatt.h"
#include "unified_comms_serial.h"
#include "FreeRTOS_CLI.h"

QueueHandle_t s4431800_QueueSetLED;
QueueHandle_t s4431800_QueueGetTime;
QueueHandle_t s4431800_QueueProccess;
QueueHandle_t s4431800_QueueSend;
QueueHandle_t s4431800_QueueTestSend;

TaskHandle_t xRadioTaskHandle;

struct TimeMessage TimingMessage;
struct LEDMessage LEDsMessage;
struct SendMessage SendMessageStruct;


/* Task Priorities ------------------------------------------------------------*/
#define mainRADIOTASK_PRIORITY					( tskIDLE_PRIORITY + 5 )
#define mainFSMTASK_PRIORITY					( tskIDLE_PRIORITY + 3 )

/* Task Stack Allocations -----------------------------------------------------*/
#define mainRADIOTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5)
#define mainFSMTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 1)


/**
 * @brief Initializes tasks and queues
 * @param None
 * @retval None
 */ 
extern void s4431800_os_init(void) {

	//create queues
	s4431800_QueueSetLED = xQueueCreate(10, sizeof(char[20]));
	s4431800_QueueGetTime = xQueueCreate(10, sizeof(TimingMessage));
    s4431800_QueueProccess = xQueueCreate(10, sizeof(char[20]));
    s4431800_QueueSend = xQueueCreate(10, sizeof(char[20]));
    s4431800_QueueTestSend = xQueueCreate(10, sizeof(char[20]));

    // Create tasks
    xTaskCreate( (void *) &s4431800_task, "Radio", mainRADIOTASK_STACK_SIZE, NULL, mainRADIOTASK_PRIORITY, NULL);
    xTaskCreate( (void *) &s4431800_task2, "CLI", mainRADIOTASK_STACK_SIZE, NULL, mainRADIOTASK_PRIORITY, NULL);

}


/**
 * @brief Reads the input from send_string
 * @param None
 * @retval None
 */
void s4431800_task2(void) {

    int8_t pcOutputString[ MAX_OUTPUT_LENGTH ];
    int8_t pcInputString[ MAX_INPUT_LENGTH ];

    for(;;) {

        if (s4431800_QueueProccess != NULL) {

            if (xQueueReceive( s4431800_QueueProccess, &pcInputString, 10 )) {

                FreeRTOS_CLIProcessCommand(pcInputString, pcOutputString, MAX_OUTPUT_LENGTH);
                // eLog(LOG_APPLICATION, LOG_INFO, "\r\nString: %s\r\n\r\n", pcInputString);
                memset( pcInputString, 0x00, MAX_INPUT_LENGTH );

            }

        }

        vTaskDelay(10);

    }
}



/**
 * @brief Task to handle queues from set LEDS and get Time commands
 * @param None
 * @retval None
 */
void s4431800_task(void) {

    struct TimeMessage TimingRecv;
    struct LEDMessage LEDsRecv;
    // struct SendMessage SendRecv;
    struct SendTest TestRecv;


    for(;;) {

        if (s4431800_QueueTestSend != NULL) {

            if (xQueueReceive( s4431800_QueueTestSend, &TestRecv, 10 )) {

                switch (TestRecv.messageType) {

                    case 0:
                        eLog(LOG_APPLICATION, LOG_ERROR, "\r\n Sending AT! \r\n\r\n");
                        char sendBuffer1[3] = "AT\r";
                        s4431800_hci_send(sendBuffer1, 3);
                        break;

                    case 1: 
                        eLog(LOG_APPLICATION, LOG_ERROR, "\r\n Sending AT! \r\n\r\n");
                        char sendBuffer2[6] = "AT&KO\r";
                        s4431800_hci_send(sendBuffer2, 6);
                        break;

                    case 2:
                        eLog(LOG_APPLICATION, LOG_ERROR, "\r\n Sending AT+CGMI! \r\n\r\n");
                        char sendBuffer3[8] = "AT+CGMI\r";
                        s4431800_hci_send(sendBuffer3, 8);
                        break;

                    case 3:
                        eLog(LOG_APPLICATION, LOG_ERROR, "\r\n Sending AT+SBDWT! \r\n\r\n");
                        char sendBuffer4[21] = "AT+SBDWT=Hello World\r";
                        s4431800_hci_send(sendBuffer4, 21);
                        break;

                    case 4:
                        eLog(LOG_APPLICATION, LOG_ERROR, "\r\n Sending AT+SBDIX! \r\n\r\n");
                        char sendBuffer5[9] = "AT+SBDIX\r";
                        s4431800_hci_send(sendBuffer5, 9);
                        break;

                    default:
                        eLog(LOG_APPLICATION, LOG_ERROR, "\r\n DEFAULT! \r\n\r\n");
                        break;                        
                    
                    }

            }
  
        }  

        // Used for time cli
        if (s4431800_QueueGetTime != NULL) {

            if (xQueueReceive( s4431800_QueueGetTime, &TimingRecv, 10 )) {

                xDateTime_t xDatetime;
            	bRtcGetDatetime(&xDatetime);
            	eRtcPrintDatetime(&xDatetime, LOG_APPLICATION, LOG_ERROR, "Time: ", "\r\n");

            }
  
        }       
        
        // Used for LED cli
        if (s4431800_QueueSetLED != NULL) {

            if (xQueueReceive( s4431800_QueueSetLED, &LEDsRecv, 10 )) {

                // Turn LEDs on
                if ('o' == LEDsRecv.typeAction) {

                    if ('r' == LEDsRecv.color) {
                        vLedsSet( LEDS_RED );
                    } else if ('b' == LEDsRecv.color) {
                        vLedsSet( LEDS_BLUE );
                    } else if ('g' == LEDsRecv.color) {
                        vLedsSet( LEDS_GREEN );
                    }

                // Turn LEDS off
                } else if ('f' == LEDsRecv.typeAction) {

                    if ('r' == LEDsRecv.color) {
                        vLedsOff( LEDS_ALL );
                    } else if ('b' == LEDsRecv.color) {
                        vLedsOff( LEDS_ALL );
                    } else if ('g' == LEDsRecv.color) {
                        vLedsOff( LEDS_ALL );
                    }


                // Toggle LEDs
                } else if ('t' == LEDsRecv.typeAction) {

                    if ('r' == LEDsRecv.color) {
                        vLedsToggle( LEDS_RED );
                    } else if ('b' == LEDsRecv.color) {
                        vLedsToggle( LEDS_BLUE );
                    } else if ('g' == LEDsRecv.color) {
                        vLedsToggle( LEDS_GREEN );
                    }

                }

            }
        }


        // // Queue used for i2creg CLI
        // if (s4431800_QueueSend != NULL) {

        //     if (xQueueReceive( s4431800_QueueSend, &SendRecv, 10 )) {

        //         s4431800_hci_package(SendRecv.readWrite, SendRecv.sid, SendRecv.regaddr, SendRecv.regVal);

        //     }
        // }

        vTaskDelay(10);
    }
}
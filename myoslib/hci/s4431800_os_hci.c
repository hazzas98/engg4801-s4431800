/********************************************************************************
 * @file    mylib/hci/s4431800_os_hci.c
 * @author  Harry Stokes 44318008
 * @date    15/04/2020
 * @brief   os hci driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4431800_os_hci_init()
 * s4431800_hci_package()
 ******************************************************************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "s4431800_hal_hci.h"
#include "s4431800_os_hci.h"
#include "s4431800_hci_packet.h"

#include "uart.h"
#include "unified_comms_serial.h"
#include "gpio.h"
#include "leds.h"
#include "log.h"

/* Task Priorities ------------------------------------------------------------*/
#define mainRADIOTASK_PRIORITY					( tskIDLE_PRIORITY + 5 )
#define mainFSMTASK_PRIORITY					( tskIDLE_PRIORITY + 3 )

/* Task Stack Allocations -----------------------------------------------------*/
#define mainRADIOTASK_STACK_SIZE		        ( configMINIMAL_STACK_SIZE * 5)
#define mainFSMTASK_STACK_SIZE	        	    ( configMINIMAL_STACK_SIZE * 1)

// Queues
QueueHandle_t s4431800_QueueReceived;
QueueHandle_t s4431800_QueueGetAcc;

// Semaphore
SemaphoreHandle_t s4431800_SemaphorePort;
SemaphoreHandle_t s4431800_SemaphoreX;
SemaphoreHandle_t s4431800_SemaphoreY;
SemaphoreHandle_t s4431800_SemaphoreZ;
SemaphoreHandle_t s4431800_SemaphorePacket;


/**
  * @brief  Initialise os hci.
  * @param  None
  * @retval None
  */
extern void s4431800_os_hci_init() {

    // Create the Queues
    s4431800_QueueReceived = xQueueCreate(10, sizeof(char[20]));
    s4431800_QueueGetAcc = xQueueCreate(10, sizeof(char[20]));

    // Create the semaphores
    s4431800_SemaphorePort = xSemaphoreCreateBinary();
    s4431800_SemaphoreX = xSemaphoreCreateBinary();
    s4431800_SemaphoreY = xSemaphoreCreateBinary();
    s4431800_SemaphoreZ = xSemaphoreCreateBinary();
    s4431800_SemaphorePacket = xSemaphoreCreateBinary();

    // Create the tasks in file
    xTaskCreate( (void *) &s4431800_hci_task, "Radio", mainRADIOTASK_STACK_SIZE, NULL, mainRADIOTASK_PRIORITY, NULL);
    xTaskCreate( (void *) &s4431800_hci_task2, "Radio", mainRADIOTASK_STACK_SIZE, NULL, mainRADIOTASK_PRIORITY, NULL);

}


/**
 * @brief Task to read accelerometer values
 * @param None
 * @retval None
 */
void s4431800_hci_task(void) {

    struct accRead recvAcc;
    

    for(;;) {

        if (s4431800_QueueGetAcc != NULL) {

            if (xQueueReceive( s4431800_QueueGetAcc, &recvAcc, 10 )) {

                char packageSend[7];

                if (recvAcc.readWrite == 'r') {

                    switch(recvAcc.axis) {

                        case 'x':
                            strcpy(packageSend, "\xAA\x01\x02\x01\xD5\x28");
                            s4431800_hci_send(packageSend, 7);
                            vTaskDelay(200);
                            break;
                        case 'y':
                            strcpy(packageSend, "\xAA\x01\x02\x01\xD5\x2A");
                            s4431800_hci_send(packageSend, 7);
                            vTaskDelay(200);
                            break;
                        case 'z':
                            strcpy(packageSend, "\xAA\x01\x02\x01\xD5\x2C");
                            s4431800_hci_send(packageSend, 7);
                            vTaskDelay(200);
                            break;
                        case 'a':
                            recvAcc.axis = 'x';
                            xQueueSendToFront(s4431800_QueueGetAcc, ( void * ) &recvAcc, ( portTickType ) 100 );
                            recvAcc.axis = 'y';
                            xQueueSendToFront(s4431800_QueueGetAcc, ( void * ) &recvAcc, ( portTickType ) 100 );
                            recvAcc.axis = 'z';
                            xQueueSendToFront(s4431800_QueueGetAcc, ( void * ) &recvAcc, ( portTickType ) 100 );
                            break;

                    }

                }

            }

        }

 

        vTaskDelay(10);

    }
}

/**
 * @brief Handles the event group[]
 * @param None
 * @retval None
 */
void s4431800_hci_task2(void) {

    EventBits_t uxBits;
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;

    for(;;) {

        char packageSend[7];

        uxBits = xEventGroupWaitBits(xCreatedEventGroup, BIT_0 | BIT_1 | BIT_2 | BIT_3, pdTRUE, pdFALSE, xTicksToWait);


        if ((uxBits & BIT_3) == BIT_3) {
            // do Nothing or break cycle
        } else if ((uxBits & (BIT_0 | BIT_1)) == (BIT_0 | BIT_1)) {
            strcpy(packageSend, "\xAA\x01\x02\x06\x00\x00");
            s4431800_hci_send(packageSend, 7);
            xEventGroupSetBits(xCreatedEventGroup, BIT_2);
        } else if ((uxBits & BIT_0) == BIT_0) {
            strcpy(packageSend, "\xAA\x01\x06\x01\xD5\x28");
            s4431800_hci_send(packageSend, 7);
            xEventGroupSetBits(xCreatedEventGroup, BIT_1);
        } else if ((uxBits & BIT_1) == BIT_1) {
            strcpy(packageSend, "\xAA\x01\x03\x03\xBB\x28");
            s4431800_hci_send(packageSend, 7);
            xEventGroupSetBits(xCreatedEventGroup, (BIT_1 | BIT_0));
        } else if ((uxBits & BIT_2) == BIT_2) {
            float ultraVal = ultra_value();
            float pitch = pitch_value();
            float roll = roll_value();
            double preassure = (float) preassure_value() / 4096;
            float altitude = ((pow((1025.0/ preassure), (1.0/5.257)) - 1) * (20.0 + 273.15)) / 0.0065;
            double pitchSin = sin((double) pitch);
            double rollTan = tan((double) roll);
            double theta = atan2(pitchSin, rollTan);
            double altitude_fine = (double) ultraVal * cos(theta);
            eLog(LOG_APPLICATION, LOG_ERROR, "\r\nUltrasonic %f, pitch = %f, roll = %f\r\n", (double) ultraVal, (double) pitch / 100, (double) roll / 100);
            eLog(LOG_APPLICATION, LOG_ERROR, "Pressure %f --> Altitude %fm\r\n", (double) preassure, (double) altitude);
            eLog(LOG_APPLICATION, LOG_ERROR, "Altitude Fine %fm\r\n", (double) altitude_fine);
            xEventGroupSetBits(xCreatedEventGroup, BIT_0);
        }

        vTaskDelay(1000);

    }
}



/**
  * @brief  Format package to send
  * @param  readWrite - read or write 
  * @param  sid - sensor ID
  * @param  addressVal - register address
  * @param  regVal - Value to write to register
  * @retval None
  */
extern void s4431800_hci_package(int readWrite, int sid, char addressVal, char regVal) {

    vLedsToggle( LEDS_GREEN);

    char packageSender[8];
    char packageLoad[8];
    strcpy(packageSender, "\xAA\x01");

    if (readWrite == 114){

        switch(sid){
            case 1:
                strcpy(packageLoad, "\x01\x01\xD5");
                break;
            case 2:
                strcpy(packageLoad, "\x01\x02\x3D");
                break;
            case 3:
                strcpy(packageLoad, "\x01\x03\xBB");
                break;
            case 4:
                strcpy(packageLoad, "\x01\x04\x53");
                break;
            case 5:
                strcpy(packageLoad, "\x01\x05\xBF");
                break;
        }

        strcat(packageSender, packageLoad);
        strcat(packageSender, &addressVal);
        s4431800_hci_send(packageSender, 6);

    } else if (readWrite == 119) {

        switch(sid){
            case 1:
                strcpy(packageLoad, "\x04\x01\xD4");
                break;
            case 2:
                strcpy(packageLoad, "\x04\x02\x3C");
                break;
            case 3:
                strcpy(packageLoad, "\x04\x03\xBA");
                break;
            case 4:
                strcpy(packageLoad, "\x04\x04\x52");
                break;
            case 5:
                strcpy(packageLoad, "\x04\x05\xBE");
                break;
        }

        strcat(packageSender, packageLoad);
        strcat(packageSender, &addressVal);
        strcat(packageSender, &regVal);
        s4431800_hci_send(packageSender, 7);

    }

}
/********************************************************************************
 * @file    mylib/hci/s4431800_os_hci.h
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

#ifndef OS_HCI_H
#define OS_HCI_H

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#include "FreeRTOS_CLI.h"

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   100

struct receivedChar {
    char recv;
};

struct accRead {
    char readWrite;
    char axis;
};

extern QueueHandle_t s4431800_QueueReceived;
extern QueueHandle_t s4431800_QueueGetAcc;

extern SemaphoreHandle_t s4431800_SemaphorePort;
extern SemaphoreHandle_t s4431800_SemaphoreX;
extern SemaphoreHandle_t s4431800_SemaphoreY;
extern SemaphoreHandle_t s4431800_SemaphoreZ;
extern SemaphoreHandle_t s4431800_SemaphorePacket;

extern void s4431800_os_hci_init();

extern void s4431800_hci_task(void);
extern void s4431800_hci_task2(void);

extern void s4431800_hci_package(int readWrite, int sid, char addressVal, char regVal);


#endif
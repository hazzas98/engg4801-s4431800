#ifndef OS_HEADER_H
#define OS_HEADER_H

/* Includes ------------------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#include "FreeRTOS_CLI.h"

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   100

struct LEDMessage {
    char typeAction;
    char color;
};

struct TimeMessage {
    char typeAction;
};

struct SendMessage {
    char readWrite;
    int sid;
    int regaddr;
    int regVal;
};

struct SendTest {
    int messageType;
};


extern QueueHandle_t s4431800_QueueSetLED;
extern QueueHandle_t s4431800_QueueGetTime;
extern QueueHandle_t s4431800_QueueProccess;
extern QueueHandle_t s4431800_QueueSend;
extern QueueHandle_t s4431800_QueueTestSend;

extern TaskHandle_t xRadioTaskHandle;

/* External function prototypes -----------------------------------------------*/
extern void s4431800_os_init(void);
extern void s4431800_task(void);
extern void s4431800_task2(void);

#endif
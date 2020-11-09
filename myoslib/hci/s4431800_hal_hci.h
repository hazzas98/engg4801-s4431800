/********************************************************************************
 * @file    mylib/hci/s4431800_hal_hci.h
 * @author  Harry Stokes 44318008
 * @date    15/04/2020
 * @brief   hal hci driver
 *
 ******************************************************************************
 *     EXTERNAL FUNCTIONS
 ******************************************************************************
 * s4431800_hal_hci_init()
 * s4431800_hci_send()
 * s4431800_hci_x_value()
 * s4431800_hci_y_value()
 * s4431800_hci_z_value()
 * s4431800_hci_port_value()
 ******************************************************************************
 */

#ifndef S4431800_HAL_HCI_H
#define S4431800_HAL_HCI_H

#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
// #include "FreeRTOS_CLI.h"
#include "event_groups.h"
#include "tdf.h"

extern EventGroupHandle_t xCreatedEventGroup;

// extern tdf_lsm6dsl_t xLSM6DSL;
// extern tdf_lps22hb_all_t xLPS22HB_ALL;
// extern tdf_3d_pose_t x3D_POSE;

/* Internal function prototypes -----------------------------------------------*/
void s4431800_hci_handler(char recvChar);
void s4431800_hal_task(void);


/* External function prototypes -----------------------------------------------*/
extern void s4431800_hal_hci_init(void);
extern void s4431800_hci_send(char * message, int count);
extern void buttonPress();

extern float ultra_value(void);
extern float pitch_value(void);
extern float roll_value(void);
extern uint32_t preassure_value(void);

#define UART1_RX_PIN        (xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 0, 8 )}
#define UART1_TX_PIN   		(xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 0, 6 )}
#define UART1_RTS_PIN   	(xGpio_t){.ucPin = NRF_GPIO_PIN_MAP( 1, 1 )}


#endif
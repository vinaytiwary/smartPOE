/*
 * _config.h
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES__CONFIG_H_
#define SOURCES__CONFIG_H_

#include "stdbool.h"
#include "stdint.h"
#include "inc/hw_memmap.h"
#include "driverlib\gpio.h"
#include "driverlib\sysctl.h"
#include "driverlib\timer.h"

#include "driverlib\i2c.h"
//#include "evse_io_exp.h"

// #define NEW_BOARD
#define WEB_COMMS_SCHEDULAR_TIME (50)

//************************ CLOCK ********************//
#define CLOCK_CONFIGURATION         (SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ)
#define SYSTEM_CLOCK                SysCtlClockGet()

//********************* SYSTICK ********************//
#define SYSTICK_FREQ                1000                //Hz


#define VER_1   (1)
#define VER_2   (2)
#define VER_3   (3)

#define HW_VER  (VER_3)
#define UART0   (0)
#define UART1   (1)
#define UART2   (2)
#define UART3   (3)
#define UART4   (4)
#define UART5   (5)
#define UART6   (6)
#define UART7   (7)

//********************* ADC ********************//
#define ADC_FILTER_1

//////////////////////////////////////////////////
#define FLASH_EN
#define RTC_ENABLE
#define EARTH_DETECT_EN
#define OFFLINE_MODE_EN
// #define UART_TX_IRQ_EN
#define USE_NETCLOSE
// #define CHECKSTR_EXP


#endif /* SOURCES__CONFIG_H_ */

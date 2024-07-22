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

// #define USE_NETCLOSE //To use or to not use???

// #define CHECKSTR_EXP

// #define ETHERNET_EN

// #define ENABLE_WDT_RESET

#ifdef ENABLE_WDT_RESET
#define WDT_IRQ_MODE
#endif  //ENABLE_WDT_RESET

#define USE_FLASH_FOR_FREQ_UPD_DATA //Move frequently updated data from eeprom to flash

#define ADC_EN

#define EVSE_TEST_BOARD         (0)
#define TIOT_V2_00_BOARD        (1)
#define HW_BOARD                (TIOT_V2_00_BOARD)
// #define HW_BOARD                (EVSE_TEST_BOARD)

#define EVSE_ADC_TEST_PINS      (0)
#define TIOT_ADC_PINS           (1)

#if HW_BOARD == TIOT_V2_00_BOARD
#define TARGET_ADC_PINS         (TIOT_ADC_PINS)
#elif   HW_BOARD == EVSE_ADC_TEST_PINS
#define TARGET_ADC_PINS      (EVSE_ADC_TEST_PINS)
#endif

// #define ENABLE_GLCD

#define ECHO_LTE_TO_DBG

#define SMPS_HARDCODED        //only for testing
#define RTR_HARDCODED         //only for testing

// #define ODU_RELAY_TST

#define ENABLE_CLI_SEI

#ifndef ENABLE_GLCD
#define LEDS_ON_GLCD_PINS
#endif  //ENABLE_GLCD

#define ENABLE_BCD_SW

#endif /* SOURCES__CONFIG_H_ */

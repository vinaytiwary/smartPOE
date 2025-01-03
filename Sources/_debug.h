/*
 * _debug.h
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES__DEBUG_H_
#define SOURCES__DEBUG_H_

#include "_config.h"

// // #define DEBUG_ADC
#define DEBUG_ADC_SIG
// // #define DEBUG_AC_ADC
// // #define DEBUG_ODU_ADC

#define DEBUG_ADC_READ_FAIL

// #define DEBUG_RTC
// // #define DEBUG_LCD_FREEZE
// // #define DEBUG_LCD_BUFF
#define DEBUG_GPIO

#define DEBUG_E2P
#define DEBUG_PC_CMDS
// // // #define DEBUG_CLOUD_CFG_DECODE

#define DEBUG_TCP_HANDLER
#define DEBUG_GPRS_CONN
#define DEBUG_GPRS_CONN_STS
#define DEBUG_TCP_CONN
#define DEBUG_WEBSOCKET_CONN
#define DEBUG_GPRS_PING
#define DEBUG_TCP_SEND
#define DEBUG_GPRS_DATA_UPLOAD
#define DEBUG_QCIPOPEN_RESP_DECODE
// // // // #define DEBUG_JSON_PKT_PREP
// // // // // #define DEBUG_WS_DISCONN
// // // // // #define DEBUG_TCP_DISCONN

// // // // // // #define DEBUG_EPOCHTIME

// // #define DEBUG_GPS_HANDLER
// // // // //#define DEBUG_GPS_DECODE
#define DEBUG_GET_LOC

// // // #define DEBUG_WEBB_COMM

// // #define DEBUG_ETHERNET
// // #define DEBUG_ETHER_TCP_CONN
// // #define DEBUG_ETHER_WS
// // #define DEBUG_ETHER_PS
// // #define DEBUG_ETHER_WSD
// // #define DEBUG_ETHER_TCP_SEND

#define DEBUG_SERVER_QUERY

// // #define DEBUG_QUERY_DECODE

// // // #define DEBUG_WEB_PREP

#define DEBUG_QUERY_STATES

// #define DEBUG_WDT

// // #define DEBUG_GPRS_RX

// #define DEBUG_FLASH_INIT
// #define DEBUG_SPCL_FLASH
// #define _DEBUG_FLASH_UART
// #define DEBUG_PASS
// #define DEBUG_FLASH
// #define DEBUG_FL_UNSENT
// #define DEBUG_READ_FREQ

#define DEBUG_BCD_SEL_SW
// // // #define DEBUG_RESTART
#define DEBUG_RELAY
// #define DEBUG_EARTHDETECT
// // // // #define DEBUG_FREQ_MEAS
// // // #define DEBUG_MAINS_FAULT
#ifdef ENABLE_GLCD
// #define DEBUG_RELAY_STATE_LCD
#endif  //ENABLE_GLCD
// // // #define DEBUG_RELAY_STATE

#define DEBUG_ALARMS

// // #define DEBUG_MAIN_ADC

// #define DEBUG_ODUM_LED

// #define DEBUG_SW_PRESS_EVENT

#define DEBUG_PWRUP_RST_REASON

#endif /* SOURCES__DEBUG_H_ */

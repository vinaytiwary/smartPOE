/*
 * Web_Comm.h
 *
 *  Created on: May 1, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_WEB_COMM_H_
#define SOURCES_WEB_COMM_H_


#include "_config.h"
#include "_common.h"

#ifndef ETHERNET_EN

#if 0
#define WEB_STATE_MC_TIME   (50)

#define GPS_READ_RATE_NC    ((2000)/ WEB_STATE_MC_TIME)
#define GPS_READ_RATE_C     ((10 * 1000)/ WEB_STATE_MC_TIME)
#define WARMUP_SECS         ((10*1000UL)/WEB_STATE_MC_TIME)

#define GPS_FAIL_TIMEOUT    ((20 * 1000)/WEB_STATE_MC_TIME)


typedef enum
{
    CONNECT_NO_STATE,
    CONNECT_POWER_ON,
    CONNECT_LOCATION,
    CONNECT_DATA_UPLOAD,
}conn_state_t;

void manage_gps_gprs(void);

#else


#define LTE_WARMUP_TIME (10000/GPRS_STATE_MC_TIME)

#define WEB_STATE_MC_TIME	(WEB_COMMS_SCHEDULAR_TIME)
// #define GPS_FAIL_TIMEOUT	(1000/WEB_STATE_MC_TIME)
#define GPS_FAIL_TIMEOUT	((20*1000UL)/WEB_STATE_MC_TIME)

#define GPS_READ_RATE_C		((10 * 1000UL)/ WEB_STATE_MC_TIME)
#define WARMUP_SECS			((10*1000UL)/WEB_STATE_MC_TIME)

// typedef enum
// {
//     GPRS_CONNCT_RESET,
//     GPRS_CONNCT_CMD_AT,
//     GPRS_CONNCT_RSP_AT,
//     GPRS_CONNCT_CMD_ECHO_OFF,
//     GPRS_CONNCT_RSP_ECHO_OFF,
// }AT_init_states_t;

typedef enum
{
	//CONNECT_POWER_ON,		// 24/1/19 SK:
	CONNECT_BEGIN,			//PP added on 05-10-23 (among other webcomm, gprs, gps chnges regarding their return statuses)
	CONNECT_LOCATION,
	CONNECT_DATA_UPLOAD,
	CONNECT_LOCATION_WAIT,	// PP(02-08-22)
}conn_state_t;
void manage_gps_gprs(void);

#endif  //if 0
#endif 	// ifNOTdef ETHERNET_EN

#endif /* SOURCES_WEB_COMM_H_ */

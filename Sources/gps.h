/*
 * gps.h
 *
 *  Created on: Apr 25, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_GPS_H_
#define SOURCES_GPS_H_

#include "_config.h"

// #ifndef ETHERNET_EN

#include "gprs.h"

#ifdef ENABLE_GPS

#define LTE_WARMUP_TIME (10000/GPRS_STATE_MC_TIME)

// #define GNS_PKT_EN	//PP(14-02-2023): this is declared here now.

// #define LAT_LEN						(11)			// No NULL always use memcpy
// #define LONG_LEN					(12)			// No NULL always use memcpy

#ifdef GNS_PKT_EN
#define MIN_COMMAS					(14)			//Fixed commas required for AT+CGNSSINFO pckt.
#define MAX_COMMAS					(16)			//Although the datasheet example has 15 commas max, 16 max are observed.
#else
#define MIN_COMMAS					(8)				//Fixed commas required for AT+CGPSINFO pckt.
#define MAX_COMMAS					(8)				//the observed max and the datasheet example also have the same commas.
#endif	//GNS_PKT_EN

#define GPS_RETRY_CNT		(3)
#define GPS_TIMEOUT			(1*1000/GPRS_STATE_MC_TIME)

#define GPS_RDY_RSP_TIMEOUT	(9000 / GPRS_STATE_MC_TIME)


//#define MAX_DATE_GPS_SIZE	(6)
//#define MAX_TIME_SIZE		(6)

#define MAX_DATE_GPS_SIZE	(7)		//6 Bytes date + 1 Byte null
#define MAX_TIME_SIZE		(9)		//6 Bytes time, skip 1 Byte '.' + 2 Bytes '0' + 1 Byte null

#define LATITUDE_DP				(6)

#define SPEED_DP				(2)
#define GPS_LAT_STATUS_BIT		(10)
#define GPS_LONG_STATUS_BIT		(11)
#define GPS_POSITION_STATUS_BIT	(12)
#define GPS_POSITION_BIT		(13)

typedef enum
{
	PARSE_FAIL,		//0
	GOT_LATLONG,	//1
	DATETIME_FAIL,	//2
	GOT_DATETIME,	//3
	ALT_FAIL,		//4
	GOT_ALT,		//5
	SOG_FAIL,		//6
	GOT_SOG,		//7
	COG_FAIL,		//8
	GOT_COG,		//9
}gps_decode_result_t;

typedef enum
{
	GPS_CONNCT_RESET,
	GPS_CMD_ECHO_OFF,
	GPS_RSP_ECHO_OFF,
	GPS_CMD_POWER,
	GPS_RSP_POWER,
	GPS_RSP_1_POWER,
	// GPS_CMD_STATUS,
	// GPS_RSP_STATUS,
	// GPS_RSP_1_STATUS,
	GPS_CMD_LOCATION,
	GPS_RSP_LOCATION,
	// GPS_RSP_1_LOCATION,
	// GPS_CMD_COLD_RESET,
	// GPS_RSP_COLD_RESET,
	// GPS_CMD_HOT_RESET,
	// GPS_RSP_HOT_RESET,
	GPS_CMD_POWER_OFF,
	GPS_RSP_POWER_OFF,
}gps_handler_state_t;

typedef enum
{
	GPS_PASS,
	GPS_FAIL,
	GPS_IN_PRG,
	GPS_WAIT
}gps_status_t;

enum {GPS_NO_NEW_MSG, GPS_MATCH_FAIL, GPS_MATCH_OK} ;

typedef struct
{
  unsigned char gps_run_sts;
  unsigned char date[MAX_DATE_GPS_SIZE];
  unsigned char utc_time[MAX_TIME_SIZE];
  unsigned char latitude[LAT_LEN];
  unsigned char longitude[LONG_LEN];
  unsigned char msl_altitude[6];
  unsigned char sog[6];
  unsigned char cog[6];
  unsigned char N_S;
  unsigned char E_W;
}__attribute__((packed))gps_info_t;

typedef struct
{
    char yy;
    char mm;
    char dd;
    char hr;
    char min;
    char sec;
    char update_time_aval;
}__attribute__((packed))gps_date_time_t;

typedef struct
{
	gps_info_t gps_info;
	unsigned char gps_ready;
	char errcode;
	unsigned int timeout;
	gprs_status_t gps_status;
	bool getLoc_sts;
}__attribute__((packed))gps_t;

typedef struct
{
	int32_t Latitude;
	int32_t Longitude;
}__attribute__((packed))gps_data_t;

gps_status_t gps_handler(void);

gps_decode_result_t gps_pkt_parsing(gps_info_t *temp_gps, char *tmpstr);

bool get_location(void);

void utcTOlocal(gps_date_time_t *timeT);

void updateGpsDateTimeToBuff(gps_date_time_t *date_time);

gprs_status_t get_gps_status();
void set_gps_status(gprs_status_t);

bool get_loc_status(void);
void set_loc_status(bool sts);

// #endif 	// ifNOTdef ETHERNET_EN

#endif	// ENABLE_GPS

#endif /* SOURCES_GPS_H_ */

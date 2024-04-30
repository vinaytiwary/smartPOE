/*
 * Clock.h
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_CLOCK_H_
#define SOURCES_CLOCK_H_

#include "stdint.h"
//#include "DateTime.h"

#include "time.h"
 
#define UNIX_OFFSET   946684800
#define SECONDS_OF_LOCAL_TIME    (19800)


#define DEFAULT_YEAR    (24)

#define SEC_IN_MIN      (60UL)
#define MIN_IN_HR       (60UL)
#define HR_IN_DAY       (24UL)
#define DAYS_IN_MON     (30UL)
#define MON_IN_YR       (12UL)
#define DAYS_IN_YR      (365UL)

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
}time_stamp_t;

typedef struct
{
    char Date[11];
    char Time[12];  //hh:mm:ss AM/PM
    char dan;
    char ampm, sec, minute, hour, date, month, year;
}time_main_t;

typedef struct
{
    char diffHrs;
    char diffMins;
    long  diffDays;
}timeDiff_t;

void update_date_time(void);
void sync_date_time(void);
timeDiff_t calcTimeDiff(time_stamp_t start, time_stamp_t stop);
unsigned int getDaysOfMonth(char month, unsigned int year);
char check_expire_time(time_stamp_t curr_time, time_stamp_t exp_time);
bool isDT_ok(void);
void get_present_time(time_stamp_t *time_stamp);
void update_rtc(char *arr, int indx);
// void get_ocpp_date_time(void);
void time_sync(void);
// void update_ocpp_date_time(void);
time_stamp_t StrToTimeStamp(uint8_t *str);
void add_date_time(time_stamp_t *updated_date_time, time_stamp_t *date_time, uint32_t diff);
uint8_t is_leap_year(uint8_t year);
unsigned char check_date_time(char *str, char *arr);
time_t convertToEpochTime(const time_stamp_t *timestamp);
time_t asUnixTime(int year, int mon, int mday, int hour, int min, int sec);
uint8_t check_dateTime_data(time_stamp_t *valid_time);

#endif /* SOURCES_CLOCK_H_ */

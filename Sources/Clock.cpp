/*
 * Clock.cpp
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */
/*
 * evse_clock.c
 *
 *  Created on: 02-Aug-2022
 *      Author: ADMIN
 */


#include "stdint.h"
#include "stdbool.h"
#include <ctype.h>
#include <time.h>
#include "_debug.h"
#include "HW_pins.h"
// #include "ocpp.h"
#include "string.h"
#include "UartCore.h"
#include "stdio.h"
#include "stdlib.h"
//#include "DateTime.h"
// #include "evse_mcu_periph.h"
#include "DS3231.h"
#include "Clock.h"
#include "_common.h"
#include "gprs.h"
#include "gps.h"

time_stamp_t time_stamp;
// extern OCPP_Data_t OCPP_Data;
time_main_t time_main;

extern gps_date_time_t gps_date_time;
extern gprs_date_time_t gprs_date_time;
void update_date_time(void)
{
    unsigned char date;
    time_stamp.sec++;
    if(time_stamp.sec >= 60)
    {
        time_stamp.min++;
        time_stamp.sec = 0;
    }

    if(time_stamp.min >= 60)
    {
        time_stamp.hour++;
        time_stamp.min = 0;
    }

    if(time_stamp.hour >= 24)
    {
        time_stamp.date++;
        time_stamp.hour = 0;
    }

    if((time_stamp.month == 1) || (time_stamp.month == 3)||(time_stamp.month == 5) || (time_stamp.month == 7)||(time_stamp.month == 8) || (time_stamp.month == 10) || (time_stamp.month == 12))
    {

        date = 31;
    }
    else if((time_stamp.month == 4) || (time_stamp.month == 6)||(time_stamp.month == 9) || (time_stamp.month == 11))
    {

        date = 30;
    }
    else if(time_stamp.month == 2)
    {

        if(time_stamp.year%4 == 0)
        {
            date = 29;
            if(time_stamp.year%100 == 0)
            {
                date = 28;
            }
            if(time_stamp.year%400 == 0)
            {
                date = 29;
            }
        }
        else
        {
            date = 28;
        }
    }

    if(time_stamp.date > date)
    {
        time_stamp.month++;
        time_stamp.date = 1;
    }

    if(time_stamp.month > 12)
    {
        time_stamp.year++;
        time_stamp.month = 1;
    }
    if(time_stamp.year > 99)
    {
        time_stamp.year = 0;
    }
/*
#ifdef DEBUG_TIME
    char buff[80];
    memset(buff,0,sizeof(buff));
    sprintf(buff,"\n%02d/%02d/20%02d %02d:%02d:%02d",time_stamp.date,time_stamp.month,time_stamp.year,time_stamp.hour,time_stamp.min,time_stamp.sec);
    vUART_SendStr(DEBUG_UART_BASE,buff);
#endif
*/
}


// void sync_date_time(void)
// {
// #ifdef DEBUG_TIME
//     vUART_SendStr(DEBUG_UART_BASE,"ocpp_time:");
//     vUART_SendStr(DEBUG_UART_BASE,OCPP_Data.Time_stamp);
// #endif
//     char temp[10];
//     unsigned int index = 0;
//     index += 2;   //skips 20
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //year
//     time_stamp.year = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //month
//     time_stamp.month = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //date
//     time_stamp.date = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //hour
//     time_stamp.hour = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //min
//     time_stamp.min = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //sec
//     time_stamp.sec = atoi(temp);
// }

unsigned char check_date_time(char *str, char *arr)
{
    int i = 0;
    char *ptr = arr;
    *ptr = 0;
    for (i = 0; i < 17; i++)
    {
        if(isdigit(str[i]))
        {
            *ptr *= 10;
            *ptr += (str[i] - '0');
        }
        else
        {
            if ((i + 1) % 3)
            {
                break;
            }
            else
            {
                switch(i)
                {
                    case 2:
                    case 5:
                        if(str[i] == '/')
                        {
                            ptr++;
                            *ptr = 0;
                        }
                    break;
                    case 8:
                        if(str[i] == ',')
                        {
                            ptr++;
                            *ptr = 0;
                        }
                    break;
                    case 11:
                    case 14:
                        if(str[i] == ':')
                        {
                            ptr++;
                            *ptr = 0;
                        }
                    break;
                    default:
                    break;
                }
                /*if ((((i == 2) || (i == 5)) && (str[i] == '/')) || ((i == 8) && (str[i] == ',')) || (((i == 11) || (i == 14)) && (str[i] == ':')))
                {

                }
                ptr++;
                *ptr = 0;*/
            }
        }
    }
    if (i == 17)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

time_t convertToEpochTime(const time_stamp_t *timestamp) 
{
    struct tm timeinfo = {0};
	time_t epoch_time = 0;

#ifdef DEBUG_EPOCHTIME
    // vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\n1CET:\n");
    // vUART_SendInt(DEBUG_UART_BASE,timestamp->date);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,timestamp->month);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,timestamp->year);
    // vUART_SendChr(DEBUG_UART_BASE,',');
    // vUART_SendInt(DEBUG_UART_BASE,timestamp->hour);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,timestamp->min);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,timestamp->sec);
#endif  //DEBUG_RTC

    // Convert year (assuming 2000 as the base year for the example)
	timeinfo.tm_year = (timestamp->year + 100); // Adjust for the base year
    // timeinfo.tm_year = (timestamp->year); // Adjust for the base year

	// Convert month, day, hour, minute, and second
	timeinfo.tm_mon = timestamp->month - 1 ; // Adjust for zero-based month
	timeinfo.tm_mday = timestamp->date;
	timeinfo.tm_hour = timestamp->hour;
	timeinfo.tm_min = timestamp->min;
	timeinfo.tm_sec = timestamp->sec;

    _tz.timezone = 0;
    _tz.daylight = 0;

	// Set other fields to reasonable values
	//timeinfo.tm_isdst = -1;

#ifdef DEBUG_EPOCHTIME
    // vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\n2CET:\n");
    // vUART_SendInt(DEBUG_UART_BASE,timeinfo.tm_mday);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,timeinfo.tm_mon);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,timeinfo.tm_year);
    // vUART_SendChr(DEBUG_UART_BASE,',');
    // vUART_SendInt(DEBUG_UART_BASE,timeinfo.tm_hour);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,timeinfo.tm_min);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,timeinfo.tm_sec);
    // vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ntm_struct:");
    // vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&timeinfo, sizeof(tm));
#endif  //DEBUG_RTC

	// Convert struct tm to epoch time
	// epoch_time = mktime(&timeinfo) + UNIX_OFFSET - SECONDS_OF_LOCAL_TIME;
    // epoch_time = mktime(&timeinfo) + UNIX_OFFSET;
    epoch_time = mktime(&timeinfo) - SECONDS_OF_LOCAL_TIME;
    // epoch_time = mktime(&timeinfo);
	
//  #ifdef DEBUG_ONLINE_PKT
//  UWriteString((char *)"P:",DBG_UART);
//  UWriteInt(epoch_time,DBG_UART);
//  #endif
	return epoch_time;
}

time_t asUnixTime(int year, int mon, int mday, int hour, int min, int sec) 
{
#ifdef DEBUG_EPOCHTIME
    // vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\n1aUT:\n");
    // vUART_SendInt(DEBUG_UART_BASE,mday);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,mon);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,year);
    // vUART_SendChr(DEBUG_UART_BASE,',');
    // vUART_SendInt(DEBUG_UART_BASE,hour);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,min);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,sec);
#endif  //DEBUG_RTC

    struct tm   t;
    // t.tm_year = year - 1900;
    t.tm_year = (year + 2000) - 1900;
    // t.tm_year = (year + 2000);
    // t.tm_year = (year);
    t.tm_mon =  mon - 1;        // convert to 0 based month
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1;            // Is Daylight saving time on? 1 = yes, 0 = no, -1 = unknown

    _tz.timezone = 0;
    _tz.daylight = 0;

#ifdef DEBUG_EPOCHTIME
    // vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\n2aUT:\n");
    // vUART_SendInt(DEBUG_UART_BASE,t.tm_mday);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,t.tm_mon);
    // vUART_SendChr(DEBUG_UART_BASE,'-');
    // vUART_SendInt(DEBUG_UART_BASE,t.tm_year);
    // vUART_SendChr(DEBUG_UART_BASE,',');
    // vUART_SendInt(DEBUG_UART_BASE,t.tm_hour);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,t.tm_min);
    // vUART_SendChr(DEBUG_UART_BASE,':');
    // vUART_SendInt(DEBUG_UART_BASE,t.tm_sec);
    // vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ntm_struct:");
    // vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&t, sizeof(tm));
#endif  //DEBUG_RTC
 
    return mktime(&t);          // returns seconds elapsed since January 1, 1970 (begin of the Epoch)
}

timeDiff_t calcTimeDiff(time_stamp_t start, time_stamp_t stop)
{
    timeDiff_t diff = {0, 0, 0};
    unsigned int startMonth=0, stopMonth=0, startYear=0, stopYear=0;
    char invalid=0;

    if(start.year <= stop.year)
    {
        startMonth = start.month;
        startYear = start.year;
        stopMonth = stop.month;
        stopYear = stop.year;

        if((startMonth <= 0) || (startMonth > 12) ||
            ((stopMonth <= 0) || (stopMonth > 12)))
        {
            invalid = 1;
        }
        else
        {
            while((startYear != stopYear) || (startMonth != stopMonth))
            {
                if(startYear == stopYear)
                {
                    if(startMonth == stopMonth)
                    {
                        invalid = 1;
                        break;
                    }
                }
                diff.diffDays += getDaysOfMonth(startMonth, startYear);

                ++startMonth;

                if(startMonth > 12)
                {
                    startMonth = 1;
                    ++startYear;
                }
            }
        }
        diff.diffDays -= start.date;      //We added the whole Month
        diff.diffDays += stop.date;       //We didn't add anything

        if(start.hour <= stop.hour)
        {
            diff.diffHrs = stop.hour - start.hour;
        }
        else
        {
            --diff.diffDays;
            diff.diffHrs = (24 - start.hour) + stop.hour;
        }

        if(start.min <= stop.min)
        {
            diff.diffMins = stop.min - start.min;
        }
        else
        {
            if(diff.diffHrs!=0)
                --diff.diffHrs;
            else
                --diff.diffDays;
            diff.diffMins = (60 - start.min) + stop.min;
        }
        if(invalid || (diff.diffDays < 0))
        {
            diff.diffDays = 0;
            diff.diffHrs = 0;
            diff.diffMins = 0;
        }
    }
    return diff;
}

unsigned int getDaysOfMonth(char month, unsigned int year)
{
    unsigned int days = 0;
    switch(month)
    {
        case 1  :
        case 3  :
        case 5  :
        case 7  :
        case 8  :
        case 10 :
        case 12 :   days += 31; break;
        case 2  :   days += 28; break;
        case 4  :
        case 6  :
        case 9  :
        case 11 :   days += 30; break;
    }
    if(month == 2)
    {
        if(!(year % 100))
        {
            if(!(year % 400))
            {
                days += 1;
            }
        }
        else
        {
            if(!(year % 4))
            {
                days += 1;
            }
        }
    }
    return days;
}

char check_expire_time(time_stamp_t curr_time, time_stamp_t exp_time)
{
    char time_expired = FALSE;

    if(curr_time.year < exp_time.year)
    {
        time_expired = FALSE;
    }
    else if(curr_time.year > exp_time.year)
    {
        time_expired = TRUE;
    }
    else
    {
        if(curr_time.month < exp_time.month)
        {
            time_expired = FALSE;
        }
        else if(curr_time.month > exp_time.month)
        {
            time_expired = TRUE;
        }
        else
        {
            if(curr_time.date < exp_time.date)
            {
                time_expired = FALSE;
            }
            else if(curr_time.date > exp_time.date)
            {
                time_expired = TRUE;
            }
            else
            {
                if(curr_time.hour < exp_time.hour)
                {
                    time_expired = FALSE;
                }
                else if(curr_time.hour > exp_time.hour)
                {
                    time_expired = TRUE;
                }
                else
                {
                    if(curr_time.min < exp_time.min)
                    {
                        time_expired = FALSE;
                    }
                    else if(curr_time.min > exp_time.min)
                    {
                        time_expired = TRUE;
                    }
                    else
                    {
                        if(curr_time.sec < exp_time.sec)
                        {
                            time_expired = FALSE;
                        }
                        else if(curr_time.sec > exp_time.sec)
                        {
                            time_expired = TRUE;
                        }
                        else
                        {
                            time_expired = TRUE;
                        }
                    }
                }
            }
        }
    }
    return time_expired;
}
#ifdef RTC_ENABLE
void getDateTime(void)
{
    char error = 0;
    int i;
    uint8_t data[8];
    for (i = 0; i < 3; i++)
    {
        error = 0;
        //RTC_Read(0x00, data, 7);
        Read_RTC(data, 7);
        /*{
            sei();
        }*/

        time_main.sec = ((((data[0] & 0b01110000) / 16) * 10) + (data[0] & 0b00001111));
        time_main.Time[8]='\0';
        time_main.Time[7]=48+(data[0] & 0b00001111);
        time_main.Time[6]=48+((data[0] & 0b01110000)>>4);
        time_main.Time[5]=':';

        time_main.minute = ((((data[1] & 0b01110000) / 16) * 10) + (data[1] & 0b00001111));
        time_main.Time[4]=48+(data[1] & 0b00001111);
        time_main.Time[3]=48+((data[1] & 0b01110000)>>4);
        time_main.Time[2]=':';

        time_main.hour = ((((data[2] & 0b00010000)/16) * 10) + (data[2] & 0b00001111));
        time_main.Time[1]=48+(data[2] & 0b00001111);
        time_main.Time[0]=48+((data[2] & 0b00010000)>>4);
        if(data[2] & 0b00100000)
        {
            time_main.ampm = 1;
            if (time_main.hour < 12)
            {
                time_main.hour = time_main.hour+12;
            }
        }
        else
        {
            time_main.ampm = 0;
            if (time_main.hour == 12)
            {
                time_main.hour = 0;
            }
        }
        time_main.dan=data[3] & 0b00000111;

        time_main.date = ((((data[4] & 0b00110000)/16) * 10) + (data[4] & 0b00001111));
        time_main.Date[1]=48+(data[4] & 0b00001111);
        time_main.Date[0]=48+((data[4] & 0b00110000)>>4);

        time_main.month = ((((data[5] & 0b00010000) / 16) * 10) + (data[5] & 0b00001111));
        time_main.Date[4]=48+(data[5] & 0b00001111);
        time_main.Date[3]=48+((data[5] & 0b00010000)>>4);
        time_main.Date[2]='/';

        time_main.Date[10]='\0';
        time_main.year = ((((data[6] & 0b11110000)/ 16) * 10) +(data[6] & 0b00001111));
        time_main.Date[9]=48+(data[6] & 0b00001111);
        time_main.Date[8]=48+((data[6] & 0b11110000)>>4);
        time_main.Date[7]='0';
        time_main.Date[6]='2';
        time_main.Date[5]='/';

        if (((time_main.date < 1) || (time_main.date > 31)) ||
            ((time_main.month < 1) || (time_main.month > 12)) ||
            ((time_main.year < DEFAULT_YEAR) || (time_main.year > 99))||            // Assuming that RTC will never go below 2016.
            (/*(time_main.hour < 0) ||*/ (time_main.hour > 23)) ||
            (/*(time_main.minute < 0) ||*/ (time_main.minute > 59)) ||
            (/*(time_main.sec < 0) ||*/ (time_main.sec > 59)))
        {
#ifdef DEBUG_RTC
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRTCcorrupted:\n");
            vUART_SendInt(DEBUG_UART_BASE,time_main.date);
            vUART_SendChr(DEBUG_UART_BASE,'-');
            vUART_SendInt(DEBUG_UART_BASE,time_main.month);
            vUART_SendChr(DEBUG_UART_BASE,'-');
            vUART_SendInt(DEBUG_UART_BASE,time_main.year);
            vUART_SendChr(DEBUG_UART_BASE,',');
            vUART_SendInt(DEBUG_UART_BASE,time_main.hour);
            vUART_SendChr(DEBUG_UART_BASE,':');
            vUART_SendInt(DEBUG_UART_BASE,time_main.minute);
            vUART_SendChr(DEBUG_UART_BASE,':');
            vUART_SendInt(DEBUG_UART_BASE,time_main.sec);
#endif  //DEBUG_RTC
            error = 1;
            //diag.sys_error_status |= (1 << RTC_FAULT);
            //update_error_sts(RTC_FAULT, TRUE);
            //set_clk_fault_sts(TRUE);
        }
        if (!error)
        {
#ifdef DEBUG_RTC
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRTCok:\n");
            vUART_SendInt(DEBUG_UART_BASE,time_main.date);
            vUART_SendChr(DEBUG_UART_BASE,'-');
            vUART_SendInt(DEBUG_UART_BASE,time_main.month);
            vUART_SendChr(DEBUG_UART_BASE,'-');
            vUART_SendInt(DEBUG_UART_BASE,time_main.year);
            vUART_SendChr(DEBUG_UART_BASE,',');
            vUART_SendInt(DEBUG_UART_BASE,time_main.hour);
            vUART_SendChr(DEBUG_UART_BASE,':');
            vUART_SendInt(DEBUG_UART_BASE,time_main.minute);
            vUART_SendChr(DEBUG_UART_BASE,':');
            vUART_SendInt(DEBUG_UART_BASE,time_main.sec);
#endif  //DEBUG_RTC
            //no error lets break and use time otherwise retry.
            //diag.sys_error_status &= ~(1 << RTC_FAULT);
            //update_error_sts(RTC_FAULT, FALSE);
            //set_clk_fault_sts(FALSE);
            break;
        }
    }
    if (i >= 3)
    {
        // Generate RTC Error flag in diagnosis.
        /*time_main.date = 31;
        time_main.month = 12;
        time_main.year = 99;
        time_main.hour = 23;
        time_main.minute = 59;
        time_main.sec = 59;*/
        time_main.date = 1;
        time_main.month = 1;
        time_main.year = 0;
        time_main.hour = 0;
        time_main.minute = 1;
        time_main.sec = 0;
        //diag.sys_error_status |= (1 << RTC_FAULT);
        //update_error_sts(RTC_FAULT, TRUE);
        //set_clk_fault_sts(FALSE);
        error = 1;
#ifdef DEBUG_RTC
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRTCreadFail:\n");
        vUART_SendInt(DEBUG_UART_BASE,time_main.date);
        vUART_SendChr(DEBUG_UART_BASE,'-');
        vUART_SendInt(DEBUG_UART_BASE,time_main.month);
        vUART_SendChr(DEBUG_UART_BASE,'-');
        vUART_SendInt(DEBUG_UART_BASE,time_main.year);
        vUART_SendChr(DEBUG_UART_BASE,',');
        vUART_SendInt(DEBUG_UART_BASE,time_main.hour);
        vUART_SendChr(DEBUG_UART_BASE,':');
        vUART_SendInt(DEBUG_UART_BASE,time_main.minute);
        vUART_SendChr(DEBUG_UART_BASE,':');
        vUART_SendInt(DEBUG_UART_BASE,time_main.sec);
#endif  //DEBUG_RTC
    }
#ifdef GPRS_ENABLE
    if (gprs_date_time.update_time_aval == TRUE)
    {
        time_sync();
    }
#endif
}

bool isDT_ok(void)
{
    if(((time_main.year >= DEFAULT_YEAR) && (time_main.year <= (DEFAULT_YEAR + YEAR_OFFSET))) && 
    ((time_main.month >= 1) && (time_main.month <= 12)) && ((time_main.date >= 1) && (time_main.date <= 31)) && 
    (((char*)time_main.hour >= 0) && (time_main.hour <= 23)) && (((char*)time_main.minute >= 0) && (time_main.minute <= 59)) && 
    (((char*)time_main.sec >= 0) && (time_main.sec <= 59)))
    {
        return true;
    }

    return false;
}

#ifdef ETHERNET_EN
void time_sync(void)
{
    if(!isDT_ok())
    {
        if(gps_date_time.update_time_aval == true)
        {
            gps_date_time.update_time_aval = false;
#ifdef DEBUG_SYNC_MODEM
				//printf("\nB2");
#endif
				
            if ((gps_date_time.yy <= time_stamp.year) && (gps_date_time.mm <= time_stamp.month) && (gps_date_time.dd <= time_stamp.date) && (gps_date_time.hr <= time_stamp.hour))
            {
#ifdef DEBUG_SYNC_MODEM
                //printf("\nC2");
#endif
                //UWriteString(" No Error in Date", UART0);
            }
            else
            {
                unsigned char data[8];
#ifdef DEBUG_RTC_SYNC
    /*
                UWriteString("\nmain_ts:\t",UART_PC);
                UWriteInt(time_main.year, UART_PC);
                UWriteData('-', UART_PC);
                UWriteInt(time_main.month, UART_PC);
                UWriteData('-', UART_PC);
                UWriteInt(time_main.date, UART_PC);
                UWriteData(',',UART_PC);
                UWriteInt(time_main.hour, UART_PC);
                UWriteData(':', UART_PC);
                UWriteInt(time_main.minute, UART_PC);
                UWriteData(':', UART_PC);
                UWriteInt(time_main.sec, UART_PC);*/
                vUART_SendStr(UART_PC,"\nRTCsync:\t");
                vUART_SendInt(UART_PC,gps_date_time.yy );
                vUART_SendChr(UART_PC,'-' );
                vUART_SendInt(UART_PC,gps_date_time.mm );
                vUART_SendChr(UART_PC,'-' );
                vUART_SendInt(UART_PC,gps_date_time.dd);
                vUART_SendChr(UART_PC,',');
                vUART_SendInt(UART_PC,gps_date_time.hr);
                vUART_SendChr(UART_PC,':');
                vUART_SendInt(UART_PC,gps_date_time.min);
                vUART_SendChr(UART_PC,':');
                vUART_SendInt(UART_PC,gps_date_time.sec);
#endif
                data[6] = (((gps_date_time.yy / 10) * 16) | (gps_date_time.yy % 10));
                //RTC_Write(0x06, data);		//Year
                data[5] = (((gps_date_time.mm / 10) * 16) | (gps_date_time.mm % 10));
                //RTC_Write(0x05, data & 0x1F);		//Month
                data[4] = (((gps_date_time.dd / 10) * 16) | (gps_date_time.dd % 10));
                //RTC_Write(0x04, data & 0x3F);		//Date
                //RTC_Write(0x03, 1 & 0x07);		//Day: 1-7, 1=Sunday
#ifdef RTC_DS3231
                //RTC_Write(0x03, (1 & 0x07));
                data[3] = (1 & 0x07);
#endif
#ifdef RTC_MCP79410
                //RTC_Write(0x03, ((1 & 0x07) | (1 << RTC_VBAT_EN_BIT)));		//Day: 1-7, 1=Sunday
                data[3] = ((1 & 0x07) | (1 << RTC_VBAT_EN_BIT));
#endif
                if(gps_date_time.hr >= 12)
                {
                    if(gps_date_time.hr > 12)
                    {
                        gps_date_time.hr %= 12;
                    }

                    gps_date_time.hr = ((gps_date_time.hr/10)<<4) | (gps_date_time.hr%10);		//hr
                    gps_date_time.hr |= (1<<5);
                }
                else
                {
                    if(gps_date_time.hr == 0)
                    {
                        gps_date_time.hr = 12;
                    }
                    gps_date_time.hr = ((gps_date_time.hr/10)<<4) | (gps_date_time.hr%10);		//hr
                }
                gps_date_time.hr |= (1<<6);		//Anand 02.07.2014
                data[2] = (gps_date_time.hr & 0x7F);
                //RTC_Write(0x02, (gprs_date_time.hr & 0x7F));		//hr
                data[1] = (((gps_date_time.min / 10) * 16) | (gps_date_time.min % 10));
                //RTC_Write(0x01, (data & 0x7F)); //min
                //DS1307Write(0x00, 0);
                data[0] = (((gps_date_time.sec / 10) * 16) | (gps_date_time.sec % 10));
                //RTC_Write(0x00, (data & 0x7F)); //sec
#ifdef RTC_DS3231
                //RTC_Write(0x00, (data & 0x7F)); //sec
                data[0] = (data[0] & 0x7F);
#endif
#ifdef RTC_MCP79410
                data[0] = ((data[0] & 0x7F) | (1 << RTC_OSC_EN_BIT));
#endif
                Write_RTC(data, 7);

                gps_date_time.update_time_aval = FALSE;		
            }
        }
    }
}
#else

void time_sync(void)
{
    if(!isDT_ok())
    {
        if(gprs_date_time.update_time_aval == true)
        {
            gprs_date_time.update_time_aval = false;
#ifdef DEBUG_SYNC_MODEM
                //printf("\nB2");
#endif

            if ((gprs_date_time.yy <= time_stamp.year) && (gprs_date_time.mm <= time_stamp.month) && (gprs_date_time.dd <= time_stamp.date) && (gprs_date_time.hr <= time_stamp.hour))
            {
#ifdef DEBUG_SYNC_MODEM
                //printf("\nC2");
#endif
                //UWriteString(" No Error in Date", UART0);
            }
            else
            {
                unsigned char data[8];
#ifdef DEBUG_RTC_SYNC
    /*
                UWriteString("\nmain_ts:\t",UART_PC);
                UWriteInt(time_main.year, UART_PC);
                UWriteData('-', UART_PC);
                UWriteInt(time_main.month, UART_PC);
                UWriteData('-', UART_PC);
                UWriteInt(time_main.date, UART_PC);
                UWriteData(',',UART_PC);
                UWriteInt(time_main.hour, UART_PC);
                UWriteData(':', UART_PC);
                UWriteInt(time_main.minute, UART_PC);
                UWriteData(':', UART_PC);
                UWriteInt(time_main.sec, UART_PC);*/
                vUART_SendStr(UART_PC,"\nRTCsync:\t");
                vUART_SendInt(UART_PC,gprs_date_time.yy );
                vUART_SendChr(UART_PC,'-' );
                vUART_SendInt(UART_PC,gprs_date_time.mm );
                vUART_SendChr(UART_PC,'-' );
                vUART_SendInt(UART_PC,gprs_date_time.dd);
                vUART_SendChr(UART_PC,',');
                vUART_SendInt(UART_PC,gprs_date_time.hr);
                vUART_SendChr(UART_PC,':');
                vUART_SendInt(UART_PC,gprs_date_time.min);
                vUART_SendChr(UART_PC,':');
                vUART_SendInt(UART_PC,gprs_date_time.sec);
#endif
                data[6] = (((gprs_date_time.yy / 10) * 16) | (gprs_date_time.yy % 10));
                //RTC_Write(0x06, data);        //Year
                data[5] = (((gprs_date_time.mm / 10) * 16) | (gprs_date_time.mm % 10));
                //RTC_Write(0x05, data & 0x1F);     //Month
                data[4] = (((gprs_date_time.dd / 10) * 16) | (gprs_date_time.dd % 10));
                //RTC_Write(0x04, data & 0x3F);     //Date
                //RTC_Write(0x03, 1 & 0x07);        //Day: 1-7, 1=Sunday
#ifdef RTC_DS3231
                //RTC_Write(0x03, (1 & 0x07));
                data[3] = (1 & 0x07);
#endif
#ifdef RTC_MCP79410
                //RTC_Write(0x03, ((1 & 0x07) | (1 << RTC_VBAT_EN_BIT)));       //Day: 1-7, 1=Sunday
                data[3] = ((1 & 0x07) | (1 << RTC_VBAT_EN_BIT));
#endif
                if(gprs_date_time.hr >= 12)
                {
                    if(gprs_date_time.hr > 12)
                    {
                        gprs_date_time.hr %= 12;
                    }

                    gprs_date_time.hr = ((gprs_date_time.hr/10)<<4) | (gprs_date_time.hr%10);      //hr
                    gprs_date_time.hr |= (1<<5);
                }
                else
                {
                    if(gprs_date_time.hr == 0)
                    {
                        gprs_date_time.hr = 12;
                    }
                    gprs_date_time.hr = ((gprs_date_time.hr/10)<<4) | (gprs_date_time.hr%10);      //hr
                }
                gprs_date_time.hr |= (1<<6);     //Anand 02.07.2014
                data[2] = (gprs_date_time.hr & 0x7F);
                //RTC_Write(0x02, (gprs_date_time.hr & 0x7F));      //hr
                data[1] = (((gprs_date_time.min / 10) * 16) | (gprs_date_time.min % 10));
                //RTC_Write(0x01, (data & 0x7F)); //min
                //DS1307Write(0x00, 0);
                data[0] = (((gprs_date_time.sec / 10) * 16) | (gprs_date_time.sec % 10));
                //RTC_Write(0x00, (data & 0x7F)); //sec
#ifdef RTC_DS3231
                //RTC_Write(0x00, (data & 0x7F)); //sec
                data[0] = (data[0] & 0x7F);
#endif
#ifdef RTC_MCP79410
                data[0] = ((data[0] & 0x7F) | (1 << RTC_OSC_EN_BIT));
#endif
                Write_RTC(data, 7);

                gprs_date_time.update_time_aval = FALSE;
            }
        }
    }
}

#endif  //ETHERNET_EN

// void time_sync(void)
// {
//     get_ocpp_date_time();
// #ifdef UTC_SERVER
//     utcTOlocal(&OCPP_Data.time_stamp);
// #endif
//     if ((OCPP_Data.time_stamp.year <= time_main.year)
//             && (OCPP_Data.time_stamp.month <= time_main.month)
//             && (OCPP_Data.time_stamp.date <= time_main.date))
// //            && OCPP_Data.time_stamp.hour <= time_main.hour
// //            && OCPP_Data.time_stamp.min <= time_main.minute
// //            && OCPP_Data.time_stamp.sec <= time_main.sec)
//     {
//         //UWriteString(" No Error in Date", UART0);
//     }
//     else
//     {
//         unsigned char data[8];
// #ifdef DEBUG_TIME
//         vUART_SendStr(UART_PC,"Enter Time:");
//         vUART_SendInt(UART_PC,OCPP_Data.time_stamp.year);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,OCPP_Data.time_stamp.month);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,OCPP_Data.time_stamp.date);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,OCPP_Data.time_stamp.hour);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,OCPP_Data.time_stamp.min);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,OCPP_Data.time_stamp.sec);
//         vUART_SendChr(UART_PC,'\n');
//         vUART_SendInt(UART_PC,time_main.year);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,time_main.month);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,time_main.date);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,time_main.hour);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,time_main.minute);
//         vUART_SendChr(UART_PC,',');
//         vUART_SendInt(UART_PC,time_main.sec);
// #endif
//         time_main.year = OCPP_Data.time_stamp.year;
//         time_main.month = OCPP_Data.time_stamp.month;
//         time_main.date = OCPP_Data.time_stamp.date;
//         time_main.hour = OCPP_Data.time_stamp.hour;
//         time_main.minute = OCPP_Data.time_stamp.min;
//         time_main.sec = OCPP_Data.time_stamp.sec;

//         data[6] = (((OCPP_Data.time_stamp.year / 10) * 16) | (OCPP_Data.time_stamp.year % 10));
//         //RTC_Write(0x06, data);        //Year
//         data[5] = (((OCPP_Data.time_stamp.month / 10) * 16) | (OCPP_Data.time_stamp.month % 10));
//         //RTC_Write(0x05, data & 0x1F);     //Month
//         data[4] = (((OCPP_Data.time_stamp.date / 10) * 16) | (OCPP_Data.time_stamp.date % 10));
//         //RTC_Write(0x04, data & 0x3F);     //Date
//         //RTC_Write(0x03, 1 & 0x07);        //Day: 1-7, 1=Sunday
// #ifdef RTC_DS3231
//         //RTC_Write(0x03, (1 & 0x07));
//         data[3] = (1 & 0x07);
// #endif
// #ifdef RTC_MCP79410
//         //RTC_Write(0x03, ((1 & 0x07) | (1 << RTC_VBAT_EN_BIT)));       //Day: 1-7, 1=Sunday
//         data[3] = ((1 & 0x07) | (1 << RTC_VBAT_EN_BIT));
// #endif
//         if(OCPP_Data.time_stamp.hour >= 12)
//         {
//             if(OCPP_Data.time_stamp.hour > 12)
//             {
//                 OCPP_Data.time_stamp.hour %= 12;
//             }

//             OCPP_Data.time_stamp.hour = ((OCPP_Data.time_stamp.hour/10)<<4) | (OCPP_Data.time_stamp.hour%10);       //hr
//             OCPP_Data.time_stamp.hour |= (1<<5);
//         }
//         else
//         {
//             if(OCPP_Data.time_stamp.hour == 0)
//             {
//                 OCPP_Data.time_stamp.hour = 12;
//             }
//             OCPP_Data.time_stamp.hour = ((OCPP_Data.time_stamp.hour/10)<<4) | (OCPP_Data.time_stamp.hour%10);       //hr
//         }
//         OCPP_Data.time_stamp.hour |= (1<<6);        //Anand 02.07.2014
//         data[2] = (OCPP_Data.time_stamp.hour & 0x7F);
//         //RTC_Write(0x02, (gprs_date_time.hr & 0x7F));      //hr
//         data[1] = (((OCPP_Data.time_stamp.min / 10) * 16) | (OCPP_Data.time_stamp.min % 10));
//         //RTC_Write(0x01, (data & 0x7F)); //min
//         //DS1307Write(0x00, 0);
//         data[0] = (((OCPP_Data.time_stamp.sec / 10) * 16) | (OCPP_Data.time_stamp.sec % 10));
//         //RTC_Write(0x00, (data & 0x7F)); //sec
// #ifdef RTC_DS3231
//         //RTC_Write(0x00, (data & 0x7F)); //sec
//         data[0] = (data[0] & 0x7F);
// #endif
// #ifdef RTC_MCP79410
//         data[0] = ((data[0] & 0x7F) | (1 << RTC_OSC_EN_BIT));
// #endif
//         //RTC_Write(0x00, data, 7)
//         Write_RTC(data, 7);


//         //diag.sys_error_status &= ~(1 << RTC_FAULT);

//         //set_clk_fault_sts(FALSE);
//         //gprs_date_time.update_time_aval = FALSE;
// /*
// #ifdef DEBUG_TIME
//         UWriteString("Leave Time", UART_PC);
//         UWriteInt(gprs_date_time.yy, UART_PC);
//         UWriteInt(gprs_date_time.mm, UART_PC);
//         UWriteInt(gprs_date_time.dd, UART_PC);
//         UWriteInt(gprs_date_time.hr, UART_PC);
//         UWriteInt(gprs_date_time.min, UART_PC);
//         UWriteInt(gprs_date_time.sec, UART_PC);
//         UWriteString("\n", UART_PC);
//         UWriteInt(time_main.year, UART_PC);
//         UWriteInt(time_main.month, UART_PC);
//         UWriteInt(time_main.date, UART_PC);
//         UWriteInt(time_main.hour, UART_PC);
//         UWriteInt(time_main.minute, UART_PC);
//         UWriteInt(time_main.sec, UART_PC);
// #endif*/
//     }
// }

/*void sync_time_gprs(void)
{
    if (get_error_sts() & (1 << RTC_FAULT))
    {
        if (getGPRSNWSts() == AVBL)
        {
            getDateTime();
        }
    }
}*/

void get_present_time(time_stamp_t *time_stamp)
{
    getDateTime();
    time_stamp->sec = time_main.sec;
    time_stamp->min = time_main.minute;
    time_stamp->hour = time_main.hour;
    time_stamp->date = time_main.date;
    time_stamp->month = time_main.month;
    time_stamp->year = time_main.year;
    time_sync();
}
void update_rtc(char *arr, int indx)
{
    //int indx = 0;
    unsigned char data[8];

    data[6] = ((arr[indx] / 10) << 4) | (arr[indx] % 10);       //Year
    indx++;
    data[5] = ((arr[indx] / 10) << 4) | (arr[indx] % 10);       //Month
    indx++;
    data[4] = ((arr[indx] / 10) << 4) | (arr[indx] % 10);       //Date
    indx++;

#ifdef RTC_DS3231
    data[3] = (arr[indx++] & 0x07);
#endif
#ifdef RTC_MCP79410
    data[3] = ((arr[indx++] & 0x07) | (1 << RTC_VBAT_EN_BIT));      //Day: 1-7, 1=Sunday
#endif

    if(arr[indx] >= 12)
    {
        if(arr[indx] > 12)
        {
            arr[indx] %= 12;
        }

        arr[indx] = ((arr[indx] / 10) << 4) | (arr[indx] % 10);     //hr
        arr[indx] |= (1<<5);
    }
    else
    {
        if(arr[indx] == 0)
        {
            arr[indx] = 12;
        }
        arr[indx] = ((arr[indx] / 10) << 4) | (arr[indx] % 10);     //hr
    }
    arr[indx] |= (1 << 6);

    data[2] = (arr[indx++] & 0x7F);     //hr

    data[1] = ((arr[indx] / 10) << 4) | (arr[indx] % 10);       //Minute
    indx++;

#ifdef RTC_DS3231
    data[0] = 0x00;
#endif
#ifdef RTC_MCP79410
    data[0] = (0x30 | (1 << RTC_OSC_EN_BIT)));
#endif
    //RTC_Write(0x00, data, 7)
    Write_RTC( data, 7);
    /*{
        sei();
    }*/
}
// void get_ocpp_date_time(void)
// {
//     char temp[10];
//     unsigned int index = 0;
//     index += 2;   //skips 20
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //year
//     OCPP_Data.time_stamp.year = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //month
//     OCPP_Data.time_stamp.month = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //date
//     OCPP_Data.time_stamp.date = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //hour
//     OCPP_Data.time_stamp.hour = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //min
//     OCPP_Data.time_stamp.min = atoi(temp);

//     index += 3;
//     memset(temp,0,sizeof(temp));
//     memcpy(temp,&OCPP_Data.Time_stamp[index],2);    //sec
//     OCPP_Data.time_stamp.sec = atoi(temp);
// }

// void update_ocpp_date_time(void)
// {
//     get_present_time(&OCPP_Data.time_stamp);
//     //utcTOlocal(&OCPP_Data.time_stamp);
//     //2022-10-13T15:18:42.089+05:30
//     memset(&OCPP_Data.Time_stamp,0,sizeof(OCPP_Data.Time_stamp));
//     //sprintf((char *)OCPP_Data.Time_stamp,"20%02d-%02d-%02dT%02d:%02d:%02d.000+05:30",OCPP_Data.time_stamp.year,OCPP_Data.time_stamp.month,OCPP_Data.time_stamp.date,OCPP_Data.time_stamp.hour,OCPP_Data.time_stamp.min,OCPP_Data.time_stamp.sec);
// #ifdef DEBUG_SPRINTF
//     vUART_SendStr(UART_PC,"1");
// #endif
//     my_sprintf((char *)OCPP_Data.Time_stamp,7,"20%02d-%02d-%02dT%02d:%02d:%02d.000+05:30",OCPP_Data.time_stamp.year,OCPP_Data.time_stamp.month,OCPP_Data.time_stamp.date,OCPP_Data.time_stamp.hour,OCPP_Data.time_stamp.min,OCPP_Data.time_stamp.sec);

// }

time_stamp_t StrToTimeStamp(uint8_t *str)
{
    time_stamp_t ts;
    char temp_buff[5];
    int index = 0;
    memset(&ts,0,sizeof(time_stamp_t));

    index = 2;
    memset(temp_buff,0,sizeof(temp_buff));
    memcpy(temp_buff,&str[index],2);
    ts.year = atoi(temp_buff);
    index += 3;

    memset(temp_buff,0,sizeof(temp_buff));
    memcpy(temp_buff,&str[index],2);
    ts.month = atoi(temp_buff);
    index += 3;

    memset(temp_buff,0,sizeof(temp_buff));
    memcpy(temp_buff,&str[index],2);
    ts.date = atoi(temp_buff);
    index += 3;

    memset(temp_buff,0,sizeof(temp_buff));
    memcpy(temp_buff,&str[index],2);
    //printf("%s",temp_buff);
    ts.hour = atoi(temp_buff);
    index += 3;

    memset(temp_buff,0,sizeof(temp_buff));
    memcpy(temp_buff,&str[index],2);
    ts.min = atoi(temp_buff);
    index += 3;

    memset(temp_buff,0,sizeof(temp_buff));
    memcpy(temp_buff,&str[index],2);
    ts.sec = atoi(temp_buff);

    return ts;
}
/**
 * Adds two date and time based on the input seconds
 * Inputs : date_time is the starting date and time, diff is the number of seconds to be added in starting date time.
 * the updated date and time will be stored in the updated_date_time
 */
void add_date_time(time_stamp_t *updated_date_time, time_stamp_t *date_time, uint32_t diff)
{
    time_stamp_t temp_d_t;
    uint8_t month_days;
    temp_d_t.year = diff/(SEC_IN_MIN*MIN_IN_HR*HR_IN_DAY*DAYS_IN_YR);
    diff %= (SEC_IN_MIN*MIN_IN_HR*HR_IN_DAY*DAYS_IN_YR);
    temp_d_t.month = diff/(SEC_IN_MIN*MIN_IN_HR*HR_IN_DAY*DAYS_IN_MON);
    diff %= (SEC_IN_MIN*MIN_IN_HR*HR_IN_DAY*DAYS_IN_MON);
    temp_d_t.date = diff/(SEC_IN_MIN*MIN_IN_HR*HR_IN_DAY);
    diff %= (SEC_IN_MIN*MIN_IN_HR*HR_IN_DAY);
    temp_d_t.hour = diff/(SEC_IN_MIN*MIN_IN_HR);
    diff %= (SEC_IN_MIN*MIN_IN_HR);
    temp_d_t.min = diff/(SEC_IN_MIN);
    diff %= (SEC_IN_MIN);
    temp_d_t.sec = diff;

    //printf("\ntemp date and time : %02d-%02d-%02d %02d:%02d:%02d",temp_d_t.year,temp_d_t.month,temp_d_t.date,temp_d_t.hour,temp_d_t.min,temp_d_t.sec);
#ifdef DEBUG_CHG_PROFILE
    char temp_buff[30];
    vUART_SendStr(DEBUG_UART_BASE, "\ntemp_d_t:");
    my_sprintf(temp_buff,7,"%02d-%02d-%02d %02d:%02d:%02d",temp_d_t.year,temp_d_t.month,temp_d_t.date,temp_d_t.hour,temp_d_t.min,temp_d_t.sec);
    vUART_SendStr(DEBUG_UART_BASE,(const uint8_t *)temp_buff);
#endif
    updated_date_time->sec = date_time->sec + temp_d_t.sec;
    updated_date_time->min = (updated_date_time->sec/SEC_IN_MIN);
    updated_date_time->sec = updated_date_time->sec % SEC_IN_MIN;

    updated_date_time->min += date_time->min + temp_d_t.min;
    updated_date_time->hour = (updated_date_time->min/MIN_IN_HR);
    updated_date_time->min = updated_date_time->min % MIN_IN_HR;

    updated_date_time->hour += date_time->hour + temp_d_t.hour;
    updated_date_time->date = (updated_date_time->hour/HR_IN_DAY);
    updated_date_time->hour = updated_date_time->hour % HR_IN_DAY;

    updated_date_time->date += date_time->date + temp_d_t.date;

    if((date_time->month == 1) ||
       (date_time->month == 3) ||
       (date_time->month == 5) ||
       (date_time->month == 7) ||
       (date_time->month == 8) ||
       (date_time->month == 10) ||
       (date_time->month == 12))
    {
        month_days = 31;
    }
    else if((date_time->month == 4) ||
            (date_time->month == 6) ||
            (date_time->month == 9) ||
            (date_time->month == 11))
    {
        month_days = 30;
    }
    else if(date_time->month == 2)
    {
        if(is_leap_year(date_time->year))
        {
            month_days = 29;
        }
        else
        {
            month_days = 28;
        }
    }
    //printf("\nmonth_days:%d",month_days);
    if(updated_date_time->date > month_days)
    {
        updated_date_time->month = (updated_date_time->date/month_days);
        updated_date_time->date = updated_date_time->date % month_days;
    }


    updated_date_time->month += date_time->month + temp_d_t.month;

    if(updated_date_time->month > MON_IN_YR)
    {
        updated_date_time->year = (updated_date_time->month/MON_IN_YR);
        updated_date_time->month = updated_date_time->month % MON_IN_YR;
    }
    updated_date_time->year += date_time->year + temp_d_t.year;
}

uint8_t is_leap_year(uint8_t year)
{
    if(year%4)  //if year is not divisible by 4 then it is not a leap year
    {
        return FALSE;
    }

    if(year%100)    //if year is divisible by 4 and it is not divisible by 100 then it is a leap year
    {
        return TRUE;
    }

    if(year%400)   //if year is divisible by 4 and also divisible by 100 then  if it is divisible by 400 then it is a leap year
    {
        return FALSE;
    }

    return TRUE;
}
uint8_t check_dateTime_data(time_stamp_t *valid_time)
{
    uint8_t Data_found = FALSE;
    if(((valid_time->year >= DEFAULT_YEAR) && (valid_time->year <= (DEFAULT_YEAR + YEAR_OFFSET))) &&
    ((valid_time->month >= 1) && (valid_time->month <= 12)) &&
    ((valid_time->date >= 1) && (valid_time->date <= 31))&&
    (((char *)valid_time->hour >= 0) && (valid_time->hour <= 23)) &&
    (((char *)valid_time->min >= 0) && (valid_time->min <= 59)) &&
    (((char *)valid_time->sec >= 0) && (valid_time->sec <= 59)))
    {

        Data_found = TRUE;
    }
    else
    {

        Data_found = FALSE;
    }
//#ifdef DEBUG_FIRMWARE
//                vUART_SendStr(DEBUG_UART_BASE, "\ndatat_fnd:");
//                vUART_SendInt(DEBUG_UART_BASE, Data_found);
//                vUART_SendStr(DEBUG_UART_BASE, "\ndate_time:");
//                vUART_SendInt(DEBUG_UART_BASE, valid_time->date);
//                vUART_SendChr(DEBUG_UART_BASE, '/');
//                vUART_SendInt(DEBUG_UART_BASE, valid_time->month);
//                vUART_SendChr(DEBUG_UART_BASE, '/');
//                vUART_SendInt(DEBUG_UART_BASE, valid_time->year);
//                vUART_SendChr(DEBUG_UART_BASE, '_');
//                vUART_SendInt(DEBUG_UART_BASE, valid_time->hour);
//                vUART_SendChr(DEBUG_UART_BASE, '-');
//                vUART_SendInt(DEBUG_UART_BASE, valid_time->min);
//                vUART_SendChr(DEBUG_UART_BASE, '-');
//                vUART_SendInt(DEBUG_UART_BASE, valid_time->sec);
//
//#endif
    return Data_found;
}

#endif





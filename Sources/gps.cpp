/*
 * gps.cpp
 *
 *  Created on: Apr 25, 2024
 *      Author: ADMIN
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

#include "_config.h"

// #ifndef ETHERNET_EN
#include "gps.h"
#include "gprs.h"
#include "UartCore.h"
#include "_common.h"
#include "E2P.h"
#include "Clock.h"
#include "ADC_Core.h"
#include "IO_cntrl.h"
#include "main.h"
#include "SysTick_Timer.h"
#include "_debug.h"
#include "Telecom_server_query.h"

extern ram_data_t ram_data;
gps_info_t gps_info;

gps_t gps;
gps_data_t gps_data;

gps_handler_state_t gps_handler_state;

gps_date_time_t gps_date_time;

uint32_t gps_interval_start = 0;
uint32_t cnt_gps_1sec = 0;

bool get_location(void)
{
	int i = 0;
    bool retVal = false;
	memset(&gps_data, 0, sizeof(gps_data_t));

	unsigned long int value = 0;
	unsigned long int temp_value = 0;
	// printf("\nlatitude string : %s\nlongitude string : %s",gps.gps_info.latitude,gps.gps_info.longitude);
//	int32_t value = 0;
//	int32_t temp_value = 0;

	if(gps.gps_info.latitude[0] != '\0')
	{
		for (i = 6-1; (i < LAT_LEN-1); i++)
		{
			temp_value *= 10;
			temp_value += gps.gps_info.latitude[i] - '0';
			// printf("\n1temp_val=%lu,gLat[%d]=%c",temp_value,i,gps.gps_info.latitude[i]);
		}
		value = (temp_value*10) / 60;
		// printf("\ntemp value:%d,\n value:%d",temp_value,value);

// #ifdef DEBUG_GET_LOC
//         // UWriteString((char *)"temp_lat:",DBG_UART);
//         // UWriteInt(temp_value,DBG_UART);
//         // UWriteData(',',DBG_UART);
//         // UWriteInt(value,DBG_UART);
//         vUART_SendStr(DEBUG_UART_BASE, "\ntemp_lat=");
//         vUART_SendInt(DEBUG_UART_BASE, temp_value);
//         vUART_SendChr(DEBUG_UART_BASE, ',');
//         vUART_SendInt(DEBUG_UART_BASE, value);
// #endif
		temp_value = 0;
		for (i = 3-1; (i < 5-1); i++)
		{
			temp_value *= 10;
			temp_value += gps.gps_info.latitude[i] - '0';
			// printf("\n2temp_val=%lu,gLat[%d]=%c",temp_value,i,gps.gps_info.latitude[i]);
		}
		value = (value + ((temp_value * 1000000) / 60));
		// printf("\ntemp value:%d,\n value:%d",temp_value,value);

// #ifdef DEBUG_GET_LOC
//         // UWriteString((char *)"2_temp_lat:",DBG_UART);
//         // UWriteInt(temp_value,DBG_UART);
//         // UWriteData(',',DBG_UART);
//         // UWriteInt(value,DBG_UART);
//         vUART_SendStr(DEBUG_UART_BASE, "\n2_temp_lat=");
//         vUART_SendInt(DEBUG_UART_BASE, temp_value);
//         vUART_SendChr(DEBUG_UART_BASE, ',');
//         vUART_SendInt(DEBUG_UART_BASE, value);
// #endif
		temp_value = 0;
		for (i = 1-1; (i < 3-1); i++)
		{
			temp_value *= 10;
			temp_value += gps.gps_info.latitude[i] - '0';
			// printf("\n3temp_val=%lu,gLat[%d]=%c",temp_value,i,gps.gps_info.latitude[i]);
		}
		

// #ifdef DEBUG_GET_LOC
//         // UWriteString((char *)"3_temp_lat:",DBG_UART);
//         // UWriteInt(temp_value,DBG_UART);
//         vUART_SendStr(DEBUG_UART_BASE, "\n3_temp_lat=");
//         vUART_SendInt(DEBUG_UART_BASE, temp_value);
// #endif
		gps_data.Latitude = (value + (temp_value * 1000000));
		// printf("\ntemp value:%d,\n lat:%d",temp_value,gps_data.Latitude);
		// if (gps.gps_info.latitude[0] == '-')
		if(gps.gps_info.N_S == 'S')
		{
			// gps_data.Latitude |= (1UL << 28);
            gps_data.Latitude = -gps_data.Latitude;
		}
	}
	else
	{
#ifdef DEBUG_GET_LOC
		// UWriteString((char*)"\nLOC_f1",DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\nLOC_f1");
#endif		
        retVal = false;
        return retVal;
    }
	
	if(gps.gps_info.longitude[0] != '\0')
	{
		i = 0;
	
		temp_value = 0;
		for (i = 7-1; (i < LONG_LEN-1); i++)
		{
			temp_value *= 10;
			temp_value += gps.gps_info.longitude[i] - '0';
			// printf("\n1temp_val=%lu,gLong[%d]=%c",temp_value,i,gps.gps_info.longitude[i]);
		}
		value = (temp_value * 10) / 60;
		// printf("\ntemp value:%d,\n value:%d",temp_value,value);

// #ifdef DEBUG_GET_LOC
//         // UWriteString((char *)"temp_long:",DBG_UART);
//         // UWriteInt(temp_value,DBG_UART);
//         // UWriteData(',',DBG_UART);
//         // UWriteInt(value,DBG_UART);
//         vUART_SendStr(DEBUG_UART_BASE, "\ntemp_long=");
//         vUART_SendInt(DEBUG_UART_BASE, temp_value);
//         vUART_SendChr(DEBUG_UART_BASE, ',');
//         vUART_SendInt(DEBUG_UART_BASE, value);
// #endif
		temp_value = 0;
		for (i = 4-1; (i < 6-1); i++)
		{
			temp_value *= 10;
			temp_value += gps.gps_info.longitude[i] - '0';
			// printf("\n2temp_val=%lu,gLong[%d]=%c",temp_value,i,gps.gps_info.longitude[i]);
		}
		value = (value + ((temp_value * 1000000) / 60));
		// printf("\ntemp value:%d,\n value:%d",temp_value,value);

// #ifdef DEBUG_GET_LOC
//         // UWriteString((char *)"temp_lat:",DBG_UART);
//         // UWriteInt(temp_value,DBG_UART);
//         // UWriteData(',',DBG_UART);
//         // UWriteInt(value,DBG_UART);
//         // vUART_SendStr(DEBUG_UART_BASE, "\ntemp_lat=");
//         vUART_SendStr(DEBUG_UART_BASE, "\n2temp_long=");
//         vUART_SendInt(DEBUG_UART_BASE, temp_value);
//         vUART_SendChr(DEBUG_UART_BASE, ',');
//         vUART_SendInt(DEBUG_UART_BASE, value);
// #endif
		temp_value = 0;
		for (i = 1-1; (i < 4-1); i++)
		{
			temp_value *= 10;
			temp_value += gps.gps_info.longitude[i] - '0';
			// printf("\n3temp_val=%lu,gLong[%d]=%c",temp_value,i,gps.gps_info.longitude[i]);
		}
		gps_data.Longitude = (value + (temp_value * 1000000));
		// printf("\ntemp value:%d,\n longitude:%d",temp_value,gps_data.Longitude);

// #ifdef DEBUG_GET_LOC
//         // UWriteString((char *)"temp_lat:",DBG_UART);
//         // UWriteInt(temp_value,DBG_UART);
//         // vUART_SendStr(DEBUG_UART_BASE, "\ntemp_lat=");
//         vUART_SendStr(DEBUG_UART_BASE, "\n3temp_lat=");
//         vUART_SendInt(DEBUG_UART_BASE, temp_value);
// #endif
	//	if (gps.gps_info.longitude[0] == '-')
		if(gps.gps_info.E_W == 'W')
		{
			// gps_data.longitude |= (1UL << 28);
            gps_data.Longitude = -gps_data.Longitude;
		}
	}
	else
	{
#ifdef DEBUG_GET_LOC
		// UWriteString((char*)"\nLOC_f2",DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\nLOC_f2");
#endif
        retVal = false;
        return retVal;
    }
	
#ifdef DEBUG_GET_LOC
	// UWriteString((char *)"LA:", DBG_UART);
	// UWriteInt(gps_data.Latitude,DBG_UART);
	// UWriteString((char *)"LO:", DBG_UART);
	// UWriteInt(gps_data.Longitude,DBG_UART);

    vUART_SendStr(DEBUG_UART_BASE, "\nLA:");
    vUART_SendInt(DEBUG_UART_BASE, gps_data.Latitude);
    vUART_SendStr(DEBUG_UART_BASE, "\nLO:");
    vUART_SendInt(DEBUG_UART_BASE, gps_data.Longitude);

	// printf("\nLA=%+03ld.%06lu", gps_data.latitude/1000000L,abs(gps_data.latitude%1000000L));
	// printf("\nLO=%+04ld.%06lu", gps_data.longitude/1000000L,abs(gps_data.longitude%1000000L));

	// printf("\nLA=%d", gps_data.latitude);
	// printf("\nLO=%d", gps_data.longitude);
	// printf("\nLA=%d.%06lu", gps_data.latitude/1000000L,abs(gps_data.latitude%1000000L));
	// printf("\nLO=%d.%06lu", gps_data.longitude/1000000L,abs(gps_data.longitude%1000000L));
#endif
    retVal = true;
    return retVal;
}

gps_status_t gps_handler(void)
{    
    gps_status_t sts = GPS_IN_PRG;

    static uint8_t gps_retry_cnt = 0;
	static uint16_t gps_wait_timeout = 0;  
    int num_bytes = 0; 

#ifdef DEBUG_GPS_HANDLER
    // UWriteString((char*)"\nGP_S:",DBG_UART);
    // UWriteInt(gps_handler_state,DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE, "\nGP_S:");
    vUART_SendInt(DEBUG_UART_BASE, gps_handler_state);
#endif

    switch(gps_handler_state)
    {
        case GPS_CONNCT_RESET:
        {
            // if (gps_wait_timeout++ > LTE_WARMUP_TIME)
            {
                gps_wait_timeout = 0;
                gps_retry_cnt = 0;
                set_gps_status(NOT_AVBL);

                gps_handler_state = GPS_CMD_ECHO_OFF;
            }
        }
        break;

        case GPS_CMD_ECHO_OFF:
        {
            set_gps_status(NOT_AVBL);
            // flushTxBuffer(LTE_UART);
#ifdef ETHERNET_EN
            flushRxBuffer(LTE_UART);
#else
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nGR_ge", DBG_UART);
                vUART_SendStr(UART_PC, "\nGR_ge");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_ECHO_OFF;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
            else
#endif  //ETHERNET_EN
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nNR_ge", DBG_UART);
                vUART_SendStr(UART_PC, "\nNR_ge");
#endif
                flushRxBuffer(LTE_UART);
                // UWriteString((char*)"ATE0\r\n",LTE_UART);
                vUART_SendStr(LTE_UART_BASE, "ATE0\r");

                gps_handler_state = GPS_RSP_ECHO_OFF;
            }
        }
        break;

        case GPS_RSP_ECHO_OFF:
        {
#ifndef ETHERNET_EN
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nGR_ger", DBG_UART);
                vUART_SendStr(UART_PC, "\nGR_ger");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_ECHO_OFF;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nNR_ger", DBG_UART);
                vUART_SendStr(UART_PC, "\nNR_ger");
#endif
#endif  //ETHERNET_EN

            switch(check_string_nobuf("OK"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_GPS_HANDLER
                    // UWriteString((char*)"\ngEOF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\ngEOF");
#endif
                    gps_handler_state = GPS_CMD_ECHO_OFF;
                    if(gps_retry_cnt++ >= RETRY_CNT)
                    {
                        gps_retry_cnt = 0;
                        gps_wait_timeout = 0;

                        sts = GPS_FAIL;
                        gps_handler_state = GPS_CMD_ECHO_OFF;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\ngEOK",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\ngEOK");
// #endif
                    gps_retry_cnt = 0;
                    gps_wait_timeout = 0;
                    
                    gps_handler_state = GPS_CMD_POWER;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\ngEOW",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\nEOW");
// #endif
                    if(gps_wait_timeout++ >= ATE0_TIMEOUT)
                    {
                        gps_wait_timeout = 0;
                        gps_handler_state = GPS_CMD_ECHO_OFF;

                        if(gps_retry_cnt++ >= RETRY_CNT)
                        {
                            gps_retry_cnt = 0;

                            sts = GPS_FAIL;
                            gps_handler_state = GPS_CMD_ECHO_OFF;
                        }
                    }
                }
                break;

                default:
                {

                }
                break;
            }
        }
        break;

        case GPS_CMD_POWER:
        {
            set_gps_status(NOT_AVBL);
            // flushTxBuffer(LTE_UART);
#ifdef ETHERNET_EN
            flushRxBuffer(LTE_UART);
#else
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nGR_gp", DBG_UART);
                vUART_SendStr(UART_PC, "\nGR_gp");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_POWER;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
            else
#endif  //ETHERNET_EN
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nNR_gp", DBG_UART);
                vUART_SendStr(UART_PC, "\nNR_gp");
#endif
                flushRxBuffer(LTE_UART);
                // UWriteString((char*)"AT+CGNSSPWR=1\r\n", LTE_UART);
                vUART_SendStr(LTE_UART_BASE, "AT+CGNSSPWR=1\r\n");
                gps_handler_state = GPS_RSP_POWER;
            }
        }
        break;

        case GPS_RSP_POWER:
        {
#ifndef ETHERNET_EN
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\n1GR_gpr", DBG_UART);
                vUART_SendStr(UART_PC, "\n1GR_gpr");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_POWER;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
#ifdef DEBUG_QUERY_STATES
            // UWriteString((char*)"\n1NR_gpr", DBG_UART);
            vUART_SendStr(UART_PC, "\n1NR_gpr");
#endif
#endif  //ETHERNET_EN
            switch(check_string_nobuf("OK"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_GPS_HANDLER
                    // UWriteString((char*)"\n1PWF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1PWF");
#endif
                    gps_handler_state = GPS_CMD_POWER;
                    if(gps_retry_cnt++ >= RETRY_CNT)
                    {
                        gps_retry_cnt = 0;
                        gps_wait_timeout = 0;

                        sts = GPS_FAIL;
                        gps_handler_state = GPS_CMD_POWER_OFF;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\n1PWK",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\n1PWK");
// #endif
                    gps_retry_cnt = 0;
                    gps_wait_timeout = 0;

                    gps_handler_state = GPS_RSP_1_POWER;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\n1PWW",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\n1PWW");
// #endif
                    if(gps_wait_timeout++ >= GPS_TIMEOUT)
                    {
                        gps_wait_timeout = 0;
                        gps_handler_state = GPS_CMD_POWER;

                        if(gps_retry_cnt++ >= RETRY_CNT)
                        {
                            gps_retry_cnt = 0;

                            sts = GPS_FAIL;
                            gps_handler_state = GPS_CMD_POWER_OFF;
                        }
                    }
                }
                break;

                default:
                {

                }
                break;
            }
        }
        break;

        case GPS_RSP_1_POWER:
        {
#ifndef ETHERNET_EN
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\n2GR_gpr", DBG_UART);
                vUART_SendStr(UART_PC, "\n2GR_gpr");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_POWER;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
#ifdef DEBUG_QUERY_STATES
            // UWriteString((char*)"\n2NR_gpr", DBG_UART);
            vUART_SendStr(UART_PC, "\n2NR_gpr");
#endif
#endif  //ETHERNET_EN
            switch(check_string_nobuf("+CGNSSPWR: READY"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_GPS_HANDLER
                    // UWriteString((char*)"\n2PWF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n2PWF");
#endif
                    flushRxBuffer(LTE_UART);
                }
                break;

                case GPRS_MATCH_OK:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\n2PWK",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\n2PWK");
// #endif
                    gps_retry_cnt = 0;
                    gps_wait_timeout = 0;
                    gps_handler_state = GPS_CMD_LOCATION;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\n2PWW",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\n2PWW");
// #endif
                    if(gps_wait_timeout++ >= GPS_RDY_RSP_TIMEOUT)
                    {
#ifdef GPS_DEBUG_SHOW_FAILS
						// UWriteString("GPS_RDY:w", UART0);
                        vUART_SendStr(DEBUG_UART_BASE, "GPS_RDY:w");
#endif					
						if (gps_retry_cnt++ >= GPS_RETRY_CNT)
						{
							gps_wait_timeout = 0;
							gps_retry_cnt = 0;
							gps_handler_state = GPS_CMD_POWER_OFF;
							flushRxBuffer(LTE_UART);
						}
						else
						{
							gps_wait_timeout = 0;
							//gps_state = GPS_CMD_POWER;
							flushRxBuffer(LTE_UART);
						}
                    }
                }
                break;

                default:
                {

                }
                break;
            }
        }
        break;

        // case GPS_CMD_STATUS:
        // {
        //     flushTxBuffer(LTE_UART);
        //     flushRxBuffer(LTE_UART);
        // }
        // break;

        // case GPS_RSP_STATUS:
        // {

        // }
        // break;

        case GPS_CMD_LOCATION:
        {
            set_gps_status(NOT_AVBL);
            // flushTxBuffer(LTE_UART);
#ifdef ETHERNET_EN
            flushRxBuffer(LTE_UART);
#else
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nGR_gl", DBG_UART);
                vUART_SendStr(UART_PC, "\nGR_gl");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_LOCATION;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
            else
#endif  //ETHERNET_EN
            {
#ifdef DEBUG_QUERY_STATES
            // UWriteString((char*)"\nNR_gl", DBG_UART);
            vUART_SendStr(UART_PC, "\nNR_gl");
#endif
            flushRxBuffer(LTE_UART);
#ifdef GNS_PKT_EN
			// UWriteString((char *)"AT+CGNSSINFO\r", LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "AT+CGNSSINFO\r");
#else
			// UWriteString((char *)"AT+CGPSINFO\r", LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "AT+CGPSINFO\r");
#endif	//GNS_PKT_EN

            gps_handler_state = GPS_RSP_LOCATION;
            }
        }
        break;

        case GPS_RSP_LOCATION:
        {
#ifndef ETHERNET_EN
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nGR_glr", DBG_UART);
                vUART_SendStr(UART_PC, "\nGR_glr");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_LOCATION;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nNR_gl", DBG_UART);
                vUART_SendStr(UART_PC, "\nNR_gl");
#endif
#endif  //ETHERNET_EN

            char tmpstr[GPRS_RX_BUFFER_MAX];
            memset(tmpstr, 0, sizeof(tmpstr));

            gps_info_t temp_gps;
	
	        gps_decode_result_t decode_result = PARSE_FAIL;

            memset((void*)&temp_gps, 0, sizeof(temp_gps));

#ifdef GNS_PKT_EN
			char resp = check_string("+CGNSSINFO: ", tmpstr, &num_bytes);
#else
			char resp = check_string("+CGPSINFO: ", tmpstr, &num_bytes);
#endif	//GNS_PKT_EN
            switch(resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_GPS_HANDLER
                    // UWriteString((char*)"\nLOCF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nLOCF");
#endif
                    gps_handler_state = GPS_CMD_LOCATION;

                    gps_retry_cnt++;
				    if (gps_retry_cnt >= GPS_RETRY_CNT)
                    {
                        gps_retry_cnt = 0;
                        // gps_wait_timeout = 0;

                        sts = GPS_FAIL;
                        gps_handler_state = GPS_CMD_POWER_OFF;
                    }
                    gps_wait_timeout = 0;
                }
                break;

                case GPRS_MATCH_OK:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\nLOCK",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\nLOCK");
// #endif
                    gps_wait_timeout = 0;
                    gps_retry_cnt = 0;

                    if(count_comma(tmpstr, 68) == MIN_COMMAS)
                    {
// #ifdef DEBUG_GPS_HANDLER
//                         // UWriteString((char*)"\nLOCK2",DBG_UART);
//                         vUART_SendStr(DEBUG_UART_BASE, "\nLOCK2");
// #endif                        
                        decode_result = gps_pkt_parsing(&temp_gps, tmpstr);

                        if(decode_result >= GOT_LATLONG)
                        {
                            if(decode_result >= GOT_DATETIME)
                            {
                                updateGpsDateTimeToBuff(&gps_date_time);
                            }
                            get_location();
                            sts = GPS_PASS;
                            gps.gps_ready = TRUE;
                            set_gps_status(AVBL);
// #ifdef DEBUG_GPS_HANDLER
//                             vUART_SendStr(DEBUG_UART_BASE, "\ngotGPS:");
//                             vUART_SendInt(DEBUG_UART_BASE, gps_interval_start);
//                             vUART_SendChr(DEBUG_UART_BASE, ',');
//                             // vUART_SendInt(DEBUG_UART_BASE, ((my_millis() - gps_interval_start)/1000));
//                             vUART_SendInt(DEBUG_UART_BASE, (my_millis() - gps_interval_start));
// #endif
                            gps_interval_start = my_millis();
                            cnt_gps_1sec++;
                        }
                        else
                        {
// #ifdef DEBUG_GPS_HANDLER
//                             // UWriteString((char*)"\nres=",DBG_UART);
//                             // UWriteInt(decode_result,DBG_UART);
//                             vUART_SendStr(DEBUG_UART_BASE, "\nGPS=?");
//                             vUART_SendInt(DEBUG_UART_BASE, decode_result);
//                             vUART_SendChr(DEBUG_UART_BASE, ',');
//                             vUART_SendInt(DEBUG_UART_BASE, gps_interval_start);
//                             vUART_SendChr(DEBUG_UART_BASE, ',');
//                             vUART_SendInt(DEBUG_UART_BASE, ((my_millis() - gps_interval_start)/1000));
// #endif
                            sts = GPS_WAIT;
                        }
                    }
                    else
                    {
#ifdef DEBUG_GPS_HANDLER
                        // UWriteString((char*)"\nLOCKF2",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\nLOCKF2");
#endif                        
                    }

                    gps_handler_state = GPS_CMD_LOCATION;
                    // sts = GPS_PASS;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\nLOCW",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\nLOCKW");
// #endif
                    if(gps_wait_timeout++ >= GPS_TIMEOUT)
                    {
                        gps_wait_timeout = 0;
                        
                        gps_handler_state = GPS_CMD_LOCATION;
                        if(gps_retry_cnt++ >= RETRY_CNT)
                        {
                            gps_retry_cnt = 0;

                            sts = GPS_FAIL;
                            gps_handler_state = GPS_CMD_POWER_OFF;
                        }
                    }
                }
                break;

                default:
                {

                }
                break;
            }
        }
        break;

        // case GPS_RSP_1_LOCATION:
        // {

        // }
        // break;

        case GPS_CMD_POWER_OFF:
        {
            set_gps_status(NOT_AVBL);
            // flushTxBuffer(LTE_UART);
#ifdef ETHERNET_EN
            flushRxBuffer(LTE_UART);
#else
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nGR_gpo", DBG_UART);
                vUART_SendStr(UART_PC, "\nGR_gpo");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_POWER_OFF;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
            else
#endif  //ETHERNET_EN
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nNR_gpo", DBG_UART);
                vUART_SendStr(UART_PC, "\nNR_gpo");
#endif
                flushRxBuffer(LTE_UART);
                // UWriteString((char*)"AT+CGNSSPWR=0\r\n",LTE_UART);
                vUART_SendStr(LTE_UART_BASE, "AT+CGNSSPWR=0\r\n");
                gps_handler_state = GPS_RSP_POWER_OFF;
            }
        }
        break;

        case GPS_RSP_POWER_OFF:
        {
#ifndef ETHERNET_EN
            bool gotReq = server_query();
            if(gotReq)
            {
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nGR_rgpo", DBG_UART);
                vUART_SendStr(UART_PC, "\nGR_rgpo");
#endif
                gps_retry_cnt = 0;
                gps_wait_timeout = 0;
                gps_handler_state = GPS_CMD_POWER_OFF;
                sts = GPS_FAIL;
                HandleQueryStates();

                break;
            }
#ifdef DEBUG_QUERY_STATES
                // UWriteString((char*)"\nNR_rgpo", DBG_UART);
                vUART_SendStr(UART_PC, "\nNR_rgpo");
#endif
#endif  //ETHERNET_EN
            switch(check_string_nobuf("OK"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_GPS_HANDLER
                    // UWriteString((char*)"\nPDF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nPDF");
#endif
                    gps_handler_state = GPS_CMD_POWER_OFF;
                    if(gps_retry_cnt++ >= RETRY_CNT)
                    {
                        gps_retry_cnt = 0;
                        gps_wait_timeout = 0;

                        sts = GPS_FAIL;
                        gps_handler_state = GPS_CMD_POWER_OFF;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\nPDK",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\nPDK");
// #endif
                    gps_retry_cnt = 0;
                    gps_wait_timeout = 0;

                    gps_handler_state = GPS_CMD_POWER;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
// #ifdef DEBUG_GPS_HANDLER
//                     // UWriteString((char*)"\nPDW",DBG_UART);
//                     vUART_SendStr(DEBUG_UART_BASE, "\nPDW");
// #endif
                    if(gps_wait_timeout++ >= GPS_TIMEOUT)
                    {
                        gps_wait_timeout = 0;

                        gps_handler_state = GPS_CMD_POWER_OFF;
                        if(gps_retry_cnt++ >= RETRY_CNT)
                        {
                            gps_retry_cnt = 0;

                            sts = GPS_FAIL;
                            gps_handler_state = GPS_CMD_ECHO_OFF;
                        }
                    }
                }
                break;

                default:
                {

                }
                break;
            }
        }
        break;

        default:
        {

        }
        break;
    }

    return sts;
}

gps_decode_result_t gps_pkt_parsing(gps_info_t *temp_gps, char *tmpstr)
{
	unsigned int i = 0, cnt = 0; // j = 0, k = 0;
	gps_decode_result_t retVal = PARSE_FAIL;
	
#ifdef DEBUG_GPS_DECODE
    // UWriteString((char*)"\nToParse=",DBG_UART);
    // UWriteString(tmpstr,DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE, "\nToParse=");
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)tmpstr);
	// printf("\nStrToParse=%s\n",tmpstr);
	// printBytes((unsigned char*)tmpstr, GPRS_RX_BUFFER_MAX);
#endif

	if(isdigit(tmpstr[i]))
	{
		for(cnt = 0; tmpstr[i] != ','; cnt++)
		{
			//total size = 11B(including null). total recieved = 10B. cnt will increase 0 to 9, so if it's >= 10, we will return false.
			if(cnt >= (LAT_LEN - 1))	
			{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\nLatF1=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\nLatF1=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
                // printf("\nLatF1=%d,%d",cnt,i);
#endif
				
				retVal = PARSE_FAIL;
				return retVal;
			}
			temp_gps->latitude[cnt] = tmpstr[i];
			
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\n1tLA=,cnt=",DBG_UART);
            // UWriteInt(temp_gps->latitude[cnt],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteString((char*)"\n1tmp=,i=",DBG_UART);
            // UWriteInt(tmpstr[i],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            // // printf("\n1temp_lat[%d]=%c,tmpstr[%d]=%c",cnt, temp_gps->latitude[cnt], i,tmpstr[i]);
#endif
			i++;
		}
		//i and cnt should be 10 here at the end of the loop due to post increment.
		
		//due to post increment, cnt should'nt be less than 10, neither should we recieve another ',' immediately after. Coz that'll make tmp_lat[] empty.
		if(/*(cnt < (LAT_LEN - 1)) ||*/ (temp_gps->latitude[0] == '\0'))	
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nLatF2=",DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nLatF2=");
            vUART_SendInt(DEBUG_UART_BASE, cnt);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
            // printf("\nLatF1=%d,%d",cnt,i);
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}
		//printf("\nstrlen(temp_lat)=%d,sizeof(temp_lat)=%d, tmpstr[%d]=%c,cnt=%d",strlen((const char*)temp_gps->latitude),sizeof(temp_gps->latitude),i,tmpstr[i],cnt);
		
		
		//before incrementing i and parsing N/S direction, we should check without loop whether next byte is not ',' 
		//(I tested isalpha() here, but it returned true for ',') and also check if the current byte is a ',' or not (although that'll be done already by the previous loop).
		if((tmpstr[i] != ',') || (tmpstr[i+1] == ','))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF2",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF2");
			// printf("\nGF2");
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}

		// printf("\nisdigit(tmpstr[%d+1])=%d,%c",i,isdigit(tmpstr[i+1]),tmpstr[i+1]);
		i++;	//i should be 11 here.
		
		temp_gps->N_S = tmpstr[i];	
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\nN_S=",DBG_UART); 
        // UWriteData(temp_gps->N_S,DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\nN_S=");
        vUART_SendChr(DEBUG_UART_BASE, temp_gps->N_S);
		// printf("\nN_S=%c",temp_gps->N_S);
#endif
		
		if((temp_gps->N_S != 'N')&&(temp_gps->N_S != 'S'))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF3",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF3");
			// printf("\nGF3");
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}

#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif

//		if((tmpstr[i+1] != ',') || (tmpstr[i+1] == '\0'))	//No need to check null coz if that was the case count_comma() would've returned <8 commas and we would'nt be here coz that uses strlen().
		if(tmpstr[i+1] != ',')
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF4",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF4");
			// printf("\nGF4");
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}
		
		i++;	//i should be 12 here.
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif

		if((tmpstr[i] != ',') || (tmpstr[i+1] == ','))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF5",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF5");
			// printf("\nGF5");
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}
		
		i++;	//i should be 13 here.
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		
		for(cnt = 0; tmpstr[i] != ','; cnt++)
		{
			//total size = 12B(including null). total recieved = 11B. cnt will increase 0 to 10, so if it's >= 11, we will return false.
			if(cnt >= (LONG_LEN - 1))	
			{

#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\nLongF1=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\nLongF1=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
                // printf("\nLongF1=%d,%d",cnt,i);
#endif
				retVal = PARSE_FAIL;
				return retVal;
			}
			temp_gps->longitude[cnt] = tmpstr[i];
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\n1tLO=,cnt=",DBG_UART);
            // UWriteInt(temp_gps->longitude[cnt],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteString((char*)"\n1tmp=,i=",DBG_UART);
            // UWriteInt(tmpstr[i],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            // // printf("\n1temp_long[%d]=%c,tmpstr[%d]=%c",cnt, temp_gps->longitude[cnt], i,tmpstr[i]);
#endif	
			i++;
		}
		//i should be 24 and cnt should be 11 here at the end of loop due to post increment.
		
		//due to post increment, cnt should'nt be less than 11, neither should we recieve another ',' immediately after. Coz that'll make tmp_long[] empty.
		if(/*(cnt < (LONG_LEN - 1)) ||*/ (temp_gps->longitude[0] == '\0'))	
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nLongF2=",DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nLongF2=");
            vUART_SendInt(DEBUG_UART_BASE, cnt);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
            // printf("\nLongF2=%d,%d",cnt,i);
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}
		// printf("\nstrlen(temp_long)=%d,sizeof(temp_long)=%d, tmpstr[%d]=%c,cnt=%d",strlen((const char*)temp_gps->longitude),sizeof(temp_gps->longitude),i,tmpstr[i],cnt);
		
		//before incrementing i and parsing E/W direction, we should check without loop whether next byte is not ',' ) 
		//and also check if the current byte is a ',' or not (although that'll be done already by the previous loop).
		if((tmpstr[i] != ',') || (tmpstr[i+1] == ','))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF6",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF6");
			// printf("\nGF6");
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}
		
		i++;	//i should be 25 here
		
		temp_gps->E_W = tmpstr[i];	
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\nE_W=",DBG_UART); 
        // UWriteData(temp_gps->E_W,DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\nE_W=");
        vUART_SendChr(DEBUG_UART_BASE, temp_gps->E_W);
		// printf("\nE_W=%c",temp_gps->E_W);
#endif
		if((temp_gps->E_W != 'E')&&(temp_gps->E_W != 'W'))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF7",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF7");
			// printf("\nGF7");
#endif
			retVal = PARSE_FAIL;
			return retVal;
		}

		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		
		retVal = GOT_LATLONG;
		
		memset(gps.gps_info.latitude, 0, LAT_LEN);
		memcpy(gps.gps_info.latitude, temp_gps->latitude, strlen((const char*)temp_gps->latitude));
		
		gps.gps_info.N_S = temp_gps->N_S;
		
		memset(gps.gps_info.longitude, 0, LONG_LEN);
		memcpy(gps.gps_info.longitude, temp_gps->longitude, strlen((const char*)temp_gps->longitude));
		
		gps.gps_info.E_W = temp_gps->E_W;
		
		if(tmpstr[i+1] != ',')
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF8",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF8");
			// printf("\nGF8");
#endif
			retVal = DATETIME_FAIL;
			return retVal;
		}
		
		i++;	//i should be 26 here
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		if((tmpstr[i] != ',') || (tmpstr[i+1] == ','))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF9",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF9");
			// printf("\nGF9");
#endif
			memset(&temp_gps->date, 0, sizeof(temp_gps->date));
			retVal = DATETIME_FAIL;
			return retVal;
		}
		
		i++;	//i should be 27 here.
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		
		for(cnt = 0; tmpstr[i] != ','; cnt++)
		{
			//total size = 7B(including null). total recieved = 6B. cnt will increase 0 to 5, so if it's >= 6, we will return false.
			if(cnt >= (MAX_DATE_GPS_SIZE - 1))
			{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\ngdF1=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\ngdF1=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
				// printf("\ngdF1=%d,%d",cnt,i);
#endif
				retVal = DATETIME_FAIL;
				return retVal;
			}
			
			temp_gps->date[cnt] = tmpstr[i];

#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\n1tD=,cnt=",DBG_UART);
            // UWriteInt(temp_gps->date[cnt],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteString((char*)"\n1tmp=,i=",DBG_UART);
            // UWriteInt(tmpstr[i],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            // // printf("\n1temp_date[%d]=%c,tmpstr[%d]=%c",cnt, temp_gps->date[cnt], i,tmpstr[i]);
#endif
			
			i++;
		}
		//i should be 33 and cnt should be 6 here at the end of loop due to post increment.
		
		//due to post increment, cnt should'nt be less than 6, neither should we recieve another ',' immediately after. Coz that'll make tmp_date[] empty.
		if(/*(cnt < (MAX_DATE_GPS_SIZE - 1)) ||*/ (temp_gps->date[0] == '\0'))	
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\ngdF2=",DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\ngdF2=");
            vUART_SendInt(DEBUG_UART_BASE, cnt);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
			// printf("\ngdF2=%d,%d",cnt,i);
#endif
			retVal = DATETIME_FAIL;
			return retVal;
		}
		// printf("\nstrlen(temp_date)=%d,sizeof(temp_date)=%d, tmpstr[%d]=%c,cnt=%d",strlen((const char*)temp_gps->date),sizeof(temp_gps->date),i,tmpstr[i],cnt);
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		if((tmpstr[i] != ',') || (tmpstr[i+1] == ','))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF10",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF10");
			// printf("\nGF10");
#endif
			retVal = DATETIME_FAIL;
			return retVal;
		}
		
		i++;	//i should be 34 here.
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		
		for(cnt = 0; tmpstr[i] != ','; cnt++)
		{
			if(cnt >= (MAX_TIME_SIZE - 1))
			{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\ngtF1=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\ngtF1=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
				// printf("\ngtF1=%d,%d",cnt,i);
#endif
				retVal = DATETIME_FAIL;
				return retVal;
			}
			
			if((tmpstr[i] != '.') /*&& cnt < (MAX_TIME_SIZE - 1)*/)
			{
				temp_gps->utc_time[cnt] = tmpstr[i];
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\n1tt=,cnt=",DBG_UART);
                // UWriteInt(temp_gps->utc_time[cnt],DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteString((char*)"\n1tmp=,i=",DBG_UART);
                // UWriteInt(tmpstr[i],DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                // // printf("\n1temp_time[%d]=%c,tmpstr[%d]=%c",cnt, temp_gps->utc_time[cnt], i,tmpstr[i]);
#endif
			}
			else if(tmpstr[i] == '.')
			{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\n2tt[i+3]=,i=",DBG_UART);
                // UWriteInt(temp_gps->utc_time[i+3],DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\n2tt[i+3]=,i=");
                vUART_SendInt(DEBUG_UART_BASE, temp_gps->utc_time[i+3]);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
                // printf("\n2tmpstr[%d + 3]=%c", i,tmpstr[i+3]);
#endif		
				cnt--;
			}
			
			i++;
		}
		//i should be 43 and cnt should be 8 here at the end of loop due to post increment.
		
		//due to post increment, cnt should'nt be less than 8, neither should we recieve another ',' immediately after. Coz that'll make tmp_date[] empty.
		if(/*(cnt < (MAX_TIME_SIZE - 1)) ||*/ (temp_gps->date[0] == '\0'))	
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\ngtF2=",DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\ngtF2=");
            vUART_SendInt(DEBUG_UART_BASE, cnt);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
            // printf("\ngtF2=%d,%d",cnt,i);
#endif
			
			retVal = DATETIME_FAIL;
			return retVal;
		}
		// printf("\nstrlen(temp_time)=%d,sizeof(temp_time)=%d, tmpstr[%d]=%c,cnt=%d",strlen((const char*)temp_gps->utc_time),sizeof(temp_gps->utc_time),i,tmpstr[i],cnt);
		
		retVal = GOT_DATETIME;
		
		memset(&gps.gps_info.date, 0, sizeof(gps.gps_info.date));
		memcpy(gps.gps_info.date, temp_gps->date, strlen((const char*)temp_gps->date));
		
		memset(&gps.gps_info.utc_time, 0, sizeof(gps.gps_info.utc_time));
		memcpy(gps.gps_info.utc_time, temp_gps->utc_time, strlen((const char*)temp_gps->utc_time));
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		if((tmpstr[i] != ',') || (tmpstr[i+1] == ','))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF11",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF11");
			// printf("\nGF11");
#endif
			retVal = ALT_FAIL;
			return retVal;
		}
		
		i++;	//i should be 44 here.
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		
		for(cnt = 0; tmpstr[i] != ','; cnt++)
		{
			if(cnt >= (sizeof(temp_gps->msl_altitude) - 1))
			{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\ngAF1=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\ngAF1=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
                // pprintf("\ngAF1=%d,%d",cnt,i);
#endif
				retVal = ALT_FAIL;
				return retVal;
			}
			temp_gps->msl_altitude[cnt] = tmpstr[i];
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\n1tA=,cnt=",DBG_UART);
                // UWriteInt(temp_gps->msl_altitude[cnt],DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteString((char*)"\n1tmp=,i=",DBG_UART);
                // UWriteInt(tmpstr[i],DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                // // printf("\n1temp_alt[%d]=%c,tmpstr[%d]=%c",cnt, temp_gps->msl_altitude[cnt], i,tmpstr[i]);
#endif
			i++;
		}
		//i should be 49 here and cnt should be 5 here at the end of loop.
		//printf("\ncnt=%d,i=%d",cnt, i);
		
		//due to post increment, cnt should'nt be less than 5, neither should we recieve another ',' immediately after. Coz that'll make tmp_date[] empty.
		if(/*(cnt < (sizeof(temp_gps->msl_altitude) - 1)) ||*/ (temp_gps->msl_altitude[0] == '\0'))	
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\ngAF2=",DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\ngAF2=");
            vUART_SendInt(DEBUG_UART_BASE, cnt);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
            // printf("\ngAF2=%d,%d",cnt,i);
#endif
			retVal = ALT_FAIL;
			return retVal;
		}	//commenting this coz we don't know the minimum length
		// printf("\nstrlen(temp_alt)=%d,sizeof(temp_alt)=%d, tmpstr[%d]=%c,cnt=%d",strlen((const char*)temp_gps->msl_altitude),sizeof(temp_gps->msl_altitude),i,tmpstr[i],cnt);
		
		retVal = GOT_ALT;
		memset(&gps.gps_info.msl_altitude, 0, sizeof(gps.gps_info.msl_altitude));
		memcpy(gps.gps_info.msl_altitude, temp_gps->msl_altitude, strlen((const char*)temp_gps->msl_altitude));
		
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		if((tmpstr[i] != ',') || (tmpstr[i+1] == ','))
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\nGF12",DBG_UART); 
            vUART_SendStr(DEBUG_UART_BASE, "\nGF12");
			// printf("\nGF12");
#endif
			retVal = SOG_FAIL;
			return retVal;
		}
		
		i++;	//i should be 50 here.
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		
		for(cnt = 0; tmpstr[i] != ','; cnt++)
		{
			if(cnt >= (sizeof(temp_gps->sog) - 1))
			{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\ngsF1=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\ngsF1=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
                // printf("\ngsF1=%d,%d",cnt,i);
#endif
				retVal = SOG_FAIL;
				return retVal;
			}
			temp_gps->sog[cnt] = tmpstr[i];
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\n1tS=,cnt=",DBG_UART);
            // UWriteInt(temp_gps->sog[cnt],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteString((char*)"\n1tmp=,i=",DBG_UART);
            // UWriteInt(tmpstr[i],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            // // printf("\n1temp_sog[%d]=%c,tmpstr[%d]=%c",cnt, temp_gps->sog[cnt], i,tmpstr[i]);
#endif
			i++;
		}
		//i should be 55 here and cnt should be 5 here at the end of loop.
		//printf("\ncnt=%d,i=%d",cnt, i);
		
		//due to post increment, cnt should'nt be less than 5, neither should we recieve another ',' immediately after. Coz that'll make tmp_date[] empty.
		if(/*(cnt < (sizeof(temp_gps->sog) - 1)) ||*/ (temp_gps->sog[0] == '\0'))	
		{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\ngsF2=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\ngsF2=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
                // printf("\ngsF2=%d,%d",cnt,i);
#endif
			retVal = SOG_FAIL;
			return retVal;
		}	//commenting this coz we don't know the minimum length
		// printf("\nstrlen(temp_sog)=%d,sizeof(temp_sog)=%d, tmpstr[%d]=%c,cnt=%d",strlen((const char*)temp_gps->sog),sizeof(temp_gps->sog),i,tmpstr[i],cnt);
		
		retVal = GOT_SOG;
		memset(&gps.gps_info.sog, 0, sizeof(gps.gps_info.sog));
		memcpy(gps.gps_info.sog, temp_gps->sog, strlen((const char*)temp_gps->sog));
		
		i++;	//i should be 56 here.
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\ntmpstr[i]=",DBG_UART); 
        // UWriteData(tmpstr[i],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // // printf("\ntmpstr[%d]=%c",i,tmpstr[i]);
#endif
		
		for(cnt = 0; tmpstr[i] != '\r' && tmpstr[i] != '\0'; cnt++)
		{
			if(cnt >= (sizeof(temp_gps->cog) - 1))
			{
#ifdef DEBUG_GPS_DECODE
                // UWriteString((char*)"\ngcF1=",DBG_UART);
                // UWriteInt(cnt,DBG_UART);
                // UWriteData(',',DBG_UART);
                // UWriteInt(i,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\ngcF1=");
                vUART_SendInt(DEBUG_UART_BASE, cnt);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
                // printf("\ngcF1=%d,%d",cnt,i);
#endif
				retVal = COG_FAIL;
				return retVal;
			}
			temp_gps->cog[cnt] = tmpstr[i];

#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\n1tC=,cnt=",DBG_UART);
            // UWriteInt(temp_gps->cog[cnt],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteString((char*)"\n1tmp=,i=",DBG_UART);
            // UWriteInt(tmpstr[i],DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            // // printf("\n1temp_cog[%d]=%c,tmpstr[%d]=%c",cnt, temp_gps->cog[cnt], i,tmpstr[i]);
#endif
			i++;
		}
		//i should be 55 here and cnt should be 5 here at the end of loop.
		//printf("\ncnt=%d,i=%d",cnt, i);
		
		//due to post increment, cnt should'nt be less than 5, neither should we recieve another ',' immediately after. Coz that'll make tmp_date[] empty.

		if(/*(cnt < (sizeof(temp_gps->cog) - 1)) ||*/ (temp_gps->cog[0] == '\0'))	
		{
#ifdef DEBUG_GPS_DECODE
            // UWriteString((char*)"\ngcF2=",DBG_UART);
            // UWriteInt(cnt,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(i,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\ngcF2=");
            vUART_SendInt(DEBUG_UART_BASE, cnt);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
            // printf("\ngcF2=%d,%d",cnt,i);
#endif
			retVal = COG_FAIL;
			return retVal;
		}	//commenting this coz we don't know the minimum length
		// printf("\nstrlen(temp_cog)=%d,sizeof(temp_cog)=%d, tmpstr[%d]=%02X,cnt=%d",strlen((const char*)temp_gps->cog),sizeof(temp_gps->cog),i,tmpstr[i],cnt);
		//printf("\nstrlen(temp_cog)=%d,sizeof(temp_cog)=%d, tmpstr[%d]=%c,cnt=%d");
//		printf("\nstrlen(temp_cog)=%d",strlen((const char*)temp_gps->cog));

		retVal = GOT_COG;
		memset(&gps.gps_info.cog, 0, sizeof(gps.gps_info.cog));
		memcpy(gps.gps_info.cog, temp_gps->cog, strlen((const char*)temp_gps->cog));
	}
	else
	{
#ifdef DEBUG_GPS_DECODE
        // UWriteString((char*)"\nGF1",DBG_UART); 
        vUART_SendStr(DEBUG_UART_BASE, "\nGF1");
        // printf("\nGF1");
#endif
		retVal = PARSE_FAIL;
	}

	return retVal;
}

void updateGpsDateTimeToBuff(gps_date_time_t *date_time)
{
    gps_date_time_t temp_time;
#ifdef GNS_PKT_EN
	temp_time.yy = (((gps.gps_info.date[2] -'0') * 10) + ((gps.gps_info.date[3] - '0')));
	temp_time.mm = (((gps.gps_info.date[4] -'0') * 10) + ((gps.gps_info.date[5] - '0')));
	temp_time.dd = (((gps.gps_info.date[6] - '0')* 10) + ((gps.gps_info.date[7] - '0')));
#else
	temp_time.dd = (((gps.gps_info.date[0] -'0') * 10) + ((gps.gps_info.date[1] - '0')));
	temp_time.mm = (((gps.gps_info.date[2] -'0') * 10) + ((gps.gps_info.date[3] - '0')));
	temp_time.yy = (((gps.gps_info.date[4] - '0')* 10) + ((gps.gps_info.date[5] - '0')));
#endif

#ifdef DEBUG_GET_LOC
		//printf("\ntemp_date1:%02d-%02d-%02d",temp_time.dd,temp_time.mm,temp_time.yy);		
#endif

	temp_time.hr = (((gps.gps_info.utc_time[0] - '0') * 10) + ((gps.gps_info.utc_time[1] - '0')));
	temp_time.min = (((gps.gps_info.utc_time[2] - '0') * 10) + ((gps.gps_info.utc_time[3] - '0')));
	temp_time.sec = (((gps.gps_info.utc_time[4] - '0') * 10) + ((gps.gps_info.utc_time[5] - '0')));

#ifdef DEBUG_GET_LOC
		//printf("\ntemp_time1:%02d:%02d:%02d",temp_time.hr,temp_time.min,temp_time.sec);		
#endif

    if (((temp_time.dd <= 0) || (temp_time.dd > 31)) ||
	((temp_time.mm <= 0) || (temp_time.mm > 12)) ||
	((temp_time.yy < (DEFAULT_YEAR)) || (temp_time.yy > (DEFAULT_YEAR) + YEAR_OFFSET))||			// Assuming that RTC will never go below 2016.
	(/*(temp_time.hr < 0) ||*/ (temp_time.hr > 23)) ||
	(/*(temp_time.min < 0) || */(temp_time.min > 59)) ||
	(/*(temp_time.sec < 0) || */(temp_time.sec > 59)))
    {
        gps_date_time.update_time_aval = false;
    }
    else
    {
        utcTOlocal(&temp_time);
        if (((temp_time.yy >= (DEFAULT_YEAR)) && (temp_time.yy <= (DEFAULT_YEAR) + YEAR_OFFSET)) &&
		((temp_time.mm >= 1) && (temp_time.mm <= 12)) &&
		((temp_time.dd >= 1) && (temp_time.dd <= 31)) &&
		(/*(temp_time.hr >= 0) &&*/ (temp_time.hr <= 23)) &&
		(/*(temp_time.min >= 0) &&*/ (temp_time.min <= 59)) &&
		(/*(temp_time.sec >= 0) &&*/ (temp_time.sec <= 59)))
        {
            date_time->yy = temp_time.yy;
            date_time->mm = temp_time.mm;
            date_time->dd = temp_time.dd;
            date_time->hr = temp_time.hr;
            date_time->min = temp_time.min;
            date_time->sec = temp_time.sec;

            gps_date_time.update_time_aval = true;
        }
        else
        {
          gps_date_time.update_time_aval = false;
        }
    }
}

void utcTOlocal(gps_date_time_t *timeT) 
{
    int DayNum;
    
	timeT->hr += 5;
	timeT->min += 30;
	

    if(timeT->min > 59)
	{
		int m;
		m = (timeT->min / 60);
		timeT->hr = (timeT->hr + m);
		timeT->min = (timeT->min % 60);
	}
	if(timeT->hr > 23)
	{
		int h;
		h = (timeT->hr / 24);
		timeT->dd = (timeT->dd + h);
		timeT->hr = (timeT->hr % 24);
	}
	if((timeT->mm == 1) || (timeT->mm == 3) || (timeT->mm == 5) || (timeT->mm == 7) || (timeT->mm == 8) || (timeT->mm == 10) || (timeT->mm == 12))
	{
		DayNum = 31;
	}
	if((timeT->mm == 4) || (timeT->mm == 6) || (timeT->mm == 9) || (timeT->mm == 11))
	{
		DayNum = 30;
	}
	if(timeT->mm == 2)
	{
	    if ((timeT->yy % 4) == 0)
	    {
	        if((timeT->yy % 100) == 0)
	        {
	            if((timeT->yy % 400) == 0)
	            {
	                DayNum = 29;
	            }
	            else
        		{
        			DayNum = 28;
        		}
	        }
	        else
    		{
    			DayNum = 29;
    		}
	    }
        else
        {
            DayNum = 28;
        }
    
    }
    if(timeT->dd > DayNum)
	{
		int d;
		d = (timeT->dd / DayNum);
		timeT->mm = (timeT->mm + d);
		timeT->dd = (timeT->dd % DayNum);
	}
	if(timeT->mm > 12)
	{
		int M;
		M = (timeT->mm / 12);
		timeT->yy = (timeT->yy + M);
		timeT->mm = (timeT->mm % 12);
	}
}

gprs_status_t get_gps_status()
{
    return gps.gps_status;
}

void set_gps_status(gprs_status_t sts)
{
    gps.gps_status = sts;
}
// #endif  // ifNOTdef ETHERNET_EN


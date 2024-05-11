/*
 * Web_Comm.cpp
 *
 *  Created on: May 1, 2024
 *      Author: ADMIN
 */

#include "Web_Comm.h"
#include "_config.h"

#ifndef ETHERNET_EN

#include "gprs.h"
#include "UartCore.h"

#include "gps.h"
#include "_debug.h"
#include "SysTick_Timer.h"
#include "main.h"

extern gprs_t gprs;
extern gps_t gps;

#if 0
// conn_state_t conn_state = CONNECT_LOCATION;
unsigned int gps_read_timeout = 0;
conn_state_t conn_state = CONNECT_POWER_ON;
unsigned int gprs_read_timeout = 0;

/*
	Description :- This function will manage both GPS and GPRS functionality of SIM808.
	- Switch between gps and gprs. By default GPRS and periodically GPS 
*/
void manage_gps_gprs(void)
{
#ifdef DEBUG_POWER_ON_RESTART
UWriteString("P1",DBG_UART);
#endif

	gps_status_t sts = GPS_IN_PRG;

	static unsigned int timeout = 0;

    switch (conn_state)
    {
        case CONNECT_POWER_ON:
        {
            if (timeout++ >= WARMUP_SECS)
            {
                timeout = 0;

                conn_state = CONNECT_LOCATION;
#ifdef DEBUG_WEBB_COMM
                // UWriteString((char *)"GPS:", DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\nGPS");
#endif
            }
        }
        break;

        case CONNECT_LOCATION:
        {
            sts = gps_handler();
    
            if ((sts == GPS_WAIT) || (sts == GPS_PASS))
            {
#ifdef DEBUG_WEBB_COMM
                // UWriteString((char *)"GPRS:", DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\nGPRS");
#endif			
                conn_state = CONNECT_DATA_UPLOAD;
                gprs_read_timeout = 0;
            }
            else
            {
                if (gprs_read_timeout++ >= GPS_FAIL_TIMEOUT)
                {
#ifdef DEBUG_WEBB_COMM
                    // UWriteString((char *)"GPRS_T:", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nGPRS_T");
#endif					
                    conn_state = CONNECT_DATA_UPLOAD;
                    gprs_read_timeout = 0;
                
                }
            }
            //if (gps.gps_ready)
            {
                gps.timeout  = GPS_READ_RATE_C;
            }
            /*else
            {
                gps.timeout = GPS_READ_RATE_NC;
                if(getFdmState() == FDM_DISPENSE_FUEL)
                {
                    gps.timeout  = GPS_READ_RATE_C;
                }
            }*/
        }
        break;

        case CONNECT_DATA_UPLOAD:
        {
            TCP_Handler();

            //if((gps_read_timeout++ >= GPS_READ_RATE))
            if((gps_read_timeout++ >= gps.timeout))
            {

                if (gprs.gprs_handler_state != GPRS_LOGS_UPLOAD)
                {
#ifdef DEBUG_WEBB_COMM
                    // UWriteString((char *)"GPS:", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nGPS_T");
#endif
                    conn_state = CONNECT_LOCATION;
                    gps_read_timeout = 0;
                }	
            }
        }
        break;

        default:
        {
#ifdef DEBUG_WEBB_COMM
            // UWriteString((char *)"\n???", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\n???");
#endif
        }
        break;
    }
#ifdef DEBUG_POWER_ON_RESTART
UWriteData('O',DBG_UART);
#endif
}
#else

extern gps_handler_state_t gps_handler_state;
extern unsigned int upload_time;

uint32_t gps_read_timeout = 0;
uint32_t gprs_read_timeout = 0;

conn_state_t conn_state = CONNECT_BEGIN;    //PP added on 05-10-23 (among other webcomm, gprs, gps chnges regarding their return statuses)

void manage_gps_gprs(void)
{
    // static uint32_t gprs_strt = 0;
    // static uint32_t gps_strt = 0;

    static uint32_t gps_diff = 0;
    // static uint32_t gprs_diff = 0;

    static unsigned int LTE_bootUp_timeout = 0;

    gps_status_t gps_sts = GPS_IN_PRG;

    switch(conn_state)
    {
        case CONNECT_BEGIN:
        {
#ifdef DEBUG_TCP_HANDLER
            // UWriteString((char*)"\nCB:", DBG_UART);
            // UWriteInt(LTE_bootUp_timeout, DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nCB:");
            vUART_SendInt(DEBUG_UART_BASE, LTE_bootUp_timeout);
#endif
            if(LTE_bootUp_timeout++ >= LTE_WARMUP_TIME)
            {
                LTE_bootUp_timeout = 0;
                conn_state = CONNECT_LOCATION;
            }
        }
        break;

        case CONNECT_LOCATION:
        {
#ifdef ENABLE_WDT_RESET
            if(get_pending_request())
            {
#ifdef DEBUG_WEBB_COMM
                UWriteString((char*)"\nRST2",DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\nRST2");
#endif
                // LTEmodule.HandlerSts = GPRS_TCP_DISCONNECT; 
                LTEmodule.HandlerSts = GPRS_WEBSOCKET_DISCONNECT; 
                conn_state = CONNECT_DATA_UPLOAD; 
            }
#endif //ENABLE_WDT_RESET               

            gps_sts = gps_handler();
            if ((gps_sts == GPS_WAIT) || (gps_sts == GPS_PASS))
            {
                gps_diff = my_millis() - gprs_read_timeout;
#ifdef DEBUG_WEBB_COMM
                // UWriteString((char *)"\nGPRS:", DBG_UART);
                // // UWriteInt(gprs_read_timeout,DBG_UART);
                // UWriteInt(gps_diff,DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\nGPRS:");
                vUART_SendInt(DEBUG_UART_BASE, gprs_read_timeout);
#endif			
                conn_state = CONNECT_DATA_UPLOAD;
                // gprs_read_timeout = 0;
            }
            else
            {
#ifdef DEBUG_WEBB_COMM
                // UWriteString((char *)"\ndiff:", DBG_UART);
                // UWriteInt((millis() - gprs_read_timeout),DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\ndiff:");
                vUART_SendInt(DEBUG_UART_BASE, (my_millis() - gprs_read_timeout));
#endif                
                // gprs_read_timeout += 50;
                // if (gprs_read_timeout++ >= GPS_FAIL_TIMEOUT)
                gps_diff = my_millis() - gprs_read_timeout;
                // if(millis() - gprs_read_timeout >= 20000UL)
                if(gps_diff >= 20000UL)
                {
#ifdef DEBUG_WEBB_COMM
                    // UWriteString((char *)"\nGPRS_T:", DBG_UART);
                    // UWriteInt(gprs_read_timeout,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nGPRS_T:");
                    vUART_SendInt(DEBUG_UART_BASE, gprs_read_timeout);      
#endif					
                    conn_state = CONNECT_DATA_UPLOAD;
                    // gprs_read_timeout = 0;
                    // gprs_read_timeout = millis();
                }
            }
        }
        break;

        case CONNECT_DATA_UPLOAD:
        {
#ifdef DEBUG_WEBB_COMM
            // UWriteString((char *)"\nCDU:", DBG_UART);
            // UWriteInt(gps_read_timeout,DBG_UART);
#endif
            TCP_Handler();

            if((gps_read_timeout++ >= GPS_READ_RATE_C))
            {
                // if(/* (gprs.gprs_handler_state != GPRS_LOGS_UPLOAD) && */ (gprs.gprs_handler_state == GPRS_SESSION_IDLE))
                if((upload_time < 25) && (gprs.gprs_handler_state == GPRS_SESSION_IDLE))
                {
                    // gprs_read_timeout = millis();
#ifdef DEBUG_WEBB_COMM
                    // UWriteString((char *)"\nGPS:", DBG_UART);
                    // UWriteInt(gps_handler_state,DBG_UART);
                    // UWriteData(',',DBG_UART);
                    // // UWriteInt(gps_read_timeout,DBG_UART);
                    // // UWriteInt(gprs_read_timeout,DBG_UART);
                    // UWriteInt((my_millis() - gprs_read_timeout), DBG_UART);

                    vUART_SendStr(DEBUG_UART_BASE, "\nGPS_T:");
                    vUART_SendInt(DEBUG_UART_BASE, gps_handler_state);      
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendInt(DEBUG_UART_BASE, (my_millis() - gprs_read_timeout));
#endif
                    // if((gps_handler_state == GPS_RSP_CMD_ENABLE) || (gps_handler_state == GPS_CMD_LOC))
                    // {
                    //     gps_handler_state = GPS_CMD_LOC;
                    // }
                    // else
                    // {
                    //     gps_handler_state = GPS_ENABLE;
                    // }
                    gprs_read_timeout = my_millis();
#ifdef DEBUG_WEBB_COMM
                    // UWriteString((char *)"\nstrt:", DBG_UART);
                    // UWriteInt(gprs_read_timeout,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nstrt:");
                    vUART_SendInt(DEBUG_UART_BASE, gprs_read_timeout);
#endif
                    conn_state = CONNECT_LOCATION;
                    gps_read_timeout = 0;                    
                }
            }
#ifdef ENABLE_WDT_RESET
            else if(get_pending_request())
            {
#ifdef DEBUG_WEBB_COMM
                UWriteString((char*)"\nRST1",DBG_UART);
#endif
                // LTEmodule.HandlerSts = GPRS_TCP_DISCONNECT;
                LTEmodule.HandlerSts = GPRS_WEBSOCKET_DISCONNECT;
            }
#endif  //ENABLE_WDT_RESET
        }
        break;

        default:
        {

        }
        break;
    }
}

#endif  // if 0



#endif  //ifNOTdef ETHERNET_EN




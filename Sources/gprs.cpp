/*
 * gprs.cpp
 *
 *  Created on: Apr 20, 2024
 *      Author: ADMIN
 */
#include <ctype.h>
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "time.h"

#include "_config.h"
#include "_debug.h"
#include "gprs.h"

#include "UartCore.h"
#include "Clock.h"
#include "_common.h"
#include "E2P.h"
#include "Display.h"
#include "main.h"
#include "Telecom_server_query.h"
#include "WDT.h"
#include "inc/hw_watchdog.h"
#include "driverlib/watchdog.h"

#ifdef FLASH_EN
#include "flashCore.h"
#include "flash_logger.h"
#endif  //FLASH_EN

gprs_t gprs;

volatile gprs_rx_data_buff_t gprs_rx_buff,gprs_resp_rx_buff;
volatile gprs_tx_data_buff_t gprs_tx_buff;
volatile gprs_temp_rx_buff_t gprs_temp_rx_buff;
volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;
gprs_date_time_t gprs_date_time;

extern e2p_device_info_t e2p_device_info;
extern cloud_config_t cloud_config;
extern e2p_router_config_t e2p_router_config;
extern ram_data_t ram_data;
extern Alarms_t Alarms;

extern uint8_t dummy_json_string[390];

// // uint8_t JSON_Tx_Buff[500];
// // uint8_t JSON_Rx_Buff[500];
// extern uint8_t JSON_Tx_Buff[513];
// extern uint8_t JSON_Rx_Buff[513];

#ifndef ETHERNET_EN

void TCP_Handler(void)
{
    char status = 0;
    static unsigned int timeout = 0;
    // static unsigned int TCP_indx = 0;

    switch(gprs.gprs_handler_state)
    {
        case GPRS_IDLE:
        {
#ifndef ENABLE_GPS
            if (timeout++ >= GSM_WARMUP_SECS)
#endif  //ENABLE_GPS
            {
                timeout = 0;

                // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                set_gprs_connct_sts(0);
                set_network_status(0);

                // setNWstatus(NOT_AVBL);
                // setServerStatus(NOT_AVBL);
                set_pending_request(false);

                set_webconn_sts(FALSE);

                gprs.gprs_connect = GPRS_CONNCT_CMD_ECHO_OFF;

                gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;

                gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;

                gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;

                gprs.tcp_ping_states = PING_CMD_ECHO_OFF;

                gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;

                gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF;

                gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF;

                gprs.gprs_handler_state = GPRS_CONNECT;

                // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
            }
#ifdef DEBUG_TCP_HANDLER
            vUART_SendStr(DEBUG_UART_BASE, "\nGI:");
            vUART_SendInt(DEBUG_UART_BASE, timeout);
#endif
        }
        break;

        case GPRS_CONNECT:
        {
            status = (char) gprs_connect();

            if(status == CON_OK)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nGCK");
#endif
                //setGPRSConnSts(AVBL);
                gprs.gprs_handler_state = GPRS_CONN_STS;
            }
            else if(status == CON_FAIL)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nGCF");
#endif
                // setNWstatus(NOT_AVBL);
                // setServerStatus(NOT_AVBL);
                gprs.gprs_handler_state = GPRS_IDLE;   
            }
            else if (status == CON_IN_PRG)
            {

            }
        }
        break;

        case GPRS_CONN_STS:
        {
            status = (char)gprs_connect_status();
            if(gprs_date_time.update_time_aval == TRUE)
            {
#ifdef GPRS_TIME_UPDATE
                getDateTime();
                //time_sync();
                time_sync_with_gprs();
#endif
            }
            if(status == CON_OK)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nGCSK");
#endif
                //gprs.connect_sts = TRUE;

                // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                set_gprs_connct_sts(1);

                gprs.gprs_handler_state = GPRS_TCP_CONNECT;
                //setGPRSConnSts(AVBL);
                // setNWstatus(AVBL);
            }
            else if(status == CON_FAIL)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nGCSF");
#endif
                //setGPRSConnSts(NOT_AVBL);
                //gprs.connect_sts = FALSE;

                // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                set_gprs_connct_sts(0);
                set_network_status(0);

                // setNWstatus(NOT_AVBL);
                // setServerStatus(NOT_AVBL);

                //PP on 30-07-24: why go to idle straight? If there's a problem it should goto 
                //gprs_connect() again and if it still fails, the CRESET cmd in gprs_connect() 
                //will restart the LTE module. What's going in the idle case and waiting for 10secs going to do?
                // gprs.gprs_handler_state = GPRS_IDLE;    
                gprs.gprs_handler_state = GPRS_CONNECT; 

            }
            else if (status == CON_IN_PRG)
            {

            }
        }
        break;

        case GPRS_TCP_CONNECT:
        {
            status = (char)tcp_connect();
            if(status == TCP_PASS)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nTCK");
#endif
                // tcp_conn_sts = TRUE;
                gprs.tcp_sts = TRUE;
                gprs.gprs_handler_state = GPRS_WEBSOCKET_CONNECT;
                //gprs.gprs_handler_state = GPRS_PING;
            }
            else if(status == TCP_FAIL)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nTCF");
#endif
                gprs.tcp_sts = FALSE;
                set_network_status(0);
                // setServerStatus(NOT_AVBL);
                // gprs.gprs_handler_state = GPRS_CONN_STS;
                gprs.gprs_handler_state = GPRS_TCP_DISCONNECT;
            }
            else if(status == TCP_ALRDY_CONN)
            {
                gprs.tcp_sts = TRUE;

#ifdef DEBUG_TCP_CONN
                vUART_SendStr(UART_PC,"ALRDY_CONN\n");
#endif
                // gprs.gprs_handler_state = GPRS_PING;
                // //gprs.gprs_handler_state = GPRS_WEBSOCKET_CONNECT;//a
                if (get_webconn_sts() == TRUE)
                {
                    gprs.gprs_handler_state = GPRS_PING;
                    set_network_status(true);
                }
                else
                {
                    gprs.gprs_handler_state = GPRS_WEBSOCKET_CONNECT;
                    set_network_status(false);
                }
            }
            else if (status == TCP_IN_PRG)
            {

            }
        }
        break;

        case GPRS_WEBSOCKET_CONNECT:
        {
            status = websckt_connect();
            if(status == WEBSOCKET_PASS)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nWCK");
#endif
                set_webconn_sts(TRUE);
                // setServerStatus(AVBL);  //PP added on 07-06-24

                //set_webconn_sts(1);
                // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                set_network_status(1);

                //gprs.gprs_handler_state = GPRS_SESSION_IDLE;//a
                gprs.gprs_handler_state = GPRS_PING;
            }
            else if(status == WEBSOCKET_FAIL)
            {
#ifdef DEBUG_TCP_HANDLER
                // vUART_SendStr(UART_PC,"\nWCK");
                vUART_SendStr(UART_PC,"\nWCF");
#endif
                // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                set_network_status(0);

                // setServerStatus(NOT_AVBL);
                set_webconn_sts(FALSE);
                // gprs.gprs_handler_state = GPRS_TCP_CONNECT;
                gprs.gprs_handler_state = GPRS_WEBSOCKET_DISCONNECT;
            }
            else if(status == WEBSOCKET_IN_PRG)
            {

            }
        }
        break;

        case GPRS_PING:
        {
            status = ping_send();
            if(status == PING_PASS)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nPK");
#endif
                set_webconn_sts(TRUE);

                // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                set_network_status(1);

                flushRxBuffer(LTE_UART);
                // setServerStatus(AVBL);
                gprs.gprs_handler_state = GPRS_SESSION_IDLE; //PP commented on 29-07-24 for testing. To uncommented after testing.
                // gprs.gprs_handler_state = GPRS_CONN_STS;    //PP added on 29-09-24 for testing read_ip_port(). To be commented later after testing.

                //gprs.gprs_handler_state = GPRS_WEBSOCKET_CONNECT;

            }
            else if(status == PING_FAIL)
            {
#ifdef DEBUG_TCP_HANDLER
                vUART_SendStr(UART_PC,"\nPF");
#endif
                //gprs.gprs_handler_state = GPRS_WEBSOCKET_CONNECT;
                set_webconn_sts(FALSE);

                // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                set_network_status(0);

                // gprs.gprs_handler_state = GPRS_TCP_CONNECT;
                gprs.gprs_handler_state = GPRS_WEBSOCKET_DISCONNECT;
            }
            else if(status == PING_IN_PRG)
            {

            }
        }
        break;

        case GPRS_WEBSOCKET_DISCONNECT:
        {
            status = websckt_disconnect();
            if(status == WS_DISCON_PASS)
            {
#ifdef DEBUG_TCP_HANDLER
                // UWriteString((char*)"\nGWDK", DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, "\nGWSDK");
#endif

                if(getServerReqType() == RESTART)
                {
#ifdef DEBUG_SERVER_QUERY
                    // UWriteString((char*)"\nRST4",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nRST4");
#endif
#ifdef ENABLE_WDT_RESET
#ifdef WDT_IRQ_MODE

                    reset_controller();  //PP 15-05-24: Will implement this when WDT is implemented.
#else
                    WatchdogUnlock(WATCHDOG0_BASE);
                    WatchdogReloadSet(WATCHDOG0_BASE, 0);
#endif  //WDT_IRQ_MODE
#else
                    SysCtlReset();
#endif  //ENABLE_WDT_RESET
                }

                set_network_status(0);
                // setServerStatus(NOT_AVBL);
                // LTEmodule.HandlerSts = GPRS_TCP_DISCONNECT;
                gprs.gprs_handler_state = GPRS_TCP_DISCONNECT;
            }
            else if(status == WS_DISCON_FAIL)
            {

                if(getServerReqType() == RESTART)
                {
#ifdef DEBUG_SERVER_QUERY
                    // UWriteString((char*)"\nRST5",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nRST5");
#endif
#ifdef ENABLE_WDT_RESET
#ifdef WDT_IRQ_MODE

                    reset_controller();  //PP 15-05-24: Will implement this when WDT is implemented.
#else
                    WatchdogUnlock(WATCHDOG0_BASE);
                    WatchdogReloadSet(WATCHDOG0_BASE, 0);
#endif  //WDT_IRQ_MODE
#else
                    SysCtlReset();
#endif  //ENABLE_WDT_RESET
                }
                //LTEmodule.HandlerSts = GPRS_CONN_STS;

                set_network_status(0);
                // setServerStatus(NOT_AVBL);
                // LTEmodule.HandlerSts = GPRS_TCP_DISCONNECT;
                gprs.gprs_handler_state = GPRS_TCP_DISCONNECT;
            }
            else if(status == WS_DISCON_IN_PRG)
            {
                
            }
        }
        break;

        case GPRS_TCP_DISCONNECT:
        {
#ifdef DEBUG_TCP_HANDLER
            // UWriteString((char*)"\nGTD", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nGTD");
#endif
            status = tcp_disconnect();
            if(status == TCP_DISCON_PASS)
            {
                // LTEmodule.HandlerSts = GPRS_CONN_STS;
                gprs.gprs_handler_state = GPRS_CONN_STS;
                set_network_status(0);
                // setServerStatus(NOT_AVBL);
            }
            else if(status == TCP_DISCON_FAIL)
            {
                // LTEmodule.HandlerSts = GPRS_CONNECT;
                // LTEmodule.HandlerSts = GPRS_CONN_STS;
                gprs.gprs_handler_state = GPRS_CONN_STS;
                set_network_status(0);
                // setServerStatus(NOT_AVBL);
            }
            else if(status == TCP_DISCON_IN_PRG)
            {

            }
        }
        break;

        case GPRS_PREPARE_LOGS:
        {
//             TCP_indx = prepare_JSON_pckt();
//             // TCP_indx = websocket_packet(dummy_json_string);
//             // setREQmode(AVBL);
//
// #ifdef DEBUG_TCP_HANDLER
//             vUART_SendStr(UART_PC,"\nTCP_indx=");
//             vUART_SendInt(UART_PC, TCP_indx);
//             vUART_SendStr(UART_PC, "\nTCP_buff=");
//             vUART_SendBytes(UART_PC, JSON_Tx_Buff, TCP_indx);
// #endif
            prepare_server_pkt();
            // gprs_tx_buff.index = prepare_JSON_pckt();
#ifdef DEBUG_TCP_HANDLER
            vUART_SendStr(UART_PC,"\nTCP_indx=");
            vUART_SendInt(UART_PC, gprs_tx_buff.index);
            vUART_SendStr(UART_PC, "\nTCP_buff=");
            vUART_SendBytes(UART_PC, (const uint8_t*)gprs_tx_buff.buffer, gprs_tx_buff.index);
#endif
            gprs.gprs_handler_state = GPRS_LOGS_UPLOAD;
        }
        break;

        case GPRS_LOGS_UPLOAD:
        {
#ifdef DEBUG_TCP_HANDLER
            vUART_SendStr(UART_PC,"\nLU");
#endif
            if(getREQmode() == AVBL)
            {
                // status = tcp_send((char *)JSON_Tx_Buff,TCP_indx);
                status = tcp_send((char *)gprs_tx_buff.buffer,gprs_tx_buff.index);

                if(status == TCP_SEND_PASS)         //pending
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\nTCP_SEND_PASS");
#endif
                    flushRxBuffer(LTE_UART);
                    if(/* Alarms.Power_ON || */ ((getRAM_Alarm() & (1 << POWER_ON_BIT)) == (1 << POWER_ON_BIT)))
                    {
                        Alarms.Power_ON = false;
                        setRAM_Alarm(POWER_ON_BIT,Alarms.Power_ON);
                    }

                    if(getServerReqType() == SERVER_URL_UPDATE)
                    {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                        vUART_SendStr(UART_PC,"\nservicedSUP:");
                        vUART_SendInt(UART_PC, getServerReqType());
#endif
                        gprs.gprs_handler_state = GPRS_WEBSOCKET_DISCONNECT;    //so that it disconnects and then connects again with the new server ipaddr & portnum.
                    }
                    else
                    {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                        vUART_SendStr(UART_PC,"\nservicedQ:");
                        vUART_SendInt(UART_PC, getServerReqType());
#endif
                        gprs.gprs_handler_state = GPRS_SESSION_IDLE;
                    }

#ifdef FLASH_EN                    
                    if(check_unsent_log())  //PP added on 11-06-24
                    {
                        decrement_unsent_log_cnt(TELECOM_OFFLINE_LOGS);
                        set_pending_request(false);
                    }
#endif  //FLASH_EN

                    // commenting this condition as the serverReqType should be reset regardless of what request was just handled, as we're in the success case.
                    if((getServerReqType() == ODU_VOLTAGE_UPDATE)||(getServerReqType() == SERVER_URL_UPDATE))
                    {
                        setServerReqType(NO_REQ);
                    }

                    if(getClientMSGType() == RESPONSE_LOG)
                    {
                        setClientMSGType(SCHEDULED_LOG);
                    }

                    //flushTxBuffer(LTE_UART);
                    //gprs_tx_buff.index = 0;
                    setREQmode(NOT_AVBL);
                    // setServerStatus(AVBL);
                    set_network_status(1);
                }
                else if(status == TCP_SEND_FAIL)
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\nTCP_SEND_FAIL");
#endif
                    // gprs.gprs_handler_state = GPRS_TCP_CONNECT;
                    gprs.gprs_handler_state = GPRS_WEBSOCKET_DISCONNECT;
                    //setREQmode(NOT_AVBL);
                    set_network_status(0);
                }
                else if(status == TCP_SEND_IN_PRG)
                {

                }
            }
            else
            {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                vUART_SendStr(UART_PC,"\nGPRS_LOGS_UPLOAD:NOT_AVBL");
#endif
            }
        }
        break;

        case GPRS_SESSION_IDLE:
        {
#ifdef DEBUG_TCP_HANDLER
            vUART_SendStr(UART_PC,"\nSI");
#endif
            static unsigned char gprs_conn_retry_time = 0;
#ifdef DEBUG_GPRS_DATA_UPLOAD
            //vUART_SendStr(UART_PC,"\ngetREQmode");
            //vUART_SendInt(UART_PC,getREQmode());
#endif
            if(get_pending_request())
            {
                switch(getServerReqType())
                {
                    case RESTART:
                    {
#ifdef DEBUG_SERVER_QUERY
                        // UWriteString((char*)"\nRST3",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\nRST3");
#endif
                        set_pending_request(false);
                        resetTCP_SubHandlers();                   
                        gprs.gprs_handler_state = GPRS_WEBSOCKET_DISCONNECT;   
                    }
                    break;

                    case ODU_VOLTAGE_UPDATE:
                    {
#ifdef DEBUG_SERVER_QUERY
                        // UWriteString((char*)"\nOVC3",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\nOVC3");
#endif                 
                        set_pending_request(false);
                        // setClientMSGType(RESPONSE_LOG); // handled in HandleQueryStates() states now.
                        resetTCP_SubHandlers();
                        gprs.gprs_handler_state = GPRS_PREPARE_LOGS;
                    }
                    break;

                    case SERVER_URL_UPDATE:
                    {
#ifdef DEBUG_SERVER_QUERY
                        // UWriteString((char*)"\neSUP",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\nSUP");
#endif 
                        set_pending_request(false);
                        // setClientMSGType(RESPONSE_LOG); // handled in HandleQueryStates() states now.
                        resetTCP_SubHandlers();
                        gprs.gprs_handler_state = GPRS_PREPARE_LOGS;
                    }
                    break;

                    case NO_REQ:
                    default:
                    {
#ifdef DEBUG_SERVER_QUERY
                        vUART_SendStr(DEBUG_UART_BASE, "\nLog");
#endif                 
                        set_pending_request(false);
                        // setClientMSGType(RESPONSE_LOG); // handled in HandleQueryStates() states now.
                        gprs.gprs_handler_state = GPRS_PREPARE_LOGS;
                    }
                    break;
                }
            }
            else if(getREQmode() == AVBL)
            {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                vUART_SendStr(UART_PC,"\nAVBL");
#endif
                if (get_webconn_sts() == TRUE)
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\nUPD_LOGS");
#endif
                    gprs.gprs_handler_state = GPRS_LOGS_UPLOAD;
                }
                else
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\nWNOT_AVBL");
#endif
                    gprs.gprs_handler_state = GPRS_TCP_CONNECT;
                }
            }
            else if(gprs_conn_retry_time++ > GPRS_CONN_RETRY_TIME)
            {
                gprs_conn_retry_time = 0;
                gprs.gprs_handler_state = GPRS_PING;
            }

//             if(get_pending_request())
//             {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                 vUART_SendStr(UART_PC,"\npr?:");
//                 vUART_SendInt(UART_PC,get_pending_request());
// #endif
//                 // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
//                 // OCPP_Server_Query_Message();
//                 // set_pending_request(0);
//
//                 TCP_indx = prepare_JSON_pckt();          
//                 set_pending_request(0);
//             }
//             else if(getREQmode() == AVBL)
//             {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                 vUART_SendStr(UART_PC,"\nAVBL");
// #endif
//                 if (get_webconn_sts() == TRUE)
//                 {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                     vUART_SendStr(UART_PC,"\nUPD_LOGS");
// #endif
//                     gprs.gprs_handler_state = GPRS_LOGS_UPLOAD;
//                 }
//                 else
//                 {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                     vUART_SendStr(UART_PC,"\nNOT_AVBL");
// #endif
//                     gprs.gprs_handler_state = GPRS_TCP_CONNECT;
//                 }
//             }
//             else if(gprs_conn_retry_time++ > GPRS_CONN_RETRY_TIME)
//             {
//                 gprs_conn_retry_time = 0;
//                 gprs.gprs_handler_state = GPRS_PING;
//             }
        }
        break;

    }
}

void resetTCP_SubHandlers(void)
{
    // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
    // gprs.tcp_ping_states = PING_CMD_CIPSEND;
    // gprs.tcp_packet_state = TCP_CIPSEND_CMD;

    gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;

    gprs.tcp_ping_states = PING_CMD_ECHO_OFF;

    gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;
}

con_status_t gprs_connect(void)
{
    con_status_t sts = CON_IN_PRG;
    static char gprs_retry_count = 0;
    static unsigned int timeout = 0;
    int num_byte = 0;

#ifdef DEBUG_TCP_HANDLER
    vUART_SendStr(DEBUG_UART_BASE, "\nGC:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.gprs_connect);
#endif

    //static gprs_connect_state_t gprs_connct_state = GPRS_CONNCT_CMD_ECHO_OFF;

    switch(gprs.gprs_connect)
    {
        case GPRS_CONNCT_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"\ncmd:1EO");
            vUART_SendChr(UART_PC,',');
#endif
            vUART_SendStr(LTE_UART_BASE, "ATE0\r");
            gprs.gprs_connect = GPRS_CONNCT_RSP_ECHO_OFF;
        }
        break;

        case GPRS_CONNCT_RSP_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"ECHO_OFF:k");
#endif
            gprs.gprs_connect = GPRS_CONNCT_CMD_CPIN;
        }
        break;

        case GPRS_CONNCT_CMD_CPIN:
        {
            vUART_SendStr(LTE_UART_BASE, "AT+CPIN?\r");
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"cmd:CPIN");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect = GPRS_CONNCT_RSP_CPIN;
        }
        break;

        case GPRS_CONNCT_RSP_CPIN:
        {
            switch (check_string_nobuf("+CPIN: READY"))
            {
                case (GPRS_MATCH_FAIL):
                {
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CPIN;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
#ifdef DEBUG_GPRS_CONN
                        vUART_SendStr(UART_PC,"CPIN:f\n");
#endif
                        gprs_retry_count = 0;
                        //gprs.module_status = NOT_AVBL;
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CPIN;

                        // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE
                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                        timeout = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CPIN:k\n");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CSQ;
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CPIN:w\n");
#endif
                    if(timeout++ >= CPIN_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CPIN;
                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            gprs_retry_count = 0;
                            //gprs.module_status = NOT_AVBL;
                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CPIN;

                            // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;
                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_CSQ:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CSQ\r");
            //setREQmode(NOT_AVBL);
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"cmd:CSQ");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect = GPRS_CONNCT_RSP_CSQ;
        }
        break;

        case GPRS_CONNCT_RSP_CSQ:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];
            char resp = check_string("+CSQ: ", tmpstr, &num_byte);

            switch (resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CSQ:f1\n");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CSQ;
                    gprs_retry_count++;
                    if (gprs_retry_count >= GPRS_RETRY_CNT)
                    {
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CSQ;

                        // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                        gprs_retry_count = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    gprs.network_strength = (tmpstr[0]-'0')*10 + (tmpstr[1]-'0');

                    if((gprs.network_strength >= MIN_NETWORK_STRENGTH_DB) && (gprs.network_strength <= MAX_NETWORK_STRENGTH_DB))
                    {
                        //setREQmode(AVBL);
#ifdef DEBUG_GPRS_CONN
                        vUART_SendStr(UART_PC,"CSQ:k\n");
#endif
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CREG;
                        gprs_retry_count = 0;
                    }
                    else
                    {
#ifdef DEBUG_GPRS_CONN
                        vUART_SendStr(UART_PC,"CSQ:f2\n");
#endif
                        gprs.gprs_connect = GPRS_CONNCT_CMD_ECHO_OFF;
                    }
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CSQ:w\n");
#endif
                    if(timeout++ >= CSQ_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CSQ;

                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            //gprs.module_status = NOT_AVBL;

                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CSQ;

                            // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;
                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_CREG:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CREG?\r");
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"cmd:CREG");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect = GPRS_CONNCT_RSP_CREG;
        }
        break;

        case GPRS_CONNCT_RSP_CREG:
        {
            switch (check_string_nobuf("+CREG: 0,1"))       
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CREG:f\n");
                    //vUART_SendChr(UART_PC,',');
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CREG;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        gprs_retry_count = 0;
                        //gprs.module_status = NOT_AVBL;
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CREG;

                        // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE
                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                        timeout = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CREG:k");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CGREG;
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CREG:w\n");
#endif
                    if(timeout++ >= CREG_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CREG;
                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            gprs_retry_count = 0;
                            // gprs.module_status = NOT_AVBL;
                            // gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CREG;

                            // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_CGREG:
        {
            flushRxBuffer(LTE_UART);    //PP added on 27-07-24
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CGREG?\r");
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"cmd:CGREG");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect = GPRS_CONNCT_RSP_CGREG;
        }
        break;

        case GPRS_CONNCT_RSP_CGREG:
        {
            switch (check_string_nobuf("+CGREG: 0,1"))
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CGREG:f\n");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CGREG;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        gprs_retry_count = 0;
                        //  gprs.module_status = NOT_AVBL;
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CGREG;

                        // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE
                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                        timeout = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CGREG:k\n");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CPSI;
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CGREG:w\n");
#endif
                    if(timeout++ >= CGREG_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CGREG;
                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            gprs_retry_count = 0;
                            //  gprs.module_status = NOT_AVBL;
                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CGREG;

                            // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_CPSI:
        {
            flushRxBuffer(LTE_UART);    //PP added on 27-07-24
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CPSI?\r");
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"cmd:CPSI");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect = GPRS_CONNCT_RSP_CPSI;
        }
        break;

        case GPRS_CONNCT_RSP_CPSI:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];
            char resp = check_string("+CPSI: ", tmpstr, &num_byte); //+CPSI: LTE,Online,404-70,0x3FA,238436118,103,EUTRAN-BAND8,3586,2,0,15,48,45,4

            switch (resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CPSI:f1\n");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CPSI;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        gprs_retry_count = 0;
                        // gprs.module_status = NOT_AVBL;
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CPSI;
                        
                        // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE
                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                        timeout = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    if(match_cpsi_data(tmpstr))
                    {
#ifdef DEBUG_GPRS_CONN
                        vUART_SendStr(UART_PC,"CPSI:k\n");
#endif
                        // sts = CON_OK;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CGDCONT;
                    }
                    else
                    {
#ifdef DEBUG_GPRS_CONN
                        vUART_SendStr(UART_PC,"CPSI:f2\n");
#endif
                        // sts = CON_FAIL;
                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                    }
                    //gprs.gprs_connect = GPRS_CONNCT_CMD_CGDCONT;
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CPSI:w\n");
#endif
                    if(timeout++ >= CPSI_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CPSI;
                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            gprs_retry_count = 0;
                            //  gprs.module_status = NOT_AVBL;
                            // gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CPSI;

                            // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_CGDCONT:
        {
            flushRxBuffer(LTE_UART);    //PP added on 27-07-24
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CGDCONT\r");
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"cmd:CGD");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect = GPRS_CONNCT_RSP_CGDCONT;
        }
        break;

        case GPRS_CONNCT_RSP_CGDCONT:
        {
            switch (check_string_nobuf("OK"))
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CGD:f\n");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CGDCONT;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        gprs_retry_count = 0;
                        // gprs.module_status = NOT_AVBL;
                        // gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CGDCONT;

                        // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE
                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                        timeout = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CGD:k");
                    vUART_SendChr(UART_PC,',');
                    vUART_SendStr(UART_PC,"gprs_connect");
#endif
//                    gprs.gprs_connect = GPRS_CONNCT_CMD_ECHO_OFF;
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CTZU;
//                    sts = CON_OK;
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CGD:w\n");
#endif

                    if(timeout++ >= CGDCONT_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CGDCONT;
                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            gprs_retry_count = 0;
                            // gprs.module_status = NOT_AVBL;
                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CGDCONT;
                            
                            // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;
        case GPRS_CONNCT_CMD_CTZU:
        {
            flushRxBuffer(LTE_UART);    //PP added on 27-07-24
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CTZU=1\r");
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"cmd:CTZU");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect = GPRS_CONNCT_RSP_CTZU;
        }
        break;

        case GPRS_CONNCT_RSP_CTZU:
        {
            switch (check_string_nobuf("OK"))
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CTZU:f\n");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CTZU;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        gprs_retry_count = 0;
                        // gprs.module_status = NOT_AVBL;
                        // gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CGDCONT;

                        // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                        gprs.gprs_connect = GPRS_CONNCT_RESET;
                        timeout = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CTZU:k");
                    vUART_SendChr(UART_PC,',');
                    vUART_SendStr(UART_PC,"gprs_connect");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_ECHO_OFF;    //PP commented on 27-07-24 for testing CRESET cmd.
                    // gprs.gprs_connect = GPRS_CONNCT_CMD_CRESET; //PP 27-07-24: to be commented after testing CRESET cmd. Only for testing.
                    sts = CON_OK;    //PP commented on 27-07-24 for testing CRESET cmd.
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CTZU:w\n");
#endif

                    // if(timeout++ >= CGDCONT_TIMEOUT) //PP on 27-07-24 to Khushal: copy-paste ke galat natije.
                    if(timeout++ >= CCLK_TIMEOUT)   //PP added on 27-07-24
                    {
                        timeout = 0;
                        // gprs.gprs_connect = GPRS_CONNCT_CMD_CGDCONT; //PP on 27-07-24 to Khushal: copy-paste ke galat natije.
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CTZU;   //PP added on 27-07-24
                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            gprs_retry_count = 0;
                            // gprs.module_status = NOT_AVBL;
                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CGDCONT;

                            // sts = CON_FAIL;  //PP commented on 01-08-24: CRESET won't work if we return FAIL and goto GPRS_IDLE

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;
        case GPRS_CONNCT_RESET:
        {
            timeout = 0;
            gprs_retry_count = 0;
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"CONN_RESET\n");
#endif

#ifdef SOFT_RST_GSM
            gprs.gprs_connect = GPRS_CONNCT_CMD_CRESET;
#else
            gprs.gprs_connect = GPRS_CONNCT_CMD_ECHO_OFF;
#endif  //SOFT_RST_GSM
        }
        break;

#ifdef SOFT_RST_GSM
        case GPRS_CONNCT_CMD_CRESET:
        {
#ifdef DEBUG_GPRS_CONN
            vUART_SendStr(UART_PC,"\nCRST");
#endif           
            flushRxBuffer(LTE_UART);    //PP added on 27-07-24
            vUART_SendStr(LTE_UART_BASE, "AT+CRESET\r\n");
            gprs.gprs_connect = GPRS_CONNCT_RSP_CRESET;
        }
        break;

        case GPRS_CONNCT_RSP_CRESET:
        {
            switch(check_string_nobuf("OK"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"\nCRST:f");
#endif
                    gprs.gprs_connect = GPRS_CONNCT_CMD_CRESET;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        gprs_retry_count = 0;
                        sts = CON_FAIL;
                        // gprs.gprs_connect = GPRS_CONNCT_RESET;  //PP 27-07-24: to be changed to GPRS_CONNCT_WAIT_CRESET later
                        gprs.gprs_connect = GPRS_CONNCT_WAIT_CRESET;
                        timeout = 0;
                    }
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CRST:k");
#endif
                    // gprs.gprs_connect = GPRS_CONNCT_RESET;  //PP 27-07-24: to be changed to GPRS_CONNCT_WAIT_CRESET later
                    gprs.gprs_connect = GPRS_CONNCT_WAIT_CRESET;
                    // sts = CON_OK;
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef DEBUG_GPRS_CONN
                    vUART_SendStr(UART_PC,"CRST:w\n");
#endif
                    if(timeout++ >= SOFT_RST_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect = GPRS_CONNCT_CMD_CRESET;
                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            gprs_retry_count = 0;
                            // gprs.module_status = NOT_AVBL;
                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_CGDCONT;
                            sts = CON_FAIL;

                            gprs.gprs_connect = GPRS_CONNCT_RESET;
                            gprs_retry_count = 0;
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

        case GPRS_CONNCT_WAIT_CRESET:
        {
            if (timeout++ >= GSM_RST_SECS)		//not getting expected data,
			{
				flushRxBuffer(LTE_UART);
				timeout = 0;
				gprs.gprs_connect = GPRS_CONNCT_RESET;
                sts = CON_FAIL;
			}
        }
        break;
#endif  //SOFT_RST_GSM

        default:
            gprs.gprs_connect = GPRS_CONNCT_RESET;
        break;

    }
    return sts;
}

con_status_t gprs_connect_status(void)
{
    con_status_t sts = CON_IN_PRG;

    static char gprs_retry_count = 0;
    static unsigned int timeout = 0;
    int num_byte = 0;

#ifdef DEBUG_TCP_HANDLER
    vUART_SendStr(DEBUG_UART_BASE, "\nGCS:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.gprs_connect_state);
#endif

    //static gprs_connect_state_t gprs_connct_state = GPRS_CONNCT_CMD_CCLK;

    switch(gprs.gprs_connect_state)
    {
        case GPRS_CONNCT_STATE_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "ATE0\r\n");
            gprs.gprs_connect_state = GPRS_CONNCT_STATE_RSP_ECHO_OFF;
#ifdef DEBUG_GPRS_CONN_STS
            vUART_SendStr(UART_PC,"\ncmd:2EO");
#endif
        }
        break;

        case GPRS_CONNCT_STATE_RSP_ECHO_OFF:
        {

            gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;
        }
        break;

        case GPRS_CONNCT_CMD_CCLK:
        {
            flushRxBuffer(LTE_UART_BASE);
            gprs_date_time.update_time_aval = FALSE;
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CCLK?\r");
#ifdef DEBUG_GPRS_CONN_STS
            vUART_SendStr(UART_PC,"cmd:CCLK");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect_state = GPRS_CONNCT_RSP_CCLK;
        }
        break;

        case GPRS_CONNCT_RSP_CCLK:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];
            gprs_date_time_t temp_date_time;
            char resp = check_string("+CCLK: \"", tmpstr, &num_byte);

            switch (resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN_STS
                    vUART_SendStr(UART_PC,"CCLK:f1\n");
#endif
                    gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;
                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;
                        //gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;
                        gprs_retry_count = 0;

                        //PP added on 29-07-24:
                        gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;
                        sts = CON_FAIL;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    if (check_date_time(tmpstr, (char *)&temp_date_time))
                    //dt_str_to_int(tmpstr)
                    {
                        if (((temp_date_time.yy >= DEFAULT_YEAR) && (temp_date_time.yy <= (DEFAULT_YEAR + YEAR_OFFSET))) &&             // Assuming that RTC will never go below 2020.
                        ((temp_date_time.mm >= 1) && (temp_date_time.mm <= 12)) &&
                        ((temp_date_time.dd >= 1) && (temp_date_time.dd <= 31))&&
                        (((char *)temp_date_time.hr >= 0) && (temp_date_time.hr <= 23)) &&
                        (((char *)temp_date_time.min >= 0) && (temp_date_time.min <= 59)) &&
                        (((char *)temp_date_time.sec >= 0) && (temp_date_time.sec <= 59)))
                        {
                            memcpy(&gprs_date_time, &temp_date_time, sizeof(gprs_date_time_t));
#ifdef DEBUG_GPRS_CONN_STS
                            vUART_SendStr(UART_PC,"CCLK:k\n");
#endif
                            gprs_date_time.update_time_aval = TRUE;
                        }
                        else
                        {
#ifdef DEBUG_GPRS_CONN_STS
                            vUART_SendStr(UART_PC,"CCLK:f2\n");
#endif
                        }
                    }
                    gprs.gprs_connect_state = GPRS_CONNCT_CMD_READ_NETOPEN;
                    //sts = CON_OK;
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN_STS
                    vUART_SendStr(UART_PC,"CCLK:w\n");
#endif
                    if(timeout++ >= CCLK_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;

                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            //  gprs.module_status = NOT_AVBL;
                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_CCLK;

                            // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;  //PP commented on 29-07-24
                            gprs_retry_count = 0;
                            //PP added on 29-07-24:
                            gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;
                            sts = CON_FAIL;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_READ_NETOPEN:
        {
            flushRxBuffer(LTE_UART);    //PP added on 29-07-24
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+NETOPEN?\r");
#ifdef DEBUG_GPRS_CONN_STS
            vUART_SendStr(UART_PC,"cmd:NO?");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect_state = GPRS_CONNCT_RSP_READ_NETOPEN;
        }
        break;

        case GPRS_CONNCT_RSP_READ_NETOPEN:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];
            //gprs.connect_sts = FALSE;
            char resp = check_string("+NETOPEN: ", tmpstr, &num_byte);
            
            switch (resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN_STS
                    vUART_SendStr(UART_PC,"NO?:f\n");
#endif
                    gprs.gprs_connect_state = GPRS_CONNCT_CMD_READ_NETOPEN;

                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_READ_NETOPEN;
                        sts = CON_FAIL;

                        // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;  //PP commented on 29-07-24
                        gprs_retry_count = 0;

                        //PP added on 29-07-24:
                        gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;
                        gprs_retry_count = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    if(tmpstr[0] == '1')
                    {
                        // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;  //PP commented on 29-07-24
                        gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;   //PP added on 29-07-24
                        sts = CON_OK;
#ifdef DEBUG_GPRS_CONN_STS
                        vUART_SendStr(UART_PC,"NO?:k\n");
#endif
                        //gprs.connect_sts = TRUE;
                        gprs_retry_count = 0;
                        timeout = 0;
                    }
                    else
                    {
#ifdef DEBUG_GPRS_CONN_STS
                        vUART_SendStr(UART_PC,"NO?:f\n");
#endif
                        //gprs.connect_sts = FALSE;
                        gprs.gprs_connect_state = GPRS_CONNCT_CMD_NETOPEN;
                    }

                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN_STS
                    vUART_SendStr(UART_PC,"NO?:w\n");
#endif
                    if(timeout++ >= NETOPEN_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect_state = GPRS_CONNCT_CMD_READ_NETOPEN;

                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            // gprs.module_status = NOT_AVBL;

                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_CMD_READ_NETOPEN;
                            sts = CON_FAIL;

                            // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;  //PP commented on 29-07-24
                            gprs_retry_count = 0;

                            //PP added on 29-07-24:
                            gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        case GPRS_CONNCT_CMD_NETOPEN:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+NETOPEN\r");
#ifdef DEBUG_GPRS_CONN_STS
            vUART_SendStr(UART_PC,"cmd:NO");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_connect_state = GPRS_CONNCT_RSP_NETOPEN;
        }
        break;

        case GPRS_CONNCT_RSP_NETOPEN:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];
            char resp = check_string("OK", tmpstr, &num_byte);
            switch (resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_CONN_STS
                    vUART_SendStr(UART_PC,"NO:f\n");
#endif
                    gprs.gprs_connect_state = GPRS_CONNCT_CMD_READ_NETOPEN;

                    if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                    {
                        //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_NETOPEN;
                        sts = CON_FAIL;

                        // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK; //PP commented on 29-07-24
                        gprs_retry_count = 0;

                        //PP added on 29-07-24:
                        gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;
                        gprs_retry_count = 0;
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    if (strstr(tmpstr, "+NETOPEN: 0"))
                    {
#ifdef DEBUG_GPRS_CONN_STS
                        vUART_SendStr(UART_PC,"NO:k\n");
#endif
                        // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK; //PP commented on 29-07-24
                        gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;   //PP added on 29-07-24

                        // gprs.connect_sts = TRUE;
                        sts = CON_OK;
                    }
                    gprs_retry_count = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_CONN_STS
                    vUART_SendStr(UART_PC,"NO:w\n");
#endif
                    if(timeout++ >= NETOPEN_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_connect_state = GPRS_CONNCT_CMD_READ_NETOPEN;

                        if (gprs_retry_count++ >= GPRS_RETRY_CNT)
                        {
                            //gprs.module_status = NOT_AVBL;

                            //gprs.errcode = CON_ERR_OFFSET + GPRS_CONNCT_RSP_NETOPEN;
                            sts = CON_FAIL;

                            // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK; //PP commented on 29-07-24
                            gprs_retry_count = 0;

                            //PP added on 29-07-24:
                            gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;
                            gprs_retry_count = 0;
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
        break;

        default:
        {
            // gprs.gprs_connect_state = GPRS_CONNCT_CMD_CCLK;  //PP commented on 29-07-24
            gprs.gprs_connect_state = GPRS_CONNCT_STATE_CMD_ECHO_OFF;   //PP added on 29-07-24
        }
        break;
    }
    return sts;
}

tcp_status_t tcp_connect(void)
{
    //static gprs_tcp_state_t tcp_state = GPRS_TCP_CMD_READ_CIPOPEN;
    static char tcp_retry_cnt = 0;
    tcp_status_t sts = TCP_IN_PRG;
    static unsigned int timeout = 0;
    //char tmpstr[TEMP_BUFF_LEN];
    // char tmpstr[200];
    char tmpstr[GPRS_RX_BUFFER_MAX];
    int num_bytes=0;

#ifdef DEBUG_TCP_HANDLER
    vUART_SendStr(DEBUG_UART_BASE, "\nTC:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.gprs_tcp_state);
#endif

    switch(gprs.gprs_tcp_state)
    {
        case GPRS_TCP_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "ATE0\r\n");
            gprs.gprs_tcp_state = GPRS_TCP_RSP_ECHO_OFF;
#ifdef DEBUG_TCP_CONN
            vUART_SendStr(UART_PC,"\ncmd:3EO");
#endif
        }
        break;

        case GPRS_TCP_RSP_ECHO_OFF:
        {
            gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN;
        }
        break;

        case GPRS_TCP_CMD_READ_CIPOPEN:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE,"AT+CIPOPEN?\r");
#ifdef  DEBUG_TCP_CONN
            vUART_SendStr(UART_PC,"cmd:CO?");
            vUART_SendChr(UART_PC,',');
#endif
            gprs.gprs_tcp_state = GPRS_TCP_RSP_READ_CIPOPEN;
        }
        break;

        case GPRS_TCP_RSP_READ_CIPOPEN:
        {
            memset(tmpstr, 0, sizeof(tmpstr));
            //switch(check_string_nobuf("+CIPOPEN: 0,\"TCP\",\"122.160.48.7\",8060,-1"))
            char resp = check_string("+CIPOPEN: 0,\"TCP\",\"",tmpstr,&num_bytes);    
            //switch(check_string_nobuf("+CIPOPEN: 0,\"TCP\",\""))     //+CIPOPEN: 0,\"TCP\",\"122.160.48.7\",8060,-1
            //switch(check_string_nobuf("122.160.48.7"))
            switch(resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"CO?:f\n");
#endif
                    //PP commented on 30-07-24: CIOPEN QUERY FAIL RETRIES commented becoz we should just do cipopen at the 1st fail
                    //when we know this cmd's going to fail here atleast 1 time at startup becoz we have'nt cipopened yet. Why 3 retries?

                    // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN;     
                    // 
                    // if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    // {
                    //     // tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_READ_CIPOPEN;
                    //     //sts = TCP_FAIL;
                    //
                    //     gprs.gprs_tcp_state = GPRS_TCP_CMD_CIPOPEN; //PP on 29-07-24: we're not doing TCP_FAIL here so we're also not resetting the state to echo off here.
                    //     tcp_retry_cnt = 0;
                    // }

                    gprs.gprs_tcp_state = GPRS_TCP_CMD_CIPOPEN; //PP on 29-07-24: we're not doing TCP_FAIL here so we're also not resetting the state to echo off here.
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    // gprs.tcp_sts = TRUE;
                    //sts = TCP_PASS;
                    // if(read_ip_port(tmpstr)) //PP commented on 30-07-24
                    if(read_ip_port(tmpstr,num_bytes))  //PP added on 30-07-24. (Copied from EVSE 3.12)
                    {
#ifdef  DEBUG_TCP_CONN
                        vUART_SendStr(UART_PC,"ip_port_matched\n");
#endif
                        sts = TCP_ALRDY_CONN;
                    }
                    else
                    {
#ifdef  DEBUG_TCP_CONN
                        vUART_SendStr(UART_PC,"ip_port_not_matched\n");
#endif
                        //sts = TCP_FAIL;
                    }

#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"CO?:k\n");
#endif
                    // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                    gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"CO?:w\n");
#endif
                    if(timeout++ >= TCP_START_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN;
                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_CMD_READ_CIPOPEN;
                            sts = TCP_FAIL;

                            // gprs.gprs_tcp_state = GPRS_TCP_CMD_CIPOPEN;  //PP commented on 29-07-24
                            gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
            }
        }
        break;

        case GPRS_TCP_CMD_CIPOPEN:
        {
            flushRxBuffer(LTE_UART);
            char buff_cipopen_cmd[50];
            // memset(tmpstr,0,sizeof(tmpstr));
            memset(buff_cipopen_cmd, 0, sizeof(buff_cipopen_cmd));
#ifdef DEBUG_TCP_CONN
            vUART_SendStr(UART_PC,"\ncfg.ipa:");
            vUART_SendStr(UART_PC,(const uint8_t *)cloud_config.ip_addr);
            vUART_SendStr(UART_PC,"\ncfg.pn:");
            vUART_SendInt(UART_PC,cloud_config.port_num);
#endif

            // my_sprintf(tmpstr, 3,"AT+CIPOPEN=0,\"TCP\",\"%s\",%d\r",cloud_config.ip_addr,cloud_config.port_num);
            my_sprintf(buff_cipopen_cmd, 3,"AT+CIPOPEN=0,\"TCP\",\"%s\",%d\r",cloud_config.ip_addr,cloud_config.port_num);

#ifdef  DEBUG_TCP_CONN
             vUART_SendStr(UART_PC,"\ncmd:CO");
             vUART_SendChr(UART_PC,',');
#endif
            // vUART_SendStr(LTE_UART_BASE,(const uint8_t *)tmpstr);
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)buff_cipopen_cmd);

#ifdef  DEBUG_TCP_CONN
             vUART_SendStr(UART_PC,"\nbuff_cipopen_cmd:");
             vUART_SendStr(UART_PC,(const uint8_t *)buff_cipopen_cmd);
#endif
            gprs.gprs_tcp_state = GPRS_TCP_RSP_CIPOPEN;
        }
        break;

        case GPRS_TCP_RSP_CIPOPEN:
        {
            memset(tmpstr, 0, sizeof(tmpstr));
            // switch (check_string_nobuf("OK"))
            char resp = check_string("OK",tmpstr,&num_bytes); 
            switch(resp)   
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"1CO:f\n");
#endif
                    if(strstr(tmpstr, "CLOSED"))    //PP added this condition on 30-04-24
                    {
#ifdef  DEBUG_TCP_CONN
                        vUART_SendStr(UART_PC,"\nNW_cl_unexpectedly");
#endif
                        tcp_retry_cnt = 0;
                        flushRxBuffer(LTE_UART);
                        sts = TCP_FAIL;
                        // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                    }
                    else
                    {
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_CIPOPEN;
                        //gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_CIPOPEN;
                            sts = TCP_FAIL; //PP uncommented on 30-04-24

                            // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                            gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"1CO:k\n");
#endif
                    if(strstr(tmpstr,"+CIPOPEN: 0,0"))
                    {
#ifdef  DEBUG_TCP_CONN
                        // UWriteString((char*)"\n1CO:k2",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\n1CO:k2");
#endif                        
                        // LTEmodule.TCP_connct_states = GPRS_TCP_CMD_READ_CIPOPEN;

                        // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                        timeout = 0;
                        sts = TCP_PASS;
                    }
                    else
                    {
#ifdef  DEBUG_TCP_CONN
                        // UWriteString((char*)"\n1CO:f2",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\n1CO:f2");
#endif
                        // LTEmodule.TCP_connct_states = GPRS_TCP_RSP_1_CIPOPEN;
                        gprs.gprs_tcp_state = GPRS_TCP_RSP_1_CIPOPEN;
                        tcp_retry_cnt = 0;
                        timeout = 0;
                        //sts = TCP_PASS;
                    }
                    // gprs.gprs_tcp_state = GPRS_TCP_RSP_1_CIPOPEN;    //PP commented on 29-07-24
                    //sts = TCP_PASS;
                    flushRxBuffer(LTE_UART);
                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"1CO:w\n");
#endif
                    if(timeout++ >= TCP_START_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_CIPOPEN;
                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_CIPOPEN;
                            sts = TCP_FAIL;

                            // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                            gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_CMD_READ_CIPOPEN;
                        sts = TCP_FAIL;

                        // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        case GPRS_TCP_RSP_1_CIPOPEN:
        {
            switch (check_string_nobuf("+CIPOPEN: 0,0"))
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"2CO:f\n");
#endif
                    gprs.gprs_tcp_state = GPRS_TCP_CMD_CIPOPEN;

                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_CIPOPEN;
                        sts = TCP_FAIL ;    //PP uncommented on 30-04-24

                        // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24    
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"2CO:k\n");
#endif
                    // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                    gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                    sts = TCP_PASS;
                    flushRxBuffer(LTE_UART);
                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef  DEBUG_TCP_CONN
                    vUART_SendStr(UART_PC,"2CO:w\n");
#endif
                    if(timeout++ >= TCP_START_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_CIPOPEN;
                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_CIPOPEN;
                            sts = TCP_FAIL;

                            // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                            gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_CMD_READ_CIPOPEN;
                        sts = TCP_FAIL;

                        // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
                        gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        default:
        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
        {
            //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_CMD_READ_CIPOPEN;
            sts = TCP_FAIL;

            // gprs.gprs_tcp_state = GPRS_TCP_CMD_READ_CIPOPEN; //PP commented on 29-07-24
            gprs.gprs_tcp_state = GPRS_TCP_CMD_ECHO_OFF;   //PP added on 29-07-24
            tcp_retry_cnt = 0;
        }
        break;
    }
    return sts;
}

websckt_sts_t websckt_connect(void)
{
    //static gprs_tcp_state_t tcp_state = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
    websckt_sts_t sts = WEBSOCKET_IN_PRG;
    static char tcp_retry_cnt = 0;
    // char tmpstr[TEMP_BUFF_LEN];
    char tmpstr[GPRS_RX_BUFFER_MAX];
    int num_byte = 0;
    static unsigned int timeout = 0;

#ifdef DEBUG_TCP_HANDLER
    vUART_SendStr(DEBUG_UART_BASE, "\nWCT:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.gprs_websocket_connect);
#endif

    switch(gprs.gprs_websocket_connect)
    {
        case GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "ATE0\r\n");
            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_RSP_ECHO_OFF;
#ifdef DEBUG_WEBSOCKET_CONN
            vUART_SendStr(UART_PC,"\ncmd:4EO");
#endif
        }
        break;

        case GPRS_WEBSOCKET_CONNCT_RSP_ECHO_OFF:
        {
            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
        }
        break;

        case GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND:
        {
            //gprs.data_mode = FALSE;
            flushRxBuffer(LTE_UART_BASE);
            vUART_SendStr(LTE_UART_BASE,"AT+CIPSEND=0\r");
#ifdef DEBUG_WEBSOCKET_CONN
            vUART_SendStr(UART_PC,"cmd:WCS");
#endif

            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_RSP_CIPSEND;
        }
        break;

        case GPRS_WEBSOCKET_CONNCT_RSP_CIPSEND:
        {
            //gprs.data_mode = FALSE;
            switch(check_string_nobuf(">"))
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"WCS:f\n");
#endif
                    //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                    vUART_SendChr(LTE_UART_BASE,0x1A);
                    gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                        sts = WEBSOCKET_FAIL;

                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                        // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"WCS:k\n");
#endif
                    gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD;

                    flushRxBuffer(LTE_UART);

                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"WCS:w\n");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                            sts = WEBSOCKET_FAIL;
                            vUART_SendChr(LTE_UART_BASE,0x1A);
                            //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                            // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_RSP_CIPSEND;
                        sts = WEBSOCKET_FAIL;

                        // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        case GPRS_WEBSOCKET_CONNCT_CMD:
        {
            char temp_buff[210];
            memset(temp_buff,0,sizeof(temp_buff));
            flushRxBuffer(LTE_UART);
            // flushTxBuffer(LTE_UART);
            // char randomKey[24];
            // memset(randomKey,0,sizeof(randomKey));
            // generateRandomKey(24,randomKey);
            char randomKey[25] = "q4xkcO42u266gldTuKaSOw==";
            // prepare_websckt_data(ip, &port, path);

#ifdef DEBUG_WEBSOCKET_CONN
            vUART_SendStr(UART_PC,"\npath:");
            vUART_SendStr(UART_PC,(const uint8_t *)cloud_config.path);
            vUART_SendStr(UART_PC,"\nip:");
            vUART_SendStr(UART_PC,(const uint8_t *)cloud_config.ip_addr);
            vUART_SendStr(UART_PC,"\nport:");
            vUART_SendInt(UART_PC,cloud_config.port_num);
#endif
            //my_sprintf(temp_buff,5,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Protocol: ocpp1.6\r\n\r\n","/RB/WS/ECMS/E4E002",gprs_config.ip_addr,gprs_config.port,randomKey);
            // my_sprintf(temp_buff,5,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Protocol: ocpp1.6\r\n\r\n",cloud_config.path,cloud_config.ip_addr,cloud_config.port_num,randomKey);
            my_sprintf(temp_buff,5,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: %s\r\n\r\n",cloud_config.path,cloud_config.ip_addr,cloud_config.port_num,randomKey);
            int final_index = strlen(temp_buff);

            temp_buff[final_index++] = 0x1A;
            vUART_SendBytes(LTE_UART_BASE,(const uint8_t *)temp_buff,final_index);

#ifdef DEBUG_WEBSOCKET_CONN
            vUART_SendStr(UART_PC,"cmd:WC\n");
            vUART_SendStr(UART_PC,"temp_buff:");
            //vUART_SendStr(UART_PC,(const uint8_t *)temp_buff);
            vUART_SendBytes(UART_PC,(const uint8_t *)temp_buff,final_index);
#endif
            // gprs.data_mode = FALSE;
            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_RSP;
        }
        break;

        case GPRS_WEBSOCKET_CONNCT_RSP:
        {
            memset(tmpstr,0,sizeof(tmpstr));
            char resp = check_string("OK",tmpstr,&num_byte);
            // switch(check_string_nobuf("OK"))
            //switch(check_string_nobuf("+CIPSEND: 206,206"))
            switch(resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"\nWC1:f");
                    vUART_SendBytes(UART_PC,(const uint8_t *)gprs_rx_buff.buffer,gprs_rx_buff.index);
#endif
                    gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;

                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_CONNCT_RSP;
                        sts = WEBSOCKET_FAIL;

                        // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"WC1:k\n");
                    //vUART_SendBytes(UART_PC,(const uint8_t *)gprs_rx_buff.buffer,gprs_rx_buff.index);
#endif
                    char *WS_upgrade_resp;
                    WS_upgrade_resp = strstr(tmpstr, "HTTP/1.1 101");   //PP added on 29-07-24
                    // WS_upgrade_resp = strstr(tmpstr, "HTTP/1.1 ");   //PP commented on 29-07-24
                    // WS_upgrade_resp = strstr((char*)gprs_rx_buff.buffer, "HTTP/1.1 ");
                    if(WS_upgrade_resp)
                    {
#ifdef DEBUG_WEBSOCKET_CONN
                        vUART_SendStr(UART_PC,"\nfoundHTTP:");
                        vUART_SendStr(UART_PC, (uint8_t*)WS_upgrade_resp);
#endif                        
                        // if((strstr((char*)gprs_rx_buff.buffer, "101"))||(strstr((char*)gprs_rx_buff.buffer, "connected")))
                        // if((strstr(tmpstr, "101"))||(strstr(tmpstr, "connected")))
                        if(strstr(tmpstr, "connected"))
                        {
#ifdef DEBUG_WEBSOCKET_CONN
                            vUART_SendStr(UART_PC,"\nWSUK");
#endif
                            // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                            sts = WEBSOCKET_PASS;
                        }
                        else
                        {
#ifdef DEBUG_WEBSOCKET_CONN
                            vUART_SendStr(UART_PC,"\nWSUF");
#endif                           
                            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_RSP_1;
                        }
                    }
                    else
                    {
#ifdef DEBUG_WEBSOCKET_CONN
                        vUART_SendStr(UART_PC,"\nnoHTTP");
#endif                 
                        num_byte = 0;
                        memset(tmpstr,0,sizeof(tmpstr)); 

                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_RSP_1;
                    }
                    //set_webconn_sts(TRUE);
                    //sts = WEBSOCKET_PASS;
                    // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_RSP_1;   //PP commented on 01-05-24
                    //flushRxBuffer(LTE_UART);

                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"WC1:w\n");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_CONNCT_RSP;
                            sts = WEBSOCKET_FAIL;

                            //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                            // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                {
                    // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                    sts = WEBSOCKET_FAIL;

                    // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                    gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                    tcp_retry_cnt = 0;
                }
                break;
            }
        }
        break;

        case GPRS_WEBSOCKET_CONNCT_RSP_1:
        {
            memset(tmpstr,0,sizeof(tmpstr));
            char resp = check_string("HTTP/1.1 101",tmpstr,&num_byte);  //PP added on 29-07-24
            // char resp = check_string("HTTP/1.1 ",tmpstr,&num_byte);  //PP commented on 29-07-24
            //switch(check_string_nobuf("HTTP/1.1 101"))
            switch(resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"\nWC2:f1");
                    vUART_SendBytes(UART_PC,(const uint8_t *)gprs_rx_buff.buffer,gprs_rx_buff.index);
#endif
                    //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;

                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_CONNCT_RSP;
                        sts = WEBSOCKET_FAIL;

                        // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    //set_webconn_sts(TRUE);
                    // if((strstr(tmpstr, "connected")) || (strstr(tmpstr,"101")))  //PP should i use this conition instead? 30-04-24
                    // if(valid_code(tmpstr))
                    // if((strstr(tmpstr, "101"))||(strstr(tmpstr, "connected")))   //PP commented on 29-07-24
                    if(strstr(tmpstr,"connected"))  //PP added on 29-07-24
                    {
#ifdef DEBUG_WEBSOCKET_CONN
                        vUART_SendStr(UART_PC,"WC2:k\n");
                        //vUART_SendBytes(UART_PC,(const uint8_t *)gprs_rx_buff.buffer,gprs_rx_buff.index);
#endif
                        sts = WEBSOCKET_PASS;
                    }
                    else
                    {
#ifdef DEBUG_WEBSOCKET_CONN
                        vUART_SendStr(UART_PC,"WC2:f2\n");
#endif
                        sts = WEBSOCKET_FAIL;
                    }

                    // setServerStatus(AVBL);

                    //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                    // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                    gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                    //flushRxBuffer(LTE_UART);

                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"WC2:w\n");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_CONNCT_RSP;
                            sts = WEBSOCKET_FAIL;

                            //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                            // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                            gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        sts = WEBSOCKET_FAIL;

                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                        // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                        gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        default:
        {
            if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
            {
                //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                sts = WEBSOCKET_FAIL;

                //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND; //PP commented on 29-07-24
                gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_ECHO_OFF;   //PP added on 29-07-24
                tcp_retry_cnt = 0;
            }
        }
        break;
    }
    return sts;
}

ping_status_t ping_send(void)
{
    // static ping_state_t ping_state = PING_CMD_CIPSEND;
    static char tcp_retry_cnt = 0;
    // char tmpstr[TEMP_BUFF_LEN];
    char tmpstr[GPRS_RX_BUFFER_MAX];
    int num_byte = 0;
    ping_status_t sts = PING_IN_PRG;
    //char locked;
    static unsigned int timeout = 0;

#ifdef DEBUG_TCP_HANDLER
    vUART_SendStr(DEBUG_UART_BASE, "\nPST:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.tcp_ping_states);
#endif

    // switch(ping_state)
    switch(gprs.tcp_ping_states)
    {
        case PING_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "ATE0\r\n");
            gprs.tcp_ping_states = PING_RSP_ECHO_OFF;
#ifdef DEBUG_GPRS_PING
            vUART_SendStr(UART_PC,"\ncmd:5EO");
#endif
        }
        break;

        case PING_RSP_ECHO_OFF:
        {
            gprs.tcp_ping_states = PING_CMD_CIPSEND;
        }
        break;

        case PING_CMD_CIPSEND:
        {
            //flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)"AT+CIPSEND=0\r");
#ifdef DEBUG_GPRS_PING
           vUART_SendStr(UART_PC,"cmd:PCS");

#endif
            // ping_state = PING_RSP_CIPSEND;
            gprs.tcp_ping_states = PING_RSP_CIPSEND;
        }
        break;

        case PING_RSP_CIPSEND:
        {
            switch(check_string_nobuf(">"))
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_PING
                    vUART_SendStr(UART_PC,"PCS:f");

#endif
                    vUART_SendChr(LTE_UART_BASE,0x1A);
                    // ping_state = PING_CMD_CIPSEND;
                    gprs.tcp_ping_states = PING_CMD_CIPSEND;
                    vUART_SendChr(LTE_UART_BASE,0x1A);
                    //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        sts = PING_FAIL;

                        // ping_state = PING_CMD_CIPSEND;

                        // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                        gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_GPRS_PING
                    vUART_SendStr(UART_PC,"PCS:k");

#endif
                    //sts = WEBSOCKET_FAIL;
                    //gprs.data_mode = TRUE;
                    // ping_state = PING_SEND_CMD;
                    gprs.tcp_ping_states = PING_SEND_CMD;
                    //sts = WEBSOCKET_PASS;
                    //flushRxBuffer(LTE_UART);

                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
#ifdef DEBUG_GPRS_PING
                        vUART_SendStr(UART_PC,"PCS:w");
#endif
                        vUART_SendChr(LTE_UART_BASE,0x1A);
                        timeout = 0;
                        // ping_state = PING_CMD_CIPSEND;
                        gprs.tcp_ping_states = PING_CMD_CIPSEND;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_RSP_CIPSEND;
                            sts = PING_FAIL;

                            // ping_state = PING_CMD_CIPSEND;

                            // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                            gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_RSP_CIPSEND;
                        sts = PING_FAIL;

                        // ping_state = PING_CMD_CIPSEND;

                        // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                        gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        case PING_SEND_CMD:
        {
            char buff[10];
            memset(buff,0,sizeof(buff));
            unsigned int index=0;
            buff[index++] = 0x89; //89 80 37 FA 21 3D 1A (ping)
            buff[index++] = 0x80;
            buff[index++] = 0x37;
            buff[index++] = 0xFA;
            buff[index++] = 0x21;
            buff[index++] = 0x3D;
            buff[index++] = 0x1A;

            vUART_SendBytes(LTE_UART_BASE,(const uint8_t *)buff,index);
#ifdef DEBUG_GPRS_PING
            vUART_SendStr(UART_PC,"\nPS");
#endif
            // ping_state = PING_SEND_RSP;
            gprs.tcp_ping_states = PING_SEND_RSP;
        }
        break;

        case PING_SEND_RSP:
        {
            memset(tmpstr,0,sizeof(tmpstr));
            char resp = check_string("OK",tmpstr,&num_byte);
            // switch(check_string_nobuf("OK"))
            switch(resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_PING
                    vUART_SendStr(UART_PC,"\n1PS:f");
#endif
                    //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                    // ping_state = PING_SEND_CMD;
                    gprs.tcp_ping_states = PING_SEND_CMD;
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_SEND_RSP;
                        sts = PING_FAIL;

                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        // ping_state = PING_SEND_CMD;

                        // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                        gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_GPRS_PING
                    vUART_SendStr(UART_PC,"\n1PS:k");
#endif
                    //if(pong_received(tmpstr))
                    //{
                        //set_webconn_sts(TRUE);
                        //
                        //sts = WEBSOCKET_PASS;
                        // ping_state = PING_SEND_RSP_1;    //PP commented this line on 30-04-24
//#ifdef DEBUG_GPRS_PING
//            vUART_SendStr(UART_PC,"\nSEND_BUFF_PING:k");
//#endif
                    //}
                    //else
                    //{
                        //set_webconn_sts(FALSE);
//#ifdef DEBUG_WEBSOCKET_CONN
//            vUART_SendStr(UART_PC,"PING_SEND_CIPSEND:f\n");
//#endif
                        //sts = WEBSOCKET_FAIL;
                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                        //
                    //}
                    //flushRxBuffer(LTE_UART);
                    char *ping_resp;
                    ping_resp = strstr(tmpstr, "+IPD2\r\n");
                    if(ping_resp)
                    {
#ifdef DEBUG_GPRS_PING
                        vUART_SendStr(UART_PC,"\nfoundIPD:");
                        vUART_SendStr(UART_PC, (uint8_t*)ping_resp);
#endif                        
                        // delete_SubStr(tmpstr, (char*)"+IPD2", 0);
                        // if(pong_received(tmpstr))
                        // if(pong_received(&ping_resp[5]))
                        if((ping_resp[7] & 0xFF) == 0x8A)
                        {
#ifdef DEBUG_GPRS_PING
                            vUART_SendStr(UART_PC,"\n1pong_recv");
#endif
                            sts = PING_PASS;
                            // ping_state = PING_CMD_CIPSEND;

                            // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                            gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24

                            //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                            //ping_state = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                        }
                        else
                        {
#ifdef DEBUG_GPRS_PING
                            vUART_SendStr(UART_PC,"\n1pong_not_recv");
#endif
                            num_byte = 0;
                            memset(tmpstr,0,sizeof(tmpstr));

                            // ping_state = PING_SEND_RSP_1;
                            gprs.tcp_ping_states = PING_SEND_RSP_1;
                        }
                    }
                    else
                    {
#ifdef DEBUG_GPRS_PING
                        vUART_SendStr(UART_PC,"\nnoIPD");
#endif 
                        num_byte = 0;
                        memset(tmpstr,0,sizeof(tmpstr));

                        // ping_state = PING_SEND_RSP_1;
                        gprs.tcp_ping_states = PING_SEND_RSP_1;
                    }

                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_WEBSOCKET_CONN
                    vUART_SendStr(UART_PC,"1PS:w\n");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
//#ifdef DEBUG_GPRS_PING
//                      vUART_SendStr(UART_PC,"\nSEND_BUFF_PING:w");
//#endif
                        timeout = 0;
                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        //ping_state = PING_SEND_CMD;
                        // ping_state = PING_CMD_CIPSEND;
                        gprs.tcp_ping_states = PING_CMD_CIPSEND;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_SEND_RSP;
                            sts = PING_FAIL;

                            //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                            //ping_state = PING_SEND_CMD;
                            // ping_state = PING_CMD_CIPSEND;

                            // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                            gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;
            }
        }
        break;

        case PING_SEND_RSP_1:
        {
            memset(tmpstr,0,sizeof(tmpstr));
            char resp = check_string("+IPD2\r\n",tmpstr,&num_byte);
            // char resp = check_string("+IPD2",tmpstr,&num_byte);
            // char resp = check_string("+IPD2\r\n",tmpstr,&num_byte);

            switch(resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_GPRS_PING
                    vUART_SendStr(UART_PC,"\n+IPD:F");
#endif
                    //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                    // ping_state = PING_SEND_CMD;
                    gprs.tcp_ping_states = PING_SEND_CMD;
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_SEND_RSP;
                        sts = PING_FAIL;

                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        // ping_state = PING_SEND_CMD;
                        // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                        gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    // if(pong_received(tmpstr))
                    if((tmpstr[0] & 0xFF) == 0x8A)
                    {
#ifdef DEBUG_GPRS_PING
                        vUART_SendStr(UART_PC,"\n2pong_recv");
#endif
                        sts = PING_PASS;
                        // ping_state = PING_CMD_CIPSEND;
                        // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                        gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        //ping_state = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                    }
                    else
                    {
#ifdef DEBUG_GPRS_PING
                        vUART_SendStr(UART_PC,"\n2pong_not_recv");
#endif
                        sts = PING_FAIL;
                        // ping_state = PING_CMD_CIPSEND;
                        // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                        gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24

                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND;
                    }
                }
                break;

                case  (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_GPRS_PING
                    vUART_SendStr(UART_PC,"\n+IPD:w");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        timeout = 0;
                       // gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_SEND_RSP;
                            sts = PING_FAIL;

                            //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                            // ping_state = PING_SEND_CMD;
                            // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                            gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        sts = PING_FAIL;

                        //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                        // ping_state = PING_SEND_CMD;
                        // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                        gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        default:
        {
            if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
            {
                // tcp.errcode = TCP_ERR_OFFSET + GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                sts = PING_FAIL;

                //gprs.gprs_websocket_connect = GPRS_WEBSOCKET_PING_CMD_CIPSEND;
                // ping_state = PING_SEND_CMD;
                // gprs.tcp_ping_states = PING_CMD_CIPSEND; //PP commented on 27-07-24
                gprs.tcp_ping_states = PING_CMD_ECHO_OFF;   //PP added on 27-07-24
                tcp_retry_cnt = 0;
            }
        }
        break;
    }
    return sts;
}

tcp_packet_status_t tcp_send(char *data_str, int len)
{
    //static gprs_tcp_state_t tcp_state = GPRS_TCP_CMD_SEND;
    static char tcp_retry_cnt = 0;
    tcp_packet_status_t sts = TCP_SEND_IN_PRG;
    static unsigned int timeout = 0;
    // char tmpstr[TEMP_BUFF_LEN];
    char tmpstr[GPRS_RX_BUFFER_MAX];
    int num_bytes = 0;
    memset(tmpstr,0,sizeof(tmpstr));

#ifdef DEBUG_TCP_HANDLER
    vUART_SendStr(DEBUG_UART_BASE, "\nTST:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.tcp_packet_state);
#endif

    switch(gprs.tcp_packet_state)
    {
        case TCP_CIPSEND_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "ATE0\r\n");
            gprs.tcp_packet_state = TCP_CIPSEND_RSP_ECHO_OFF;
#ifdef DEBUG_GPRS_DATA_UPLOAD
            vUART_SendStr(UART_PC,"\ncmd:6EO");
#endif
        }
        break;

        case TCP_CIPSEND_RSP_ECHO_OFF:
        {
            gprs.tcp_packet_state = TCP_CIPSEND_CMD;
        }
        break;

        case TCP_CIPSEND_CMD:
        {
            char buff_cmd_cipsend[20];
            memset(buff_cmd_cipsend,0,sizeof(buff_cmd_cipsend));
            //flushRxBuffer(LTE_UART);
            // my_sprintf(tmpstr,2,"AT+CIPSEND=0,%d\r", len);
            my_sprintf(buff_cmd_cipsend,2,"AT+CIPSEND=0,%d\r", len);
#ifdef DEBUG_TCP_SEND
            vUART_SendStr(UART_PC,"\ncmd:TCS");
#endif
            // vUART_SendStr(LTE_UART_BASE,(const uint8_t *)tmpstr);
            vUART_SendStr(LTE_UART_BASE,(const uint8_t *)buff_cmd_cipsend);
            gprs.tcp_packet_state = TCP_CIPSEND_RSP;
        }
        break;

        case TCP_CIPSEND_RSP:
        {
            switch(check_string_nobuf(">"))
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_TCP_SEND
                    vUART_SendStr(UART_PC,"\nTCS:f");
#endif
                    vUART_SendChr(LTE_UART_BASE,0x1A);
                    gprs.tcp_packet_state = TCP_CIPSEND_CMD;

                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_SEND;
                        sts = TCP_SEND_FAIL;

                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
#ifdef DEBUG_TCP_SEND
                    vUART_SendStr(UART_PC,"\nTCS:k");
#endif
                    gprs.tcp_packet_state = TCP_SEND_PACKET_CMD;
                    //flushRxBuffer(LTE_UART);

                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_TCP_SEND
                    vUART_SendStr(UART_PC,"\nTCS:w");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        vUART_SendChr(LTE_UART_BASE,0x1A);
                        timeout = 0;
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                            // tcp.errcode = TCP_ERR_OFFSET + TCP_CIPSEND_RSP;
                            sts = TCP_SEND_FAIL;

                            // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                            gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        // tcp.errcode = TCP_ERR_OFFSET + TCP_CIPSEND_CMD;
                        sts = TCP_SEND_FAIL;

                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        case TCP_SEND_PACKET_CMD:
        {
            flushRxBuffer(LTE_UART);
#ifdef DEBUG_TCP_SEND
            vUART_SendStr(UART_PC,"\ncmd:TS");
            vUART_SendStr(UART_PC,"\nlen:");
            vUART_SendInt(UART_PC,gprs_rx_buff.index);
            vUART_SendChr(UART_PC, ',');
            vUART_SendBytes(UART_PC, (const uint8_t *)data_str,len);
#endif
            vUART_SendBytes(LTE_UART_BASE,(const uint8_t *)data_str,len);
            gprs.tcp_packet_state = TCP_SEND_PACKET_RSP;
        }
        break;

        case TCP_SEND_PACKET_RSP:
        {
            memset(tmpstr,0,sizeof(tmpstr));
            char resp = check_string("OK",tmpstr,(int *)&num_bytes);
            // switch(check_string_nobuf("OK"))
            switch(resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_TCP_SEND
                    vUART_SendStr(UART_PC,"\n1TS:f");
#endif
                    gprs.tcp_packet_state = TCP_CIPSEND_CMD;

                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_DATA_SEND;
                        sts = TCP_SEND_FAIL;

                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
//                     if(get_upload_data() == RESPONSE)
//                     {
// #ifdef DEBUG_TCP_SEND
//                         vUART_SendStr(UART_PC,"TCP_SEND_PACKET_CMD:k");
//                         vUART_SendStr(UART_PC,"\nit_is_response_data");
// #endif
//                         sts = TCP_SEND_PASS;
//                         gprs.tcp_packet_state = TCP_CIPSEND_CMD;
//                         set_upload_data(REQUEST);
//                     }
//                     else
//                     {
// #ifdef DEBUG_TCP_SEND
//                         vUART_SendStr(UART_PC,"TCP_SEND_PACKET_CMD:k");
//                         vUART_SendStr(UART_PC,"\nit_is_req_data");
// #endif
//                         gprs.tcp_packet_state = TCP_SEND_PACKET_RSP_1;
//                     }

                    //flushRxBuffer(LTE_UART);
#ifdef DEBUG_TCP_SEND
                    // UWriteString((char*)"\n1KTS",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1KTS");
#endif

//                     if(respos_recvd(tmpstr)) //PP commented on 30-07-24
//                     {
// #ifdef DEBUG_TCP_SEND
//                         vUART_SendStr(UART_PC,"\n1TS:k2");
// #endif
//                         sts = TCP_SEND_PASS;
//                         // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
//                         gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
//                     }
//                     else
//                     {
// #ifdef DEBUG_TCP_SEND
//                         vUART_SendStr(UART_PC,"\n1TS:f2");
// #endif
//                         gprs.tcp_packet_state = TCP_SEND_PACKET_RSP_1;
//                     }

                    char *data_resp;
                    int data_len = 0;

                    data_resp = strstr(tmpstr,"+IPD");
                    if(data_resp)
                    {
                        data_len = strlen(data_resp);
                        if(respos_recvd(&data_resp[4],data_len))    ////PP added on 30-07-24. (Copied from EVSE 3.12)
                        {
#ifdef DEBUG_TCP_SEND
                            vUART_SendStr(DEBUG_UART_BASE, "\nTRSP");
#endif  
                            sts = TCP_SEND_PASS;
                            gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;
                        }
                        else
                        {
#ifdef DEBUG_TCP_SEND
                            vUART_SendStr(DEBUG_UART_BASE, "\nnoRSP");
#endif                     
                            num_bytes = 0;
                            memset(tmpstr,0,sizeof(tmpstr));

                            gprs.tcp_packet_state = TCP_SEND_PACKET_RSP_1;
                        }
                    }
                    else
                    {
#ifdef DEBUG_TCP_SEND
                        vUART_SendStr(DEBUG_UART_BASE, "\nnoTIPD");
#endif                     
                        num_bytes = 0;
                        memset(tmpstr,0,sizeof(tmpstr));

                        gprs.tcp_packet_state = TCP_SEND_PACKET_RSP_1;
                    }

                    // gprs.tcp_packet_state = TCP_SEND_PACKET_RSP_1;

                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_TCP_SEND
                    vUART_SendStr(UART_PC,"\n1TS:w");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
                           //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_DATA_SEND;
                            sts = TCP_SEND_FAIL;

                            // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                            gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_DATA_SEND;
                        sts = TCP_SEND_FAIL;

                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        case TCP_SEND_PACKET_RSP_1:
        {
            memset(tmpstr,0,sizeof(tmpstr));
            char resp = check_string("+IPD",tmpstr,(int *)&num_bytes);
            //switch (check_string_nobuf("+IPD"))
            switch(resp)
            {
                case (GPRS_MATCH_FAIL):
                {
#ifdef DEBUG_TCP_SEND
                    vUART_SendStr(UART_PC,"\n2TS:f");
#endif
                    gprs.tcp_packet_state = TCP_CIPSEND_CMD;

                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_DATA_SEND;
                        sts = TCP_SEND_FAIL;

                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case (GPRS_MATCH_OK):
                {
                    // if(respos_recvd(tmpstr)) //PP commented on 30-07-24
                    if(respos_recvd(tmpstr,num_bytes))  //PP added on 30-07-24. (Copied from EVSE 3.12)
                    {
#ifdef DEBUG_TCP_SEND
                        vUART_SendStr(UART_PC,"\n2TS:k");
#endif
                        // PP commented on 27-04-24: will uncomment these later. Some of these are from other EVSE files, will have to see what's redundant and what's not.
                        // GPRS_Server_Response();
                        sts = TCP_SEND_PASS;
                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                    }
                    else
                    {
#ifdef DEBUG_TCP_SEND
                        vUART_SendStr(UART_PC,"\nResp_not_recvd");

#endif
                        sts = TCP_SEND_FAIL;
                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                    }
                    //sts = TCP_SEND_PASS;
                    // gprs.tcp_packet_state = TCP_CIPSEND_CMD;
                    //flushRxBuffer(LTE_UART);
                    tcp_retry_cnt = 0;
                    timeout = 0;
                }
                break;

                case (GPRS_NO_NEW_MSG):
                {
#ifdef DEBUG_TCP_SEND
                    vUART_SendStr(UART_PC,"\nTS:w");
#endif
                    if(timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        timeout = 0;
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD;

                        if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                        {
#ifdef DEBUG_TCP_SEND
                            vUART_SendStr(UART_PC,"\nTSw:tf");
#endif
                            //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_DATA_SEND;
                            sts = TCP_SEND_FAIL;

                            // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                            gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                            tcp_retry_cnt = 0;
                        }
                        //flushRxBuffer(LTE_UART);
                    }
                }
                break;

                default:
                {
                    if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
                    {
                        //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_DATA_SEND;
                        sts = TCP_SEND_FAIL;

                        // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                        gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                        tcp_retry_cnt = 0;
                    }
                }
                break;
            }
        }
        break;

        default:
        {
            if (tcp_retry_cnt++ >= TCP_RETRY_CNT)
            {
                //tcp.errcode = TCP_ERR_OFFSET + GPRS_TCP_RSP_DATA_SEND;
                sts = TCP_SEND_FAIL;

                // gprs.tcp_packet_state = TCP_CIPSEND_CMD;    //PP commented on 29-07-24
                gprs.tcp_packet_state = TCP_CIPSEND_CMD_ECHO_OFF;   //PP added on 29-07-24
                tcp_retry_cnt = 0;
            }
        }
        break;
    }
    return sts;
}

WS_discon_status_t websckt_disconnect(void)
{
    WS_discon_status_t sts = WS_DISCON_IN_PRG;
    static uint8_t conn_retry_cnt = 0;
    static uint16_t conn_timeout = 0;  
    int num_bytes = 0;

    // static tcp_webskt_disconnect_states_t tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;

#ifdef DEBUG_TCP_HANDLER
    // UWriteString((char*)"\nWDIS:", DBG_UART);
    // UWriteInt(gprs.tcp_webskt_disconnect_states,DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE, "\nWSD:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.tcp_webskt_disconnect_states);
#endif
    // switch(LTEmodule.TCP_websckt_states)
    // switch(tcp_webskt_disconnect_states)
    switch(gprs.tcp_webskt_disconnect_states)
    {
        case GPRS_WDISCONN_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "ATE0\r\n");
            gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_RSP_ECHO_OFF;
#ifdef DEBUG_GPRS_DATA_UPLOAD
            vUART_SendStr(UART_PC,"\ncmd:7EO");
#endif
        }
        break;

        case GPRS_WDISCONN_RSP_ECHO_OFF:
        {
            gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
        }
        break;

        case GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND:
        {
#ifdef DEBUG_WS_DISCONN
            // UWriteString((char*)"\nWDCS", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nWSDCS");
#endif
            // gprs_tx_buff.locked = 0;
            // gprs_tx_buff.index = 0;
            // memset((char*)&gprs_tx_buff.buffer, 0, GPRS_TX_BUFFER_MAX);
            
            flushRxBuffer(LTE_UART);

            // UWriteString((char*)"AT+CIPSEND=0,6\r\n",LTE_UART);
            // vUART_SendStr(DEBUG_UART_BASE, "AT+CIPSEND=0,6\r\n");    //iss zurm ke liye to tumhe fasi hogi, fasi!
            vUART_SendStr(LTE_UART_BASE, "AT+CIPSEND=0,6\r\n");

            // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_RSP_CIPSEND;
            gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_RSP_CIPSEND;
        }
        break;

        case GPRS_WEBSOCKET_DISCONNCT_RSP_CIPSEND:
        {
            switch(check_string_nobuf(">"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\nWDCSF", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nWSDCSF");
#endif
                    //LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                    gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                    
                    if(conn_retry_cnt++ >= RETRY_CNT)
                    {
                        conn_retry_cnt = 0;
                        conn_timeout = 0;

                        sts = WS_DISCON_FAIL;
                        // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                        // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                        gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef  DEBUG_WS_DISCONN
                    // UWriteString((char*)"\nWDCS:k",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nWSDCS:k");
#endif                   
                    // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD;
                    gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD;
                    conn_retry_cnt = 0;
                    conn_timeout = 0;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\nWDCSW", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\nWSDCSW");
#endif
                    // if(conn_timeout++ >= ATE0_TIMEOUT)
                    if(conn_timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        conn_timeout = 0;
                        //LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                        gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;

                        if(conn_retry_cnt++ >= RETRY_CNT)
                        {
                            conn_retry_cnt = 0;
                            sts = WS_DISCON_FAIL;

                            // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                            // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                            gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                        }
                        flushRxBuffer(LTE_UART);
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

        case GPRS_WEBSOCKET_DISCONNCT_CMD:
        {
#ifdef DEBUG_WS_DISCONN
            // UWriteString((char*)"\nWDC1", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nWSDC1");
#endif
            // gprs_tx_buff.locked = 0;
            // gprs_tx_buff.index = 0;
            // memset((char*)&gprs_tx_buff.buffer, 0, GPRS_TX_BUFFER_MAX);
            
            flushRxBuffer(LTE_UART);

            char buff[8] = {0x88,0x80,0x37,0xFA,0x21,0x3D};

            // UWriteString((char*)buff,LTE_UART);
            // vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)buff);  //iss zurm ke liye to tumhe fasi hogi, fasi!
            vUART_SendStr(LTE_UART_BASE, (uint8_t*)buff);

#ifdef DEBUG_WS_DISCONN
            // UWriteString((char*)"\nWDC:", DBG_UART);
            // UWriteString(buff, DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nWSDC:");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)buff);
#endif

            // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_RSP;
            gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_RSP;            
        }
        break;

        case GPRS_WEBSOCKET_DISCONNCT_RSP:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];
            memset(tmpstr, 0, sizeof(tmpstr));

#ifdef DEBUG_WS_DISCONN
            // UWriteString((char*)"\n1WDR:", DBG_UART);
            // UWriteInt(gprs_rx_buff.index, DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteString((char*)gprs_rx_buff.buffer,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\n1WSDR:");
            vUART_SendInt(DEBUG_UART_BASE, gprs_rx_buff.index);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)gprs_rx_buff.buffer);
#endif  //DEBUG_WS_DISCONN

            char resp = check_string("OK",tmpstr,&num_bytes);
            switch(resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\n1WDF", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1WSDF:");
#endif           
                    //LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                    gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;

                    if(conn_retry_cnt++ >= RETRY_CNT)
                    {
                        conn_retry_cnt = 0;
                        conn_timeout = 0;

                        sts = WS_DISCON_FAIL;

                        // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                        // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                        gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                    }
                    // flushRxBuffer(LTE_UART);         
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\n1WDK1", DBG_UART);
                    // UWriteString(tmpstr,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1WSDK1");
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)tmpstr);
#endif
                    if(strstr(tmpstr,"+IPCLOSE: 0,1"))
                    {
#ifdef DEBUG_WS_DISCONN
                        // UWriteString((char*)"\n1WDK2", DBG_UART);
                        // UWriteString(tmpstr,DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\n1WSDK2");
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)tmpstr);
#endif                
                        // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                        // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                        gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                        conn_retry_cnt = 0;
                        conn_timeout = 0;
                        sts = WS_DISCON_PASS;        
                    }
                    else
                    {
#ifdef DEBUG_WS_DISCONN
                        // UWriteString((char*)"\n1WDF2", DBG_UART);
                        // UWriteString(tmpstr,DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\n1WSDF2");
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)tmpstr);
#endif 
                        // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_RSP_1;
                        gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_RSP_1;
                        conn_retry_cnt = 0;
                        conn_timeout = 0;
                    }
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\n1WDW", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1WSDW");
#endif
                    // if(conn_timeout++ >= ATE0_TIMEOUT)
                    if(conn_timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        conn_timeout = 0;

                        //LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                        gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;

                        if(conn_retry_cnt++ >= RETRY_CNT)
                        {
                            conn_retry_cnt = 0;
                            sts = WS_DISCON_FAIL;

                            // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                            // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                            gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                        }
                        flushRxBuffer(LTE_UART);
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

        case GPRS_WEBSOCKET_DISCONNCT_RSP_1:
        {
            static uint16_t ws_disconnct_timeout = 0;
            static uint8_t ws_diconnct_retry = 0;

            char tmpstr[GPRS_RX_BUFFER_MAX];

            memset(tmpstr, 0, sizeof(tmpstr));
#ifdef DEBUG_WS_DISCONN
            // UWriteString((char*)"\n2WDR:", DBG_UART);
            // UWriteInt(gprs_rx_buff.index, DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteString((char*)gprs_rx_buff.buffer,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\n2WSDR:");
            vUART_SendInt(DEBUG_UART_BASE, gprs_rx_buff.index);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)gprs_rx_buff.buffer);
#endif  //DEBUG_WS_DISCONN

            char resp = check_string("+IPCLOSE: 0,1",tmpstr, &num_bytes);
            switch(resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\n2WDF", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n2WSDF");
#endif
                    gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                    if(ws_diconnct_retry++ >= RETRY_CNT)
                    {
                        ws_diconnct_retry = 0;
                        //ws_disconnct_timeout = 0;

                        sts = WS_DISCON_FAIL;
                        // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                        gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\n2WDK", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n2WSDK");
#endif 
                    // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                    // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                    gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                    ws_diconnct_retry = 0;
                    ws_disconnct_timeout = 0;
                    sts = WS_DISCON_PASS;
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef DEBUG_WS_DISCONN
                    // UWriteString((char*)"\n2WDW", DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n2WSDW");
#endif            
                    // if(ws_disconnct_timeout++ >= WEBSOCKET_CONNECT_TIMEOUT)
                    if(ws_disconnct_timeout++ >= TCP_SEND_TIMEOUT)
                    {
                        ws_disconnct_timeout = 0;

                        // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                        gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;

                        if(ws_diconnct_retry++ >= RETRY_CNT)
                        {
                            ws_diconnct_retry = 0;
                            sts = WS_DISCON_FAIL;

                            // LTEmodule.TCP_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;
                            // gprs.tcp_webskt_disconnect_states = GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND;    //PP commented on 29-07-24
                            gprs.tcp_webskt_disconnect_states = GPRS_WDISCONN_CMD_ECHO_OFF; //PP added on 29-07-24
                        }
                        flushRxBuffer(LTE_UART);
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

TCP_discon_status_t tcp_disconnect(void)
{
    TCP_discon_status_t sts = TCP_DISCON_IN_PRG;
    static uint8_t conn_retry_cnt = 0;
    static uint16_t conn_timeout = 0;  
    int num_bytes = 0;

#ifdef DEBUG_TCP_HANDLER
    vUART_SendStr(DEBUG_UART_BASE, "\nTCL:");
    vUART_SendInt(DEBUG_UART_BASE, gprs.tcp_disconnct_states);
#endif

    // static tcp_disconnct_states_t tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;

    // switch(LTEmodule.TCP_disconnct_states)
    // switch(tcp_disconnct_states)
    switch(gprs.tcp_disconnct_states)
    {
        case GPRS_TCPCLOSE_CMD_ECHO_OFF:
        {
            flushRxBuffer(LTE_UART);
            vUART_SendStr(LTE_UART_BASE, "ATE0\r\n");
            gprs.tcp_disconnct_states = GPRS_TCPCLOSE_RSP_ECHO_OFF;
#ifdef DEBUG_GPRS_DATA_UPLOAD
            vUART_SendStr(UART_PC,"\ncmd:8EO");
#endif
        }
        break;

        case GPRS_TCPCLOSE_RSP_ECHO_OFF:
        {
            gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
        }
        break;

        case GPRS_TCP_CMD_CIPCLOSE:
        {
#ifdef DEBUG_TCP_DISCONN
            // UWriteString((char*)"\nTCL", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nTCL");
#endif
            // gprs_tx_buff.locked = 0;
            // gprs_tx_buff.index = 0;
            // memset((char*)&gprs_tx_buff.buffer, 0, GPRS_TX_BUFFER_MAX);
            
            flushRxBuffer(LTE_UART);

            // UWriteString((char*)"AT+CIPCLOSE=0\r\n",LTE_UART);
            // vUART_SendStr(DEBUG_UART_BASE, "AT+CIPCLOSE=0\r\n"); //iss zurm ke liye to tumhe fasi hogi, fasi!
            vUART_SendStr(LTE_UART_BASE, "AT+CIPCLOSE=0\r\n");

            // LTEmodule.TCP_disconnct_states = GPRS_TCP_RSP_CIPCLOSE;
            gprs.tcp_disconnct_states = GPRS_TCP_RSP_CIPCLOSE;
        }
        break;

        case GPRS_TCP_RSP_CIPCLOSE:
        {
#ifdef  DEBUG_TCP_DISCONN
            // UWriteString((char*)"\n1CC:",DBG_UART);
            // UWriteString((char*)gprs_rx_buff.buffer,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(gprs_rx_buff.index,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\n1CC");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)gprs_rx_buff.buffer);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, gprs_rx_buff.index);
#endif
            char tmpstr[GPRS_RX_BUFFER_MAX];

            memset(tmpstr, 0, sizeof(tmpstr));
            char resp = check_string("OK",tmpstr, &num_bytes);
            // switch(check_string_nobuf("OK"))
            switch(resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef  DEBUG_TCP_DISCONN
                    // UWriteString((char*)"\n1CC:f:",DBG_UART);
                    // UWriteInt(conn_retry_cnt,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1CC:f:");
                    vUART_SendInt(DEBUG_UART_BASE, conn_retry_cnt);
#endif
                    //LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                    gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;

                    if(conn_retry_cnt++ >= RETRY_CNT)
                    {
                        conn_retry_cnt = 0;
                        //conn_timeout = 0;
#ifdef USE_NETCLOSE
                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                        gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
#else
                        sts = TCP_DISCON_FAIL;
                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                        // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                        gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
#endif
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef  DEBUG_TCP_DISCONN
                    // UWriteString((char*)"\n1CC:k",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1CC:k");
#endif
                    if(strstr(tmpstr, "+CIPCLOSE: 0,0"))
                    {
#ifdef  DEBUG_TCP_DISCONN
                        // UWriteString((char*)"\n1CC:k2",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\n1CC:k2");
#endif

#ifdef USE_NETCLOSE
                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                        gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                        conn_retry_cnt = 0;
                        conn_timeout = 0;
#else
                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;

                        // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                        gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
                        conn_retry_cnt = 0;
                        conn_timeout = 0;
                        sts = TCP_DISCON_PASS;
#endif
                    }
                    else
                    {
#ifdef  DEBUG_TCP_DISCONN
                        // UWriteString((char*)"\n1CC:f2",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\n1CC:f2");
#endif
                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_RSP_1_CIPCLOSE;
                        gprs.tcp_disconnct_states = GPRS_TCP_RSP_1_CIPCLOSE;
                        conn_retry_cnt = 0;
                        conn_timeout = 0;
                    }
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef  DEBUG_TCP_DISCONN
                    // UWriteString((char*)"\n1CC:w",DBG_UART);
                    // UWriteInt(conn_timeout,DBG_UART);
                    // UWriteData(',',DBG_UART);
                    // UWriteInt(conn_retry_cnt,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1CC:w");
                    vUART_SendInt(DEBUG_UART_BASE, conn_timeout);
                    vUART_SendChr(DEBUG_UART_BASE,',');
                    vUART_SendInt(DEBUG_UART_BASE, conn_retry_cnt);
#endif
                    // if(conn_timeout++ >= CIPOPEN_TIMEOUT)
                    if(conn_timeout++ >= TCP_START_TIMEOUT)
                    {
                        conn_timeout = 0;

                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                        gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;

                        if(conn_retry_cnt++ >= RETRY_CNT)
                        {
                            conn_retry_cnt = 0;
                            
#ifdef USE_NETCLOSE
                            // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                            gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
#else
                            sts = TCP_DISCON_FAIL;
                            // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;

                            // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                            gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
#endif  //USE_NETCLOSE
                        }
                        flushRxBuffer(LTE_UART);
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

        case GPRS_TCP_RSP_1_CIPCLOSE:
        {
            static uint16_t tcp_stop_timeout = 0;
            static uint8_t tcp_stop_retry = 0;
#ifdef  DEBUG_TCP_DISCONN
            // UWriteString((char*)"\n2CC:",DBG_UART);
            // UWriteString((char*)gprs_rx_buff.buffer,DBG_UART);
            // UWriteData(',',DBG_UART);
            // UWriteInt(gprs_rx_buff.index,DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\n2CC");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)gprs_rx_buff.buffer);
            vUART_SendChr(DEBUG_UART_BASE,',');
            vUART_SendInt(DEBUG_UART_BASE, gprs_rx_buff.index);
#endif

            // switch(check_string_nobuf("+CIPOPEN: 0,0"))  //iss zurm ke liye to tumhe fasi hogi, fasi!
            switch(check_string_nobuf("+CIPCLOSE:0,0"))
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef  DEBUG_TCP_DISCONN
                    // UWriteString((char*)"\n2CC:f:",DBG_UART);
                    // UWriteInt(tcp_stop_retry,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n2CC:f:");
                    vUART_SendInt(DEBUG_UART_BASE, tcp_stop_retry);
#endif
                    gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                    if(tcp_stop_retry++ >= RETRY_CNT)
                    {
                        tcp_stop_retry = 0;
                        //tcp_stop_timeout = 0;

#ifdef USE_NETCLOSE
                        gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
#else
                        sts = TCP_DISCON_FAIL;
                        
                        // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                        gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
#endif  //USE_NETCLOSE
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef  DEBUG_TCP_DISCONN
                    // UWriteString((char*)"\n2CC:k",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\n2CC:k");
#endif

#ifdef USE_NETCLOSE
                    // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                    gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                    tcp_stop_retry = 0;
                    tcp_stop_timeout = 0;
#else
                    // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                    // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                    gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
                    tcp_stop_retry = 0;
                    tcp_stop_timeout = 0;
                    sts = TCP_DISCON_PASS;
#endif  //USE_NETCLOSE
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef  DEBUG_TCP_DISCONN
                    // UWriteString((char*)"\n2CO:w:",DBG_UART);
                    // UWriteInt(tcp_stop_timeout,DBG_UART);
                    // UWriteData(',',DBG_UART);
                    // UWriteInt(tcp_stop_retry,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n2CC:w");
                    vUART_SendInt(DEBUG_UART_BASE, tcp_stop_timeout);
                    vUART_SendChr(DEBUG_UART_BASE,',');
                    vUART_SendInt(DEBUG_UART_BASE, tcp_stop_retry);
#endif
                    // if(tcp_stop_timeout++ >= CIPOPEN_TIMEOUT)
                    if(tcp_stop_timeout++ >= TCP_START_TIMEOUT)
                    {
                        tcp_stop_timeout = 0;

                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                        gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;

                        if(tcp_stop_retry++ >= RETRY_CNT)
                        {
                            tcp_stop_retry = 0;
#ifdef USE_NETCLOSE
                            // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                            gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
#else
                            sts = TCP_DISCON_FAIL;

                            // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                            // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                            gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
#endif  //USE_NETCLOSE
                        }
                        flushRxBuffer(LTE_UART);
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

#ifdef USE_NETCLOSE
        case GPRS_TCP_CMD_NETCLOSE:
        {
#ifdef DEBUG_TCP_DISCONN
            // UWriteString((char*)"\nTNC", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nTNC");
#endif 
            // flushTxBuffer(LTE_UART);
            flushRxBuffer(LTE_UART);

            // UWriteString((char*)"AT+NETCLOSE\r\n", LTE_UART);
            // vUART_SendStr(DEBUG_UART_BASE, "AT+NETCLOSE\r\n");   //iss zurm ke liye to tumhe fasi hogi, fasi!
            vUART_SendStr(LTE_UART_BASE, "AT+NETCLOSE\r\n");
            // LTEmodule.TCP_disconnct_states = GPRS_TCP_RSP_NETCLOSE;
            gprs.tcp_disconnct_states = GPRS_TCP_RSP_NETCLOSE;
        }
        break;

        case GPRS_TCP_RSP_NETCLOSE:
        {
            char tmpstr[GPRS_RX_BUFFER_MAX];
            char resp = check_string("OK", tmpstr, &num_bytes);
            switch(resp)
            {
                case GPRS_MATCH_FAIL:
                {
#ifdef  DEBUG_TCP_CONN
                    // UWriteString((char*)"\n1NC:f:",DBG_UART);
                    // UWriteInt(conn_retry_cnt,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1NC:f:");
                    vUART_SendInt(DEBUG_UART_BASE, conn_retry_cnt);
#endif
                    // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_NETCLOSE;
                    gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;

                    if(conn_retry_cnt++ >= RETRY_CNT)
                    {
                        conn_retry_cnt = 0;
                        //conn_timeout = 0;

                        sts = TCP_DISCON_FAIL;
                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                        // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                        gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
                    }
                    flushRxBuffer(LTE_UART);
                }
                break;

                case GPRS_MATCH_OK:
                {
#ifdef  DEBUG_TCP_CONN
                    // UWriteString((char*)"\n1NC:k",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1NC:k");
#endif
                    if(strstr(tmpstr, "+NETCLOSE: 0"))
                    {
                        // LTEmodule.TCP_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;
                        // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                        gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24

                        sts = TCP_DISCON_PASS;
                        
                        conn_retry_cnt = 0;
                        conn_timeout = 0;
                    }
                }
                break;

                case GPRS_NO_NEW_MSG:
                {
#ifdef  DEBUG_TCP_CONN
                    // UWriteString((char*)"\n1NC:w",DBG_UART);
                    // UWriteInt(conn_timeout,DBG_UART);
                    // UWriteData(',',DBG_UART);
                    // UWriteInt(conn_retry_cnt,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1NC:w");
                    vUART_SendInt(DEBUG_UART_BASE, conn_timeout);
                    vUART_SendChr(DEBUG_UART_BASE,',');
                    vUART_SendInt(DEBUG_UART_BASE, conn_retry_cnt);
#endif
                    // if(conn_timeout++ >= CIPOPEN_TIMEOUT)
                    if(conn_timeout++ >= TCP_START_TIMEOUT)
                    {
                        conn_timeout = 0;

                        // LTEmodule.TCP_connct_states = GPRS_TCP_CMD_NETCLOSE;
                        gprs.tcp_disconnct_states = GPRS_TCP_CMD_NETCLOSE;

                        if(conn_retry_cnt++ >= RETRY_CNT)
                        {
                            conn_retry_cnt = 0;
                            sts = TCP_DISCON_FAIL;

                            // LTEmodule.TCP_connct_states = GPRS_TCP_CMD_CIPCLOSE;
                            // gprs.tcp_disconnct_states = GPRS_TCP_CMD_CIPCLOSE;   //PP commented on 29-07-24
                            gprs.tcp_disconnct_states = GPRS_TCPCLOSE_CMD_ECHO_OFF; //PP added on 29-07-24
                        }
                        flushRxBuffer(LTE_UART);
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
#endif  //USE_NETCLOSE

        default:
        {

        }
        break;
    }

    return sts;
}


// //modded by poorva for telecom_iot to avoid infinite loops. added protection conditions to break / return out of here.
// bool read_ip_port(char *tmpstr) 
// {
//     //int i=0,j=0,cnt=0;
//     unsigned int i=0,j=0,cnt=0;
//     char temp_buff[6];
//     char ip_addr[20];
//     char temp_port[6];
    
//     bool retVal = false;
    
// 	// cloud_config_t data;
// 	// cloud_config_data(&data);
// 	// memset(&data,0,sizeof(cloud_config_t));
//     memset(ip_addr,0,sizeof(ip_addr));
//     memset(temp_buff,0,sizeof(temp_buff));
//     memset(temp_port,0,sizeof(temp_port));

//     //gprs_read_config_data();

//     while(tmpstr[i] != '"')
//     {
//         ip_addr[i]=(tmpstr[i] & 0xFF);
//         i++;
//         if(i > IP_ADDR_LEN)
//         {
// 	        retVal = false;
// 		    break;
//         }
//     }
// #ifdef DEBUG_QCIPOPEN_RESP_DECODE
//     UWriteString((char *)"\nIP:",DBG_UART);
//     UWriteBytes((unsigned char *)ip_addr,i,DBG_UART);
//     UWriteData(',',DBG_UART);
//     UWriteString((char *)cloud_config.ip_addr,DBG_UART);
// #endif
//     i = i+2;

//     while(tmpstr[i] != ',')
//     {
//         temp_buff[j] = (tmpstr[i] & 0xFF);
//         j++;
//         i++;
//         if(j > PORT_LEN)
//         {
// 	        break;
//         }
//     }
    
// #ifdef DEBUG_GPRS_OVERALL
// UWriteData('G', DBG_UART);
// #endif
//     //port = atoi(temp_buff);
//     ltoa(cloud_config.port_num,temp_port,10);
//     // my_ltoa(cloud_config.port_num,temp_port,10); //PP commented on 26-04-24: maybe the built-in ltoa() works in this TI controller?
// 	// my_ltoa(data.port_num,temp_port,10);
// #ifdef DEBUG_GPRS_OVERALL
// UWriteData('H', DBG_UART);
// #endif
// #ifdef DEBUG_QCIPOPEN_RESP_DECODE
//     //UWriteString(DBG_UART,"\nIP:");
//     //UWriteBytes(DBG_UART,ip_addr,i);
//     UWriteString((char *)"\nPORT:",DBG_UART);
//     UWriteBytes((unsigned char *)temp_buff,j,DBG_UART);
//     UWriteData(',',DBG_UART);
//     UWriteString(temp_port,DBG_UART);
// #endif

//     //if((!(strcmp(ip_addr,(const char*)TCP_IP))) && (!(strcmp(temp_buff,temp_port))))
//     // if((!(strcmp(ip_addr,(const char *)data.ip_addr))) && (!(strcmp(temp_buff,temp_port))))
//     if((!(strcmp(ip_addr,(const char *)cloud_config.ip_addr))) && (!(strcmp(temp_buff,temp_port))))
//     {
// #ifdef DEBUG_QCIPOPEN_RESP_DECODE
//         UWriteString((char*)"\nIPK",DBG_UART);
// #endif
//         retVal = true;
//     }
//     else
//     {
// #ifdef DEBUG_QCIPOPEN_RESP_DECODE
//         UWriteString((char*)"\nIPF",DBG_UART);
// #endif
//         retVal = false;
//     }
//     return retVal;
// }   

//PP added on 30-07-24 (Copied from EVSE 3.12)
int read_ip_port(char *tmpstr,int length)
{
    int i=0,j=0;
    char temp_buff[6];
    char ip_addr[20];
    char temp_port[6];

    memset(ip_addr,0,sizeof(ip_addr));
    memset(temp_buff,0,sizeof(temp_buff));
    memset(temp_port,0,sizeof(temp_port));

    //gprs_read_config_data();

    while(tmpstr[i] != '"')
    {
        if(i > length)
        {
            break;
        }
        ip_addr[i]=tmpstr[i];
        i++;
    }
#ifdef DEBUG_QCIPOPEN_RESP_DECODE
    vUART_SendStr(UART_PC,"\nIP:");
    vUART_SendBytes(UART_PC,(const uint8_t *)ip_addr,i);
    vUART_SendStr(UART_PC,"\ncc_ip:");
    vUART_SendStr(UART_PC,(const uint8_t *)cloud_config.ip_addr);

#endif
    i = i+2;

    while(tmpstr[i] != ',')
    {
        if(i >= length)
        {
            break;
        }
        temp_buff[j] = tmpstr[i];
        j++;
        i++;
    }
    //port = atoi(temp_buff);
    ltoa(cloud_config.port_num,temp_port,10);

#ifdef DEBUG_QCIPOPEN_RESP_DECODE
    //vUART_SendStr(UART_PC,"\nIP:");
    //vUART_SendBytes(UART_PC,(const uint8_t *)ip_addr,i);
    vUART_SendStr(UART_PC,"\nPORT:");
    vUART_SendBytes(UART_PC,(const uint8_t *)temp_buff,j);
    vUART_SendStr(UART_PC,"\ncc_port:");
    vUART_SendStr(UART_PC,(const uint8_t *)temp_port);
#endif

    if(!(strcmp(ip_addr,(const char*)cloud_config.ip_addr)) && (!(strcmp(temp_buff,temp_port))))
    {
        return 1;
    }
    else
    {
        return 0;
    }
    //gprs_write_config_data();
}

//modded by poorva for telecom_iot to avoid infinite loops. added protection conditions to break / return out of here.
char match_cpsi_data(char *tmpstr)
{
    char system_mode[10],operation_mode[10];
    //int i=0,j=0;
    unsigned int i=0,j=0;

    memset(system_mode,0,sizeof(system_mode));
    memset(operation_mode,0,sizeof(operation_mode));

    for(;tmpstr[i] != ',';)
	{
		if(i >= strlen((const char*)"WCDMA"))
		{
			return FALSE;
		}
		system_mode[i] = tmpstr[i];
		i++;
	}
    i++;
    for(; tmpstr[i] != ',';)
	{
		if(j >= strlen((const char*)"Online"))
		{
			return FALSE;
		}
		operation_mode[j] = tmpstr[i];
		i++;
		j++;
	}

    if(((!strcmp(system_mode,"LTE")) || (!strcmp(system_mode,"GSM")) || (!strcmp(system_mode,"WCDMA"))) && (!strcmp(operation_mode,"Online")))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

char pong_received(char *tmpstr)
{
#ifdef DEBUG_GPRS_PING
        vUART_SendStr(UART_PC,"\npong_recv:");
        vUART_SendStr(UART_PC, (uint8_t*)tmpstr);
#endif 
    //int i=0,j=0;
    //char temp_buff[4],sts;
    char sts;
    //char data[GPRS_RX_BUFFER_MAX];
    //tcp_state = GPRS_WEBSOCKET_PING_SEND_CMD;
//    while(tmpstr[i] != '\r')    //to find length {'+','I','P','D','2','\r','\n'};
//    {
//        temp_buff[j] = tmpstr[i];
//        i++;
//        j++;
//    }
//
//    gprs.data_length = atoi(temp_buff);
    delete_SubStr(tmpstr, (char *)"\r",'\n');   //PP 26-04-24: delete_SubStr has potential infinite loop. Need to mod later
//#ifdef DEBUG_GPRS_PING
//            vUART_SendStr(UART_PC,"\ndelet_tmpstr:");
//            vUART_SendStr(UART_PC,(const uint8_t *)tmpstr);
//
//#endif

//    if(tmpstr[i+1] == '\n')     //to find data {0x8A,0x00,'\r','\n'}
//    {
//        j=0;
//        i++;
//        //while(tmpstr[i]!='\r')
//        //{
//            data[j++] = tmpstr[++i];

            //char sts=0;
            if((tmpstr[0] & 0xFF) == 0x8A)
            {
                sts = TRUE;
            }
            else
            {
                sts = FALSE;
            }
        //}

    return sts;
}

char valid_code(char *tmpstr)
{
    char temp_buff[5];
    int i=0;
    memset(temp_buff,0,sizeof(temp_buff));
    // while(tmpstr[i] != 0x20)
    while((tmpstr[i] & 0xFF) != 0x20)   //PP 26-04-24: potential infinite loop? need to mod later
    {
        temp_buff[i] = tmpstr[i];
        i++;
    }
    if(!(strcmp(temp_buff,"101")))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// char respos_recvd(char *buff)
// {
//     //int i = 0;
//     unsigned int i = 0;
// 
//     while(buff[i] != '\n')  //might become an infinite loop, protection condition needed.
//     {
//         i++;
//     }
//     i++;
//     if((buff[i] & 0xFF)== 0x81)
//     {
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }

char respos_recvd(char *buff,int len)   //PP added on 30-07-24 (copied from EVSE 3.12 FW version)
{
    int i = 0;

    while(buff[i] != '\n')
    {
        i++;
        if(i > len)
        {
            break;
        }
    }
    i++;
    if(buff[i] == 0x81)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#endif  //ifNOTdef ETHERNET_EN

void generateMaskKey(uint8_t maskKey[4])
{
    int i=0;
    srand(time(NULL));

    for ( i = 0; i < 4; i++)
    {
        maskKey[i] = rand() % 256;
    }
}

int get_rx_data(char *copy_here)
{
    int retval = 0;
    //memset(gprs_rx_buff.buffer,0,sizeof(gprs_rx_buff.buffer));
    //communication(GPRS_UART, FALSE);
    if (copy_here)
    {
        //PP commented this on 26-04-24 coz this if condition doesn't seem relevant to telecom iot as of now.
        /* if(getEVSE_main_state() == EVSE_CONFIG) //|| (get_frmwr_update_state() == HEX_FILE_UPDATE_PRG))
        {
            memcpy (copy_here, (const void *)gprs_temp_rx_buff.buff, gprs_temp_rx_buff.index);
            copy_here[gprs_temp_rx_buff.index] = '\0';

            retval = gprs_temp_rx_buff.index;
            gprs_temp_rx_buff.index = 0;
        }
        else */
        {
            memcpy (copy_here, (const void *)gprs_rx_buff.buffer, gprs_rx_buff.index);
           // gprs.gprs_rx_buff_len = gprs_rx_buff.index;

//#ifdef DEBUG_RANDOM_
//            vUART_SendStr(UART_PC,"\ngprs_rx_buff:");
//           vUART_SendStr(UART_PC,(const uint8_t *)gprs_rx_buff.buffer);
//           vUART_SendStr(UART_PC,"\ngprs_rx.index:");
//           vUART_SendInt(UART_PC,gprs_rx_buff.index);
//#endif
            copy_here[gprs_rx_buff.index] = '\0';

            retval = gprs_rx_buff.index;
            gprs_rx_buff.index = 0; 
        }
    }
    return retval;
}

char check_string(const char *str, char *copy_here, int* numbytes)
{
    char retval = GPRS_NO_NEW_MSG;
    int i, j,ip_str_len;
    char *lock_ptr = NULL;

    ip_str_len = strlen(str);
    if (!copy_here)
    {
        retval = IS_FAILURE;
        return retval;
    }

    //PP commented this on 26-04-24 coz this if condition doesn't seem relevant to telecom iot as of now.
    /* if((getEVSE_main_state() == EVSE_CONFIG))
    {
        lock_ptr = (char*)&gprs_temp_rx_buff.locked;
    }
    else */
    {
        lock_ptr = (char*)&gprs_rx_buff.locked;
    }
    if(*lock_ptr == LOCKED )
    {
//#ifdef DEBUG_RANDOM_
//        //vUART_SendStr(UART_PC,"\nin_LOCKED");
//#endif

        *lock_ptr = UNLOCKED;
        *numbytes = get_rx_data(copy_here);
        
        if((str[0]) != '\0')
        {
            if(*numbytes > 0)
            {
                for (i=0; i < *numbytes; i++)
                {
                    if (!memcmp(&copy_here[i], str, ip_str_len)) break;
                }
                if(i >= *numbytes)
                {
                    return retval = GPRS_MATCH_FAIL;
                }
                retval = GPRS_MATCH_OK;
                for(j = 0; j < (*numbytes - i - ip_str_len); j++)
                {
                    copy_here[j] = copy_here[j + i + ip_str_len];
                }
                *numbytes = j;
                copy_here[j] = '\0';
            }
        }
        else
        {
            retval = GPRS_MATCH_OK;
        }
    }
    return (retval);
}


char check_string_nobuf(const char *str)
{
    int len = 0;
    char tmpstr[GPRS_RX_BUFFER_MAX];

#ifdef DEBUG_WEBSOCKET_CONN
    // vUART_SendStr(UART_PC,"\ncsn");
#endif

    return check_string(str, tmpstr, &len);

}

// #ifndef ETHERNET_EN


void set_upload_data(upload_data_t upload_data)
{
    gprs.upload_data = upload_data;
}
upload_data_t get_upload_data(void)
{
    return gprs.upload_data;
}

void set_webconn_sts(uint8_t sts)
{
    gprs.websocket_sts = sts;
}

uint8_t get_webconn_sts(void)
{
    return gprs.websocket_sts;
}

void set_gprs_connct_sts(uint8_t sts)
{
    gprs.connect_sts = sts;
}
uint8_t get_gprs_connct_sts(void)
{
    return gprs.connect_sts;
}

// void setNWstatus(gprs_status_t sts)
// {
//     gprs.NW_connSts = sts;
// }
//
// gprs_status_t getNW_status(void)
// {
//     return gprs.NW_connSts;
// }
//
// void setServerStatus(gprs_status_t sts)
// {
//     gprs.serverConn_Sts = sts;
// }
//
// gprs_status_t getServerStatus(void)
// {
//     return gprs.serverConn_Sts;
// }

// #endif  // ifNOTdef ETHERNET_EN





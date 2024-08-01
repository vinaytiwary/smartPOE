/*
 * Telecom_server_query.cpp
 *
 *  Created on: May 13, 2024
 *      Author: ADMIN
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "Telecom_server_query.h"
#include "HW_pins.h"
#include "_debug.h"
#include "UartCore.h"
#include "_common.h"
#include "E2P.h"
#include "main.h"
#include "gprs.h"
#include "Web_Comm.h"
#ifdef FLASH_EN
#include "flash_logger.h"
#include "flashCore.h"
#endif //FLASH_EN

#ifdef ETHERNET_EN
#include "Telecom_Ethernet.h"

extern volatile ethernet_rx_buff_t ethernet_rx_buff;//,ethernet_resp_buff;
extern volatile ethernet_tx_buff_t ethernet_tx_buff;

extern Telecom_Ethernet_t Telecom_Ethernet;
#endif  //ETHERNET_EN

extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

extern gprs_t gprs;

extern e2p_device_info_t e2p_device_info;
extern cloud_config_t cloud_config;

extern e2p_router_config_t e2p_router_config;
extern ram_data_t ram_data;
extern Alarms_t Alarms;

#ifdef FLASH_EN
extern FL_log_data_t FLR_log_data;
#endif  //FLASH_EN

#ifndef ETHERNET_EN
#ifdef ENABLE_GPS
extern conn_state_t conn_state;
#endif  //ENABLE_GPS
#endif  //ETHERNET_EN

Telecom_server_query_t Telecom_server_query;
Network_status_t network_status;

uint8_t dummy_json_string[390];
uint8_t server_response_str[27] = "{ \"response\": \"accepted\" }";

// uint8_t JSON_Tx_Buff[513];
// uint8_t JSON_Rx_Buff[513];

char* Query_decode(char *Query_String ,const char *Query_data, char *destination, int max_len)
{
	char* found_key = 0;
    char *ret_addr = 0;
    int i = 0, j = 0;

    // char destination_data[15];
    char destination_data[22];
	memset(destination_data, 0, sizeof(destination_data));

    i = strlen(Query_data);

#ifdef DEBUG_QUERY_DECODE
    vUART_SendStr(DEBUG_UART_BASE, "\nQDlen=");
    vUART_SendInt(DEBUG_UART_BASE, i);
    vUART_SendStr(DEBUG_UART_BASE, "\nQS=");
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)Query_String);
#endif

    // found_key = strstr((char*)Query_String,Query_data);
#ifdef ETHERNET_EN
    found_key = my_strstr((const char *)Query_String, (const char *)Query_data,Telecom_Ethernet.ether_rx_buff_len);   //pending
    // found_key = strstr((char *)Query_String, (char *)Query_data);
#else
    found_key = strstr((char *)Query_String, (char *)Query_data);
#endif

    if(found_key)
    {
#ifdef DEBUG_QUERY_DECODE
        vUART_SendStr(DEBUG_UART_BASE, "\n1KF=");
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)found_key);
        vUART_SendStr(DEBUG_UART_BASE, "\n2KF=");
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)&found_key[i]);
#endif


        if(found_key[i] == ':')
        {
#ifdef DEBUG_QUERY_DECODE
            // printf("\n3found_key");
            // printf("\nf[%d]=%c",i,found_key[i]);
            vUART_SendStr(DEBUG_UART_BASE, "\n3KF1,i=");
            vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
#endif

            i++;

#ifdef DEBUG_QUERY_DECODE
            // printf("\nf[%d]=%c",i,found_key[i]);
            vUART_SendStr(DEBUG_UART_BASE, "\n3KF2,i=");
            vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
            vUART_SendChr(DEBUG_UART_BASE, ',');
            vUART_SendInt(DEBUG_UART_BASE, i);
#endif
            
            if((found_key[i]=='"')||(found_key[i]=='['))
            {
#ifdef DEBUG_QUERY_DECODE
                // printf("\n4found_key");
                // printf("\nf[%d]=%c",i,found_key[i]);
                vUART_SendStr(DEBUG_UART_BASE, "\n4KF,i=");
                vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
#endif
                if((found_key[i]=='['))
			    {
                    i++;
#ifdef DEBUG_QUERY_DECODE
                    // printf("\n5found_key");
                    // printf("\nf[%d]=%c",i,found_key[i]);
                    vUART_SendStr(DEBUG_UART_BASE, "\n5KF,i=");
                    vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendInt(DEBUG_UART_BASE, i);
#endif

                    if(found_key[i] == ']')
                    {
#ifdef DEBUG_QUERY_DECODE
                        // printf("\nfound [], returnig 0");
                        vUART_SendStr(DEBUG_UART_BASE, "\nerr[]");
#endif
                        return 0;
                    }
			    }

                i++;

#ifdef DEBUG_QUERY_DECODE
                // printf("\n6found_key");
                // printf("\nf[%d]=%c",i,found_key[i]);
                vUART_SendStr(DEBUG_UART_BASE, "\n6KF,i=");
                vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
#endif
                for(;(found_key[i] != '"');)
                {
                    if(j > max_len)
                    {
#ifdef DEBUG_QUERY_DECODE
                        // printf("\n1cantFindEnd %d,%d",j, max_len);
                        vUART_SendStr(DEBUG_UART_BASE, "\n1NoEnd");
#endif                     
                        return 0;
                    }
                    // destination_data[j++] = found_key[i++];
                    destination_data[j] = found_key[i];

#ifdef DEBUG_QUERY_DECODE
                    // printf("\n1destination_data[%d]=%c,found_key[%d]=%c",j,destination_data[j],i,found_key[i]);
                    vUART_SendStr(DEBUG_UART_BASE, "\n1dd[j],j,kf[i],i");
                    vUART_SendChr(DEBUG_UART_BASE, (uint8_t)destination_data[j]);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendInt(DEBUG_UART_BASE, j);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendInt(DEBUG_UART_BASE, i);
#endif                   
                    j++;
                    i++;
                }
            }
            else
            {
#ifdef DEBUG_QUERY_DECODE
                // printf("\n7found_key");
                // printf("\nf[%d]=%c",i,found_key[i]);
                vUART_SendStr(DEBUG_UART_BASE, "\n7KF,i=");
                vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, i);
#endif

                // for(;((found_key[i] != ',')||(found_key[i] != '}'));)
                // for(;(!((found_key[i] == ',')||(found_key[i] == '}')));)
                for(;((found_key[i] != ',')&&(found_key[i] != '}'));)
                {
                    if(j > max_len)
                    {
#ifdef DEBUG_QUERY_DECODE
                        // printf("\n2cantFindEnd %d,%d",j, max_len);
                        vUART_SendStr(DEBUG_UART_BASE, "\n2NoEnd");
#endif
                        return 0;
                    }
#ifdef DEBUG_QUERY_DECODE
                    // printf("\n2destination_data[%d]=%c,found_key[%d]=%c",j,destination_data[j],i,found_key[i]);
                    vUART_SendStr(DEBUG_UART_BASE, "\n2dd[j],j,kf[i],i");
                    vUART_SendChr(DEBUG_UART_BASE, (uint8_t)destination_data[j]);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendInt(DEBUG_UART_BASE, j);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendChr(DEBUG_UART_BASE, (uint8_t)found_key[i]);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendInt(DEBUG_UART_BASE, i);
#endif
                    destination_data[j++] = found_key[i++];
                }
            }

#ifdef DEBUG_QUERY_DECODE
            vUART_SendStr(DEBUG_UART_BASE, "\nj=");
            vUART_SendInt(DEBUG_UART_BASE, j);
            vUART_SendStr(DEBUG_UART_BASE, "\n1dest=");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)destination_data);
#endif

            // memcpy((char *)destination,destination_data,j);
            memcpy((char *)destination,destination_data,strlen((const char*)destination_data));

            // printf("\ndest_data = %s\n",destination_data);
            // printBytes((unsigned char*)destination_data,j);
#ifdef DEBUG_QUERY_DECODE
            // printf("\ndest = %s\n",destination);
            // printBytes((unsigned char*)destination,j);
            vUART_SendStr(DEBUG_UART_BASE, "\n2dest=");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)destination);
#endif


            // printf("\n&found_key[%d]=%s\n",i,&found_key[i]);
            // printBytes((unsigned char*)&found_key[i],strlen(&found_key[i]));

            ret_addr = &found_key[i];
            
            // printf("\nret_addr=%s\n",ret_addr);
            // printBytes((unsigned char*)ret_addr,strlen(ret_addr));
            
            // return ret_addr;
        }
        else
        {
#ifdef DEBUG_QUERY_DECODE
            // printf("\nno':'found");
            vUART_SendStr(DEBUG_UART_BASE, "\nNo:F");
#endif
        }
    }
    else
    {
#ifdef DEBUG_QUERY_DECODE
        // printf("\n1no_key_found");
        vUART_SendStr(DEBUG_UART_BASE, "\nKNF");
#endif
        
        return 0;
    }

    return ret_addr;
}

// bool server_query(void)
// {
//     char decoded_query[15];
//     char* found_brace = 0;
//     char* ret_addr;
//     bool retVal = false;
//     // char temp_buff[3];
//     uint8_t ODU_temp_cfg;
// 
//     memset(decoded_query, 0, sizeof(decoded_query));
//     // memset(temp_buff, 0, sizeof(temp_buff));
// 
// #ifndef ETHERNET_EN
//     // found_brace = strstr((char *)gprs_rx_buff.buffer,"{");
//     found_brace = my_strstr((const char*) gprs_rx_buff.buffer, "{",
//                                       gprs_rx_buff.index);
// #else
//     // found_brace = strstr((char *)ethernet_rx_buff.buffer,"{");
//     found_brace = my_strstr((const char*) ethernet_rx_buff.buffer, "{",
//                                       ethernet_rx_buff.index);
// #endif  //ETHERNET_EN
// 
//     if(found_brace)
//     {
// #ifdef DEBUG_SERVER_QUERY
//         // printf("\nfound_brace!");
//         vUART_SendStr(DEBUG_UART_BASE, "\nFB");
// #endif
//         ret_addr = Query_decode(found_brace,"\"request\"",decoded_query, strlen((const char*)"\"restart\"}"));
//         if(ret_addr)
//         {
// #ifdef DEBUG_SERVER_QUERY
//             vUART_SendStr(DEBUG_UART_BASE, "\n1Q=");
//             vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)decoded_query);
// #endif
// 
//             if((!memcmp(decoded_query, "restart", strlen((const char*)"restart"))))
//             {
//                 if(getServerReqType() == NO_REQ)
//                 {
//                     set_pending_request(true);
//                     setServerReqType(RESTART);
//                     retVal = true;
//                 }
//                 else
//                 {
// 
//                 }
//                
//             }
//         }
//         else
//         {
// #ifdef DEBUG_SERVER_QUERY
//             vUART_SendStr(DEBUG_UART_BASE, "\nnotRSTcmd");
// #endif
//             memset(decoded_query, 0, sizeof(decoded_query));
//             ret_addr = NULL;
//             ret_addr = Query_decode(found_brace,"\"odu_voltage\"",decoded_query, strlen((const char*)"\"odu_voltage\"}"));
// 
// #ifdef DEBUG_SERVER_QUERY
//             vUART_SendStr(DEBUG_UART_BASE, "\n2Q=");
//             vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)decoded_query);
// #endif
//             if(ret_addr)
//             {
// #ifndef ETHERNET_EN
//                 flushRxBuffer(LTE_UART);
// #else
//                 flush_ether_rx_buff();
// #endif  //ETHERNET_EN
//                 ODU_temp_cfg = (strlen((const char*)decoded_query) == 2)? atoi(decoded_query): 0;
// 
// #ifdef DEBUG_SERVER_QUERY
//                 vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nddlen:");
//                 vUART_SendInt(DEBUG_UART_BASE, strlen((const char*)decoded_query));
//                 vUART_SendChr(DEBUG_UART_BASE, ',');
//                 vUART_SendInt(DEBUG_UART_BASE, ODU_temp_cfg);
// #endif                
// 
//                 if((ODU_temp_cfg == 0)||(ODU_temp_cfg > 48)||(ODU_temp_cfg < 12)||((ODU_temp_cfg != 30) && ((ODU_temp_cfg % 12) != 0)))
//                 {
// #ifdef DEBUG_SERVER_QUERY
//                     vUART_SendStr(DEBUG_UART_BASE, "\noutrange:");
//                     vUART_SendInt(DEBUG_UART_BASE, ODU_temp_cfg);
// #endif
//                 }
//                 else
//                 {
//                     ODU_temp_cfg = (ODU_temp_cfg == 30)? ODU_temp_cfg/10 : ODU_temp_cfg/12;
// #ifdef DEBUG_SERVER_QUERY
//                     vUART_SendStr(DEBUG_UART_BASE, "\nrangeK:");
//                     vUART_SendInt(DEBUG_UART_BASE, ODU_temp_cfg);
// #endif
//                     e2p_router_config.router2 = (voltage_mode_t)ODU_temp_cfg;
//                     e2p_write_voltage_config();
//                     // SetRouterMode(e2p_router_config.router2, ROUTER_2);  //PP 13-05-24: For later when we have the hardware.
//                     ram_data.supply_mode_R2 = (e2p_router_config.router2 == MODE_36V)? (e2p_router_config.router2 * 10) : (e2p_router_config.router2 * 12); //PP 14-03-24: this is to be uncommented if ckt successfully creates 30V
//                   
//                     if(getServerReqType() == NO_REQ)
//                     {
//                         set_pending_request(true);
//                         setServerReqType(ODU_VOLTAGE_UPDATE);
//                         retVal = true;
//                     }
//                     else
//                     {
// 
//                     }
//                 }
//             }
//             else
//             {
// #ifdef DEBUG_SERVER_QUERY
//                 vUART_SendStr(DEBUG_UART_BASE,"\nnotODUcmd");
// 
// #endif           
//                 setServerReqType(NO_REQ);
//             }
//         }
//     }
//     else
//     {
// #ifdef DEBUG_SERVER_QUERY
//         vUART_SendStr(DEBUG_UART_BASE,"\nNFB");
//         // printf("\nno_brace_found");
// #endif
//     }
//     return retVal;
// }

bool server_query(void)
{
    // char decoded_query[15];
    char decoded_query[22];
    char* found_brace = 0;
    char* ret_addr = NULL;
    bool retVal = false;
    // char temp_buff[3];
    uint8_t ODU_temp_cfg;

    memset(decoded_query, 0, sizeof(decoded_query));
    // memset(temp_buff, 0, sizeof(temp_buff));

#ifndef ETHERNET_EN
    // found_brace = strstr((char *)gprs_rx_buff.buffer,"{");
    found_brace = my_strstr((const char*) gprs_rx_buff.buffer, "{",
                                      gprs_rx_buff.index);
#else
    // found_brace = strstr((char *)ethernet_rx_buff.buffer,"{");
    found_brace = my_strstr((const char*) ethernet_rx_buff.buffer, "{",
                                      ethernet_rx_buff.index);
#endif  //ETHERNET_EN

    if(found_brace)
    {
#ifdef DEBUG_SERVER_QUERY
        // printf("\nfound_brace!");
        vUART_SendStr(DEBUG_UART_BASE, "\nFB");
#endif
        // ret_addr = Query_decode(found_brace,"\"request\"",decoded_query, strlen((const char*)"\"restart\"}"));
        // if(ret_addr)
        if(ret_addr = Query_decode(found_brace,"\"request\"",decoded_query, strlen((const char*)"\"restart\"}")))
        {
#ifdef DEBUG_SERVER_QUERY
            vUART_SendStr(DEBUG_UART_BASE, "\n1Q=");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)decoded_query);
#endif

            if((!memcmp(decoded_query, "restart", strlen((const char*)"restart"))))
            {
                if(getServerReqType() == NO_REQ)
                {
                    set_pending_request(true);
                    setServerReqType(RESTART);
                    retVal = true;
                }
                else
                {

                }
                
            }
        }
#if 1
        // else
        // else if(ret_addr = Query_decode(found_brace,"\"odu_voltage\"",decoded_query, strlen((const char*)"\"odu_voltage\"}")))
        else if(ret_addr = Query_decode(found_brace,"\"odu_voltage\"",decoded_query, strlen((const char*)"\"odu_voltage\"")))
        {
#ifdef DEBUG_SERVER_QUERY
            vUART_SendStr(DEBUG_UART_BASE, "\nnotRSTcmd");
#endif
            // memset(decoded_query, 0, sizeof(decoded_query));
            // ret_addr = NULL;
            // ret_addr = Query_decode(found_brace,"\"odu_voltage\"",decoded_query, strlen((const char*)"\"odu_voltage\"}"));

#ifdef DEBUG_SERVER_QUERY
            vUART_SendStr(DEBUG_UART_BASE, "\n2Q=");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)decoded_query);
#endif
            // if(ret_addr)
            // {
#ifndef ETHERNET_EN
                flushRxBuffer(LTE_UART);
#else
                flush_ether_rx_buff();
#endif  //ETHERNET_EN
                ODU_temp_cfg = (strlen((const char*)decoded_query) == 2)? atoi(decoded_query): 0;

#ifdef DEBUG_SERVER_QUERY
                vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nddlen:");
                vUART_SendInt(DEBUG_UART_BASE, strlen((const char*)decoded_query));
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, ODU_temp_cfg);
#endif                

                if((ODU_temp_cfg == 0)||(ODU_temp_cfg > 48)||(ODU_temp_cfg < 12)||((ODU_temp_cfg != 30) && ((ODU_temp_cfg % 12) != 0)))
                {
#ifdef DEBUG_SERVER_QUERY
                    vUART_SendStr(DEBUG_UART_BASE, "\noutrange:");
                    vUART_SendInt(DEBUG_UART_BASE, ODU_temp_cfg);
#endif
                }
                else
                {
                    ODU_temp_cfg = (ODU_temp_cfg == 30)? ODU_temp_cfg/10 : ODU_temp_cfg/12;
#ifdef DEBUG_SERVER_QUERY
                    vUART_SendStr(DEBUG_UART_BASE, "\nrangeK:");
                    vUART_SendInt(DEBUG_UART_BASE, ODU_temp_cfg);
#endif
                    
                    if(getServerReqType() == NO_REQ)
                    {
#if 0
                        e2p_router_config.router2 = (voltage_mode_t)ODU_temp_cfg;
                        e2p_write_voltage_config();
                        // SetRouterMode(e2p_router_config.router2, ROUTER_2);  //PP 13-05-24: For later when we have the hardware.
                        ram_data.supply_mode_R2 = (e2p_router_config.router2 == MODE_36V)? (e2p_router_config.router2 * 10) : (e2p_router_config.router2 * 12); //PP 14-03-24: this is to be uncommented if ckt successfully creates 30V
#endif  //if 0                        
                        set_pending_request(true);
                        setServerReqType(ODU_VOLTAGE_UPDATE);
                        retVal = true;
                    }
                    else
                    {

                    }
                }
            // }
//             else
//             {
// #ifdef DEBUG_SERVER_QUERY
//                 vUART_SendStr(DEBUG_UART_BASE,"\nnotODUcmd");
//
// #endif           
//                 setServerReqType(NO_REQ);
//             }
        }
#endif  //if 0
        // else if(ret_addr = Query_decode(found_brace, "\"server_url\"", decoded_query, strlen((const char*)"\"server_url\"")))
        else if(ret_addr = Query_decode(found_brace, "\"server_url\"", decoded_query, 22))
        {
            unsigned int i = 0;
            // char decoded_url[23];   //PP 18-05-24: 15 Bytes max ip address, 1B for ':', 5B for port num, 1B for the ending double quote '"', 1B for ending null = 23 Bytes
            char decoded_url[33];
            memset(decoded_url, 0, sizeof(decoded_url));
#ifdef DEBUG_SERVER_QUERY
            vUART_SendStr(DEBUG_UART_BASE,"\nnotODUcmd");
            vUART_SendStr(DEBUG_UART_BASE, "\n3Q=");
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)decoded_query);
#endif

#ifndef ETHERNET_EN
                flushRxBuffer(LTE_UART);
#else
                flush_ether_rx_buff();
#endif  //ETHERNET_EN
            // // for(int i = 0; decoded_query[i] != '"'; i++)
            // // for(; decoded_query[i] != '"';)
            // for(; decoded_query[i] != '\0';)
            // {
            //     if(i > (sizeof(decoded_url) - 1))
            //     {
            //         memset(decoded_url, 0, sizeof(decoded_url));
            //         break;
            //     }
            //     decoded_url[i] = decoded_url[i];
            //     i++;
            // }

            if(getServerReqType() == NO_REQ)
            {
                memcpy(decoded_url, decoded_query, strlen((const char*)decoded_query));
                memcpy(&decoded_url[strlen((const char*)&decoded_url)], "/ws/?token=", strlen((const char*)"/ws/?token="));
#ifdef DEBUG_SERVER_QUERY
                vUART_SendStr(DEBUG_UART_BASE, "\nDSU=");
                vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)decoded_url);
#endif

                set_url(decoded_url);
                cloud_config_data();
                set_pending_request(true);
                setServerReqType(SERVER_URL_UPDATE);
                retVal = true;
            }
            else
            {

            }
        }
        else if(ret_addr == NULL)
        {
#ifdef DEBUG_SERVER_QUERY
            // vUART_SendStr(DEBUG_UART_BASE,"\nnotODUcmd");
            vUART_SendStr(DEBUG_UART_BASE,"\nnotSUPcmd");
#endif           
            // commenting resetting to NO_REQ here coz it should be resetted when the request's sesponse is successfully sent. 
            // We check for REQType in successfull tcp_send() so that if it's server url update request, we can reconnect to that 
            // instead of going to SESSION_IDLE. But when we get "{ "response": "accepted" } by TELECOM444" server echo for 
            // accepting the server url update request, this condition "notSUPcmd" executed causing setServerReqType(NO_REQ) 
            // and that causes us to never reconnect to the new server for a while. For the rest of the requests it'd be fine 
            // but as a standard we should only reset it in tcp_send() successfull or 30s periodically.

            // setServerReqType(NO_REQ);        
        }
    }
    else
    {
#ifdef DEBUG_SERVER_QUERY
        vUART_SendStr(DEBUG_UART_BASE,"\nNFB");
        // printf("\nno_brace_found");
#endif
    }
    return retVal;
}


unsigned int prepare_JSON_pckt(void)
{
    char earth_temp[7];
    unsigned long epoch_time = 0;
	memset(earth_temp, 0, sizeof(earth_temp));

    unsigned int retVal = 0;

    epoch_time = convertToEpochTime(&ram_data.ram_time);
    // epoch_time = asUnixTime(ram_data.ram_time.year, ram_data.ram_time.month, ram_data.ram_time.date, ram_data.ram_time.hour, ram_data.ram_time.min, ram_data.ram_time.sec);

    if(ram_data.ram_EXTI_cnt.earth_cnt)
	{
		memcpy(earth_temp, "true", strlen((const char*)"true"));
	}
	else
	{
		memcpy(earth_temp, "false", strlen((const char*)"false"));
	}

#ifdef DEBUG_JSON_PKT_PREP
    vUART_SendStr(DEBUG_UART_BASE,"\nram_data:");
    vUART_SendStr(DEBUG_UART_BASE,"\nrfc=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_EXTI_cnt.freq_cnt);
    vUART_SendStr(DEBUG_UART_BASE,"\tref=");
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)earth_temp);
    vUART_SendStr(DEBUG_UART_BASE,"\nrACP=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\nrRC=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router1);
    vUART_SendStr(DEBUG_UART_BASE,"\tOC=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router2);
    vUART_SendStr(DEBUG_UART_BASE,"\nrACN=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.NE_AC_Voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\tOV=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendStr(DEBUG_UART_BASE,"\nChgV=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Charger_voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\tBATTV=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\net=");
    vUART_SendInt(DEBUG_UART_BASE,epoch_time);
    vUART_SendStr(DEBUG_UART_BASE,"\nrLa=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.Latitude);
    vUART_SendStr(DEBUG_UART_BASE,"\trLo=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.Longitude);
    vUART_SendStr(DEBUG_UART_BASE,"\nRS=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.supply_mode_R1);
    vUART_SendStr(DEBUG_UART_BASE,"\tOS=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.supply_mode_R2);
    vUART_SendStr(DEBUG_UART_BASE,"\nrA=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_alarms);
#endif

    memset(dummy_json_string, 0, sizeof(dummy_json_string));
	// my_sprintf((char*)dummy_json_string, 26,"{\"telecom\":[{\"deviceId\":\"%s\",\"timestamp\":\"%01d\",\"batteryVoltage\":%01d.%03d,\"chargerVoltage\":%01d.%03d,\"router1Voltage\":%01d.%03d,\"router1Current\":%01d.%03d,\"router2Voltage\":%01d.%03d,\"router2Current\":%01d.%03d,\"inputVoltage\":%01d.%03d,\"frequency\":%01d.%02d,\"earthDetected\":%s,\"router1SupplyMode\":%01d,\"router2SupplyMode\":%01d,\"longitude\":%01d.%06d,\"latitude\":%01d.%06d,\"alarms\":%01d}]}",
	// 		e2p_device_info.device_id,epoch_time,ram_data.ram_ADC.DC_Battery_voltage/1000, ram_data.ram_ADC.DC_Battery_voltage%1000,ram_data.ram_ADC.DC_Charger_voltage/1000,ram_data.ram_ADC.DC_Charger_voltage%1000,ram_data.ram_ADC.DC_Voltage_router1/1000,ram_data.ram_ADC.DC_Voltage_router1%1000,ram_data.ram_ADC.DC_current_router1/1000,ram_data.ram_ADC.DC_current_router1%1000,ram_data.ram_ADC.DC_Voltage_router2/1000,ram_data.ram_ADC.DC_Voltage_router2%1000,ram_data.ram_ADC.DC_current_router2/1000,ram_data.ram_ADC.DC_current_router2%1000,ram_data.ram_ADC.PN_AC_Voltage/1000,ram_data.ram_ADC.PN_AC_Voltage%1000,(ram_data.ram_EXTI_cnt.freq_cnt * 100)/100,(ram_data.ram_EXTI_cnt.freq_cnt * 100)%100,earth_temp,ram_data.supply_mode_R1,ram_data.supply_mode_R2,ram_data.Longitude/1000000L,abs(ram_data.Longitude%1000000L),ram_data.Latitude/1000000L,abs(ram_data.Latitude%1000000L), ram_data.ram_alarms);

    my_sprintf((char*)dummy_json_string, 26,"{\"telecom\":[{\"deviceId\":\"%s\",\"timestamp\":\"%01d\",\"batteryVoltage\":%01d.%03d,\"chargerVoltage\":%01d.%03d,\"router1Voltage\":%01d.%03d,\"router1Current\":%01d.%03d,\"router2Voltage\":%01d.%03d,\"router2Current\":%01d.%03d,\"inputVoltage\":%01d.%03d,\"frequency\":%01d.%02d,\"earthDetected\":%s,\"router1SupplyMode\":%01d,\"router2SupplyMode\":%01d,\"longitude\":%01d.%06d,\"latitude\":%01d.%06d,\"alarms\":%01d}]}",
			e2p_device_info.device_id,epoch_time,ram_data.ram_ADC.DC_Battery_voltage/1000, ram_data.ram_ADC.DC_Battery_voltage%1000,ram_data.ram_ADC.DC_Charger_voltage/1000,ram_data.ram_ADC.DC_Charger_voltage%1000,ram_data.ram_ADC.NE_AC_Voltage/1000,ram_data.ram_ADC.NE_AC_Voltage%1000,ram_data.ram_ADC.DC_current_router1/1000,ram_data.ram_ADC.DC_current_router1%1000,ram_data.ram_ADC.DC_Voltage_router2/1000,ram_data.ram_ADC.DC_Voltage_router2%1000,ram_data.ram_ADC.DC_current_router2/1000,ram_data.ram_ADC.DC_current_router2%1000,ram_data.ram_ADC.PN_AC_Voltage/1000,ram_data.ram_ADC.PN_AC_Voltage%1000,(ram_data.ram_EXTI_cnt.freq_cnt * 100)/100,(ram_data.ram_EXTI_cnt.freq_cnt * 100)%100,earth_temp,ram_data.supply_mode_R1,ram_data.supply_mode_R2,ram_data.Longitude/1000000L,abs(ram_data.Longitude%1000000L),ram_data.Latitude/1000000L,abs(ram_data.Latitude%1000000L), ram_data.ram_alarms);

    // my_sprintf((char*)dummy_json_string, 27,"{\"telecom\":[{\"deviceId\":\"%s\",\"timestamp\":\"%01d\",\"batteryVoltage\":%01d.%03d,\"chargerVoltage\":%01d.%03d,\"router1Voltage\":%01d.%03d,\"router1Current\":%01d.%03d,\"router2Voltage\":%01d.%03d,\"router2Current\":%01d.%03d,\"inputVoltage\":%01d.%03d,\"frequency\":%01d.%02d,\"earthDetected\":%s,\"router1SupplyMode\":%01d,\"router2SupplyMode\":%01d,\"longitude\":%01d.%06d,\"latitude\":%01d.%06d,\"alarms\":%01d,\"FW_VER\":\"%s\"}]}",
	// 		e2p_device_info.device_id,epoch_time,ram_data.ram_ADC.DC_Battery_voltage/1000, ram_data.ram_ADC.DC_Battery_voltage%1000,ram_data.ram_ADC.DC_Charger_voltage/1000,ram_data.ram_ADC.DC_Charger_voltage%1000,ram_data.ram_ADC.NE_AC_Voltage/1000,ram_data.ram_ADC.NE_AC_Voltage%1000,ram_data.ram_ADC.DC_current_router1/1000,ram_data.ram_ADC.DC_current_router1%1000,ram_data.ram_ADC.DC_Voltage_router2/1000,ram_data.ram_ADC.DC_Voltage_router2%1000,ram_data.ram_ADC.DC_current_router2/1000,ram_data.ram_ADC.DC_current_router2%1000,ram_data.ram_ADC.PN_AC_Voltage/1000,ram_data.ram_ADC.PN_AC_Voltage%1000,(ram_data.ram_EXTI_cnt.freq_cnt * 100)/100,(ram_data.ram_EXTI_cnt.freq_cnt * 100)%100,earth_temp,ram_data.supply_mode_R1,ram_data.supply_mode_R2,ram_data.Longitude/1000000L,abs(ram_data.Longitude%1000000L),ram_data.Latitude/1000000L,abs(ram_data.Latitude%1000000L), ram_data.ram_alarms, FIRMWARE_VERSION);

#ifdef DEBUG_JSON_PKT_PREP
    // UWriteString((char*)"\nPrep=",DBG_UART);
    // UWriteBytes((unsigned char*)dummy_json_string, strlen((const char*)dummy_json_string),DBG_UART);

    vUART_SendStr(DEBUG_UART_BASE, "\nPrep=");
    vUART_SendBytes(DEBUG_UART_BASE, dummy_json_string, strlen((const char*)dummy_json_string));
#endif

    retVal = websocket_packet(dummy_json_string);
    setREQmode(AVBL);

    return retVal;
}

#ifdef FLASH_EN
unsigned int prepare_unsentJSON_pckt(void)
{
    char earth_temp[7];
    unsigned long epoch_time = 0;
	memset(earth_temp, 0, sizeof(earth_temp));

    unsigned int retVal = 0;

    epoch_time = convertToEpochTime(&FLR_log_data.ram_data.ram_time);
    // epoch_time = asUnixTime(FLR_log_data.ram_data.ram_time.year, FLR_log_data.ram_data.ram_time.month, FLR_log_data.ram_data.ram_time.date, FLR_log_data.ram_data.ram_time.hour, FLR_log_data.ram_data.ram_time.min, FLR_log_data.ram_data.ram_time.sec);

    if(FLR_log_data.ram_data.ram_EXTI_cnt.earth_cnt)
	{
		memcpy(earth_temp, "true", strlen((const char*)"true"));
	}
	else
	{
		memcpy(earth_temp, "false", strlen((const char*)"false"));
	}

#ifdef DEBUG_JSON_PKT_PREP
    vUART_SendStr(DEBUG_UART_BASE,"\n2FL_log:");
    vUART_SendStr(DEBUG_UART_BASE,"\n2Ffc=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_EXTI_cnt.freq_cnt);
    vUART_SendStr(DEBUG_UART_BASE,"\t2Fef=");
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)earth_temp);
    vUART_SendStr(DEBUG_UART_BASE,"\n2FACP=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\n2FRC=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_ADC.DC_current_router1);
    vUART_SendStr(DEBUG_UART_BASE,"\t2FOC=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_ADC.DC_current_router2);
    vUART_SendStr(DEBUG_UART_BASE,"\n2FACN=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_ADC.NE_AC_Voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\t2FOV=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendStr(DEBUG_UART_BASE,"\n2FChgV=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_ADC.DC_Charger_voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\t2FBATTV=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\n2Fet=");
    vUART_SendInt(DEBUG_UART_BASE,epoch_time);
    vUART_SendStr(DEBUG_UART_BASE,"\n2FLa=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.Latitude);
    vUART_SendStr(DEBUG_UART_BASE,"\t2FLo=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.Longitude);
    vUART_SendStr(DEBUG_UART_BASE,"\n2FRS=");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.supply_mode_R1);
    vUART_SendStr(DEBUG_UART_BASE,"\t2FOS=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.supply_mode_R2);
    vUART_SendStr(DEBUG_UART_BASE,"\n2FA=");
    vUART_SendInt(DEBUG_UART_BASE,FLR_log_data.ram_data.ram_alarms);
#endif

    memset(dummy_json_string, 0, sizeof(dummy_json_string));
	// my_sprintf((char*)dummy_json_string, 26,"{\"telecom\":[{\"deviceId\":\"%s\",\"timestamp\":\"%01d\",\"batteryVoltage\":%01d.%03d,\"chargerVoltage\":%01d.%03d,\"router1Voltage\":%01d.%03d,\"router1Current\":%01d.%03d,\"router2Voltage\":%01d.%03d,\"router2Current\":%01d.%03d,\"inputVoltage\":%01d.%03d,\"frequency\":%01d.%02d,\"earthDetected\":%s,\"router1SupplyMode\":%01d,\"router2SupplyMode\":%01d,\"longitude\":%01d.%06d,\"latitude\":%01d.%06d,\"alarms\":%01d}]}",
	// 		e2p_device_info.device_id,epoch_time,FLR_log_data.ram_data.ram_ADC.DC_Battery_voltage/1000, FLR_log_data.ram_data.ram_ADC.DC_Battery_voltage%1000,FLR_log_data.ram_data.ram_ADC.DC_Charger_voltage/1000,FLR_log_data.ram_data.ram_ADC.DC_Charger_voltage%1000,FLR_log_data.ram_data.ram_ADC.DC_Voltage_router1/1000,FLR_log_data.ram_data.ram_ADC.DC_Voltage_router1%1000,FLR_log_data.ram_data.ram_ADC.DC_current_router1/1000,FLR_log_data.ram_data.ram_ADC.DC_current_router1%1000,FLR_log_data.ram_data.ram_ADC.DC_Voltage_router2/1000,FLR_log_data.ram_data.ram_ADC.DC_Voltage_router2%1000,FLR_log_data.ram_data.ram_ADC.DC_current_router2/1000,FLR_log_data.ram_data.ram_ADC.DC_current_router2%1000,FLR_log_data.ram_data.ram_ADC.PN_AC_Voltage/1000,FLR_log_data.ram_data.ram_ADC.PN_AC_Voltage%1000,(FLR_log_data.ram_data.ram_EXTI_cnt.freq_cnt * 100)/100,(FLR_log_data.ram_data.ram_EXTI_cnt.freq_cnt * 100)%100,earth_temp,FLR_log_data.ram_data.supply_mode_R1,FLR_log_data.ram_data.supply_mode_R2,FLR_log_data.ram_data.Longitude/1000000L,abs(FLR_log_data.ram_data.Longitude%1000000L),FLR_log_data.ram_data.Latitude/1000000L,abs(FLR_log_data.ram_data.Latitude%1000000L), FLR_log_data.ram_data.ram_alarms);
    my_sprintf((char*)dummy_json_string, 26,"{\"telecom\":[{\"deviceId\":\"%s\",\"timestamp\":\"%01d\",\"batteryVoltage\":%01d.%03d,\"chargerVoltage\":%01d.%03d,\"router1Voltage\":%01d.%03d,\"router1Current\":%01d.%03d,\"router2Voltage\":%01d.%03d,\"router2Current\":%01d.%03d,\"inputVoltage\":%01d.%03d,\"frequency\":%01d.%02d,\"earthDetected\":%s,\"router1SupplyMode\":%01d,\"router2SupplyMode\":%01d,\"longitude\":%01d.%06d,\"latitude\":%01d.%06d,\"alarms\":%01d}]}",
			e2p_device_info.device_id,epoch_time,FLR_log_data.ram_data.ram_ADC.DC_Battery_voltage/1000, FLR_log_data.ram_data.ram_ADC.DC_Battery_voltage%1000,FLR_log_data.ram_data.ram_ADC.DC_Charger_voltage/1000,FLR_log_data.ram_data.ram_ADC.DC_Charger_voltage%1000,FLR_log_data.ram_data.ram_ADC.NE_AC_Voltage/1000,FLR_log_data.ram_data.ram_ADC.NE_AC_Voltage%1000,FLR_log_data.ram_data.ram_ADC.DC_current_router1/1000,FLR_log_data.ram_data.ram_ADC.DC_current_router1%1000,FLR_log_data.ram_data.ram_ADC.DC_Voltage_router2/1000,FLR_log_data.ram_data.ram_ADC.DC_Voltage_router2%1000,FLR_log_data.ram_data.ram_ADC.DC_current_router2/1000,FLR_log_data.ram_data.ram_ADC.DC_current_router2%1000,FLR_log_data.ram_data.ram_ADC.PN_AC_Voltage/1000,FLR_log_data.ram_data.ram_ADC.PN_AC_Voltage%1000,(FLR_log_data.ram_data.ram_EXTI_cnt.freq_cnt * 100)/100,(FLR_log_data.ram_data.ram_EXTI_cnt.freq_cnt * 100)%100,earth_temp,FLR_log_data.ram_data.supply_mode_R1,FLR_log_data.ram_data.supply_mode_R2,FLR_log_data.ram_data.Longitude/1000000L,abs(FLR_log_data.ram_data.Longitude%1000000L),FLR_log_data.ram_data.Latitude/1000000L,abs(FLR_log_data.ram_data.Latitude%1000000L), FLR_log_data.ram_data.ram_alarms);

#ifdef DEBUG_JSON_PKT_PREP
    // UWriteString((char*)"\nPrep=",DBG_UART);
    // UWriteBytes((unsigned char*)dummy_json_string, strlen((const char*)dummy_json_string),DBG_UART);

    vUART_SendStr(DEBUG_UART_BASE, "\nPrep=");
    vUART_SendBytes(DEBUG_UART_BASE, dummy_json_string, strlen((const char*)dummy_json_string));
#endif

    retVal = websocket_packet(dummy_json_string);
    setREQmode(AVBL);

    return retVal;
}
#endif  //FLASH_EN

#if 0 
unsigned int websocket_packet(uint8_t *request)
{
	uint8_t opcode=0x01;
    uint16_t Length;
    uint8_t maskKey[4];
    unsigned int i = 0, JSON_indx = 0;

    memset(JSON_Tx_Buff, 0, sizeof(JSON_Tx_Buff));

    maskKey[0] = 0x37;
    maskKey[1] = 0xFA;
    maskKey[2] = 0x21;
    maskKey[3] = 0x3D;

    Length = strlen((char *)request);
    
    JSON_Tx_Buff[0] = opcode|0x80;

#ifdef DEBUG_WEB_PREP
    // UWriteString((char*)"\nJSON_Tx_Buff[0]=", DBG_UART);
    // UWriteData((JSON_Tx_Buff[0] & 0xFF), DBG_UART);
#endif
    
    if(Length<126)
    {
        JSON_Tx_Buff[1] = Length|0x80;
        JSON_indx += 2;
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n1JSON_Tx_Buff[1]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[1] & 0xFF), DBG_UART);
#endif
    }
    else
    {
        JSON_Tx_Buff[1] = 0xFE;
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[1]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[1] & 0xFF), DBG_UART);
#endif
        JSON_Tx_Buff[2] = ((Length >> 8) & 0xFF);
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[2]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[2] & 0xFF), DBG_UART);
#endif
        JSON_Tx_Buff[3] = (Length & 0xFF);
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[3]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[3] & 0xFF), DBG_UART);
#endif
        JSON_indx += 4;
    }
    
    memcpy((void*)&JSON_Tx_Buff[JSON_indx],maskKey,sizeof(maskKey));
    JSON_indx += sizeof(maskKey);

#ifdef DEBUG_WEB_PREP
    // // for(int k = 0; k < JSON_indx; k++)
    // for(unsigned int k = 0; k < JSON_indx; k++)
	// {
    //     // UWriteString((char*)"\n2JSON_Tx_Buff[i]=", DBG_UART);
    //     // UWriteData((JSON_Tx_Buff[k] & 0xFF), DBG_UART);
	// 	// //printf("\n3JSON_Tx_Buff[%d] = %02X",k,(JSON_Tx_Buff[k] & 0xFF));
	// }
#endif
    
    for(i=0; i<Length;i++)
    {
        JSON_Tx_Buff[JSON_indx++] = request[i]^maskKey[i%4];
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n\n4JSON_Tx_Buff[i]=", DBG_UART);
        // UWriteData(',', DBG_UART);
        // UWriteInt(JSON_indx, DBG_UART);
        // UWriteData(',', DBG_UART);
        // UWriteData((request[i]^maskKey[i%4]), DBG_UART);
#endif        
    }
    
    return JSON_indx;
}
#endif  //if 0

#if 1
#ifndef ETHERNET_EN
unsigned int websocket_packet(uint8_t *request)
{
	uint8_t opcode=0x01;
    uint16_t Length;
    uint8_t maskKey[4];
    unsigned int i = 0; //, JSON_indx = 0;
    flushTxBuffer(LTE_UART);


    maskKey[0] = 0x37;
    maskKey[1] = 0xFA;
    maskKey[2] = 0x21;
    maskKey[3] = 0x3D;

    Length = strlen((char *)request);
    
#ifdef DEBUG_WEB_PREP
    vUART_SendStr(UART_PC,"Length:");
    vUART_SendInt(UART_PC,Length);
#endif
    gprs_tx_buff.buffer[gprs_tx_buff.index++] = opcode|0x80;

#ifdef DEBUG_WEB_PREP
    // UWriteString((char*)"\nJSON_Tx_Buff[0]=", DBG_UART);
    // UWriteData((JSON_Tx_Buff[0] & 0xFF), DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE, "\ngWTB[0]");
    vUART_SendChr(DEBUG_UART_BASE, (gprs_tx_buff.buffer[0] & 0xFF));
#endif
    
    if(Length<126)
    {
        gprs_tx_buff.buffer[gprs_tx_buff.index++] = Length|0x80;
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n1JSON_Tx_Buff[1]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[1] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n1gWTB[1]");
        vUART_SendChr(DEBUG_UART_BASE, (gprs_tx_buff.buffer[1] & 0xFF));
#endif
    }
    else
    {
        gprs_tx_buff.buffer[gprs_tx_buff.index++] = 0xFE;
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[1]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[1] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n2gWTB[1]");
        vUART_SendChr(DEBUG_UART_BASE, (gprs_tx_buff.buffer[1] & 0xFF));
#endif
        gprs_tx_buff.buffer[gprs_tx_buff.index++] = ((Length >> 8) & 0xFF);
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[2]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[2] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n2gWTB[2]");
        vUART_SendChr(DEBUG_UART_BASE, (gprs_tx_buff.buffer[2] & 0xFF));
#endif
        gprs_tx_buff.buffer[gprs_tx_buff.index++] = (Length & 0xFF);
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[3]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[3] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n2gWTB[3]");
        vUART_SendChr(DEBUG_UART_BASE, (gprs_tx_buff.buffer[3] & 0xFF));
#endif
    }

    memcpy((void*)&gprs_tx_buff.buffer[gprs_tx_buff.index],maskKey,sizeof(maskKey));
    gprs_tx_buff.index += sizeof(maskKey);

#ifdef DEBUG_WEB_PREP
    // // for(int k = 0; k < JSON_indx; k++)
    // for(unsigned int k = 0; k < JSON_indx; k++)
	// {
    //     // UWriteString((char*)"\n2JSON_Tx_Buff[i]=", DBG_UART);
    //     // UWriteData((JSON_Tx_Buff[k] & 0xFF), DBG_UART);
	// 	// //printf("\n3JSON_Tx_Buff[%d] = %02X",k,(JSON_Tx_Buff[k] & 0xFF));
	// }
#endif
    
    for(i=0; i<Length;i++)
    {
        gprs_tx_buff.buffer[gprs_tx_buff.index++] = request[i]^maskKey[i%4];
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n\n4JSON_Tx_Buff[i]=", DBG_UART);
        // UWriteData(',', DBG_UART);
        // UWriteInt(JSON_indx, DBG_UART);
        // UWriteData(',', DBG_UART);
        // UWriteData((request[i]^maskKey[i%4]), DBG_UART);
#endif        
    }
    
#ifdef DEBUG_WEB_PREP
    vUART_SendStr(UART_PC,"\nTx_b:");
    vUART_SendBytes(UART_PC,(const uint8_t *)gprs_tx_buff.buffer,gprs_tx_buff.index);
#endif
    
    // return JSON_indx; 
    return gprs_tx_buff.index;
}
#else
unsigned int websocket_packet(uint8_t *request)
{
	uint8_t opcode=0x01;
    uint16_t Length;
    uint8_t maskKey[4];
    unsigned int i = 0; //, JSON_indx = 0;

    // flushTxBuffer(LTE_UART);
    flush_ether_tx_buff();


    maskKey[0] = 0x37;
    maskKey[1] = 0xFA;
    maskKey[2] = 0x21;
    maskKey[3] = 0x3D;

    Length = strlen((char *)request);
    
#ifdef DEBUG_WEB_PREP
    vUART_SendStr(UART_PC,"Length:");
    vUART_SendInt(UART_PC,Length);
#endif
    // gprs_tx_buff.buffer[gprs_tx_buff.index++] = opcode|0x80;
    ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = opcode|0x80;

#ifdef DEBUG_WEB_PREP
    // UWriteString((char*)"\nJSON_Tx_Buff[0]=", DBG_UART);
    // UWriteData((JSON_Tx_Buff[0] & 0xFF), DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE, "\neWTB[0]");
    vUART_SendChr(DEBUG_UART_BASE, (ethernet_tx_buff.buffer[0] & 0xFF));
#endif
    
    if(Length<126)
    {
        // gprs_tx_buff.buffer[gprs_tx_buff.index++] = Length|0x80;
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = Length|0x80;
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n1JSON_Tx_Buff[1]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[1] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n1eWTB[1]");
        vUART_SendChr(DEBUG_UART_BASE, (ethernet_tx_buff.buffer[1] & 0xFF));
#endif
    }
    else
    {
        // gprs_tx_buff.buffer[gprs_tx_buff.index++] = 0xFE;
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = 0xFE;
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[1]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[1] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n2eWTB[1]");
        vUART_SendChr(DEBUG_UART_BASE, (ethernet_tx_buff.buffer[1] & 0xFF));
#endif
        // gprs_tx_buff.buffer[gprs_tx_buff.index++] = ((Length >> 8) & 0xFF);
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = ((Length >> 8) & 0xFF);
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[2]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[2] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n2eWTB[2]");
        vUART_SendChr(DEBUG_UART_BASE, (ethernet_tx_buff.buffer[2] & 0xFF));
#endif
        // gprs_tx_buff.buffer[gprs_tx_buff.index++] = (Length & 0xFF);
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = (Length & 0xFF);
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n2JSON_Tx_Buff[3]=", DBG_UART);
        // UWriteData((JSON_Tx_Buff[3] & 0xFF), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, "\n2eWTB[3]");
        vUART_SendChr(DEBUG_UART_BASE, (ethernet_tx_buff.buffer[3] & 0xFF));
#endif
    }

    // memcpy((void*)&gprs_tx_buff.buffer[gprs_tx_buff.index],maskKey,sizeof(maskKey));
    // gprs_tx_buff.index += sizeof(maskKey);

    memcpy((void*)&ethernet_tx_buff.buffer[ethernet_tx_buff.index],maskKey,sizeof(maskKey));
    ethernet_tx_buff.index += sizeof(maskKey);

#ifdef DEBUG_WEB_PREP
    // // for(int k = 0; k < JSON_indx; k++)
    // for(unsigned int k = 0; k < JSON_indx; k++)
	// {
    //     // UWriteString((char*)"\n2JSON_Tx_Buff[i]=", DBG_UART);
    //     // UWriteData((JSON_Tx_Buff[k] & 0xFF), DBG_UART);
	// 	// //printf("\n3JSON_Tx_Buff[%d] = %02X",k,(JSON_Tx_Buff[k] & 0xFF));
	// }
#endif
    
    for(i=0; i<Length;i++)
    {
        // gprs_tx_buff.buffer[gprs_tx_buff.index++] = request[i]^maskKey[i%4];
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = request[i]^maskKey[i%4];
#ifdef DEBUG_WEB_PREP
        // UWriteString((char*)"\n\n4JSON_Tx_Buff[i]=", DBG_UART);
        // UWriteData(',', DBG_UART);
        // UWriteInt(JSON_indx, DBG_UART);
        // UWriteData(',', DBG_UART);
        // UWriteData((request[i]^maskKey[i%4]), DBG_UART);
#endif        
    }
    
#ifdef DEBUG_WEB_PREP
    vUART_SendStr(UART_PC,"\nTx_b:");
    vUART_SendBytes(UART_PC,(const uint8_t *)ethernet_tx_buff.buffer,ethernet_tx_buff.index);
#endif
    
    // return JSON_indx; 
    // return gprs_tx_buff.index;
    return ethernet_tx_buff.index;
}
#endif  //ETHERNET_EN

#endif  //if 0


void HandleQueryStates(void)    //Call to this should be only made once, not periodically otherwise it will keep changing states even when we are in the middle of processing a query.
{
    switch(getServerReqType())
    {
        case RESTART:
        {
#ifdef DEBUG_QUERY_STATES
            // UWriteString((char*)"\nRSTreq",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nRSTreq");
#endif
#ifndef ETHERNET_EN
            // LTEmodule.HandlerSts = GPRS_TCP_DISCONNECT;
            gprs.gprs_handler_state = GPRS_WEBSOCKET_DISCONNECT;
#ifdef  ENABLE_GPS
            conn_state =  CONNECT_DATA_UPLOAD;
#endif  //ENABLE_GPS
            set_pending_request(false); //PP added on 15-05-24: This was not being rest although it will restart the MCU before that.
#else
            //PP 14-05-24: make a ETHER_WEBSOCKET_DISCONNECT cmd later, for now only decode server message
            Telecom_Ethernet.ethernet_state = ETHER_WS_DISCONN;
            resetEther_SubHandlers();
            set_pending_request(false); //PP added on 15-05-24: This was not being rest although it will restart the MCU before that.
#endif  //ETHERNET_EN
        }
        break;

        case ODU_VOLTAGE_UPDATE:
        {
#ifdef DEBUG_QUERY_STATES
            // UWriteString((char*)"\nOVCreq",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nOVCreq");
#endif
#ifndef ETHERNET_EN
            // LTEmodule.HandlerSts = GPRS_SESSION_IDLE;
            setClientMSGType(RESPONSE_LOG);
            // prepare_server_pkt();    //will call this in PREPARE_LOGS case instead.
            gprs.gprs_handler_state = GPRS_SESSION_IDLE;
#ifdef  ENABLE_GPS
            conn_state =  CONNECT_DATA_UPLOAD;
#endif  //ENABLE_GPS            
#else
            Telecom_Ethernet.ethernet_state = ETHER_SESSION_IDLE;
            setClientMSGType(RESPONSE_LOG);
            // prepare_server_pkt();    //will call this in PREPARE_LOGS case instead.
#endif //ETHERNET_EN
        }
        break;

        case SERVER_URL_UPDATE:
        {
#ifdef DEBUG_QUERY_STATES
            // UWriteString((char*)"\nSUPreq",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nSUPreq");
#endif
#ifndef ETHERNET_EN
            // LTEmodule.HandlerSts = GPRS_SESSION_IDLE;
            setClientMSGType(RESPONSE_LOG);
            // prepare_server_pkt();    //will call this in PREPARE_LOGS case instead.
            gprs.gprs_handler_state = GPRS_SESSION_IDLE;
#ifdef  ENABLE_GPS
            conn_state =  CONNECT_DATA_UPLOAD;
#endif  //ENABLE_GPS            
#else
            Telecom_Ethernet.ethernet_state = ETHER_SESSION_IDLE;
            setClientMSGType(RESPONSE_LOG);
            // prepare_server_pkt();    //will call this in PREPARE_LOGS case instead.
#endif //ETHERNET_EN           
        }
        break;

        case NO_REQ:
        default:
        {
#ifdef DEBUG_QUERY_STATES
            // UWriteString((char*)"\nNoReq",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nNoReq");
#endif
            setClientMSGType(SCHEDULED_LOG);
            // set_pending_request(false);
            // prepare_server_pkt();    //will call this in PREPARE_LOGS case instead.
        }
        break;
    }
}

void prepare_server_pkt(void)
{
    switch(getClientMSGType())
    {
        case RESPONSE_LOG:
        {
#ifdef DEBUG_SERVER_QUERY
            // UWriteString((char*)"\nREQ=1", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nREQ=R");
#endif
            websocket_packet(server_response_str);
            setREQmode(AVBL);
        }
        break;

#ifdef FLASH_EN
        case UNSENT_LOGS:
        {
#ifdef DEBUG_SERVER_QUERY
            vUART_SendStr(DEBUG_UART_BASE, "\nREQ=U");
#endif
            get_unsent_logs();
            prepare_unsentJSON_pckt();
            setREQmode(AVBL);
        }
        break;
#endif //FLASH_EN

        default:
        case SCHEDULED_LOG:
        {
#ifdef DEBUG_SERVER_QUERY
            // UWriteString((char*)"\nREQ=2", DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, "\nREQ=S");
#endif
            prepare_JSON_pckt();
            setREQmode(AVBL);
        }
        break;
    }
}

void set_pending_request(bool request)
{
    Telecom_server_query.pending_request = request;
}

bool get_pending_request(void)
{
    return Telecom_server_query.pending_request;
}

void setREQmode(gprs_status_t sts)
{
    Telecom_server_query.send_request = sts;
}

gprs_status_t getREQmode(void)
{
    return Telecom_server_query.send_request;
}

void setServerReqType (server_request_type_t req_type)
{
    Telecom_server_query.server_request_type = req_type;
}

server_request_type_t getServerReqType(void)
{
    return Telecom_server_query.server_request_type;
}

void setClientMSGType (client_message_type_t msg_type)
{
    Telecom_server_query.client_message_type = msg_type;
}

client_message_type_t getClientMSGType(void)
{
    return Telecom_server_query.client_message_type;
}

void set_network_status(uint8_t status)
{
    network_status.server_status = status;
}

uint8_t get_network_status(void)
{
    return network_status.server_status;
}



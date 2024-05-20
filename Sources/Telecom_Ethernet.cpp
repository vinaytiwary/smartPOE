/*
 * Ethernet.cpp
 *
 *  Created on: May 2, 2024
 *      Author: ADMIN
 */

#include "_config.h"

#ifdef ETHERNET_EN
#include "stdint.h"
#include "stdbool.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"

#include "HW_pins.h"
#include "UartCore.h"
#include "SysTick_Timer.h"
#include "E2P.h"
#include "SPI.h"
#include "_debug.h"
#include "_common.h"
#include "Telecom_Ethernet.h"

#include <Sources/UIPEthernet/ethernet_comp.h>
#include <Sources/UIPEthernet/mbed/IPAddress.h>
#include <Sources/UIPEthernet/UIPClient.h>
#include <Sources/UIPEthernet/UIPEthernet.h>
#include "gprs.h"
#include "main.h"
#include "Telecom_server_query.h"


Telecom_Ethernet_t Telecom_Ethernet;
UIPClient client,Eclient;
volatile ethernet_rx_buff_t ethernet_rx_buff;//,ethernet_resp_buff;
volatile ethernet_tx_buff_t ethernet_tx_buff;
// extern volatile gprs_rx_data_buff_t gprs_rx_buff,gprs_resp_rx_buff;
// extern volatile gprs_tx_data_buff_t gprs_tx_buff;

// ethernet_state_t ethernet_state = ETHER_INIT;
// uint8_t ether_connect_sts;
// uint8_t ether_network_sts;
// extern system_info_t system_info;
extern cloud_config_t cloud_config;
extern gprs_t gprs;

// extern uint8_t JSON_Tx_Buff[513];
// extern uint8_t JSON_Rx_Buff[513];

extern Alarms_t Alarms;

void vETHERNETSPIInit(void)
{
    SysCtlPeripheralEnable(ETHERNET_SPI_PERIPH);
    GPIOPinConfigure(ETHERNET_SCK_GPIO_CFG);
    // GPIOPinConfigure(ETHERNET_CS_GPIO_CFG);
    GPIOPinConfigure(ETHERNET_MISO_GPIO_CFG);
    GPIOPinConfigure(ETHERNET_MOSI_GPIO_CFG);
    GPIOPinTypeSSI(ETHERNET_SPI_PORT_BASE, ETHERNET_CLK_PIN | ETHERNET_MISO_PIN | ETHERNET_MOSI_PIN);
    SSIConfigSetExpClk(ETHERNET_SPI_BASE, SYSTEM_CLOCK, ETHERNET_SPI_FRAME_FORMAT, ETHERNET_SPI_MODE, ETHERNET_SPI_CLOCK, ETHERNET_SPI_DATA_WIDTH);
    SSIEnable(ETHERNET_SPI_BASE);

    GPIOPinTypeGPIOInput(ETHERNET_PHYINT_PORT_BASE, ETHERNET_PHYINT_PIN);
    //GPIOPinWrite(ETHERNET_PHYINT_PORT_BASE, ETHERNET_PHYINT_PIN, ETHERNET_PHYINT_PIN);

    GPIOPinTypeGPIOOutput(ETHERNET_RST_PORT_BASE, ETHERNET_RST_PIN);
    GPIOPinWrite(ETHERNET_RST_PORT_BASE, ETHERNET_RST_PIN, ETHERNET_RST_PIN);

    GPIOPinTypeGPIOOutput(ETHERNET_SPI_PORT_BASE, ETHERNET_CS_PIN);
    GPIOPinWrite(ETHERNET_SPI_PORT_BASE, ETHERNET_CS_PIN, ETHERNET_CS_PIN);
}

#ifdef ETHERNET_EN
void ethernet_init(void)
{
//    uint8_t sts = 0;
    uint8_t mac[6] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
    Ethernet.begin(mac); //Configure IP address via DHCP
    //Serial.print(F("localIP: "));
#ifdef DEBUG_ETHERNET
    vUART_SendStr(UART_PC, "localIP: ");
    IPAddress localIP = Ethernet.localIP();
    vUART_SendInt(UART_PC, localIP[0]);
    vUART_SendInt(UART_PC, localIP[1]);
    vUART_SendInt(UART_PC, localIP[2]);
    vUART_SendInt(UART_PC, localIP[3]);
    vUART_SendStr(UART_PC, "subnetMask: ");
    localIP = Ethernet.subnetMask();
    vUART_SendInt(UART_PC, localIP[0]);
    vUART_SendInt(UART_PC, localIP[1]);
    vUART_SendInt(UART_PC, localIP[2]);
    vUART_SendInt(UART_PC, localIP[3]);
    vUART_SendStr(UART_PC, "gatewayIP: ");
    localIP = Ethernet.gatewayIP();
    vUART_SendInt(UART_PC, localIP[0]);
    vUART_SendInt(UART_PC, localIP[1]);
    vUART_SendInt(UART_PC, localIP[2]);
    vUART_SendInt(UART_PC, localIP[3]);
    vUART_SendStr(UART_PC, "dnsServerIP: ");
    localIP = Ethernet.dnsServerIP();
    vUART_SendInt(UART_PC, localIP[0]);
    vUART_SendInt(UART_PC, localIP[1]);
    vUART_SendInt(UART_PC, localIP[2]);
    vUART_SendInt(UART_PC, localIP[3]);
#endif
    flush_ether_tx_buff();
    flush_ether_rx_buff();
//    set_ethernet_connct_sts(TRUE);
    //return sts;
}
#endif  //ETHERNET_EN

void ethernet_handler(void)
{
    //static ethernet_state_t ethernet_state = ETHER_INIT;
    static uint32_t start_millis = 0;
    static uint8_t ether_ws_sts = FALSE;
    char status = 0;
    uint8_t dhcp_sts = 0;
    // static unsigned int TCP_indx = 0;
    // switch(ethernet_state)
    switch(Telecom_Ethernet.ethernet_state)
    {
        case ETHER_INIT:
        {
            start_millis = my_millis();
            // ethernet_state = ETHER_DHCP_CONN;
            Telecom_Ethernet.ethernet_state = ETHER_DHCP_CONN;
            ether_ws_sts = FALSE;
            set_ethernet_NWstatus(FALSE);
            set_ethernet_connct_sts(FALSE);

#ifdef DEBUG_ETHERNET
            // vUART_SendStr(UART_PC,"\nether_tcp_conn:");
            vUART_SendStr(UART_PC, "\nstartDHCP:");
            vUART_SendInt(UART_PC, (start_millis/1000));
#endif
        }
        break;

        case ETHER_DHCP_CONN:
        {
// #ifdef DEBUG_ETHERNET
//             vUART_SendStr(UART_PC,"\neDHCP:");
//             vUART_SendInt(UART_PC, ((my_millis() - start_millis)/1000));
// #endif
            dhcp_sts = Ethernet.dhcp_handler();
            // dhcp_sts = request_DHCP_lease();
            if(dhcp_sts)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\ndhcp_pass:");
                vUART_SendInt(UART_PC, ((my_millis() - start_millis)/1000));
#endif
                set_ethernet_connct_sts(TRUE);
                // ethernet_state = ETHER_TCP_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;
            }
            else
            {
// #ifdef DEBUG_ETHERNET
//                 vUART_SendStr(UART_PC,"\ndhcp_f:");
//                 vUART_SendInt(UART_PC, ((my_millis() - start_millis)/1000));
// #endif
                // _dhcp_state = STATE_DHCP_START;
                set_ethernet_NWstatus(FALSE);
                set_ethernet_connct_sts(FALSE);

                // ethernet_state = ETHER_DHCP_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_DHCP_CONN;
            }
        }
        break;


        case ETHER_TCP_CONN:
        {
            ether_tcp_sts_t sts = ether_tcp_connect();
            if(sts == ETHER_TCP_ALREADY_CONN)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_tcp_conn:ak");
                vUART_SendStr(UART_PC, "\nWS_sts=");
                vUART_SendInt(UART_PC, ether_ws_sts);
#endif
                if(ether_ws_sts == TRUE)
                {
                    // ethernet_state = ETHER_PING;
                    Telecom_Ethernet.ethernet_state = ETHER_PING;
                    set_ethernet_connct_sts(TRUE);
                }
                else
                {
                    // ethernet_state = ETHER_PING;
                    Telecom_Ethernet.ethernet_state = ETHER_WS_CONN;
                    set_ethernet_connct_sts(TRUE);
                }
            }
            else if(sts == ETHER_TCP_CON_PASS)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_tcp_conn:k");
#endif
                // ethernet_state = ETHER_WS_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_WS_CONN;
                // Telecom_Ethernet.ethernet_state = ETHER_PING;   //PP added for testing bad request size  //it was 590 Bytes
                set_ethernet_connct_sts(TRUE);
            }
            else if(sts == ETHER_TCP_CON_FAIL)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_tcp_conn:f");
#endif
                set_ethernet_NWstatus(FALSE);
                set_ethernet_connct_sts(FALSE);

                // _dhcp_state = STATE_DHCP_START;
                // ethernet_state = ETHER_DHCP_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_DHCP_CONN;
            }
        }
        break;

        case ETHER_WS_CONN:
        {
            ether_ws_sts_t sts = ws_connect();
            // if(ws_connect() == ETHER_WS_CON_PASS)
            if(sts == ETHER_WS_CON_PASS)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_ws_conn:k");
#endif
                set_ethernet_NWstatus(TRUE);
                set_ethernet_connct_sts(TRUE);

                ether_ws_sts = TRUE;
                // // ethernet_state = ETHER_PING;
                Telecom_Ethernet.ethernet_state = ETHER_PING;    //PP on 16-05-24: commented for testing websocket disconnect.
                // Telecom_Ethernet.ethernet_state = ETHER_WS_DISCONN; //PP on 16-05-24: added for testing.
            }
            // else if(ws_connect() == ETHER_WS_CON_FAIL)
            else if(sts == ETHER_WS_CON_FAIL)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_ws_conn:f");
#endif
                set_ethernet_NWstatus(FALSE);

                ether_ws_sts = FALSE;
                // _dhcp_state = STATE_DHCP_START;
                set_ethernet_connct_sts(FALSE);
                // ethernet_state = ETHER_TCP_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;
            }
        }
        break;

        case ETHER_WS_DISCONN:
        {
            ether_wsdisconn_sts_t sts = ws_disconn();
            // if(ws_disconn() == ETHER_WS_DISCON_PASS)
            if(sts == ETHER_WS_DISCON_PASS)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_wsdis_conn:k");
#endif
                set_ethernet_NWstatus(FALSE);
                set_ethernet_connct_sts(FALSE);

                ether_ws_sts = FALSE;
                // ethernet_state = ETHER_PING;
                // Telecom_Ethernet.ethernet_state = ETHER_WS_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;   //It won't respond if I do WSconn again so I did TCPconn after this instead and it ran ok. Not getting into client.stop() coz so far it's not neede.
            }
            // else if(ws_disconn() == ETHER_WS_CON_FAIL)
            else if(sts == ETHER_WS_CON_FAIL)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_wsdis_conn:f");
#endif
                set_ethernet_NWstatus(FALSE);

                ether_ws_sts = FALSE;
                // _dhcp_state = STATE_DHCP_START;
                set_ethernet_connct_sts(FALSE);
                // ethernet_state = ETHER_TCP_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;
            }
        }
        break;

        case ETHER_PING:
        {
            ether_ping_status_t sts = ether_ping_send();
            // if(ether_ping_send() == ETHER_PING_PASS)
            if(sts == ETHER_PING_PASS)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_ping_send:k");
#endif
                ether_ws_sts = TRUE;
                set_ethernet_NWstatus(TRUE);
                set_ethernet_connct_sts(TRUE);

                // ethernet_state = ETHER_SESSION_IDLE;
                Telecom_Ethernet.ethernet_state = ETHER_SESSION_IDLE;
            }
            // else if(ether_ping_send() == ETHER_PING_FAIL)
            else if(sts == ETHER_PING_FAIL)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether_ping_send:f");
#endif
                ether_ws_sts = FALSE;
                set_ethernet_NWstatus(FALSE);

                //_dhcp_state = STATE_DHCP_START;
                set_ethernet_connct_sts(FALSE);
                // ethernet_state = ETHER_TCP_CONN;
                Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;
            }

        }
        break;

        case ETHER_PREPARE_LOGS:
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
            // ethernet_tx_buff.index = prepare_JSON_pckt();
#ifdef DEBUG_TCP_HANDLER
            vUART_SendStr(UART_PC,"\nTCP_indx=");
            vUART_SendInt(UART_PC, ethernet_tx_buff.index);
            vUART_SendStr(UART_PC, "\nTCP_buff=");
            vUART_SendBytes(UART_PC, (const uint8_t*)ethernet_tx_buff.buffer, ethernet_tx_buff.index);
#endif
            // ethernet_state = ETHER_LOG_UPLOAD;
            Telecom_Ethernet.ethernet_state = ETHER_LOG_UPLOAD;
        }
        break;

        case ETHER_LOG_UPLOAD:
        {
            if(getREQmode() == AVBL)
            {
                // status = ether_tcp_send((char *)ethernet_tx_buff.buffer, ethernet_tx_buff.index);
                // status = ether_tcp_send((char *)JSON_Tx_Buff,TCP_indx);
                status = ether_tcp_send((char *)ethernet_tx_buff.buffer,ethernet_tx_buff.index);

                if(status == ETHER_TCP_SEND_PASS)         //pending
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\nTCP_SEND_PASS");
#endif
                    if(/* Alarms.Power_ON || */ ((getRAM_Alarm() & (1 << POWER_ON_BIT)) == (1 << POWER_ON_BIT)))
                    {
                        Alarms.Power_ON = false;
                        setRAM_Alarm(POWER_ON_BIT,Alarms.Power_ON);
                    }

                    // ethernet_state = ETHER_SESSION_IDLE;
                    Telecom_Ethernet.ethernet_state = ETHER_SESSION_IDLE;

                    if(getServerReqType() == ODU_VOLTAGE_UPDATE)
                    {
                        setServerReqType(NO_REQ);
                    }

                    if(getClientMSGType() == RESPONSE_LOG)
                    {
                        setClientMSGType(SCHEDULED_LOG);
                    }

                    //flushTxBuffer(LTE_UART);
                    //ethernet_tx_buff.index = 0;
                    setREQmode(NOT_AVBL);
                }
                else if(status == ETHER_TCP_SEND_FAIL)
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\nTCP_SEND_FAIL");
#endif
                    // ethernet_state = ETHER_TCP_CONN;
                    Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;
                    //setREQmode(NOT_AVBL);
                }
                // else if(status == TCP_SEND_IN_PRG)
                // {
                //
                // }
            }
            else
            {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                vUART_SendStr(UART_PC,"\nGPRS_LOGS_UPLOAD:NOT_AVBL");
#endif
            }
        }
        break;

        case ETHER_SESSION_IDLE:
        {
            static unsigned char retry_time = 0;
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
                        vUART_SendStr(DEBUG_UART_BASE, "\neRST3");
#endif
                        set_pending_request(false);
                        // gprs.gprs_handler_state = GPRS_WEBSOCKET_DISCONNECT;     //PP 15-05-24: make a similar command for ETHERNET later
                        Telecom_Ethernet.ethernet_state = ETHER_WS_DISCONN;
                        resetEther_SubHandlers();
                    }
                    break;

                    case ODU_VOLTAGE_UPDATE:
                    {
#ifdef DEBUG_SERVER_QUERY
                        // UWriteString((char*)"\nOVC3",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\neOVC3");
#endif                 
                        set_pending_request(false);
                        // setClientMSGType(RESPONSE_LOG); // handled in HandleQueryStates() states now.
                        Telecom_Ethernet.ethernet_state = ETHER_PREPARE_LOGS;
                        resetEther_SubHandlers();
                    }
                    break;

                    case SERVER_URL_UPDATE:
                    {
#ifdef DEBUG_SERVER_QUERY
                        // UWriteString((char*)"\neSUP",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, "\neSUP");
#endif 
                        set_pending_request(false);
                        // setClientMSGType(RESPONSE_LOG); // handled in HandleQueryStates() states now.
                        Telecom_Ethernet.ethernet_state = ETHER_PREPARE_LOGS;
                        resetEther_SubHandlers();
                    }
                    break;

                    case NO_REQ:
                    default:
                    {
#ifdef DEBUG_SERVER_QUERY
                        vUART_SendStr(DEBUG_UART_BASE, "\neLog");
#endif                 
                        set_pending_request(false);
                        // setClientMSGType(RESPONSE_LOG); // handled in HandleQueryStates() states now.
                        Telecom_Ethernet.ethernet_state = ETHER_PREPARE_LOGS;
                    }
                    break;
                }
            }
            else if(getREQmode() == AVBL)
            {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                vUART_SendStr(UART_PC,"\neAVBL");
#endif
                if (ether_ws_sts == TRUE)
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\neUPD_LOGS");
#endif
                    // ethernet_state = ETHER_LOG_UPLOAD;
                    Telecom_Ethernet.ethernet_state = ETHER_LOG_UPLOAD;
                }
                else
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"\neWNOT_AVBL");
#endif
                    // ethernet_state = ETHER_TCP_CONN;
                    Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;
                }
            }
            else if(retry_time++ > ETHER_CONN_RETRY_TIME)
            {
                retry_time = 0;
                // ethernet_state = ETHER_PING;
                Telecom_Ethernet.ethernet_state = ETHER_PING;
            }
//             if(get_pending_request())
//             {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                 vUART_SendStr(UART_PC, "\nget_pending_request:");
//                 vUART_SendInt(UART_PC, get_pending_request());
// #endif
//                 // PP commented on 03-05-24, this is evse relevant function:
//                 // OCPP_Server_Query_Message();
//
//                 TCP_indx = prepare_JSON_pckt();
//                 set_pending_request(0);
//             }
//             else if(getREQmode() == AVBL)
//             {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                 vUART_SendStr(UART_PC,"\nAVBL");
// #endif
//                 if (ether_ws_sts == TRUE)
//                 {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                     vUART_SendStr(UART_PC,"\nUPLOADING_LOGS");
// #endif
//                     // ethernet_state = ETHER_LOG_UPLOAD;
//                     Telecom_Ethernet.ethernet_state = ETHER_LOG_UPLOAD;
//                 }
//                 else
//                 {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                     vUART_SendStr(UART_PC,"\nWS_NOT_AVBL");
// #endif
//                     // ethernet_state = ETHER_TCP_CONN;
//                     Telecom_Ethernet.ethernet_state = ETHER_TCP_CONN;
//                 }
//             }
//             else if(retry_time++ > ETHER_CONN_RETRY_TIME)
//             {
//                 retry_time = 0;
//                 // ethernet_state = ETHER_PING;
//                 Telecom_Ethernet.ethernet_state = ETHER_PING;
//             }
//             // else
//             // {
//             //         //gprs.gprs_handler_state = GPRS_PING;
//             //     if(freq_updated_data.unsent_ocpp_cmd_logs)
//             //     {
//             //         setREQmode(AVBL);
//             //     }
//             //     //setREQmode(AVBL);
//             //     else
//             //     {
//             //         //gprs.gprs_handler_state = GPRS_PING;
//             //     }
//             // }
        }
        break;
    }
}

#if 0
ether_tcp_sts_t ether_tcp_connect(void)
{
    if (client.connected())
    {
#ifdef DEBUG_ETHERNET
        vUART_SendStr(UART_PC, "\ntcp_conn");
#endif

        return ETHER_TCP_ALREADY_CONN;
    }

    //if(client.connect(IPAddress(122, 160, 48, 7), 8060))
    if(client.connect(IPAddress(192, 168, 1, 101), 5000))
    {
#ifdef DEBUG_ETHERNET
        vUART_SendStr(UART_PC, "\ntcp_con:k");
#endif
        return ETHER_TCP_CON_PASS;
    }

#ifdef DEBUG_ETHERNET
    vUART_SendStr(UART_PC, "\ntcp_con:f");
#endif
    return ETHER_TCP_CON_FAIL;

}
#else

ether_tcp_sts_t ether_tcp_connect(void)
{
    ether_tcp_sts_t sts = ETHER_TCP_CON_PRG;
    // const char ip[20] = ""
    if (client.connected())
    {
#ifdef DEBUG_ETHERNET
        vUART_SendStr(UART_PC, "\ntcp_conn");
#endif

        sts = ETHER_TCP_ALREADY_CONN;
    }
    else
    {
        // uint8_t conn_sts = client.connect((const char *)cloud_config.ip_addr, cloud_config.port_num);
        // uint8_t conn_sts = client.connect(IPAddress(192, 168, 1, 101), 5000);
        uint8_t conn_sts = client.connect(IPAddress(192, 168, 1, 101), 10001);
#ifdef DEBUG_ETHERNET
        vUART_SendStr(UART_PC, "\nconn_sts:");
        vUART_SendInt(UART_PC, conn_sts);
#endif
        if (conn_sts == 1)
        {
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC, "\ntcp_con:k");
#endif
            sts = ETHER_TCP_CON_PASS;
        }
        else if (conn_sts == 0)
        {
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC, "\ntcp_con:f");
#endif
            sts = ETHER_TCP_CON_FAIL;
        }
    }

    return sts;
}
#endif  // if 0

ether_ws_sts_t ws_connect(void)
{
    // static ether_ws_cmd_t ether_ws_cmd = ETHER_WS_CMD;
    ether_ws_sts_t sts = ETHER_WS_CON_PRG;
#ifdef DEBUG_ETHERNET
    vUART_SendStr(UART_PC,"\nWC:");
    vUART_SendInt(UART_PC, Telecom_Ethernet.ether_ws_cmd);
    // vUART_SendInt(UART_PC, ether_ws_cmd);
#endif
    // switch(ether_ws_cmd)
    switch(Telecom_Ethernet.ether_ws_cmd)
    {
        case ETHER_WS_CMD:
        {
            //prepare websocket connect command
            char temp_buff[210];
            memset(temp_buff,0,sizeof(temp_buff));
            // char randomKey[24];
            // memset(randomKey,0,sizeof(randomKey));
            // generateRandomKey(24,randomKey);
            // //char* randomKey = generateRandomKey(24);
            char randomKey[25] = "q4xkcO42u266gldTuKaSOw==";

            flush_ether_rx_buff();
            //my_sprintf(temp_buff,5,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Protocol: ocpp1.6\r\n\r\n","/RB/WS/ECMS/E4E003","122.160.48.7",8060,randomKey);
            // my_sprintf(temp_buff,5,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Protocol: ocpp1.6\r\n\r\n",cloud_config.path,cloud_config.ip_addr,cloud_config.port_num,randomKey);
            // my_sprintf(temp_buff,5,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Protocol: ocpp1.6\r\n\r\n",cloud_config.path,"192.168.1.101",cloud_config.port_num,randomKey);
            my_sprintf(temp_buff,5,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: %s\r\n\r\n",cloud_config.path,"192.168.1.101",cloud_config.port_num,randomKey);

            int final_index = strlen(temp_buff);

            //temp_buff[final_index++] = 0x1A;
            client.write(temp_buff,final_index);

            //send ws command
            //client.write(ws_req);
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC,"\nws_conn:cmd");
            vUART_SendBytes(UART_PC,(const uint8_t *)temp_buff,final_index);
#endif
            // ether_ws_cmd = ETHER_WS_RSP;
            Telecom_Ethernet.ether_ws_cmd = ETHER_WS_RSP;
        }
        break;

        case ETHER_WS_RSP:
        {
            //char rx_buff[500];
            static uint16_t timeout,retry_cnt;
            //read response
            //if(client.available())
            if(ethernet_rx_buff.locked)
            {
                ethernet_rx_buff.locked = UNLOCKED;
                //uint8_t size = client.available();
                //uint8_t msg[size];
                //client.read((uint8_t *)&rx_buff, size);
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nws_conn:rsp:");
                vUART_SendBytes(UART_PC,(const uint8_t *)/*rx_buff*/ethernet_rx_buff.buffer,ethernet_rx_buff.index);
#endif
                char *resp = strstr((const char *)/*rx_buff*/ethernet_rx_buff.buffer,"HTTP/1.1 ");
                if(resp)
                {
                    if(ether_valid_code((char *)&resp[strlen("HTTP/1.1 ")]))
                    {
#ifdef DEBUG_ETHERNET
                        vUART_SendStr(UART_PC,"\nws_conn:rsp:k");
#endif
                        timeout = 0;
                        retry_cnt = 0;
                        sts = ETHER_WS_CON_PASS;
                        // ether_ws_cmd = ETHER_WS_CMD;    //PP 16-05-24: added RESET state. Was not here before.
                        Telecom_Ethernet.ether_ws_cmd = ETHER_WS_CMD;   //PP 16-05-24: added RESET state. Was not here before.
                    }
                    else
                    {
#ifdef DEBUG_ETHERNET
                        vUART_SendStr(UART_PC,"\nws_conn:rsp:f");
#endif
                        timeout = 0;
                        retry_cnt = 0;
                        sts = ETHER_WS_CON_FAIL;
                        // ether_ws_cmd = ETHER_WS_CMD;    //PP 16-05-24: added RESET state. Was not here before.
                        Telecom_Ethernet.ether_ws_cmd = ETHER_WS_CMD;   //PP 16-05-24: added RESET state. Was not here before.
                    }
                    //sts = ETHER_WS_CON_PASS;
                }
                else
                {
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC,"\nnoHTTP");
#endif
                    timeout = 0;
                    retry_cnt = 0;
                    sts = ETHER_WS_CON_FAIL;
                    // ether_ws_cmd = ETHER_WS_CMD;    //PP 16-05-24: added RESET state. Was not here before.
                    Telecom_Ethernet.ether_ws_cmd = ETHER_WS_CMD;   //PP 16-05-24: added RESET state. Was not here before.
                }
            }
            else
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nwsw:");
                vUART_SendInt(UART_PC, timeout);
                vUART_SendChr(UART_PC, ',');
                vUART_SendInt(UART_PC, retry_cnt);
#endif
                if(++timeout >= ((10*1000)/50))
                {
                    timeout = 0;
                    // ether_ws_cmd = ETHER_WS_CMD;
                    Telecom_Ethernet.ether_ws_cmd = ETHER_WS_CMD;
                    if(++retry_cnt >= 3)
                    {
                        retry_cnt = 0;
                        sts = ETHER_WS_CON_FAIL;
#ifdef DEBUG_ETHERNET
                        vUART_SendStr(UART_PC,"\nws_conn:rsp:f");
#endif
                    }
                }
            }
        }
        break;
    }
    return sts;
}

ether_ping_status_t ether_ping_send(void )
{
    ether_ping_status_t sts = ETHER_PING_IN_PRG;
    // static ether_ping_cmd_t ether_ping_cmd = ETHER_PING_CMD;
#ifdef DEBUG_ETHERNET
    vUART_SendStr(UART_PC,"\nPS:");
    // vUART_SendInt(UART_PC, ether_ping_cmd);
    vUART_SendInt(UART_PC, Telecom_Ethernet.ether_ping_cmd);
#endif
    // switch(ether_ping_cmd)
    switch(Telecom_Ethernet.ether_ping_cmd)
    {
        case ETHER_PING_CMD:
        {
            //prepare websocket connect command
            //char buff[] = {0x81,0x9D,0x37,0xFA,0x21,0x3D,0x6C,0xC8,0x0D,0x1F,0x06,0xC3,0x13,0x0F,0x04,0xC8,0x11,0x0C,0x15,0xD6,0x03,0x75,0x52,0x9B,0x53,0x49,0x55,0x9F,0x40,0x49,0x15,0xD6,0x5A,0x40,0x6A};
            char buff[] = {0x89,0x80,0x37,0xFA,0x21,0x3D};
            client.write(buff,sizeof(buff));
            flush_ether_rx_buff();
            // ether_ping_cmd = ETHER_PING_RSP;
            Telecom_Ethernet.ether_ping_cmd = ETHER_PING_RSP;
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC,"\nping_snd:cmd:");
            vUART_SendStr(UART_PC, (uint8_t*)buff);
#endif
        }
        break;

        case ETHER_PING_RSP:
        {
            //uint8_t rx_buff[100];
            static uint16_t timeout,retry_cnt;
            //read response
            if(ethernet_rx_buff.locked)
            {
                ethernet_rx_buff.locked = UNLOCKED;
                //uint8_t size = client.available();
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nPSR:");
                vUART_SendStr(UART_PC, (uint8_t*)ethernet_rx_buff.buffer);
                vUART_SendChr(UART_PC, ',');
                vUART_SendInt(UART_PC, ethernet_rx_buff.index);
#endif
                //size = client.read((uint8_t *)&ethernet_rx_buff.buffer,size);
                //ethernet_rx_buff.index += size;
                if(ethernet_pong_received((uint8_t *)ethernet_rx_buff.buffer))
                {
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC,"\npong_recv");
#endif
                    sts = ETHER_PING_PASS;
                    //ether_ping_cmd = ETHER_HRT_CMD;
                    flush_ether_rx_buff();  //PP added on 15-05-24: as this was not being cleared till the next cmd and since it contains 0x8A, 0x00. 0x00 reamaining in the buffer caused parsing issues when we recieved a server_request() after this.
                }
                else
                {
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC,"\npong_not_recv");
#endif
                    //sts = ETHER_PING_PASS;
                    sts = ETHER_PING_FAIL;
                    // ether_ping_cmd = ETHER_PING_CMD;
                    Telecom_Ethernet.ether_ping_cmd = ETHER_PING_CMD;
                }
                //free(rx_buff);
                // ether_ping_cmd = ETHER_PING_CMD;
                Telecom_Ethernet.ether_ping_cmd = ETHER_PING_CMD;
                timeout = 0;
                retry_cnt = 0;
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nping_snd:rsp:");
                vUART_SendBytes(UART_PC,(const uint8_t *)&ethernet_rx_buff.buffer, ethernet_rx_buff.index);
#endif
            }
            else
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nPSW:");
                vUART_SendStr(UART_PC, (uint8_t*)ethernet_rx_buff.buffer);
                vUART_SendChr(UART_PC, ',');
                vUART_SendInt(UART_PC, ethernet_rx_buff.index);
#endif
                if(++timeout >= ((10*1000)/50))
                {
                    timeout = 0;
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC,"\nping:rsp:to");
#endif
                    // ether_ping_cmd = ETHER_PING_CMD;
                    Telecom_Ethernet.ether_ping_cmd = ETHER_PING_CMD;
                    if(++retry_cnt >= 3)
                    {
                        retry_cnt = 0;
                        sts = ETHER_PING_FAIL;
#ifdef DEBUG_ETHERNET
                        vUART_SendStr(UART_PC,"\nping:rsp:rf");
#endif
                    }
                }
            }
        }
        break;    
#if 0
        case ETHER_HRT_CMD:
        {
            //prepare websocket connect command
            char buff[] = {0x81,0x9D,0x37,0xFA,0x21,0x3D,0x6C,0xC8,0x0D,0x1F,0x06,0xC3,0x13,0x0F,0x04,0xC8,0x11,0x0C,0x15,0xD6,0x03,0x75,0x52,0x9B,0x53,0x49,0x55,0x9F,0x40,0x49,0x15,0xD6,0x5A,0x40,0x6A};
            //char buff[] = {0x89,0x80,0x37,0xFA,0x21,0x3D};
            client.write(buff,sizeof(buff));
            ether_ping_cmd = ETHER_HRT_RSP;
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC,"\nhrt:cmd");
#endif
        }
        break;

        case ETHER_HRT_RSP:
        {
            //uint8_t rx_buff[100];
            static uint16_t timeout,retry_cnt;
            //read response
            if(client.available())
            {
                uint8_t size = client.available();
                // //uint8_t msg[size];
                // client.read((uint8_t *)&rx_buff, size);
                size = client.available();
                uint8_t* rx_buff = (uint8_t*)malloc(size+1);
                memset(rx_buff, 0, size+1);
                size = client.read(rx_buff,size);

                free(rx_buff);
                sts = ETHER_PING_PASS;
                ether_ping_cmd = ETHER_PING_CMD;
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nhrt:rsp:");
                vUART_SendBytes(UART_PC,(const uint8_t *)rx_buff, size);
#endif
            }
            else
            {
                if(++timeout >= ((10*1000)/50))
                {
                    timeout = 0;
                    ether_ping_cmd = ETHER_HRT_CMD;
                    if(++retry_cnt >= 3)
                    {
                        retry_cnt = 0;
                        ether_ping_cmd = ETHER_PING_CMD;
                        sts = ETHER_PING_FAIL;
#ifdef DEBUG_ETHERNET
                        vUART_SendStr(UART_PC,"\nhrt:rsp:rf");
#endif
                    }
                }
            }
        }
        break;
#endif  // if 0
    }   //switch-case end
    return sts;
}

ether_wsdisconn_sts_t ws_disconn(void )
{
    ether_wsdisconn_sts_t sts = ETHER_WS_DISCON_PRG;
    static ether_wsdis_cmd_t ether_wsdis_cmd = ETHER_WSDIS_CMD;
#ifdef DEBUG_ETHERNET
    vUART_SendStr(UART_PC,"\neWDIS:");
    vUART_SendInt(UART_PC, ether_wsdis_cmd);
#endif
    switch(ether_wsdis_cmd)
    {
        case ETHER_WSDIS_CMD:
        {
            //prepare websocket connect command
            //char buff[] = {0x81,0x9D,0x37,0xFA,0x21,0x3D,0x6C,0xC8,0x0D,0x1F,0x06,0xC3,0x13,0x0F,0x04,0xC8,0x11,0x0C,0x15,0xD6,0x03,0x75,0x52,0x9B,0x53,0x49,0x55,0x9F,0x40,0x49,0x15,0xD6,0x5A,0x40,0x6A};
            // char buff[] = {0x89,0x80,0x37,0xFA,0x21,0x3D};
            // char buff[] = {0x88,0x80,0x37,0xFA,0x21,0x3D};
            char buff[8] = {0x88,0x80,0x37,0xFA,0x21,0x3D};
            client.write(buff,sizeof(buff));
            flush_ether_rx_buff();
            ether_wsdis_cmd = ETHER_WSDIS_RSP;
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC,"\nwdis:cmd:");
            vUART_SendStr(UART_PC, (uint8_t*)buff);
#endif
        }
        break;

        case ETHER_WSDIS_RSP:
        {
            //uint8_t rx_buff[100];
            static uint16_t timeout,retry_cnt;
            //read response
            if(ethernet_rx_buff.locked)
            {
                ethernet_rx_buff.locked = UNLOCKED;
                //uint8_t size = client.available();
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\neWDR:");
                vUART_SendStr(UART_PC, (uint8_t*)ethernet_rx_buff.buffer);
                vUART_SendChr(UART_PC, ',');
                vUART_SendInt(UART_PC, ethernet_rx_buff.index);
#endif
//                 //size = client.read((uint8_t *)&ethernet_rx_buff.buffer,size);
//                 //ethernet_rx_buff.index += size;
//                 if(ethernet_pong_received((uint8_t *)ethernet_rx_buff.buffer))
//                 {
// #ifdef DEBUG_ETHERNET
//                     vUART_SendStr(UART_PC,"\npong_recv");
// #endif
//                     sts = ETHER_WS_DISCON_PASS;
//                     //ether_wsdis_cmd = ETHER_HRT_CMD;
//                     flush_ether_rx_buff();  //PP added on 15-05-24: as this was not being cleared till the next cmd and since it contains 0x8A, 0x00. 0x00 reamaining in the buffer caused parsing issues when we recieved a server_request() after this.
//                 }
//                 else
//                 {
// #ifdef DEBUG_ETHERNET
//                     vUART_SendStr(UART_PC,"\npong_not_recv");
// #endif
//                     //sts = ETHER_PING_PASS;
//                     sts = ETHER_WS_DISCON_FAIL;
//                     ether_wsdis_cmd = ETHER_WSDIS_CMD;
//                 }
//                 //free(rx_buff);
//                 ether_wsdis_cmd = ETHER_WSDIS_CMD;
// #ifdef DEBUG_ETHERNET
//                 vUART_SendStr(UART_PC,"\nping_snd:rsp:");
//                 vUART_SendBytes(UART_PC,(const uint8_t *)&ethernet_rx_buff.buffer, ethernet_rx_buff.index);
// #endif

                if(((ethernet_rx_buff.buffer[0] & 0xFF) == 0x88) && ((ethernet_rx_buff.buffer[1] & 0xFF) == 0x02) 
                && ((ethernet_rx_buff.buffer[2] & 0xFF) == 0x03) && ((ethernet_rx_buff.buffer[3]) & 0xFF) == 0xE8)
                {
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC, "\neWDk:");
                    vUART_SendStr(UART_PC, (uint8_t*)ethernet_rx_buff.buffer);
                    vUART_SendChr(UART_PC, ',');
                    vUART_SendInt(UART_PC, ethernet_rx_buff.index);
#endif
                    timeout = 0;
                    retry_cnt = 0;
                    sts = ETHER_WS_DISCON_PASS;
                    ether_wsdis_cmd = ETHER_WSDIS_CMD;
                    flush_ether_rx_buff();
                }
                else
                {
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC, "\neWDf:");
                    vUART_SendStr(UART_PC, (uint8_t*)ethernet_rx_buff.buffer);
                    vUART_SendChr(UART_PC, ',');
                    vUART_SendInt(UART_PC, ethernet_rx_buff.index);
#endif
                    timeout = 0;
                    retry_cnt = 0;
                    sts = ETHER_WS_DISCON_FAIL;
                    ether_wsdis_cmd = ETHER_WSDIS_CMD;
                }
                
            }
            else
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\neWDW:");
                vUART_SendStr(UART_PC, (uint8_t*)ethernet_rx_buff.buffer);
                vUART_SendChr(UART_PC, ',');
                vUART_SendInt(UART_PC, ethernet_rx_buff.index);
#endif
                if(++timeout >= ((10*1000)/50))
                {
                    timeout = 0;
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC,"\nWD:rsp:to");
#endif
                    ether_wsdis_cmd = ETHER_WSDIS_CMD;
                    if(++retry_cnt >= 3)
                    {
                        retry_cnt = 0;
                        sts = ETHER_WS_DISCON_FAIL;
#ifdef DEBUG_ETHERNET
                        vUART_SendStr(UART_PC,"\nWD:rsp:rf");
#endif
                    }
                }
            }
        }
        break;
    }   //switch-case end
    return sts;
}

char ethernet_pong_received(uint8_t *tmpstr)
{
    char sts;

    if((tmpstr[0] & 0xFF) == 0x8A)
    {
        sts = TRUE;
    }
    else
    {
        sts = FALSE;
    }


    return sts;
}

char ether_valid_code(char *tmpstr)
{
    if(tmpstr == NULL)
    {
        return FALSE;
    }
#ifdef DEBUG_ETHERNET
    vUART_SendStr(UART_PC,"\ntmpstr:");
    // vUART_SendStr(UART_PC,(const uint8_t *)tmpstr);
    vUART_SendBytes(UART_PC, (uint8_t*)tmpstr, strlen((const char*)tmpstr));
#endif
    char temp_buff[5];
    int i=0;
    memset(temp_buff,0,sizeof(temp_buff));
    while(tmpstr[i] != 0x20)
    {
#ifdef DEBUG_ETHERNET
    vUART_SendChr(UART_PC,'\n');
    vUART_SendChr(UART_PC, tmpstr[i]);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, i);
#endif
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


void flush_ether_rx_buff(void)
{
    // memset((void *)&ethernet_rx_buff,0,sizeof(gprs_rx_data_buff_t));
    memset((void *)&ethernet_rx_buff,0,sizeof(ethernet_rx_buff_t));
}

void flush_ether_tx_buff(void)
{
    // memset((void *)&ethernet_tx_buff,0,sizeof(gprs_tx_data_buff_t));
    memset((void *)&ethernet_tx_buff,0,sizeof(ethernet_tx_buff_t));
}

ether_tcp_packet_status_t ether_tcp_send(char *data_str, int len)
{
    // static ether_tcp_pckt_state_t ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
    ether_tcp_packet_status_t ether_tcp_packet_status = ETHER_TCP_SEND_IN_PRG;
    static char retry_cnt = 0;
    static unsigned int timeout = 0;
    // switch(ether_tcp_pckt_state)
    switch(Telecom_Ethernet.ether_tcp_pckt_state)
    {
        case ETHER_TCP_SEND_CMD:
        {
            client.write(data_str,len);
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC,"\ntcp_cmd:");
            vUART_SendBytes(UART_PC,(const uint8_t *)data_str, len);
#endif
            flush_ether_rx_buff();
//             if(get_upload_data() == RESPONSE)
//             {
// #ifdef DEBUG_GPRS_DATA_UPLOAD
//                 vUART_SendStr(UART_PC,"TCP_SEND_PACKET_CMD:k");
//                 vUART_SendStr(UART_PC,"\nit_is_response_data");
// #endif
//                 ether_tcp_packet_status = ETHER_TCP_SEND_PASS;
//                 // ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
//                 Telecom_Ethernet.ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
//
//                 set_upload_data(REQUEST);
//             }
//             else
//             {
// 				// ether_tcp_pckt_state = ETHER_TCP_SEND_RSP;
//                 Telecom_Ethernet.ether_tcp_pckt_state = ETHER_TCP_SEND_RSP;
//             }
            Telecom_Ethernet.ether_tcp_pckt_state = ETHER_TCP_SEND_RSP;  
        }
        break;

        case ETHER_TCP_SEND_RSP:
        {
            if(ethernet_rx_buff.locked)
            {
                ethernet_rx_buff.locked = UNLOCKED;
                // uint8_t size = client.available();
                // size = client.available();
                // size = client.read((uint8_t *)&ethernet_rx_buff.buffer,size);
                // ethernet_rx_buff.index += size;
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\ntcp:rsp:");
                vUART_SendBytes(UART_PC,(const uint8_t *)ethernet_rx_buff.buffer, ethernet_rx_buff.index);
#endif

#ifdef DEBUG_GPRS_DATA_UPLOAD
                vUART_SendStr(UART_PC,"TCP_SEND:k2");
#endif
                flush_ether_rx_buff();  //PP added on 15-05-24: as this was not being cleared till 30secs and server_query() called in check_ethernet_msg() was parsing it again as it found a brace every time.
                ether_tcp_packet_status = ETHER_TCP_SEND_PASS;
                // ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
                Telecom_Ethernet.ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
#if 0
                if(get_upload_data() == RESPONSE)
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"TCP_SEND_PACKET_CMD:k");
                    vUART_SendStr(UART_PC,"\nit_is_response_data");
#endif
                    ether_tcp_packet_status = ETHER_TCP_SEND_PASS;
                    ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;

                    set_upload_data(REQUEST);
                }
                else
                if(get_upload_data() == REQUEST)
                {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                    vUART_SendStr(UART_PC,"TCP_SEND_PACKET_CMD:k");
                    vUART_SendStr(UART_PC,"\nit_is_req_data");
#endif
                    if(ether_respos_recvd((char *)ethernet_rx_buff.buffer))
                    {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                        vUART_SendStr(UART_PC,"TCP_SEND_PACKET_RSP_1:k");
#endif

#if 0
                        Ethernet_Server_Response();
#endif  //if 0
                        ether_tcp_packet_status = ETHER_TCP_SEND_PASS;
                        ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
                    }
                    else
                    {
#ifdef DEBUG_GPRS_DATA_UPLOAD
                        vUART_SendStr(UART_PC,"\nResp_not_recvd");
#endif
                        ether_tcp_packet_status = ETHER_TCP_SEND_FAIL;
                        ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
                    }
                }
                //free(rx_buff);
#endif  //if 0
            }
            else
            {
                if(++timeout >= ((10*1000)/50))
                {
                    timeout = 0;
#ifdef DEBUG_ETHERNET
                    vUART_SendStr(UART_PC,"\ntcp:rsp:to");
#endif
                    // ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
                    Telecom_Ethernet.ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
                    if(++retry_cnt >= 3)
                    {
                        retry_cnt = 0;
                        ether_tcp_packet_status = ETHER_TCP_SEND_FAIL;
#ifdef DEBUG_ETHERNET
                        vUART_SendStr(UART_PC,"\ntcp:rsp:rf");
#endif
                    }
                }
            }
        }
        break;

        default:
        break;
    }
    return ether_tcp_packet_status;
}

#if 0
void Ethernet_Server_Response(void)
{
    uint8_t *result;

    result = (uint8_t *)strstr((char *)ethernet_rx_buff.buffer,"[");
// #ifdef DEBUG_RANDOM
//     vUART_SendStr(UART_PC,"gprs_recv:");
//     vUART_SendBytes(UART_PC,result,sizeof(ethernet_rx_buff.index));
// #endif

    if(result/*[0]=='['*/)
    {
#ifdef DEBUG_GPRS_RX
        vUART_SendStr(UART_PC,"found_result");
        // vUART_SendBytes(UART_PC,result,sizeof(result));
#endif
        EVSE_OCPP_CMS_DATA_Filter();
    }
    else
    {
#ifdef DEBUG_GPRS_RX
        vUART_SendStr(UART_PC,"not_found_result");
        // vUART_SendBytes(UART_PC,result,sizeof(result));
#endif
       // memset((void *)ethernet_rx_buff.buffer, 0, strlen((char *)ethernet_rx_buff.buffer));
       // ethernet_rx_buff.index= 0;

    }
}
#endif  //if 0


void ether_websocket_packet(uint8_t *request)
{
    uint8_t opcode=0x01;
    uint16_t Length;//,index = 0;
    // uint8_t Data[500];
    // uint8_t FinalBuff[510];
    uint8_t maskKey[4];
    int i;
    //flushTxBuffer(LTE_UART);
    flush_ether_tx_buff();

#ifdef DEBUG_ETHERNET
    vUART_SendStr(UART_PC,"\nreq:");
    vUART_SendStr(UART_PC,request);
#endif
    generateMaskKey(maskKey);

#ifdef DEBUG_WEB_PREP
    vUART_SendStr(UART_PC,"mask_key:");
    vUART_SendBytes(UART_PC,(const uint8_t *)maskKey,sizeof(maskKey));
#endif

    Length = strlen((char *)request);

#ifdef DEBUG_WEB_PREP
    vUART_SendStr(UART_PC,"Length:");
    vUART_SendInt(UART_PC,Length);
#endif

    ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = opcode|0x80;

    if(Length<126)
    {
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = Length|0x80;
    }
    else
    {
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = 0xFE;
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = ((Length >> 8) & 0xFF);
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = (Length & 0xFF);
    }

    memcpy((void*)&ethernet_tx_buff.buffer[ethernet_tx_buff.index],maskKey,sizeof(maskKey));
    ethernet_tx_buff.index += sizeof(maskKey);

    for(i=0; i<Length;i++)
    {
        ethernet_tx_buff.buffer[ethernet_tx_buff.index++] = request[i]^maskKey[i%4];
    }

#ifdef DEBUG_WEB_PREP
    vUART_SendStr(UART_PC,"\n ethernet_tx_buff.buffer:");
    vUART_SendBytes(UART_PC,(const uint8_t *)ethernet_tx_buff.buffer,ethernet_tx_buff.index);
#endif

}

char ether_respos_recvd(char *buff)
{
//    int i = 0;
//
//    if(buff[i] == 0x81)
//    {
//        return TRUE;
//    }
//    else
//    {
//        return FALSE;
//    }
    return get_pkt_recv();
}

void set_ethernet_connct_sts(uint8_t sts)
{
    // ether_connect_sts = sts;
    Telecom_Ethernet.ether_connect_sts = sts;
}
uint8_t get_ethernet_connct_sts(void)
{
    // return ether_connect_sts;
    return Telecom_Ethernet.ether_connect_sts;
}

void set_ethernet_NWstatus(uint8_t sts)
{
    // ether_network_sts = sts;
    Telecom_Ethernet.ether_network_sts = sts;
}
uint8_t get_ethernet_NWstatus(void)
{
    // return ether_network_sts;
    return Telecom_Ethernet.ether_network_sts;
}

bool get_pkt_recv(void)
{
    return Telecom_Ethernet.pkt_recv;
}

void set_pkt_recv(bool pktRecv)
{
    Telecom_Ethernet.pkt_recv = pktRecv;
}


void check_ethernet_message(void)
{
    // char temp_buff[FIRMWARE_PACKET_LEN+1];
    uint8_t *result;
    uint8_t shift_bytes = 0;
    static uint8_t retry_coun = 0;
    // char temp_buff[550];
    char temp_buff[ETHER_RX_BUFFER_MAX];
    uint16_t size = 0;
    // uint16_t len = 0;
    size = client.available();

    if (size > 0)
    {
#ifdef DEBUG_ETHERNET
        vUART_SendStr(UART_PC, "\navbl_size:");
        vUART_SendInt(UART_PC, size);
#endif
        while(1)
        {
            memset(temp_buff,0,sizeof(temp_buff));

#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC, "\nA");
#endif

            // client.read((uint8_t*) &ethernet_rx_buff.buffer[ethernet_rx_buff.index], size);
            // client.read((uint8_t*)temp_buff, sizeof(temp_buff)-1);
            if(size)
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC, "\nB");
#endif
                client.read((uint8_t*)temp_buff, size);
                // client.read((uint8_t*)temp_buff, RX_BUFFER_MAX);
               
                memcpy((void *)&ethernet_rx_buff.buffer[ethernet_rx_buff.index],temp_buff,size);
#ifdef DEBUG_ETHERNET
                // vUART_SendStr(UART_PC, "\ntcp_recv_len:");
                // vUART_SendInt(UART_PC, ethernet_rx_buff.index);
                // vUART_SendStr(UART_PC, "\ntcp_recv:");
                // vUART_SendBytes(UART_PC, (const uint8_t*) ethernet_rx_buff.buffer,ethernet_rx_buff.index);
#endif
            }

// #ifdef DEBUG_ETHERNET
//             vUART_SendStr(UART_PC, "\ntemp_buff");
//             vUART_SendStr(UART_PC,(const uint8_t *) temp_buff);
// #endif
//             if(temp_buff[0] != '\0')
//             {
//
//                 //memcpy((void *)&ethernet_rx_buff.buffer[ethernet_rx_buff.index],temp_buff,strlen(temp_buff));
//                 ethernet_rx_buff.index += strlen(temp_buff);
//             }
//             if (!strlen(temp_buff))
//             {
//                 break;
//             }
            ethernet_rx_buff.index += size;
            // len += size;

#ifdef DEBUG_ETHERNET
        //    vUART_SendStr(UART_PC, "\nether_rx_index:");
        //    vUART_SendInt(UART_PC, ethernet_rx_buff.index);
        //    vUART_SendStr(UART_PC, "\nether_rx_buff:");
        //    vUART_SendBytes(UART_PC,(const uint8_t *) ethernet_rx_buff.buffer,ethernet_rx_buff.index);
#endif

            size = 0;
            size = client.available();

            if(size)
            {

            }
            else if(++retry_coun > 3)
            {
#ifdef DEBUG_ETHERNET
                // vUART_SendStr(UART_PC, "\nnot_found_avbl_size:");
                vUART_SendStr(UART_PC, "\nno_avbl_size:");
                vUART_SendStr(UART_PC, "\neri:");
                vUART_SendInt(UART_PC, ethernet_rx_buff.index);
                vUART_SendStr(UART_PC, "\nerb:");
                vUART_SendBytes(UART_PC,(const uint8_t *) ethernet_rx_buff.buffer,ethernet_rx_buff.index);
#endif
                retry_coun = 0;
                break;
            }

        }
        // gprs.gprs_rx_buff_len = ethernet_rx_buff.index;  //PP commented on 04-05-24: 19-04
        Telecom_Ethernet.ether_rx_buff_len = ethernet_rx_buff.index; 

        if(ethernet_rx_buff.buffer[0] == 0x81)
        {
            if(ethernet_rx_buff.buffer[1] == 0x7E)
            {
                shift_bytes = 4;
            }
            else
            {
                shift_bytes = 2;
            }

            memcpy((void *)&ethernet_rx_buff.buffer,(void *)&ethernet_rx_buff.buffer[shift_bytes],ethernet_rx_buff.index);
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC, "\nether_1_rx_buff:");
            vUART_SendBytes(UART_PC, (const uint8_t*) ethernet_rx_buff.buffer,ethernet_rx_buff.index);
            vUART_SendChr(UART_PC, ',');
            vUART_SendInt(UART_PC, Telecom_Ethernet.ether_rx_buff_len);
            vUART_SendStr(DEBUG_UART_BASE, "\nTCP_SEND:k1");
#endif
            ethernet_rx_buff.index -= shift_bytes;
            set_pkt_recv(TRUE);

        }
        else
        {
#ifdef DEBUG_ETHERNET
           vUART_SendStr(UART_PC, "\nin_else");
#endif
            set_pkt_recv(FALSE);
        }

#ifdef DEBUG_ETHERNET
        vUART_SendStr(UART_PC, "\ntcp_recv:");
        vUART_SendBytes(UART_PC, (const uint8_t*) ethernet_rx_buff.buffer,ethernet_rx_buff.index);
#endif
        bool gotReq = server_query();
        if(gotReq)
        {
            HandleQueryStates();
        }
#if 0
        result = (uint8_t*) my_strstr((const char*) ethernet_rx_buff.buffer, "[",
                                      ethernet_rx_buff.index);

        if (result)
        {
            // GPRS_OCPP_CMS_DATA_Filter();
            ethernet_rx_buff.locked = LOCKED;
#ifdef DEBUG_ETHERNET
           vUART_SendStr(UART_PC, "\nresult:");
           vUART_SendStr(UART_PC, (const uint8_t*) result);
#endif

// #if 0
            Find_message_key_id((uint8_t*) result);
#ifdef DEBUG_ETHERNET
            vUART_SendStr(UART_PC, "\nmsg_type:");
            vUART_SendChr(UART_PC, CMS_message_type);
#endif
            if (CMS_message_type == '2')
            {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(DEBUG_UART_BASE, "cms_req:");
#endif
                memset(CMS_Req_Message_Id, 0, sizeof(CMS_Req_Message_Id));
                memcpy(CMS_Req_Message_Id, RX_Message_id,
                       sizeof(RX_Message_id));
                memset((void*) &gprs_resp_rx_buff, 0,
                       sizeof(gprs_rx_data_buff_t));
                memcpy((void*) &gprs_resp_rx_buff, (const void*) &ethernet_rx_buff,
                       sizeof(gprs_rx_data_buff_t));


                memset((void*) &ethernet_rx_buff.buffer, 0,
                       sizeof(gprs_rx_data_buff_t));
                set_upload_data(RESPONSE);
                if (ethernet_state != ETHER_SESSION_IDLE)
                {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether:sess_idl:f");
#endif
                    set_pending_request(1);
                }
                else
                {
#ifdef DEBUG_ETHERNET
                vUART_SendStr(UART_PC,"\nether:sess_idl:k");
#endif
                    set_pending_request(0);
                    OCPP_Server_Query_Message();
                }  
            }
            else
            {
                ethernet_rx_buff.locked = LOCKED;
            }
// #endif  // if 0

        }
        else
        {
            ethernet_rx_buff.locked = LOCKED;
        }
        //ethernet_OCPP_CMS_DATA_Filter();
#endif  //if 0
        ethernet_rx_buff.locked = LOCKED;
    }   // IF(size > 0)
}

char HTTP_valid_code(char *tmpstr)
{
    if(tmpstr == NULL)
    {
        return FALSE;
    }

// #ifdef DEBUG_ETHERNET
//    vUART_SendStr(UART_PC,"\ntmpstr:");
//    vUART_SendStr(UART_PC,(const uint8_t *)tmpstr);
// #endif

    char temp_buff[5];
    int i=0;
    memset(temp_buff,0,sizeof(temp_buff));
    while(tmpstr[i] != 0x20)
    {
        temp_buff[i] = tmpstr[i];
        i++;
    }
    if(!(strcmp(temp_buff,"200")))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void resetEther_SubHandlers(void)
{
    Telecom_Ethernet.ether_ws_cmd = ETHER_WS_CMD;
    Telecom_Ethernet.ether_ping_cmd = ETHER_PING_CMD;
    Telecom_Ethernet.ether_tcp_pckt_state = ETHER_TCP_SEND_CMD;
}









#endif  //ETHERNET_EN




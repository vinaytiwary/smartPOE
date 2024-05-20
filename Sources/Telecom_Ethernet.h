/*
 * Telecom_Ethernet.h
 *
 *  Created on: May 2, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_TELECOM_ETHERNET_H_
#define SOURCES_TELECOM_ETHERNET_H_

#include "_config.h"

#ifdef ETHERNET_EN

// #define ETHER_TX_BUFFER_MAX  (500)
// #define ETHER_RX_BUFFER_MAX  (128)
// // #define ETHER_RX_BUFFER_MAX  (500)

// #define ETHER_TX_BUFFER_MAX  (513)
// #define ETHER_RX_BUFFER_MAX  (513)

#define ETHER_TX_BUFFER_MAX  (2600)
#define ETHER_RX_BUFFER_MAX  (2600)

#define ETHER_CONN_RETRY_TIME    (10000/GPRS_STATE_MC_TIME)

typedef enum
{
    ETHER_INIT,
    ETHER_DHCP_CONN,
    ETHER_TCP_CONN,
    ETHER_WS_CONN,
    ETHER_WS_DISCONN,
    ETHER_PING,
    ETHER_PREPARE_LOGS,
    ETHER_LOG_UPLOAD,
    ETHER_SESSION_IDLE,
}ethernet_state_t;

typedef enum
{
    ETHER_TCP_CON_FAIL,
    ETHER_TCP_CON_PASS,
    ETHER_TCP_ALREADY_CONN,
    ETHER_TCP_CON_PRG,
}ether_tcp_sts_t;

typedef enum
{
    ETHER_WS_CON_FAIL,
    ETHER_WS_CON_PASS,
    ETHER_WS_CON_PRG,
}ether_ws_sts_t;

typedef enum
{
    ETHER_WS_CMD,
    ETHER_WS_RSP,
}ether_ws_cmd_t;

typedef enum
{
    ETHER_TCP_SEND_PASS,
    ETHER_TCP_SEND_FAIL,
    ETHER_TCP_SEND_IN_PRG
}ether_tcp_packet_status_t;

typedef enum
{
    ETHER_PING_PASS,
    ETHER_PING_FAIL,
    ETHER_PING_IN_PRG
}ether_ping_status_t;

typedef enum
{
    ETHER_WS_DISCON_FAIL,
    ETHER_WS_DISCON_PASS,
    ETHER_WS_DISCON_PRG,
}ether_wsdisconn_sts_t;

typedef enum
{
    ETHER_WSDIS_CMD,
    ETHER_WSDIS_RSP,
}ether_wsdis_cmd_t;

typedef enum
{
    ETHER_PING_CMD,
    ETHER_PING_RSP,
#if 0
    ETHER_HRT_CMD,
    ETHER_HRT_RSP,
#endif  //if 0
}ether_ping_cmd_t;

typedef enum
{
    ETHER_WEBSOCKET_PASS,
    ETHER_WEBSOCKET_FAIL,
    ETHER_WEBSOCKET_IN_PRG
}ether_websckt_sts_t;

typedef enum
{
    ETHER_TCP_SEND_CMD,
    ETHER_TCP_SEND_RSP,
}ether_tcp_pckt_state_t;

typedef struct  {
    char locked;
    char buffer[ETHER_TX_BUFFER_MAX];
    unsigned int index;
}ethernet_tx_buff_t;

typedef struct  {
    char locked;
    char buffer[ETHER_RX_BUFFER_MAX];
    unsigned int index;
    unsigned char elapsed;
}ethernet_rx_buff_t;

typedef struct
{
    ethernet_state_t ethernet_state;
    ether_ws_cmd_t ether_ws_cmd;
    ether_ping_cmd_t ether_ping_cmd;
    ether_tcp_pckt_state_t ether_tcp_pckt_state;
    bool ether_connect_sts;
    bool ether_network_sts;
    bool pkt_recv;
    unsigned int ether_rx_buff_len;
}__attribute__((packed))Telecom_Ethernet_t;

void vETHERNETSPIInit(void);

void ethernet_handler(void);

ether_tcp_sts_t ether_tcp_connect(void);

ether_ws_sts_t ws_connect(void);

ether_ping_status_t ether_ping_send(void);

ether_wsdisconn_sts_t ws_disconn(void);

char ethernet_pong_received(uint8_t *tmpstr);

char ether_valid_code(char *tmpstr);

void flush_ether_rx_buff(void);

void flush_ether_tx_buff(void);

ether_tcp_packet_status_t ether_tcp_send(char *data_str, int len);

void ether_websocket_packet(uint8_t *request);

char ether_respos_recvd(char *buff);

void set_ethernet_connct_sts(uint8_t sts);

uint8_t get_ethernet_connct_sts(void);

void set_ethernet_NWstatus(uint8_t sts);

uint8_t get_ethernet_NWstatus(void);

void check_ethernet_message(void);

char HTTP_valid_code(char *tmpstr);

void resetEther_SubHandlers(void);

void ethernet_init(void);

bool get_pkt_recv(void);

void set_pkt_recv(bool pktRecv);

#endif  //ETHERNET_EN
#endif /* SOURCES_TELECOM_ETHERNET_H_ */


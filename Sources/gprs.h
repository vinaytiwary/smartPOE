/*
 * gprs.h
 *
 *  Created on: Apr 20, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_GPRS_H_
#define SOURCES_GPRS_H_

#include <stdint.h>
#include "_config.h"

#define YEAR_OFFSET     (10)

#define GPRS_STATE_MC_TIME      (50)
#define TCP_STATE_MC_TIME       (50)

// #define GPRS_TX_BUFFER_MAX      (500)
// #define GPRS_RX_BUFFER_MAX      (128)
#define GPRS_TX_BUFFER_MAX      (500)
#define GPRS_RX_BUFFER_MAX      (500)

#define TEMP_BUFF_LEN           (300)

//#define IMEI_LEN          (16)
#define LOCKED              (1)
#define UNLOCKED            (0)
#define IS_SUCCESS          (1)
#define IS_FAILURE          (0)

#define lock(a) {a = LOCKED;}
#define unlock(a) {a = UNLOCKED;}

#define TCP_RETRY_CNT           (3)
#define GPRS_RETRY_CNT          (3)

#define CON_ERR_OFFSET      (0xA0)
#define TCP_ERR_OFFSET      (0xC0)

#define MIN_NETWORK_STRENGTH_DB     (2)
#define MAX_NETWORK_STRENGTH_DB     (30)

#define RETRY_CNT (3)
// #define ATE0_TIMEOUT	(200/GPRS_STATE_MC_TIME)
#define ATE0_TIMEOUT	(500/GPRS_STATE_MC_TIME)

#define GSM_WARMUP_SECS         ((10*1000)/GPRS_STATE_MC_TIME)

#define GPRS_CONN_RETRY_TIME    (10000/GPRS_STATE_MC_TIME)      //10Sec

#define TCP_RETRY_CNT           (3)
#define GPRS_RETRY_CNT          (3)

#define CPIN_TIMEOUT            (9000/GPRS_STATE_MC_TIME)       //
//#define CPIN_TIMEOUT            (4000/GPRS_STATE_MC_TIME)
#define CREG_TIMEOUT            (9000/GPRS_STATE_MC_TIME)
#define CGREG_TIMEOUT           (9000/GPRS_STATE_MC_TIME)
#define NETOPEN_TIMEOUT         (12000/GPRS_STATE_MC_TIME)
#define CSQ_TIMEOUT             (9000/GPRS_STATE_MC_TIME)       //
#define CPSI_TIMEOUT            (9000/GPRS_STATE_MC_TIME)       //
#define CGDCONT_TIMEOUT         (9000/GPRS_STATE_MC_TIME)
#define CCLK_TIMEOUT            (9000/GPRS_STATE_MC_TIME)      //

#define TCP_START_TIMEOUT       (4000/TCP_STATE_MC_TIME)
//#define TCP_CONN_TIMEOUT        (5000/TCP_STATE_MC_TIME)
//#define TCP_CONN_WAIT_TIMEOUT   (70000/TCP_STATE_MC_TIME)
#define TCP_SEND_TIMEOUT        (4000/TCP_STATE_MC_TIME)
//#define TCP_DATA_SEND_TIMEOUT   (5000/TCP_STATE_MC_TIME)
#define SESSION_IDLE_TIMEOUT    (4000/TCP_STATE_MC_TIME)

//#define WEBSOCKET_CIPSEND_FIXED_CNT     (10)
//#define WEBSOCKET_IPD_FIXED_CNT         (100)
//#define WEBSOCKET_LEN_CNT               (3)
//#define WEBSOCKET_MAX_LEN               (4)

//#define GPRS_CONFIG_MAX_DATA            (24)
//#define GPRS_IP_LEN                     (20)
//#define GPRS_PORT_LEN                 (4)
//#define GPRS_UNUSED_LEN                 (1)

typedef enum
{
    GPRS_NO_NEW_MSG,
    GPRS_MATCH_FAIL,
    GPRS_MATCH_OK
} match_t;


typedef enum
{
    NOT_AVBL,
    AVBL,
}gprs_status_t;

typedef enum
{
    REQUEST,
    RESPONSE
}upload_data_t;

typedef enum
{
    CON_OK,
    CON_FAIL,
    CON_IN_PRG
}con_status_t;

typedef enum
{
    TCP_PASS,
    TCP_FAIL,
    TCP_ALRDY_CONN,
    TCP_IN_PRG
}tcp_status_t;

typedef enum
{
    TCP_SEND_PASS,
    TCP_SEND_FAIL,
    TCP_SEND_IN_PRG
}tcp_packet_status_t;

typedef enum
{
    WS_DISCON_PASS,
    WS_DISCON_FAIL,
    WS_DISCON_IN_PRG
}WS_discon_status_t;

typedef enum
{
    TCP_DISCON_PASS,
    TCP_DISCON_FAIL,
    TCP_DISCON_IN_PRG
}TCP_discon_status_t;

typedef enum
{
    TCP_CIPSEND_CMD,
    TCP_CIPSEND_RSP,
    TCP_SEND_PACKET_CMD,
    TCP_SEND_PACKET_RSP,
    TCP_SEND_PACKET_RSP_1
}tcp_packet_state_t;

typedef enum
{
    PING_PASS,
    PING_FAIL,
    PING_IN_PRG
}ping_status_t;

typedef enum
{
    WEBSOCKET_PASS,
    WEBSOCKET_FAIL,
    WEBSOCKET_IN_PRG
}websckt_sts_t;

typedef enum
{
    GPRS_IDLE,
    GPRS_CONNECT,
    GPRS_CONN_STS,
    GPRS_TCP_CONNECT,
    GPRS_WEBSOCKET_CONNECT,
    GPRS_PING,
    GPRS_WEBSOCKET_DISCONNECT,
	GPRS_TCP_DISCONNECT,
    GPRS_PREPARE_LOGS,
    GPRS_LOGS_UPLOAD,
    GPRS_SESSION_IDLE,
}gprs_handler_state_t;

typedef enum
{
    GPRS_CONNCT_RESET,
    GPRS_CONNCT_CMD_ECHO_OFF,
    GPRS_CONNCT_RSP_ECHO_OFF,
    GPRS_CONNCT_CMD_CPIN,
    GPRS_CONNCT_RSP_CPIN,
    GPRS_CONNCT_CMD_CSQ,
    GPRS_CONNCT_RSP_CSQ,
    GPRS_CONNCT_CMD_CREG,
    GPRS_CONNCT_RSP_CREG,
    GPRS_CONNCT_CMD_CGREG,
    GPRS_CONNCT_RSP_CGREG,
    GPRS_CONNCT_CMD_CPSI,
    GPRS_CONNCT_RSP_CPSI,
    GPRS_CONNCT_CMD_CGDCONT,
    GPRS_CONNCT_RSP_CGDCONT
}gprs_connect_t;


typedef enum
{
    GPRS_CONNCT_CMD_CCLK,
    GPRS_CONNCT_RSP_CCLK,
    GPRS_CONNCT_CMD_READ_NETOPEN,
    GPRS_CONNCT_RSP_READ_NETOPEN,
    GPRS_CONNCT_CMD_NETOPEN,
    GPRS_CONNCT_RSP_NETOPEN,
}gprs_connect_state_t;

typedef enum
{
    GPRS_TCP_CMD_READ_CIPOPEN,
    GPRS_TCP_RSP_READ_CIPOPEN,
    GPRS_TCP_CMD_CIPOPEN,
    GPRS_TCP_RSP_CIPOPEN,
    GPRS_TCP_RSP_1_CIPOPEN
}gprs_tcp_state_t;

typedef enum
{
    /*GPRS_WEBSOCKET_PING_CMD_CIPSEND,
    GPRS_WEBSOCKET_PING_RSP_CIPSEND,
    GPRS_WEBSOCKET_PING_SEND_CMD,
    GPRS_WEBSOCKET_PING_SEND_RSP,
    GPRS_WEBSOCKET_PING_SEND_RSP_1,*/
    GPRS_WEBSOCKET_CONNCT_CMD_CIPSEND,
    GPRS_WEBSOCKET_CONNCT_RSP_CIPSEND,
    GPRS_WEBSOCKET_CONNCT_CMD,
    GPRS_WEBSOCKET_CONNCT_RSP,
    GPRS_WEBSOCKET_CONNCT_RSP_1
}gprs_websocket_connect_t;

typedef enum
{
    GPRS_WEBSOCKET_DISCONNCT_CMD_CIPSEND,
    GPRS_WEBSOCKET_DISCONNCT_RSP_CIPSEND,
    GPRS_WEBSOCKET_DISCONNCT_CMD,
    GPRS_WEBSOCKET_DISCONNCT_RSP,
    GPRS_WEBSOCKET_DISCONNCT_RSP_1,
}tcp_webskt_disconnect_states_t;

typedef enum
{
    GPRS_TCP_CMD_CIPCLOSE,
	GPRS_TCP_RSP_CIPCLOSE,
    GPRS_TCP_RSP_1_CIPCLOSE,
#ifdef USE_NETCLOSE
    GPRS_TCP_CMD_NETCLOSE,
    GPRS_TCP_RSP_NETCLOSE,
#endif  //USE_NETCLOSE
}tcp_disconnct_states_t;

typedef enum
{
   //WEBSOCKET_RX_IDLE,
   WEBSOCKET_RX_CIPSEND_PLUS,
   WEBSOCKET_RX_IPD_PLUS,
   //WEBSOCKET_RX_0D1,
   //WEBSOCKET_RX_0D2,
   //WEBSOCKET_RX_0A1,
   //WEBSOCKET_RX_0A2,
   WEBSOCKET_RX_DATA,
   WEBSOCKET_DATA_LEN,
   WEBSOCKET_RX_BODYREAD,
   WEBSOCKET_RX_OVERFLOW

}websocket_rx_state_t;

typedef enum
{
    PING_CMD_CIPSEND,
    PING_RSP_CIPSEND,
    PING_SEND_CMD,
    PING_SEND_RSP,
    PING_SEND_RSP_1
}ping_state_t;

typedef enum
{
    GPRS_RX_IDLE=0,
    GPRS_RX_INPROG,
} GPRS_rx_states;

typedef struct
{
    unsigned char elapsed;
    GPRS_rx_states state;
    // websocket_rx_state_t websocket_rx_state;
}gprs_rx_isr_handler_t;


typedef struct
{
    char locked;
    char buffer[GPRS_TX_BUFFER_MAX];
    unsigned int index;
}gprs_tx_data_buff_t;

typedef struct
{
    char locked;
    char buffer[GPRS_RX_BUFFER_MAX];
    unsigned int index;
}gprs_rx_data_buff_t;

typedef struct
{
    char locked;
    char *buff;
    unsigned int index;
}gprs_temp_rx_buff_t;

typedef struct
{
    char yy;
    char mm;
    char dd;
    char hr;
    char min;
    char sec;
    char update_time_aval;
}gprs_date_time_t;

typedef struct
{
    //gprs_status_t module_status;
    //gprs_status_t network_status;
    //char imei[IMEI_LEN];
    //unsigned char gprs_config_sts;
    //gprs_status_t server_status;

    gprs_handler_state_t gprs_handler_state;
    gprs_connect_t gprs_connect;
    gprs_connect_state_t gprs_connect_state;
    gprs_tcp_state_t gprs_tcp_state;
    gprs_websocket_connect_t gprs_websocket_connect;
    ping_state_t tcp_ping_states;
    tcp_packet_state_t tcp_packet_state;
    tcp_webskt_disconnect_states_t tcp_webskt_disconnect_states;
    tcp_disconnct_states_t tcp_disconnct_states;
    upload_data_t upload_data;
    unsigned char network_strength;
    unsigned char connect_sts;
    unsigned char tcp_sts;
    unsigned char websocket_sts;
    gprs_status_t NW_connSts;
	gprs_status_t serverConn_Sts;
//    char errcode;
    unsigned int gprs_rx_buff_len;
    //unsigned int data_length;
//    unsigned char data_mode;

} gprs_t;

//typedef struct
//{
//    uint8_t ip_addr[GPRS_IP_LEN];       //20
//    uint16_t port;                      //2
//    uint8_t path[]
////    char unused[GPRS_UNUSED_LEN];       //1
////    char cheksum;                       //1
//}gprs_config_t;                         //24

void TCP_Handler(void);
void resetTCP_SubHandlers(void);
con_status_t gprs_connect(void);
con_status_t gprs_connect_status(void);
tcp_status_t tcp_connect(void);
//tcp_status_t tcp_close(void);
//tcp_status_t tcp_send(char *, int);
tcp_packet_status_t tcp_send(char *data_str, int len);
websckt_sts_t websckt_connect(void);
WS_discon_status_t websckt_disconnect();
TCP_discon_status_t tcp_disconnect();
int get_rx_data(char *copy_here);
char check_string(const char *str, char *copy_here, int* numbytes);
char check_string_nobuf(const char *str);
//char getGPRSConfigErrSts(void);
//void setGPRSConfigErrSts(char sts);

char pong_received(char *tmpstr);

ping_status_t ping_send(void);
bool read_ip_port(char *tmpstr);
char match_cpsi_data(char *tmpstr);
char valid_code(char *tmpstr);

void generateMaskKey(uint8_t maskKey[4]);

char respos_recvd(char *tmpstr);
void set_webconn_sts(uint8_t sts);
uint8_t get_webconn_sts(void);
void GPRS_Server_Response(void);
void GPRS_Server_Request(void);
void GPRS_OCPP_CMS_DATA_Filter(void);
upload_data_t get_upload_data(void);
void set_upload_data(upload_data_t);

void set_gprs_connct_sts(uint8_t sts);
uint8_t get_gprs_connct_sts(void);

void setNWstatus(gprs_status_t sts);
gprs_status_t getNW_status(void);
void setServerStatus(gprs_status_t sts);
gprs_status_t getServerStatus(void);
// void prepare_websckt_data(char *ip, int *port, char *path);

#endif /* SOURCES_GPRS_H_ */

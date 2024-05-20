/*
 * Telecom_server_query.h
 *
 *  Created on: May 13, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_TELECOM_SERVER_QUERY_H_
#define SOURCES_TELECOM_SERVER_QUERY_H_

#include "gprs.h"

typedef enum
{
    NO_REQ,
    RESTART,
    ODU_VOLTAGE_UPDATE,
    SERVER_URL_UPDATE,
}server_request_type_t;

typedef enum
{
    SCHEDULED_LOG,
    RESPONSE_LOG,
}client_message_type_t;

typedef struct
{
    server_request_type_t server_request_type;
    client_message_type_t client_message_type;
    gprs_status_t send_request;
    bool pending_request;
}Telecom_server_query_t;

char *Query_decode(char *Query_String, const char *Query_data, char *destination, int max_len);

bool server_query(void);

unsigned int prepare_JSON_pckt(void);

unsigned int websocket_packet(uint8_t *request);

void HandleQueryStates(void);

void prepare_server_pkt(void);

void set_pending_request(bool);

bool get_pending_request(void);

void setREQmode(gprs_status_t sts);

gprs_status_t getREQmode(void);

void setServerReqType(server_request_type_t req_type);

server_request_type_t getServerReqType(void);

void setClientMSGType(client_message_type_t msg_type);

client_message_type_t getClientMSGType(void);

#endif /* SOURCES_TELECOM_SERVER_QUERY_H_ */

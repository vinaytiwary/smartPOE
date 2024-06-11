/*
 * flash_logger.h
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_FLASH_LOGGER_H_
#define SOURCES_FLASH_LOGGER_H_

#include <stdint.h>
#include "_config.h" 

#if defined(FLASH_EN)

#include "main.h"

#define TR_UNUSED_LEN   (9)
#define FREQ_UNUSED_LEN (12)

typedef enum
{
    SCHEDULED_FL_TLOG,
}FL_log_type_t;

typedef enum
{
    TELECOM_OFFLINE_LOGS,
}FL_unsent_logs_type_t;

typedef struct
{
    unsigned char  MR_BR_ser;                               // For Master/Backup Record Serial Number
    unsigned long curr_data_write_addr_telecom_log;
	unsigned long curr_upload_addr_telecom_log;
    char head_rear_position;
}FL_data_t; //10 Bytes

//We don't need to make an additional event type variable, just save a log in flash
//and send it to server whenever there's an alarm state change, because those kindof 
//logics (event type) can be implemented on server itself.
typedef struct
{
    // Alarm_event_t event_type; //enum
    ram_data_t ram_data;            //54 Bytes
    uint8_t dummy[TR_UNUSED_LEN];   //9 Bytes
    uint8_t chksum;                 //1 Byte
}__attribute__((packed)) FL_log_data_t; //64 Bytes

typedef struct
{
	unsigned char cnt;                      //1B
	// unsigned int unsent_telecom_logs_IOT;   //2B //iss zurm ke liye to tumhe fassi hogi fassi
    uint16_t unsent_telecom_logs_IOT;       //2B
	char unused[FREQ_UNUSED_LEN];           //12B		
	char chksum;                            //1B
}__attribute__((packed)) freq_updated_data_t;   //16 Bytes


unsigned long flashGetCurrDataWriteAddr(char type);
unsigned long flashGetCurrUploadAddr(char type);

int8_t flashGetSerMR_BR(void);

void flashWriteTR(void);

void clear_logs(void);

void clear_log_id(void);

void clear_flash(void);

unsigned long get_log_cnt(int trans_type);

unsigned long get_addr(int trans_type, unsigned long record_num);

int get_log_data(int trans_type, unsigned long addr, char *data);

void flashEraseFreqUpdDataSector(void);

void flashClearFreqUpdatedConfig(void);

void flashWriteFreqUpdData(freq_updated_data_t *log_data, unsigned long addr);

void flashWriteFreqUpdatedConfig(void);

void flashReadFreqUpdData(freq_updated_data_t *freq_updated_data, unsigned long addr, int size);

char readFreqUpdData_flash(void);

unsigned int decrement_unsent_log_cnt(int upload_code);

unsigned int increment_unsent_log_cnt(int trans_type);

char check_unsent_log(void);

void get_unsent_logs(void);

void prepare_OfflineTelecomData(void);

void save_TELECOM_data(void);

void save_OfflineTelecomData(void);

#endif  //FLASH_EN

#endif /* SOURCES_FLASH_LOGGER_H_ */


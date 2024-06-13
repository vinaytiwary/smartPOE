/*
 * E2P.h
 *
 *  Created on: Apr 25, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_E2P_H_
#define SOURCES_E2P_H_

#include "stdint.h"
#include "stdbool.h"
#include "driverlib/sysctl.h"

#include "driverlib/eeprom.h"
#include "_config.h"
#include "IO_cntrl.h"

#define DEVICE_ID_MAX_LEN	(15)
#define CLOUD_URL_MAX_LEN	(127)

#define IP_ADDR_LEN         (20)
#define PATH_LEN            (192)
#define PORT_LEN            (6)

#define E2P_VOLTGE_CONFIG_ADDR         (0X00)   //0 
#define E2P_CLOUD_CFG_ADDR             (0x08)   //8
#define E2P_DEVICE_CFG_ADDR            (0x88)   //136
#define E2P_DEVICE_LOC_ADDR            (0x98)   //152
#define E2P_CONFIG_TIME_ADDR           (0xA8)   //168



typedef struct 
{
  voltage_mode_t router1;    //1B
  voltage_mode_t router2;    //1B
  char reserved[5];         //5B
  char checksum;            //1B
}__attribute__((packed))e2p_router_config_t;    //8B

typedef struct
{
  char cloud_url[CLOUD_URL_MAX_LEN];        //127B
  char checksum;                            //1B
}__attribute__((packed))e2p_cloud_config_t; //128B

typedef struct
{
	uint8_t device_id[DEVICE_ID_MAX_LEN];  //15B
	uint8_t checksum;					   //1B
}__attribute__((packed))e2p_device_info_t; //16B

typedef struct
{
  int32_t latitude;     //4B
  int32_t longitude;    //4B
  char reserved[7];     //7B
  char checksum;        //1B
}__attribute__((packed))e2p_location_info_t;    //16B

typedef struct
{
  uint8_t config_time;  //1B
  uint8_t reserved[6];  //6B
  uint8_t checksum;     //1B
}__attribute__((packed))e2p_config_time_t;        //8bytes

typedef struct
{
   uint8_t ip_addr[IP_ADDR_LEN];
   uint16_t port_num;
   uint8_t path[PATH_LEN];
}__attribute__((packed))cloud_config_t;

void vPERIPH_E2PInit(void);

void e2p_write_voltage_config();

bool e2p_read_voltage_config();

void e2p_write_cloud_cfg(void);

bool e2p_read_cloud_cfg(void);
void e2p_write_device_cfg(void);
bool e2p_read_device_cfg(void);

void e2p_write_location(void);
bool e2p_read_location(void);
void e2p_write_cfg_time(void);
bool e2p_read_cfg_time(void);

char * get_url();

bool set_url(char *url);

bool set_device_id(char *id);

char *get_device_id(void);

bool set_fixed_location(char *location);

char *get_fixed_location();

#if 0
bool set_current_location();
#endif  // if 0

bool cloud_config_data(void);

#endif /* SOURCES_E2P_H_ */

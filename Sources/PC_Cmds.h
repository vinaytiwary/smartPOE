/*
 * PC_Cmds.h
 *
 *  Created on: Apr 25, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_PC_CMDS_H_
#define SOURCES_PC_CMDS_H_


#define LEN_INDX    1
#define CMD_INDX    2
#define DATA_INDX   3

#define ACK					(0x03)     
#define NACK				(0x04)
#define ERROR_INVALID_CMD	(0x05)
#define ERROR_INVALID_STATE	(0xA1)

typedef enum
{
    RUNNING_MODE,
    CONFIG_MODE,
}sys_mode_t;

typedef enum
{
	SET_MODE           = 0x01,
	SET_GPRS_URL       = 0x02,
	GET_GPRS_URL       = 0x03,
	SET_DEVICE_ID      = 0x04,
	GET_DEVICE_ID      = 0x05,
    SET_FIXED_LOC      = 0x06,
    GET_FIXED_LOC      = 0x07,	
    SET_ROUTER_CFG     = 0x08,
    GET_ROUTER_CFG     = 0x09,
    SET_LCD_UPD_TIME   = 0x0A,  
    GET_LCD_UPD_TIME   = 0x0B, 
    SET_CURRENT_LOC    = 0x0C,
    FACTORY_RESET      = 0x0D,
    SET_RTC_DATETIME   = 0x0E,
    GET_RTC_DATETIME   = 0x0F,
#ifdef FLASH_EN
    WRITE_FLASH        = 0x10,
    READ_FLASH         = 0x11,
    ERASE_SECTOR_FLASH = 0x12,
    READ_FLASH_STATUS  = 0x13,
    FLASH_INIT         = 0x14,
    CLEAR_LOGS         = 0x15,
    ADD_DUMMY_TELECOM_FL_LOGS = 0x16,
#endif //FLASH_EN
}cmd_t;


void decodeMsgPC_Uart(void);
void response(cmd_t cmd, int uart_no);
void tx_pending_dataPC(void);
void preparePC_ResponsePacket(cmd_t cmd, char *data, unsigned char len, int sendAsIs, int immediateSend);

void set_system_state(sys_mode_t state);

sys_mode_t get_system_state(void);


#endif /* SOURCES_PC_CMDS_H_ */

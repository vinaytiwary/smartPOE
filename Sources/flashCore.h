/*
 * flash.h
 *
 *  Created on: Apr 23, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_FLASHCORE_H_
#define SOURCES_FLASHCORE_H_

#include "_config.h"

#include <stdint.h>

#if defined(FLASH_EN)

#ifndef SST25VF064C
#define SST26VF064B
#endif

#define FLASH_WP_ENABLE

#define FLASH_READ_TIMEOUT      (100)
#define SECTOR_SIZE       ((unsigned long)0x1000)

#define MAX_NUM_OF_TELECOM_LOGS	(0x2D77)	//(11640-1) = 11639 or 0x2D77.

// #ifdef OFFLINE_MODE_EN
// #define MR_BR_UNUSED_LEN (54)
// // #define MR_BR_UNUSED_LEN (38)
// #else
// #define MR_BR_UNUSED_LEN (54)
// // #define MR_BR_UNUSED_LEN (50)
// #endif

#ifdef OFFLINE_MODE_EN
#define MR_BR_UNUSED_LEN (22)
// #define MR_BR_UNUSED_LEN (38)
#else
#define MR_BR_UNUSED_LEN (22)
// #define MR_BR_UNUSED_LEN (50)
#endif

#define FL_MR_BR_LEN		(32)								/* Master/Backup Packet Length */

#define FL_MAX_MR_ENTRIES	(200)
#define FL_MR_START_ADDR	((unsigned long)0x00)
#define FL_MR_MAX_ADDR		(FL_MR_START_ADDR + (FL_MAX_MR_ENTRIES * FL_MR_BR_LEN))							//0x1900

#define FL_MR_BKP_ADDR		((unsigned long)0x2000)
#define FL_MR_BKP_MAX_ADDR	(FL_MR_BKP_ADDR + (FL_MAX_MR_ENTRIES * FL_MR_BR_LEN))							//0x3900

#define FL_TLOG_LEN			(64)					//change from 32 to 64		//HJ 11-07-2017
#define FL_TLOG_START_ADDR	((unsigned long)0x4000)
#define FL_MAX_TLOG_ADDR	(((unsigned long)MAX_NUM_OF_TELECOM_LOGS * FL_TLOG_LEN) + FL_TLOG_START_ADDR)	//0xB9DC0

//Frequently updated data moved from eeprom to flash // KP 20-4-2018
#define FL_FREQ_UPD_DATA_SIZE			(16)
#define FL_FREQ_UPD_DATA_MAX_ENTRIES	(100)
#define FL_FREQ_UPD_DATA_START_ADDR		((unsigned long)0xBA000)		//Reserve one sector fto store circular buffer for frequently updated data
#define FL_FREQ_UPD_DATA_MAX_ADDR		((unsigned long)(FL_FREQ_UPD_DATA_START_ADDR + (FL_FREQ_UPD_DATA_SIZE * FL_FREQ_UPD_DATA_MAX_ENTRIES)))		//0xBA640

typedef struct
{
	unsigned char pkt_ser;
	unsigned long curr_data_write_addr_telecom_log;
	unsigned long curr_upload_addr_telecom_log;
	char dummy[MR_BR_UNUSED_LEN];
	char pkt_data_chksm;
} __attribute__((packed)) MR_BR_t;

void CS_Low(void);
void CS_High(void);
void Reset_Hold_Low(void);
void Reset_Hold_High(void);
void WP_Low(void);
void WP_High(void);
void flash_port_assign(void);
void init_flash_pins(void);
void enable_wp_pin(void);
void flashSwReset(void);
void WRCR(char byte);
void WRDI(void);
void WREN(void);
void CE_Low();
void CE_High();
void SCK_Low();
void SCK_High();
void flashPacketProgram(char* buff, uint16_t size, unsigned long addr);
void readContToBuff(unsigned long Dst, unsigned int no_bytes, char* buff);
void remove_block_protection(void);
void flashEraseSector(unsigned long, unsigned long);
void flashCopyMR(char *, char *);
void Sector_Erase(unsigned long Dst);
void SetSI_Output();
void SetSI_Input();
void SetSO_Output();
void SetSO_Input();
void WBPR(char state);
void Wait_Busy();
void SI_Low();
int flashInit(void);
unsigned char Read_Status_Register();
unsigned char Get_Byte();
void Send_Byte(unsigned char out);
void SI_High();
void Chip_Erase();
void flashClrChargingLogs(void);
#ifdef OFFLINE_MODE_EN
void flashClrCmdLogs(void);
void flashClrFreqUpdLogs(void);
#endif

void addDummyFL_TelecomLogs(unsigned int no_of_logs);

void flashEraseMaster(void);                                //komal
void flashEraseBackup(void);
int updateFlashCurrAddr(void);
void flashClrTR_Logs(void);



#endif	//FLASH_EN



#endif /* SOURCES_FLASHCORE_H_ */

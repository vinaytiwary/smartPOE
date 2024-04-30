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
#ifdef OFFLINE_MODE_EN
#define MR_BR_UNUSED_LEN (38)
#else
#define MR_BR_UNUSED_LEN (50)
#endif

#define TOTALIZER_SIZE          (8)
#define TOTALIZER_MAX_ENTRIES   (100)
#define TOTALIZER_START_ADDR    ((unsigned long)0x0)
#define TOTALIZER_MAX_ADDR      ((unsigned long)(TOTALIZER_START_ADDR + (TOTALIZER_SIZE * TOTALIZER_MAX_ENTRIES)))  //0x320

#define CHG_PROFILE_SIZE         (0x40)
#define CHG_PROFILE_MAX_ENTRIES   (0x7F)
#define CHG_PROFILE_START_ADDR    ((unsigned long)0x1000)
#define CHG_PROFILE_MAX_ADDR      ((unsigned long)(CHG_PROFILE_START_ADDR + (CHG_PROFILE_SIZE * CHG_PROFILE_MAX_ENTRIES)))    //2FC0

#define CHG_SCH_PERIOD_SIZE         (0x10)
#define CHG_SCH_PERIOD_MAX_ENTRIES   (0x1FF)
#define CHG_SCH_PERIOD_START_ADDR    ((unsigned long)0x3000)
#define CHG_SCH_PERIOD_MAX_ADDR      ((unsigned long)(CHG_SCH_PERIOD_START_ADDR + (CHG_SCH_PERIOD_SIZE * CHG_SCH_PERIOD_MAX_ENTRIES)))    //4FF0
#define FL_MR_BR_LEN        (64)                                /* Master/Backup Packet Length */
#define FL_MAX_MR_ENTRIES   (100)
#define FL_MR_START_ADDR    ((unsigned long)0x6000)
#define FL_MR_MAX_ADDR      (FL_MR_START_ADDR + (FL_MAX_MR_ENTRIES * FL_MR_BR_LEN))//7900

#define FL_MR_BKP_ADDR      ((unsigned long)0x8000)
#define FL_MR_BKP_MAX_ADDR  (FL_MR_BKP_ADDR + (FL_MAX_MR_ENTRIES * FL_MR_BR_LEN)) //9900

#define FL_FREQ_UPD_DATA_SIZE           (16)
#define FL_FREQ_UPD_DATA_MAX_ENTRIES    (100)
#define FL_FREQ_UPD_DATA_START_ADDR     ((unsigned long)0xA000)       //Reserve one sector fto store circular buffer for frequently updated data
#define FL_FREQ_UPD_DATA_MAX_ADDR       ((unsigned long)(FL_FREQ_UPD_DATA_START_ADDR + (FL_FREQ_UPD_DATA_SIZE * FL_FREQ_UPD_DATA_MAX_ENTRIES))) //A640

#define FL_CMD_LEN           (64)       //KOMAL
#define MAX_NUM_OF_CMD_LOGS    (0x1388)//5000
#define FL_CMD_START_ADDR    ((unsigned long)0xB000)
#define FL_MAX_CMD_ADDR      (((unsigned long)MAX_NUM_OF_CMD_LOGS * FL_CMD_LEN) + FL_CMD_START_ADDR)//0x59200


#define FL_CR_LEN           (64) //change from 32 to 64
#define MAX_NUM_OF_CHAR_LOGS    (0x5DC)//(0x7FFF)
#define FL_CR_START_ADDR    ((unsigned long)0x5A000)
#define FL_MAX_CR_ADDR      (((unsigned long)MAX_NUM_OF_CHAR_LOGS * FL_CR_LEN) + FL_CR_START_ADDR)//0x71700

#ifdef OFFLINE_MODE_EN

#define FL_LOCAL_LIST_LEN          (32)
#define FL_NUM_OF_LOCAL_LIST        (0xFF)

#define FL_LOCAL_LIST_START_ADDR   ((unsigned long)0x72000)
#define FL_LOCAL_LIST_MAX_ADDR     (((unsigned long)FL_NUM_OF_LOCAL_LIST * FL_LOCAL_LIST_LEN) + FL_LOCAL_LIST_START_ADDR)//0x73FE0(if FL_NUM_OF_LOCAL_LIST=24)

#define FL_LOCAL_LIST_BKP_START_ADDR   ((unsigned long)0x74000)
#define FL_LOCAL_LIST_BKP_MAX_ADDR     (((unsigned long)FL_NUM_OF_LOCAL_LIST * FL_LOCAL_LIST_LEN) + FL_LOCAL_LIST_BKP_START_ADDR)
//75FE0(if FL_NUM_OF_LOCAL_LIST=24)
#endif

#define FL_HEX_PKT_LEN                  (0x40)
#define FL_HEX_PKT_MAX_NUM              (unsigned long)(0x2000)
#define FL_HEX_FILE_START_ADDR          ((unsigned long)0x300000)
#define FL_MAX_HEX_FILE_ADDR            ((unsigned long)(FL_HEX_FILE_START_ADDR + (FL_HEX_PKT_LEN * FL_HEX_PKT_MAX_NUM)))//0x380000

typedef struct                                                                                  //komal
{
    unsigned char pkt_ser;                                                       //1
    unsigned long curr_data_write_addr_charging_log;                             //4
    unsigned long curr_upload_addr_charging_log;                                 //4
    unsigned long transaction_id_charging_log;                                   //4
#ifdef OFFLINE_MODE_EN
    unsigned long curr_data_write_addr_cmd_log;                         //4
    unsigned long curr_upload_addr_cmd_log;                             //4
    unsigned long transaction_id_cmd_log;                               //4
#endif
    char dummy[MR_BR_UNUSED_LEN];                                                //38 or 50
    char pkt_data_chksm;                                                         //1
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
void addDummyChargingLogs(unsigned int);

void flashEraseMaster(void);                                //komal
void flashEraseBackup(void);
int updateFlashCurrAddr(void);
#endif



#endif /* SOURCES_FLASHCORE_H_ */

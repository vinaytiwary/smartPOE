/*
 * flash_logger.cpp
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */
#include <stdio.h>
#include "_config.h"
#if defined(FLASH_EN)
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "flash_logger.h"
#include "flashCore.h"
#include "_common.h"
#include "_debug.h"
#include "UartCore.h"
#include "main.h"

#include "gprs.h"
#include "Telecom_Ethernet.h"

static FL_data_t FL_data = {0,FL_TLOG_START_ADDR,FL_TLOG_START_ADDR};

FL_log_data_t FL_log_data;
FL_log_data_t FLR_log_data;

freq_updated_data_t freq_updated_data;

ram_data_t ram_data;

void flashWriteMR_BR(void)
{
    MR_BR_t MR;
    if(FL_data.MR_BR_ser >= FL_MAX_MR_ENTRIES)		//Rollover of Master + Backup Flash Memory Logs
    {
            flashEraseMaster();
            flashEraseBackup();
            FL_data.MR_BR_ser = 0;
    }

	MR.pkt_ser = FL_data.MR_BR_ser;
	
	MR.curr_data_write_addr_telecom_log = FL_data.curr_data_write_addr_telecom_log;
	
	MR.curr_upload_addr_telecom_log = FL_data.curr_upload_addr_telecom_log;

	MR.pkt_data_chksm = getChecksum((unsigned char*)&MR, FL_MR_BR_LEN /*- MR_BR_UNUSED_LEN*/ - 1);
	
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)&MR, FL_MR_BR_LEN, ((((unsigned long)FL_data.MR_BR_ser) * FL_MR_BR_LEN) + FL_MR_START_ADDR));
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
	
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)&MR, FL_MR_BR_LEN, ((((unsigned long)FL_data.MR_BR_ser)*FL_MR_BR_LEN) + FL_MR_BKP_ADDR));
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
	
#ifdef _DEBUG_FLASH_UART
	// UWriteString((char*)"AaYa3", UART_PC);		//Debug
	// UWriteData(FL_data.MR_BR_ser, UART_PC);
	// UWriteData(FL_data.curr_data_write_addr_telecom_log&0xff, UART_PC);
	// UWriteData((FL_data.curr_data_write_addr_telecom_log>>8)&0xff, UART_PC);
	// UWriteData((FL_data.curr_data_write_addr_telecom_log>>16)&0xff, UART_PC);		//Debug

    vUART_SendStr(UART_PC, "\nwMR:");
    vUART_SendInt(UART_PC, MR.pkt_ser);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, MR.curr_data_write_addr_telecom_log);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, MR.curr_upload_addr_telecom_log);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, MR.pkt_data_chksm);

    vUART_SendStr(UART_PC, "\nwBR:");
    vUART_SendInt(UART_PC, MR.pkt_ser);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, MR.curr_data_write_addr_telecom_log);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, MR.curr_upload_addr_telecom_log);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, MR.pkt_data_chksm);
#endif
	
	FL_data.MR_BR_ser++;
}

int updateFlashCurrAddr(void)                                                                                   //komal
{

    int sts = TRUE;
    FL_data.MR_BR_ser = 0;
    MR_BR_t MR, BR, MR_backup;
    char pass_TR = FALSE/*, pass_RR = FALSE*/, flash_read_retries = 1;

    //cli();
    while(FL_data.MR_BR_ser < FL_MAX_MR_ENTRIES)
    {
/*#ifdef DEBUG_SPCL_FLASH
        vUART_SendInt(DEBUG_UART_BASE,FL_data.MR_BR_ser);
#endif*/
        WREN();
        readContToBuff((((unsigned long)FL_data.MR_BR_ser) * FL_MR_BR_LEN) + FL_MR_START_ADDR, FL_MR_BR_LEN, (char*)&MR);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable

#ifdef DEBUG_PASS
        vUART_SendStr(DEBUG_UART_BASE,"\nMRval:");
        //vUART_SendBytes(DEBUG_UART_BASE,(const uint8_t *)&MR,sizeof(MR_BR_t));
        vUART_SendInt(DEBUG_UART_BASE,(((unsigned long)FL_data.MR_BR_ser) * FL_MR_BR_LEN) + FL_MR_START_ADDR);
        vUART_SendChr(DEBUG_UART_BASE,',');
        vUART_SendInt(DEBUG_UART_BASE,MR.pkt_ser);
        vUART_SendChr(DEBUG_UART_BASE,',');
        vUART_SendInt(DEBUG_UART_BASE,MR.curr_data_write_addr_telecom_log);
        vUART_SendChr(DEBUG_UART_BASE,',');
        vUART_SendInt(DEBUG_UART_BASE,MR.curr_upload_addr_telecom_log);
#endif

        WREN();
        readContToBuff((((unsigned long)FL_data.MR_BR_ser) * FL_MR_BR_LEN) + FL_MR_BKP_ADDR, FL_MR_BR_LEN, (char*)&BR);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef DEBUG_PASS
        vUART_SendStr(DEBUG_UART_BASE,"\nBRval:");
        //vUART_SendBytes(DEBUG_UART_BASE,(const uint8_t *)&BR,sizeof(MR_BR_t));
        vUART_SendInt(DEBUG_UART_BASE,(((unsigned long)FL_data.MR_BR_ser) * FL_MR_BR_LEN) + FL_MR_BKP_ADDR);
        vUART_SendChr(DEBUG_UART_BASE,',');
        vUART_SendInt(DEBUG_UART_BASE,BR.pkt_ser);
        vUART_SendChr(DEBUG_UART_BASE,',');
        vUART_SendInt(DEBUG_UART_BASE,BR.curr_data_write_addr_telecom_log);
        vUART_SendChr(DEBUG_UART_BASE,',');
        vUART_SendInt(DEBUG_UART_BASE,BR.curr_upload_addr_telecom_log);
#endif


/*#ifdef DEBUG_FLASH
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t *)"\nserial_number");
        vUART_SendInt(DEBUG_UART_BASE,MR.pkt_ser);
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t *)"\ncurr_addr");
        vUART_SendInt(DEBUG_UART_BASE,MR.curr_data_write_addr_charging_log);

#endif*/
        /*#ifdef _DEBUG_FLASH_UART
            WREN();
            readContToBuff((((unsigned long)FL_data.MR_BR_ser)*FL_MR_BR_LEN)+FL_MR_BKP_ADDR, FL_MR_BR_LEN, buff);
            //Wait_Busy();
            WRDI();             // HJ 29-12-2015    // Write Disable

            for(i=0 ; i<FL_MR_BR_LEN ; i++)
            {
                UWriteData(buff[i], UART0);
            }
            for(i=0 ; i<FL_MR_BR_LEN ; i++)
            {
                UWriteData(*(MR_ptr+i), UART0);
                UWriteData(*(BR_ptr+i), UART0);
            }
            for(i=0 ; i<FL_MR_BR_LEN ; i++)
            {
                UWriteData(*(MR_ptr+i), UART0);
                UWriteData(*(BR_ptr+i), UART0);
            }
        #endif*/

        // Checking if both Datas from Backup & Master are Same
#ifdef _DEBUG_FLASH_UART
        //UWriteString((char*)"CompareOK", UART_PC);
#endif
        if(MR.pkt_ser == FL_data.MR_BR_ser)
        {
#ifdef DEBUG_PASS
            vUART_SendStr(DEBUG_UART_BASE,"\nserOk");
#endif


            if(MR.pkt_data_chksm == getChecksum((unsigned char*)&MR,FL_MR_BR_LEN-1))      //Anand 04-07-15
            {
#ifdef DEBUG_PASS
                vUART_SendStr(DEBUG_UART_BASE,"\nCKM");
#endif
                FL_data.MR_BR_ser++;

#ifdef DEBUG_PASS
                vUART_SendStr(DEBUG_UART_BASE,"\nmrbrser");
                vUART_SendInt(DEBUG_UART_BASE,FL_data.MR_BR_ser);
#endif
                pass_TR = TRUE;
                //Taking Backup before Next Read
                flashCopyMR((char*)&MR, (char*)&MR_backup);
            }
            else
            {
                if(BR.pkt_data_chksm == getChecksum((unsigned char*)&BR,FL_MR_BR_LEN-1))      //Anand 18-07-15
                {
                    FL_data.MR_BR_ser++;
                    pass_TR = TRUE;
                    //Taking Backup before Next Read
                    flashCopyMR((char*)&BR, (char*)&MR_backup);     //BR=src, MR_backup=dst
#ifdef DEBUG_PASS
                    vUART_SendStr(DEBUG_UART_BASE,"\nCKB");
#endif
                }
                else
                {
                    flash_read_retries++;
#ifdef DEBUG_PASS
                    vUART_SendStr(DEBUG_UART_BASE,"\nCFMB");
#endif
                }
            }
        }
        else
        {
            if(MR.pkt_ser == 0xFF)
                flash_read_retries= 4;
            else
                ++flash_read_retries;
        }
        //can Send Flash Data Corrupted Error
        if(flash_read_retries>3)
        {
            break;
        }
    }
    if(pass_TR) //We got a Valid ADDR Packet. Now Extract the curr_flash_addr
    {
#ifdef  DEBUG_PASS
        vUART_SendStr(DEBUG_UART_BASE,"\nFLrK");
#endif
        FL_data.MR_BR_ser = (MR_backup.pkt_ser+1);

        FL_data.curr_data_write_addr_telecom_log = MR_backup.curr_data_write_addr_telecom_log;

#ifdef  DEBUG_PASS
        vUART_SendStr(DEBUG_UART_BASE,"\nFLc1:");
        vUART_SendInt(DEBUG_UART_BASE,FL_data.curr_data_write_addr_telecom_log);
#endif
        FL_data.curr_upload_addr_telecom_log = MR_backup.curr_upload_addr_telecom_log;

#ifdef  DEBUG_PASS
        vUART_SendStr(DEBUG_UART_BASE,"\nFLu1:");
        vUART_SendInt(DEBUG_UART_BASE,FL_data.curr_upload_addr_telecom_log);
#endif

        if(flashGetCurrDataWriteAddr(SCHEDULED_FL_TLOG) != flashGetCurrUploadAddr(SCHEDULED_FL_TLOG))
        {
            FL_data.head_rear_position = 1;
            // FL_char_data_t curr_upload_log_data_charging;
            FL_log_data_t curr_upload_log_data_telecom;
            WREN();
            readContToBuff(FL_data.curr_upload_addr_telecom_log, FL_TLOG_LEN, (char*)&curr_upload_log_data_telecom);
            Wait_Busy();        // HJ 12-08-2016
            WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef DEBUG_PASS
            vUART_SendStr(DEBUG_UART_BASE,"\naddrnomatch");
#endif
        }
        else
        {
            FL_data.head_rear_position = 0;
        }
    }
    else if(FL_data.MR_BR_ser==0)       //if 1st Packet is FF, it means Bowser/Flash is New
    {
#ifdef DEBUG_PASS
        vUART_SendStr(DEBUG_UART_BASE,"\n1NEW_FLASH");
#endif
        FL_data.curr_data_write_addr_telecom_log = FL_TLOG_START_ADDR;
        FL_data.curr_upload_addr_telecom_log = FL_TLOG_START_ADDR;

#ifdef DEBUG_PASS
        // vUART_SendStr(DEBUG_UART_BASE,"\n4");
#endif

        flashEraseMaster();
#ifdef DEBUG_PASS
        // vUART_SendStr(DEBUG_UART_BASE,"\n5");
#endif
        flashEraseBackup();
#ifdef DEBUG_PASS
        // vUART_SendStr(DEBUG_UART_BASE,"\n6");
#endif
        flashWriteMR_BR();
#ifdef DEBUG_PASS
        // vUART_SendStr(DEBUG_UART_BASE,"\n4");
#endif

#ifdef _DEBUG_FLASH_UART
        // UWriteString((char*)"AaYa1", UART_PC);      //Debug
        // UWriteData(FL_data.curr_data_write_addr_telecom_log & 0xff, UART_PC);
        // UWriteData((FL_data.curr_data_write_addr_telecom_log >> 8) & 0xff, UART_PC);
        // UWriteData((FL_data.curr_data_write_addr_telecom_log >> 16) & 0xff, UART_PC);      //Debug

        vUART_SendStr(UART_PC, "\n2NEW_FLASH:");
        vUART_SendInt(UART_PC, FL_data.MR_BR_ser);
        vUART_SendChr(UART_PC, ',');
        vUART_SendInt(UART_PC, FL_data.curr_data_write_addr_telecom_log);
        vUART_SendChr(UART_PC, ',');
        vUART_SendInt(UART_PC, FL_data.curr_upload_addr_telecom_log);
#endif
    }
    else
    {
#ifdef _DEBUG_FLASH_UART
        // UWriteString((char*)"Fail", UART_PC);
        vUART_SendStr(UART_PC, "\nFL_readF:");
        vUART_SendInt(UART_PC, flash_read_retries);
#endif
        FL_data.curr_data_write_addr_telecom_log = FL_TLOG_START_ADDR;
        sts = FALSE;
    }
    //sei();      //Anand 28.03.14
#ifdef DEBUG_SPCL_FLASH
    // UWriteString((char*)"MR Comp", UART_PC);
    vUART_SendStr(UART_PC, "MR_Comp");
#endif
#ifdef OFFLINE_MODE_EN
#ifdef DEBUG_FLASH
    vUART_SendStr(DEBUG_UART_BASE,"\nMBsn:");
    vUART_SendInt(DEBUG_UART_BASE,FL_data.MR_BR_ser);
    vUART_SendStr(DEBUG_UART_BASE,"\nFLc2:");
    vUART_SendInt(DEBUG_UART_BASE,FL_data.curr_data_write_addr_telecom_log);
    vUART_SendStr(DEBUG_UART_BASE,"\nFLu2:");
    vUART_SendInt(DEBUG_UART_BASE,FL_data.curr_upload_addr_telecom_log);
#endif


#endif //OFFLINE_MODE_EN
    return sts;
}

unsigned long flashGetCurrDataWriteAddr(char type)
{
    unsigned long retVal = 0;
    switch(type)
    {
        case SCHEDULED_FL_TLOG:
        default:
        {
            retVal = FL_data.curr_data_write_addr_telecom_log;
        }
        break;
    }
    return retVal;
}

unsigned long flashGetCurrUploadAddr(char type)     //Anand 04-07-15
{
    unsigned long retVal = 0;
    switch(type)
    {
        case SCHEDULED_FL_TLOG:
        default:
        {
            retVal = FL_data.curr_upload_addr_telecom_log;
        }
        break;
    }
    return retVal;
}

int8_t flashGetSerMR_BR(void)
{
	return FL_data.MR_BR_ser;
}

void flashWriteTR(void)
{
	// FL_log_data.transaction_id_dispense_log = ++FL_data.transaction_id_dispense_log;		// Start transaction with 1
	
	if(FL_data.curr_data_write_addr_telecom_log >= FL_MAX_TLOG_ADDR)		//Anand 08.10.2014	Note : TODO
	{
		FL_data.curr_upload_addr_telecom_log = 0x1000 + FL_TLOG_START_ADDR;
			
		if(FL_data.curr_upload_addr_telecom_log >= FL_MAX_TLOG_ADDR)		// For one sector size.
		{
			FL_data.curr_upload_addr_telecom_log = FL_TLOG_START_ADDR;
		}
		
		FL_data.curr_data_write_addr_telecom_log = FL_TLOG_START_ADDR;
		flashEraseSector(FL_TLOG_START_ADDR, FL_MAX_TLOG_ADDR);
		
		FL_data.head_rear_position = 0;		// We did a skip so new upload tx id is required.
	}
	else    //We don't want two skips.
		if(FL_data.curr_upload_addr_telecom_log == FL_data.curr_data_write_addr_telecom_log)		//We want this for successful execution.
		{
			if(((FL_data.curr_data_write_addr_telecom_log % 0x1000) == 0) && (FL_data.curr_data_write_addr_telecom_log != FL_TLOG_START_ADDR))		//Anand 18-07-15
			{
				flashEraseSector(FL_data.curr_data_write_addr_telecom_log, FL_MAX_TLOG_ADDR);
				FL_data.curr_upload_addr_telecom_log += 0x1000;
				if(FL_data.curr_upload_addr_telecom_log >= FL_MAX_TLOG_ADDR)
				{
					FL_data.curr_upload_addr_telecom_log = FL_TLOG_START_ADDR;
				}
			
				FL_data.head_rear_position = 0;		//We did a skip so new upload tx id is required.
			}
		}
	//HJ 12-8-2016       Calculate checksum
	FL_log_data.chksum = getChecksum((unsigned char*)&FL_log_data, (FL_TLOG_LEN - 1 - TR_UNUSED_LEN));
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)&FL_log_data, FL_TLOG_LEN, FL_data.curr_data_write_addr_telecom_log);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write DisableWRDI();
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
#ifdef DEBUG_FLASH_DISPENSE_DATA
char temp_[66];
	WREN();
	readContToBuff(FL_data.curr_data_write_addr_telecom_log, FL_TLOG_LEN, temp_);		//KP 28-03-2015
	Wait_Busy();	//??				// HJ 12-08-2016 Uncomment
	WRDI();
UWriteString((char*)"Log_data:", UART_PC);
UWriteBytes((unsigned char*)&temp_, sizeof(FL_log_data_t), UART_PC);
UWriteString((char*)"log:", UART_PC);
ltoa(FL_log_data.transaction_id_dispense_log, temp_, 10);
UWriteString(temp_, UART_PC);
UWriteString("\nFuel:", UART_PC);
itoa((FL_log_data.remaining_bowser_fuel / 1000), temp_, 10);
UWriteString(temp_, UART_PC);
UWriteString("\nFreq_addr:", UART_PC);
ltoa(FL_data.curr_data_write_addr_telecom_log, temp_, 10);
UWriteString(temp_, UART_PC);
#endif
	FL_data.curr_data_write_addr_telecom_log += (FL_TLOG_LEN);

#ifdef DEBUG_FLASH
    vUART_SendStr(UART_PC, "\nwTR:");
    vUART_SendInt(UART_PC, FL_log_data.chksum);
    vUART_SendStr(UART_PC, "\nFL_d:");
    vUART_SendInt(UART_PC, FL_data.curr_data_write_addr_telecom_log);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, FL_data.curr_upload_addr_telecom_log);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, FL_data.head_rear_position);
#endif  //DEBUG_FLASH

	flashWriteMR_BR();
}

int validate_FL_addr(char type, unsigned long addr)
{
	switch(type)
	{
        case SCHEDULED_FL_TLOG:
        default:
        {
            if((addr >= FL_TLOG_START_ADDR) && (addr < FL_MAX_TLOG_ADDR))
			{
				return TRUE;
			}
        }
        break;
	}
	return FALSE;
}

void clear_logs(void)
{
#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(UART_PC, "CF1");
#endif  //DEBUG_FLASH_INIT

    flashClrTR_Logs();
    flashClrFreqUpdLogs();

	flashEraseMaster();
	flashEraseBackup();
	
	updateFlashCurrAddr();
	clear_log_id();

}

void clear_log_id(void)
{	
	// HJ 16-07-2016
	freq_updated_data.unsent_telecom_logs_IOT = 0;

	//HJ 24-7-2016
	freq_updated_data.cnt = 0;
#ifdef USE_FLASH_FOR_FREQ_UPD_DATA
	flashClearFreqUpdatedConfig();	
#else
	eepromClearFreqUpdatedConfig();
#endif
}

void clear_flash(void)
{
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	Chip_Erase();
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable 
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
}

unsigned long get_log_cnt(int trans_type)
{
	unsigned long log_cnt = 0;
	unsigned long curr_write_addr = flashGetCurrDataWriteAddr(trans_type);
	unsigned long curr_upload_addr = flashGetCurrUploadAddr(trans_type);

	switch(trans_type)
	{
		case SCHEDULED_FL_TLOG:
		default:
		{
			if(curr_upload_addr > curr_write_addr)
			{
				log_cnt = ((( FL_MAX_TLOG_ADDR - curr_upload_addr) + (curr_write_addr - FL_TLOG_START_ADDR)) / FL_TLOG_LEN);
			}
			else
			{
				log_cnt = ((curr_write_addr - curr_upload_addr) / FL_TLOG_LEN);
			}
		}
		break;
	}
	return log_cnt;
}

unsigned long get_addr(int trans_type, unsigned long record_num)
{
	unsigned long addr = 0;
	unsigned long curr_write_addr = flashGetCurrDataWriteAddr(trans_type);
	switch(trans_type)
	{
		case SCHEDULED_FL_TLOG:
		default:
		{
			if((curr_write_addr - (record_num * FL_TLOG_LEN)) >= FL_TLOG_START_ADDR)		//Subtraction must not result negative coz the variable is unsigned!
			{
				addr = curr_write_addr  - (record_num * FL_TLOG_LEN);
			}
			else
			{
				addr = FL_MAX_TLOG_ADDR + curr_write_addr - FL_TLOG_START_ADDR - (record_num * FL_TLOG_LEN);
			}
		}
		break;
	}
	return addr;
}

int get_log_data(int trans_type, unsigned long addr, char *data)
{
	int data_read_ok = 0;
	switch(trans_type)
	{
		case SCHEDULED_FL_TLOG:
		default:
		{
			if(validate_FL_addr(trans_type, addr))
			{
				WREN();
				readContToBuff(addr, FL_TLOG_LEN, data);		//KP 28-03-2015
				Wait_Busy();	//??				// HJ 12-08-2016 Uncomment
				WRDI();
				data_read_ok = 1;
			}
		}
		break;
	}
	return data_read_ok;
}

void flashEraseFreqUpdDataSector(void)		//KP 20-4-2018
{
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	Sector_Erase(FL_FREQ_UPD_DATA_START_ADDR);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
}

void flashClearFreqUpdatedConfig(void)
{
    //cli();
    // Lets clear the entire circular buffer
    flashEraseFreqUpdDataSector();
    freq_updated_data.cnt = 1;
    //Lets copy latest data on first row
    freq_updated_data.chksum = getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1));        //22.05.2014
    //eeprom_write_block(&freq_updated_data, (void*)addr, sizeof(freq_updated_data_t));     //R
    flashWriteFreqUpdData(&freq_updated_data, FL_FREQ_UPD_DATA_START_ADDR);

    //sei();
}

void flashWriteFreqUpdData(freq_updated_data_t *log_data, unsigned long addr)
{	
#ifdef FLASH_WP_ENABLE
	remove_block_protection();
#endif
	WREN();
	flashPacketProgram((char*)log_data, FL_FREQ_UPD_DATA_SIZE, addr);
	Wait_Busy();
	WRDI();				// HJ 29-12-2015    // Write DisableWRDI();
#ifdef FLASH_WP_ENABLE
	WBPR(0);
#endif
#ifdef DEBUG_FLASH_FREQ_DATA
UWriteString((char*)"freq_as:", UART_PC);
UWriteBytes((unsigned char*)log_data, sizeof(freq_updated_data_t), UART_PC);
#endif
}

void flashWriteFreqUpdatedConfig(void)
{
    //cli();
    ++freq_updated_data.cnt;

    unsigned long addr = FL_FREQ_UPD_DATA_START_ADDR + ((freq_updated_data.cnt - 1) * sizeof(freq_updated_data_t));

#ifdef DEBUG_FL_UNSENT
	vUART_SendStr(DEBUG_UART_BASE,"\nWFr:");
	vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.cnt);
	vUART_SendChr(DEBUG_UART_BASE,',');
	vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.unsent_telecom_logs_IOT);
    vUART_SendChr(DEBUG_UART_BASE,',');
    vUART_SendInt(DEBUG_UART_BASE, addr);
#endif

    if((addr < FL_FREQ_UPD_DATA_MAX_ADDR) && freq_updated_data.cnt)
    {
        freq_updated_data.chksum = getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/));

#ifdef DEBUG_FL_UNSENT
        vUART_SendStr(DEBUG_UART_BASE,"\nWFr.chk:");
        vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.chksum);
#endif

        flashWriteFreqUpdData(&freq_updated_data, addr);        //R
    }
    else
    {
#ifdef DEBUG_FL_UNSENT
        vUART_SendStr(DEBUG_UART_BASE,"\nWFr>max/FR_cnt=0");
#endif
        //freq_updated_data.cnt = 1;            //Lets keep cnt within 1 to 10, otherwise it will create problem after 255 transactions.
        flashClearFreqUpdatedConfig();
    }
    //sei();

}

void flashReadFreqUpdData(freq_updated_data_t *freq_updated_data, unsigned long addr, int size)
{
	WREN();
	readContToBuff(addr, size, (char *)freq_updated_data);
	WRDI();				// HJ 29-12-2015    // Write Disable
}

char readFreqUpdData_flash(void)
{
    char read_ok = 1;
    int i = 0;
    unsigned long addr;
    freq_updated_data_t freq_updated_data_;
    unsigned char j=1;
    j = 1;
    //cli();
#ifdef DEBUG_READ_FREQ
    vUART_SendStr(DEBUG_UART_BASE,"\nrFreq:");
    //vUART_SendBytes(DEBUG_UART_BASE,(const uint8_t *)&cmd_data.idtag, sizeof(cmd_data.idtag));
#endif

    for(i=0 ; i<3 ; i++)
    {
        do{
            addr = (FL_FREQ_UPD_DATA_START_ADDR + (j++ * FL_FREQ_UPD_DATA_SIZE));

#ifdef DEBUG_READ_FREQ
			vUART_SendStr(DEBUG_UART_BASE,"\nFraddrs:");
            vUART_SendInt(DEBUG_UART_BASE,sizeof(freq_updated_data));
            vUART_SendChr(DEBUG_UART_BASE,',');
			vUART_SendInt(DEBUG_UART_BASE,(addr - FL_FREQ_UPD_DATA_SIZE));
            vUART_SendChr(DEBUG_UART_BASE,',');
			vUART_SendInt(DEBUG_UART_BASE,addr);
#endif

            if(addr < FL_FREQ_UPD_DATA_MAX_ADDR)
            {
                flashReadFreqUpdData(&freq_updated_data, (addr - FL_FREQ_UPD_DATA_SIZE), FL_FREQ_UPD_DATA_SIZE);
                flashReadFreqUpdData(&freq_updated_data_, addr, FL_FREQ_UPD_DATA_SIZE);

//#ifdef DEBUG_READ_FREQ
//    			vUART_SendStr(DEBUG_UART_BASE,"\nfreq_updated_data:");
//    			vUART_SendBytes(DEBUG_UART_BASE,(const uint8_t *)&freq_updated_data,sizeof(freq_updated_data));
//#endif

#ifdef DEBUG_READ_FREQ
                vUART_SendStr(DEBUG_UART_BASE,"\nFrcnts:");
                vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.cnt);
                vUART_SendChr(DEBUG_UART_BASE,',');
                vUART_SendInt(DEBUG_UART_BASE,freq_updated_data_.cnt);
                vUART_SendStr(DEBUG_UART_BASE, "\nFrunsnt:");
                vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.unsent_telecom_logs_IOT);
                vUART_SendChr(DEBUG_UART_BASE,',');
                vUART_SendInt(DEBUG_UART_BASE,freq_updated_data_.unsent_telecom_logs_IOT);
#endif

                if(freq_updated_data_.cnt == 0xFF)
                {
#ifdef DEBUG_READ_FREQ
					vUART_SendStr(DEBUG_UART_BASE,"\nRFr.cnt:");
					vUART_SendInt(DEBUG_UART_BASE,freq_updated_data_.cnt);
#endif
    				break;
                }
            }
            else
            {
                memcpy(&freq_updated_data, &freq_updated_data_, FL_FREQ_UPD_DATA_SIZE);     //At the end the second one is holding the Gold
                //lets erase the sector and put this on first location
                //freq_updated_data.cnt = 1;
                flashClearFreqUpdatedConfig();
#ifdef DEBUG_READ_FREQ
                vUART_SendStr(DEBUG_UART_BASE,"\nFr=Fr+1");
#endif
                break;
            }
        }while((freq_updated_data.cnt + 1) == freq_updated_data_.cnt);

        if(freq_updated_data.chksum == getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/)))   // check how your wrote in writecfg
        {
#ifdef DEBUG_READ_FREQ
			vUART_SendStr(DEBUG_UART_BASE,"\nRFr.C:");
			vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.chksum);
            vUART_SendChr(DEBUG_UART_BASE,',');
            vUART_SendInt(DEBUG_UART_BASE,getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/)));
#endif
            read_ok = 1;
            break;
        }
        else
        {
#ifdef DEBUG_READ_FREQ
			vUART_SendStr(DEBUG_UART_BASE,"\nRFr.cF:");
			vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.chksum);
            vUART_SendChr(DEBUG_UART_BASE,',');
            vUART_SendInt(DEBUG_UART_BASE,getChecksum((unsigned char*)(&freq_updated_data), (sizeof(freq_updated_data_t) - 1/* - BD_FREQ_UNUSED_LEN*/)));
#endif
            j = 1;
            read_ok = 0;
        }
    }
    //sei();
    if(i>=3)
    {
#ifdef DEBUG_READ_FREQ
		vUART_SendStr(DEBUG_UART_BASE,"\niii:");
		//vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.chksum);
#endif

        read_ok = 0;
        //Log Sys error and go to config mode
        //error_sts(TRUE);
    }
    if (!read_ok)
    {
#ifdef DEBUG_READ_FREQ
		vUART_SendStr(DEBUG_UART_BASE,"\nread!:");
		//vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.chksum);
#endif
        //get_remaining_fuel();
        freq_updated_data.unsent_telecom_logs_IOT= 0;
        //freq_updated_data.unsent_refuel_logs_GPRS = 0;
    }

#ifdef DEBUG_READ_FREQ
		vUART_SendStr(DEBUG_UART_BASE,"\nnthg:");
		//vUART_SendInt(DEBUG_UART_BASE,freq_updated_data.chksum);
#endif

    return read_ok;

}

unsigned int decrement_unsent_log_cnt(int upload_code)
{
    switch(upload_code)
    {
		case TELECOM_OFFLINE_LOGS:
		default:
		{
			if(freq_updated_data.unsent_telecom_logs_IOT)
            {
                --freq_updated_data.unsent_telecom_logs_IOT;
            }
		}
		break;
    }

    flashWriteFreqUpdatedConfig();
    //eepromWriteFreqUpdatedConfig();
    return 1;
}

unsigned int increment_unsent_log_cnt(int trans_type)
{
    switch(trans_type)
    {
		case TELECOM_OFFLINE_LOGS:
		default:
		{
			++freq_updated_data.unsent_telecom_logs_IOT;
		}
		break;
    }
/*
#ifdef USE_FLASH_FOR_FREQ_UPD_DATA
    flashWriteFreqUpdatedConfig();
#else
    eepromWriteFreqUpdatedConfig();
#endif*/
    return 1;
}

char check_unsent_log(void)
{
	if(freq_updated_data.unsent_telecom_logs_IOT)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void get_unsent_logs(void)
{
    unsigned long addr;
    if(freq_updated_data.unsent_telecom_logs_IOT)
    {
#ifdef DEBUG_READ_FREQ
            vUART_SendStr(UART_PC,"\nFOUND_UNSENT:");
            vUART_SendInt(UART_PC,freq_updated_data.unsent_telecom_logs_IOT);
#endif
        addr = get_addr(SCHEDULED_FL_TLOG, freq_updated_data.unsent_telecom_logs_IOT);
#ifdef DEBUG_READ_FREQ
            vUART_SendStr(UART_PC,"\nUaddr:");
            vUART_SendInt(UART_PC,addr);
#endif
        get_log_data(SCHEDULED_FL_TLOG,addr,(char *)&FLR_log_data);    //CMD_AUTHORIZE,CMD_START_TRANSACTION,CMD_STOP_TRANSACTION,CMD_STATUS_NOTIFICATION,

#ifdef DEBUG_READ_FREQ
            // vUART_SendStr(UART_PC,"\nvalues-");
            // vUART_SendStr(UART_PC,"cmd:");
            // vUART_SendInt(UART_PC,ocpp_cmd_data.cmd_type);
            // vUART_SendStr(UART_PC,"\nerrorcode");
            // vUART_SendInt(UART_PC,ocpp_cmd_data.error_code);
            // vUART_SendStr(UART_PC,"\nstatus/reason");
            // vUART_SendInt(UART_PC,ocpp_cmd_data.status_reason);
            // vUART_SendStr(UART_PC,"cmd:");
            // vUART_SendInt(UART_PC,ocpp_cmd_data.cmd_type);
            vUART_SendStr(UART_PC, "\nFLR_dt:");
            vUART_SendInt(UART_PC, FLR_log_data.ram_data.ram_time.date);
            vUART_SendChr(UART_PC, '-');
            vUART_SendInt(UART_PC, FLR_log_data.ram_data.ram_time.month);
            vUART_SendChr(UART_PC, '-');
            vUART_SendInt(UART_PC, FLR_log_data.ram_data.ram_time.year);
            vUART_SendChr(UART_PC, ',');
            vUART_SendInt(UART_PC, FLR_log_data.ram_data.ram_time.hour);
            vUART_SendChr(UART_PC, ':');
            vUART_SendInt(UART_PC, FLR_log_data.ram_data.ram_time.min);
            vUART_SendChr(UART_PC, ':');
            vUART_SendInt(UART_PC, FLR_log_data.ram_data.ram_time.sec);
#endif
    }
}

void save_OfflineTelecomData(void)
{
	prepare_OfflineTelecomData();
	save_TELECOM_data();

#ifdef ETHERNET_EN
	if(!get_ethernet_NWstatus() || check_unsent_log())
	{
#ifdef DEBUG_FL_UNSENT
	    vUART_SendStr(DEBUG_UART_BASE,"\nunsnt");
#endif
		increment_unsent_log_cnt(TELECOM_OFFLINE_LOGS);
		flashWriteFreqUpdatedConfig();
	}
#else
	if(!getServerStatus() || check_unsent_log())
	{
		increment_unsent_log_cnt(TELECOM_OFFLINE_LOGS);
		flashWriteFreqUpdatedConfig();
	}
#endif 	//ETHERNET_EN
}

void prepare_OfflineTelecomData(void)
{
	memcpy(&FL_log_data.ram_data, &ram_data, sizeof(ram_data_t));
	FL_log_data.chksum = 0;
#ifdef DEBUG_FLASH
    vUART_SendStr(DEBUG_UART_BASE,"\n1FL_log:");
    vUART_SendStr(DEBUG_UART_BASE,"\n1Ffc=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_EXTI_cnt.freq_cnt);
    vUART_SendStr(DEBUG_UART_BASE,"\n1FACP=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\n1FRC=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_ADC.DC_current_router1);
    vUART_SendStr(DEBUG_UART_BASE,"\t1FOC=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_ADC.DC_current_router2);
    vUART_SendStr(DEBUG_UART_BASE,"\n1FACN=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_ADC.NE_AC_Voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\t1FOV=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendStr(DEBUG_UART_BASE,"\n1FChgV=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_ADC.DC_Charger_voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\t1FBATTV=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendStr(DEBUG_UART_BASE,"\n1FLa=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.Latitude);
    vUART_SendStr(DEBUG_UART_BASE,"\t1FLo=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.Longitude);
    vUART_SendStr(DEBUG_UART_BASE,"\n1FRS=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.supply_mode_R1);
    vUART_SendStr(DEBUG_UART_BASE,"\t1FOS=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.supply_mode_R2);
    vUART_SendStr(DEBUG_UART_BASE,"\n1FA=");
    vUART_SendInt(DEBUG_UART_BASE,FL_log_data.ram_data.ram_alarms);
#endif
}

void save_TELECOM_data(void)
{
	flashWriteTR();
}

#endif  //FLASH_EN


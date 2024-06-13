/*
 * flash.cpp
 *
 *  Created on: Apr 23, 2024
 *      Author: ADMIN
 */

// #include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "_config.h"
#include "driverlib/ssi.h"

#if defined(FLASH_EN)

#include "flashCore.h"
#include "HW_pins.h"
#include "flash_logger.h"
#include "SPI.h"
#include "_debug.h"
#include "UartCore.h"
#include "SysTick_Timer.h"

// extern FL_char_data_t FL_char_data;  //PP (24-04-24) commenting this till I have'nt made my own structs for flash for Telecom_IoT.
// extern totalizer_t totalizer;        //PP (24-04-24) commenting this till I have'nt made my own structs for flash for Telecom_IoT.

void Reset_Hold_Low(void)
{
    GPIOPinWrite(FLASH_PORT_BASE, FLASH_RST_PIN, 0);
    //_delay_ms(1);                     /* clear Hold pin */
    //_delay_us(1);         // HJ 13-08-2016
}


void Reset_Hold_High(void)
{
    GPIOPinWrite(FLASH_PORT_BASE, FLASH_RST_PIN, FLASH_RST_PIN);
    //SPIPORT|=(1<<RST);      /* set Hold pin */
    //_delay_us(1);         // HJ 13-08-2016
}


void WP_Low(void)
{
    GPIOPinWrite(FLASH_PORT_BASE, FLASH_WP_PIN, 0);
    //_delay_us(1);         // HJ 13-08-2016
}


void WP_High(void)
{
    GPIOPinWrite(FLASH_PORT_BASE, FLASH_WP_PIN, FLASH_WP_PIN);
    //_delay_us(1);         // HJ 13-08-2016
}

void flash_port_assign(void)
{
#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t *)"\nFL3");
#endif
#ifndef HARD_FLASH_SPI
    // SSIDisable(FLASH_SPI_BASE);
    GPIOPinTypeGPIOOutput(FLASH_SPI_PORT_BASE,FLASH_MOSI_PIN);
    GPIOPinTypeGPIOOutput(FLASH_SPI_PORT_BASE,FLASH_CLK_PIN);
    GPIOPinTypeGPIOInput(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN);
#endif

    GPIOPinTypeGPIOOutput(FLASH_PORT_BASE, FLASH_RST_PIN);
    GPIOPinTypeGPIOOutput(FLASH_PORT_BASE, FLASH_WP_PIN);
    GPIOPinTypeGPIOOutput(FLASH_PORT_BASE, FLASH_CS_PIN);

    //GPIOPinWrite(FLASH_SPI_PORT_BASE, FLASH_CLK_PIN | FLASH_MOSI_PIN, 0);
    GPIOPinWrite(FLASH_PORT_BASE, FLASH_CS_PIN | FLASH_WP_PIN, 0);
    GPIOPinWrite(FLASH_PORT_BASE,FLASH_RST_PIN,FLASH_RST_PIN);
#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t *)"\nFL4");
#endif
}

void init_flash_pins(void)
{
    // SSIDisable(FLASH_SPI_BASE);
    GPIOPinTypeGPIOOutput(FLASH_SPI_PORT_BASE,FLASH_MOSI_PIN);
    GPIOPinTypeGPIOOutput(FLASH_SPI_PORT_BASE,FLASH_CLK_PIN);
    GPIOPinTypeGPIOInput(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN);
}

int flashInit(void)
{
#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t *)"\nFL2");
#endif
    //Portassign();        //ADD BY ROHIT
    flash_port_assign();

#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t *)"\nFLinit");
#endif
#if 1
    CE_High();           //ADD BY ROHIT
    //WP_High();            //ADD BY ROHIT      // HJ 27-7-2016   Move to WREN() and WRDI()
    WP_Low();               // Lets ensure that on start up flash is write protected
    Reset_Hold_High(); //ADD BY ROHIT
    SCK_Low();   //set clock to low initial state
    //_delay_ms(10);
    vMAIN_DelayMS(10);

    //Lets give a reset on power up //Kamlesh 24-01-2015
#ifdef SST26VF064B
    flashSwReset();
    //   Enable write protection pin functionality
    enable_wp_pin();
#endif
     //Edit by Anand

#ifdef SST25VF064C
    WREN();
    EWSR();
    WRSR(0x7F);     //Reset BPL Bit
    WREN();
    EWSR();
    WRSR(0);            //Reset All BP0-BP3
#else
    //remove_block_protection();
#endif
#endif  //#if 0
    return updateFlashCurrAddr();
    // return 1;
}

#if 0   //PP (24-04-24) commenting this till I have'nt made my own structs for flash for Telecom_IoT.
void addDummyChargingLogs(unsigned int no_of_logs)   
{
    unsigned int i=0;
    uint8_t dummy_id[7] = {0x9A, 0x0B, 0x54, 0x1C, 0x00, 0x00, 0x00};
    FL_char_data.reservation_id=2;
    FL_char_data.EVT_type=CHARGING_START;
    FL_char_data.Reason=REA_DUMMY;
    FL_char_data.connector_id='1';
    memcpy(&FL_char_data.Id_tag,dummy_id,sizeof(FL_char_data.Id_tag));
    vMAIN_DelayMS(1000);

    for(i=0;i<no_of_logs;i++)
    {
        FL_char_data.meter_value=rand()%(1000)+(totalizer.system_totalizer*1000+totalizer.system_totalizer_dp);
        // get_present_time(&FL_char_data.time_stamp);  //PP (24-04-24) commenting this till I have'nt implemented RTC
        FlashWriteChargeEvents();
        vMAIN_DelayMS(1);

    }
}

#ifdef OFFLINE_MODE_EN
void flashClrFreqUpdLogs(void)
{
    unsigned int sectors_to_erase = ((FL_FREQ_UPD_DATA_MAX_ADDR - FL_FREQ_UPD_DATA_START_ADDR) / SECTOR_SIZE) + 1;
    unsigned long addr = FL_FREQ_UPD_DATA_START_ADDR;

    while(sectors_to_erase--)
    {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
        addr += SECTOR_SIZE;
    }
}


void flashClrCmdLogs(void)
{
    unsigned int sectors_to_erase = ((FL_MAX_CMD_ADDR - FL_CMD_START_ADDR) / SECTOR_SIZE) + 1;
    unsigned long addr = FL_CMD_START_ADDR;

    while(sectors_to_erase--)
    {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
        addr += SECTOR_SIZE;
    }
}
#endif

void flashClrChargingLogs(void)
{
    unsigned int sectors_to_erase = ((FL_MAX_CR_ADDR - FL_CR_START_ADDR) / SECTOR_SIZE) + 1;
    unsigned long addr = FL_CR_START_ADDR;

    while(sectors_to_erase--)
    {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
        addr += SECTOR_SIZE;
    }
}
#endif  // if 0

void flashClrTR_Logs(void)
{
#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(UART_PC, "CTR");
#endif  //DEBUG_FLASH_INIT

    unsigned int sectors_to_erase = ((FL_MAX_TLOG_ADDR - FL_TLOG_LEN) / SECTOR_SIZE) + 1;
    unsigned long addr = FL_MAX_TLOG_ADDR;

    while(sectors_to_erase--)
    {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
        addr += SECTOR_SIZE;
    }
}

void flashClrFreqUpdLogs(void)
{
#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(UART_PC, "CFU");
#endif  //DEBUG_FLASH_INIT

    unsigned int sectors_to_erase = ((FL_FREQ_UPD_DATA_MAX_ADDR - FL_FREQ_UPD_DATA_START_ADDR) / SECTOR_SIZE) + 1;
    unsigned long addr = FL_FREQ_UPD_DATA_START_ADDR;

    while(sectors_to_erase--)
    {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
        addr += SECTOR_SIZE;
    }
}

void addDummyFL_TelecomLogs(unsigned int no_of_logs)
{
    unsigned int i = 0;
    FL_log_data_t dummy_FL_Log;
    memset(&dummy_FL_Log, 0, sizeof(FL_log_data_t));

    dummy_FL_Log.ram_data.ram_ADC.PN_AC_Voltage = 260000;
    dummy_FL_Log.ram_data.ram_ADC.DC_Battery_voltage = 12000;
    dummy_FL_Log.ram_data.ram_ADC.DC_Charger_voltage = 12000;
    dummy_FL_Log.ram_data.ram_ADC.DC_current_router1 = 500;
    dummy_FL_Log.ram_data.ram_ADC.DC_current_router2 = 500;
    dummy_FL_Log.ram_data.ram_ADC.DC_Voltage_router1 = 24000;
    dummy_FL_Log.ram_data.ram_ADC.DC_Voltage_router2 = 30000;
    dummy_FL_Log.ram_data.ram_alarms = 0;
    dummy_FL_Log.ram_data.Status = 0;
    dummy_FL_Log.ram_data.Latitude = 26263863;
    dummy_FL_Log.ram_data.Longitude = 73008957;
    dummy_FL_Log.ram_data.supply_mode_R1 = 24;
    dummy_FL_Log.ram_data.supply_mode_R2 = 30;

    vMAIN_DelayMS(1000);    //disable WDT while testing this

    for(i=0;i<no_of_logs;i++)
    {
        get_present_time(&dummy_FL_Log.ram_data.ram_time);
        flashWriteTR();
        vMAIN_DelayMS(1);
    }
}

void flashSwReset(void)
{
    CE_Low();
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x66);
    SPI_transrecv(FLASH_SPI_BASE,0x99);
#else
    Send_Byte(0x66);
    Send_Byte(0x99);
#endif
    CE_High();
}


void enable_wp_pin(void)
{
    WP_High();          //??
    WREN();
    CE_Low();
    WRCR(0x88);
    CE_High();
    WRDI();
    WP_Low();
}

void WREN(void)
{
    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x06);
#else
    Send_Byte(0x06);            /* send WREN command */
#endif
    CE_High();              /* disable device */
}

void WRDI(void)
{
    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x04);
#else
    Send_Byte(0x04);            /* send WREN command */
#endif
    CE_High();              /* disable device */
}

void WRCR(char byte)
{
    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x01);            /* select write to configration register */
    SPI_transrecv(FLASH_SPI_BASE,0x00);
    SPI_transrecv(FLASH_SPI_BASE,byte);            /* data that will change the status of BPx
                         or BPL (only bits 2,3,4,5,7 can be written) */
#else
    Send_Byte(0x01);            /* select write to configration register */
    Send_Byte(0x00);
    Send_Byte(byte);            /* data that will change the status of BPx
                         or BPL (only bits 2,3,4,5,7 can be written) */
#endif
    CE_High();              /* disable the device */
}

void CE_High()
{
    GPIOPinWrite(FLASH_PORT_BASE,FLASH_CS_PIN,FLASH_CS_PIN);
#ifdef RFID_ENABLE
    GPIODirModeSet(FLASH_SPI_PORT_BASE,FLASH_MOSI_PIN|FLASH_CLK_PIN|FLASH_MISO_PIN,GPIO_DIR_MODE_HW);
    vRFIDSPIInit();
#endif
    //FLASH_PORT_BASE|=(1<<FLASH_CS_PIN);
    //LATF = LATF | (0x04);                 /* set CE high */
    //_delay_us(1);         // HJ 13-08-2016
}
void CE_Low()
{
    //flash_port_assign();    //
    // init_flash_pins();
    GPIOPinWrite(FLASH_PORT_BASE,FLASH_CS_PIN,0);
    //FLASH_PORT_BASE&=~(1<<FLASH_CS_PIN);
    //_delay_ms(1);                     /* clear CE low */
    //_delay_us(1);         // HJ 13-08-2016
}

void SCK_High()
{
    GPIOPinWrite(FLASH_SPI_PORT_BASE,FLASH_CLK_PIN,FLASH_CLK_PIN);
    //FLASH_SPI_PORT_BASE|=(1<<FLASH_CLK_PIN);          /* set SCK high */
    //_delay_us(1);         // HJ 13-08-2016
}


void SCK_Low()
{
    GPIOPinWrite(FLASH_SPI_PORT_BASE,FLASH_CLK_PIN,0);
    //FLASH_SPI_PORT_BASE|=(1<<FLASH_CLK_PIN);     /* clear SCK low */
    //_delay_us(1);         // HJ 13-08-2016
}

void readContToBuff(unsigned long Dst, unsigned int no_bytes, char* buff)
{
    //cli();
    unsigned long i = 0;
    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x03);            /* read command */
    SPI_transrecv(FLASH_SPI_BASE,((Dst & 0xFFFFFF) >> 16));    /* send 3 address bytes */
    SPI_transrecv(FLASH_SPI_BASE,((Dst & 0xFFFF) >> 8));
    SPI_transrecv(FLASH_SPI_BASE,Dst & 0xFF);

    for (i = 0; i < no_bytes; i++)      /* read until no_bytes is reached */
    {
        buff[i] = SPI_transrecv(FLASH_SPI_BASE,0x00);  /* receive bytes */
#ifdef DEBUG_FLASH
        vUART_SendChr(UART_PC,buff[i]);
#endif
    }
#else
    Send_Byte(0x03);            /* read command */
    Send_Byte(((Dst & 0xFFFFFF) >> 16));    /* send 3 address bytes */
    Send_Byte(((Dst & 0xFFFF) >> 8));
    Send_Byte(Dst & 0xFF);

    for (i = 0; i < no_bytes; i++)      /* read until no_bytes is reached */
    {
        buff[i] = Get_Byte();   /* receive bytes */
    }
#endif
    CE_High();              /* disable device */
    //sei();
}

void flashEraseMaster(void)
{
#ifdef DEBUG_PASS
    // UWriteString((char*)"FEM:",UART_PC);
    vUART_SendStr(UART_PC, "\nFEM");
#endif
    unsigned long addr;
    for(addr = FL_MR_START_ADDR ; addr < FL_MR_MAX_ADDR ; addr+=SECTOR_SIZE)
    {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
    }
}

void flashCopyMR(char* src, char* dst)
{
    char i;
    for(i=0 ; i<FL_MR_BR_LEN ; i++)
        (*(dst+i)) = (*(src+i));
}

void flashEraseSector(unsigned long addr, unsigned long max_addr)
{
    //unsigned long addr = FL_HEX_FILE_START_ADDR;
    unsigned int sector_to_erase = (max_addr - addr)/SECTOR_SIZE;
    int i;
//#ifdef DEBUG_LOCAL_LIST_DATA
//                vUART_SendStr(DEBUG_UART_BASE, "\naddrInFlash");
//                vUART_SendInt(DEBUG_UART_BASE,addr);
//                vUART_SendChr(DEBUG_UART_BASE,',');
//                vUART_SendInt(DEBUG_UART_BASE,max_addr);
//
//#endif
    if(addr < max_addr)     //Anand 20-7-15
    {
        for(i = 0; i < sector_to_erase; i++)
        {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
            addr += SECTOR_SIZE;


        }
//#ifdef DEBUG_LOCAL_LIST_DATA
//                vUART_SendStr(DEBUG_UART_BASE, "\nInCondition");
//                vUART_SendInt(DEBUG_UART_BASE,addr);
//#endif
//#ifdef FLASH_WP_ENABLE
//        remove_block_protection();
//#endif
//        WREN();
//        Sector_Erase(addr);
//        Wait_Busy();
//        WRDI();             // HJ 29-12-2015    // Write Disable
//#ifdef FLASH_WP_ENABLE
//        WBPR(0);
//#endif
    }
}

void flashEraseBackup(void)     //Anand
{
#ifdef DEBUG_PASS
    // UWriteString((char*)"FEB:",UART_PC);
    vUART_SendStr(UART_PC, "\nFEB");
#endif
unsigned long addr;
    for(addr = FL_MR_BKP_ADDR ; addr < FL_MR_BKP_MAX_ADDR ; addr+=SECTOR_SIZE)
    {
#ifdef FLASH_WP_ENABLE
        remove_block_protection();
#endif
        WREN();
        Sector_Erase(addr);
        Wait_Busy();
        WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        WBPR(0);
#endif
    }
}

void flashPacketProgram(char* buff, uint16_t size, unsigned long addr)      //Added by Anand : Writes a Packet to flash_curr_data_write_addr
{
    unsigned long i;
    i=0;

    //cli();

    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x02);            /* send Byte Program command */
    SPI_transrecv(FLASH_SPI_BASE,((addr & 0xFFFFFF) >> 16));   /* send 3 address bytes */
    SPI_transrecv(FLASH_SPI_BASE,((addr & 0xFFFF) >> 8));
    SPI_transrecv(FLASH_SPI_BASE,addr & 0xFF);
    for (i = 0; i < size; i++)
    {
        SPI_transrecv(FLASH_SPI_BASE,buff[i]); /* send byte to be programmed */
    }
#else
    Send_Byte(0x02);            /* send Byte Program command */
    Send_Byte(((addr & 0xFFFFFF) >> 16));   /* send 3 address bytes */
    Send_Byte(((addr & 0xFFFF) >> 8));
    Send_Byte(addr & 0xFF);
    for (i=0;i<size;i++)
    {
        Send_Byte(buff[i]); /* send byte to be programmed */
    }
#endif
    CE_High();              /* disable device */
    //sei();      //??
}

void Wait_Busy()
{
  unsigned int timeout = 0;
  //cli();      //Anand 18-07-15
  unsigned char temp = 0;
  SetSO_Input();
  CE_Low();
#ifdef HARD_FLASH_SPI
  do
  {
      temp = SPI_transrecv(FLASH_SPI_BASE,0x05);
      //_delay_us(100);
      vMAIN_DelayMS(1);
      if (timeout++ > FLASH_READ_TIMEOUT)             //Max chip erase time is 50msec.lets wait for 100msec
      {
          timeout = 0;
#ifdef SST26VF064B
          flashSwReset();
#endif
#ifdef DEBUG_SPCL_FLASH
          UWriteString((char *)"\nHARD SPI TIMEOUT", UART_PC);
#endif
          break;
      }
      else
      {
#ifdef DEBUG_SPCL_FLASH
          //UWriteString((char *)"\nSPI Busy", UART_PC);
#endif
      }
  } while (temp & (1 << 0));
#else
  Send_Byte(0x05);        /* send RDSR command */
  SCK_High();
  SCK_Low();
  SCK_High();
  SCK_Low();
  SCK_High();
  SCK_Low();
  SCK_High();
  SCK_Low();
  SCK_High();
  SCK_Low();
  SCK_High();
  SCK_Low();
  SCK_High();
  SCK_Low();
  /*temp = ((SPIPIN & (1<<SO))>>SO);        //Get the Value of Busy bit in the status register
  temp = ((SPIPIN & (1<<SO))>>SO);        //Get the Value of Busy bit in the status register
  temp = ((SPIPIN & (1<<SO))>>SO);        //Get the Value of Busy bit in the status register
  temp = ((SPIPIN & (1<<SO))>>SO);        //Get the Value of Busy bit in the status register
  temp = ((SPIPIN & (1<<SO))>>SO);        //Get the Value of Busy bit in the status register
  */
    temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
    temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
    temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
    temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
    temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
  while (temp==1)
  {
      SCK_High();
      SCK_Low();
      SCK_High();
      SCK_Low();
      SCK_High();
      SCK_Low();
      SCK_High();
      SCK_Low();
      SCK_High();
      SCK_Low();
      SCK_High();
      SCK_Low();
      SCK_High();
      SCK_Low();
      SCK_High();
      SCK_Low();
      temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
      temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
      temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;       //Get the Value of Busy bit in the status register
      temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
      temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;        //Get the Value of Busy bit in the status register
      //_delay_ms(1);               // HJ 26-7-2016
      vMAIN_DelayMS(1);
      if (timeout++ > FLASH_READ_TIMEOUT)             //Max chip erase time is 50msec.lets wait for 100msec
      {
          //flashInit();                  // HJ 27-6-2016    We dont know what else to do.
          // Lets try reset
          /*Reset_Hold_Low();
          _delay_ms(5);
          Reset_Hold_High();*/
#ifdef SST26VF064B
          flashSwReset();
#endif
#ifdef DEBUG_SPCL_FLASH
        //   UWriteString((char *)"\nSPI TIMEOUT", UART_PC);
        vUART_SendStr(UART_PC, "\nSPItout");
#endif
          break;
      }
  }
#endif
  ///////
  SCK_Low();
  SI_Low();
  ////////
  CE_High();
  //sei();      //Anand 18-07-15
}

void Sector_Erase(unsigned long Dst)
{
    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x20);        //11-04-2020 SK
    SPI_transrecv(FLASH_SPI_BASE,((Dst & 0xFFFFFF) >> 16));        //11-04-2020 SK
    SPI_transrecv(FLASH_SPI_BASE,((Dst & 0xFFFF) >> 8));       //11-04-2020 SK
    SPI_transrecv(FLASH_SPI_BASE,Dst & 0xFF);      //11-04-2020 SK
#else
    Send_Byte(0x20);            /* send Sector Erase command */
    Send_Byte(((Dst & 0xFFFFFF) >> 16));    /* send 3 address bytes */
    Send_Byte(((Dst & 0xFFFF) >> 8));
    Send_Byte(Dst & 0xFF);
#endif
    CE_High();              /* disable device */
}

void remove_block_protection(void)
{

    WP_High();
    WREN();
    CE_Low();
    //WP_High();
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x98);    //Remove Write Protection from entire flash
#else
    Send_Byte(0x98);    //Remove Write Protection from entire flash
#endif
    //WP_Low();
    CE_High();
    WRDI();
    WP_Low();

}

void WBPR(char state)
{
    unsigned char data = 0;
    int i;
    if(!state)          // if disable write 0xFF;
    {
        data = 0xFF;
    }
   // cli();
    WP_High();
    WREN();
    //Send_Byte(0x98);  //Remove Write Protection from entire flash
    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x42);            /* send RDSR command */
    if(!state)          // if disable write 0xFF;
    {
        SPI_transrecv(FLASH_SPI_BASE,0x55);
        SPI_transrecv(FLASH_SPI_BASE,0x55);
    }
    else
    {
        SPI_transrecv(FLASH_SPI_BASE,0x00);
        SPI_transrecv(FLASH_SPI_BASE,0x00);
    }
    for (i = 0; i < 16; i++)        /* read until no_bytes is reached */
    {
        SPI_transrecv(FLASH_SPI_BASE,data);
    }
#else
    Send_Byte(0x42);            /* send RDSR command */
    if(!state)          // if disable write 0xFF;
    {
        Send_Byte(0x55);
        Send_Byte(0x55);
    }
    else
    {
        Send_Byte(0x00);
        Send_Byte(0x00);
    }
    for (i = 0; i < 16; i++)        /* read until no_bytes is reached */
    {
        Send_Byte(data);
    }
#endif
    CE_High();              /* disable device */
    WRDI();
    WP_Low();
    //sei();
}


void SetSI_Output()
{
    GPIOPinTypeGPIOOutput(FLASH_SPI_PORT_BASE,FLASH_MOSI_PIN);
    //_delay_ms(1);
    //_delay_us(1);         // HJ 13-08-2016
}
void SetSI_Input()
{
    GPIOPinTypeGPIOInput(FLASH_SPI_PORT_BASE,FLASH_MOSI_PIN);
    //_delay_ms(1);
    //_delay_us(1);         // HJ 13-08-2016
}
void SetSO_Output()
{
    GPIOPinTypeGPIOOutput(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN);
    //_delay_ms(1);
    //_delay_us(1);         // HJ 13-08-2016
}
void SetSO_Input()
{
    GPIOPinTypeGPIOInput(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN);
//  _delay_ms(1);
    //_delay_us(1);         // HJ 13-08-2016
}
void SI_Low()
{
    GPIOPinWrite(FLASH_SPI_PORT_BASE,FLASH_MOSI_PIN,0);
    //SPIPORT&=~(1<<SI);      /* clear SI low */
    //_delay_us(1);         // HJ 13-08-2016
}

void SI_High()
{
    GPIOPinWrite(FLASH_SPI_PORT_BASE,FLASH_MOSI_PIN,FLASH_MOSI_PIN);
    //SPIPORT|=(1<<SI);       /* set SI high */
    //_delay_us(1);         // HJ 13-08-2016
}
unsigned char Read_Status_Register()
{
    unsigned char byte = 0;
    CE_Low();               /* enable device */
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0x05);
    byte = SPI_transrecv(FLASH_SPI_BASE,0x00);
#else
    Send_Byte(0x05);            /* send RDSR command */
    byte = Get_Byte();          /* receive byte */
#endif
    CE_High();              /* disable device */
    return byte;
}

void Send_Byte(unsigned char out)
{
    SetSI_Output();
    //_delay_us(10);        //??

    if ((out & 0x80) ==0x80)
        {SI_High();}
    else
        {SI_Low();}

    SCK_High();         /* toggle clock high */

    SCK_Low();          /* toggle clock low */

    if ((out & 0x40) ==0x40)
        {SI_High();}
    else
        {SI_Low();}

    SCK_High();         /* toggle clock high */

    SCK_Low();          /* toggle clock low */


    if ((out & 0x20) ==0x20)
        {SI_High();}
    else
        {SI_Low();}

    SCK_High();         /* toggle clock high */

    SCK_Low();          /* toggle clock low */


    if ((out & 0x10) ==0x10)
        {SI_High();}
    else
        {SI_Low();}

    SCK_High();         /* toggle clock high */

    SCK_Low();          /* toggle clock low */

    if ((out & 0x08) ==0x08)
        {SI_High();}
    else
        {SI_Low();}
    SCK_High();         /* toggle clock high */

    SCK_Low();          /* toggle clock low */

    if ((out & 0x04) ==0x04)
        {SI_High();}
    else
        {SI_Low();}

    SCK_High();         /* toggle clock high */

    SCK_Low();          /* toggle clock low */


    if ((out & 0x02) ==0x02)
        {SI_High();}
    else
        {SI_Low();}
    SCK_High();         /* toggle clock high */

    SCK_Low();          /* toggle clock low */


    if ((out & 0x01) ==0x01)
        {SI_High();}
    else
        {SI_Low();}

    SCK_High();         /* toggle clock high */

    //SetSI_Input();  //Set them as Inputs      //TT 13 Aug
    SetSO_Input();

    SCK_Low();          /* toggle clock low */
/////
SI_Low();
}

unsigned char Get_Byte()
{
    unsigned char i = 0, in = 0, temp = 0;
    SetSO_Input();

    //_delay_us(10);        //Changed from 1 ms         //HJ 14-08-2016 Change from 100us
    for (i = 0; i < 8; i++)
    {
        in = (in << 1);         /* shift 1 place to the left or shift in 0 */
        //temp = ((SPIPIN & (1<<SO))>>SO) ;           /* save input */
        temp = GPIOPinRead(FLASH_SPI_PORT_BASE,FLASH_MISO_PIN)/FLASH_MISO_PIN;
        SCK_High();         /* toggle clock high */
        if (temp == 1)              /* check to see if bit is high */
            in = in | 0x01;         /* if high, make bit high */
            SCK_Low();          /* toggle clock low */
    }
    return in;

}
/************************************************************************/
/* PROCEDURE: Chip_Erase                        */
/*                                  */
/* This procedure erases the entire Chip.               */
/*                                  */
/* Input:                               */
/*      None                            */
/*                                  */
/* Returns:                             */
/*      Nothing                         */
/************************************************************************/
void Chip_Erase()
{
    CE_Low();               /* enable device */
#ifdef SST25VF064C
#ifdef HARD_FLASH_SPI
    SPI_transrecv(0x60);        //11-04-2020 SK
#else
    Send_Byte(0x60);
#endif
#else
#ifdef HARD_FLASH_SPI
    SPI_transrecv(FLASH_SPI_BASE,0xC7);        //11-04-2020 SK
#else
    Send_Byte(0xC7);
#endif
#endif
    CE_High();              /* disable device */
}

#endif  //FLASH_EN


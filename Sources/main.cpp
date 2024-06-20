

/**
 * main.c
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "flashCore.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "inc/hw_memmap.h"

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#include "_config.h"
#include "SysTick_Timer.h"
#include "IO_cntrl.h"
#include "UartCore.h"
#include "ADC_Core.h"
#include "DS3231.h"
#include "Clock.h"
#include "Display.h"
#include "E2P.h"
#include "HW_pins.h"
#include "gprs.h"
#include "gps.h"
#include "PC_Cmds.h"
#include "Web_Comm.h"
#include "Telecom_Ethernet.h"
#include "Telecom_server_query.h"
#include "main.h"
#include "_common.h"
#include "_debug.h"
#include "WDT.h"
#include "flashCore.h"
#include "flash_logger.h"

#ifdef ETHERNET_EN
#include "Sources/UIPEthernet/UIPEthernet.h"
#include <Sources/UIPEthernet/ethernet_comp.h>
#endif  // ETHERNET_EN

extern scheduler_t scheduler;

extern Rx_Buff_t   pc_uart_rx, display_uart_rx;
extern Tx_Buff_t   pc_uart_tx;//, display_uart_tx;
extern display_tx_t display_tx;

extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

extern gps_data_t gps_data;

extern time_stamp_t time_stamp;
extern measurements_t measurements;

extern e2p_router_config_t e2p_router_config;
extern e2p_cloud_config_t e2p_cloud_config;
extern e2p_device_info_t e2p_device_info;
extern e2p_location_info_t e2p_location_info;
extern e2p_config_time_t e2p_config_time;

extern voltage_mode_t voltage_mode;
#if 0
extern sys_config_t sys_config;
#endif  //if 0

ram_data_t ram_data;
Alarms_t Alarms;

extern uint32_t cnt_gps_1sec;

extern sys_mode_t System_mode;

char dummyDateBuff[7] = {0x18, 0x04, 0x19, 0x03, 0x13, 0x1F, 0x00};

unsigned int upload_time = 0;
unsigned int save_offline_time = 0;

int main(void)
{
    // static unsigned int upload_time = 0;

	// Initialize clock and peripherals //
    vMAIN_InitClockPeripherals();
	init_config();

    // flashInit();

#if RTC_SIMULATOR
    //update_date_time();
#elif defined(RTC_ENABLE)
    get_present_time(&ram_data.ram_time);
    // update_rtc(&dummyDateBuff[0], 0);
#endif
    
#ifdef  ENABLE_WDT_RESET
    initWDT();
#endif  //ENABLE_WDT_RESET

#ifdef ETHERNET_EN
    ethernet_init();
#endif  // ETHERNET_EN

	// update_rtc(&dummyDateBuff[0], 0);
	IntMasterEnable();
#ifdef DEBUG_RESTART
	vUART_SendStr(DEBUG_UART_BASE, "\nPRG STRT");
#endif

#if 0
	while(1)
	{
		vGPIO_Toggle(LED_PORT_BASE, LED2_PIN,  LED2_PIN);	//PP(25-04-24) for testing
		// GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_LOW);
		readADC(SIG_AC_VOLTAGE_ADC);
		// GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
	}
#endif

#if 1
	while(1)
	{
#ifdef ENABLE_WDT_RESET
        feedWDT();
#endif  //ENABLE_WDT_RESET
        vUART_CheckFrameTimeout(LTE_UART);
		if(scheduler.flg10ms == HIGH)
        {
            scheduler.flg10ms = LOW;
#ifdef ETHERNET_EN
            check_ethernet_message();
#endif  //ETHERNET_EN
		}

		if(scheduler.flg20ms == HIGH)
		{
			scheduler.flg20ms = LOW;
			// vGPIO_Toggle(LED_PORT_BASE, LED2_PIN,  LED2_PIN);	//PP(24-04-24) for testing
		}

		if(scheduler.flg50ms == HIGH)
        {
            scheduler.flg50ms = LOW;

			//vInput_PollingRead();
            // if(!System_mode)
            if(get_system_state() != CONFIG_MODE)
            {
#ifndef ETHERNET_EN
                // TCP_Handler();
                // gps_handler();
                manage_gps_gprs();
#else 
                ethernet_handler();
                gps_handler();   //PP commented on 16-05-24 for WDISCONN testing
#endif  //ETHERNET_EN
            }
		}

		if(scheduler.flg100ms == HIGH)
        {
            scheduler.flg100ms = LOW;
            // ToggleLEDs();
            vGPIO_Toggle(LED_PORT_BASE, LED1_PIN, LED1_PIN );
            decodeMsgPC_Uart();
		}

		if(scheduler.flg600ms == HIGH)
        {
            scheduler.flg600ms = LOW;

		}

		if(scheduler.flg1sec == HIGH)
        {
            scheduler.flg1sec = LOW;
#ifdef DEBUG_FREQ_MEAS
            vUART_SendStr(DEBUG_UART_BASE,"\nfreq:");
            vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_EXTI_cnt.freq_cnt);
#endif

			vGPIO_Toggle(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_PIN_0);
			// vInput_PollingRead();	//commenting this as I'm using these pins for ADC testing.

#if HW_BOARD == TIOT_V2_00_BOARD
            readBCD_SelectorSW();
#endif

#ifdef ADC_EN
			GetAdcData();
#endif  //ADC_EN
			if(ram_data.ram_EXTI_cnt.freq_cnt)
			{
                if(vEarthDetect())
                {
                    ram_data.ram_EXTI_cnt.earth_cnt = true;
#ifdef DEBUG_EARTHDETECT
                    vUART_SendStr(UART_PC,"\nisEARTH!");
#endif
                }
                else
                {
                    ram_data.ram_EXTI_cnt.earth_cnt = false;
                }
			}
			update_ram_data();
#if 0
			Data_Screen_lcd();
#endif  //if 0

// #if HW_BOARD == TIOT_V2_00_BOARD
// #ifdef DEBUG_ADC_SIG
//             get_ADC_SIGarray(SIG_ODU_VOLTAGE_ADC, ADC_INDX_ODUV);
// #endif  // DEBUG_ADC_SIG
// #endif  //HW_BOARD == TIOT_V2_00_BOARD


            if(get_system_state() != CONFIG_MODE)
            {
                if(++upload_time >= 30)
                {
                    upload_time = 0;	
#ifdef FLASH_EN
                    if((!get_pending_request()) && (getREQmode() == NOT_AVBL) && (!check_unsent_log())) //PP added on 11-06-24
#else
                    if(!get_pending_request() && getREQmode() == NOT_AVBL)   //PP commented on 11-06-24
#endif  //FLASH_EN
                    {
                        set_pending_request(true);
                        setServerReqType(NO_REQ);
                        setClientMSGType(SCHEDULED_LOG);
#ifdef DEBUG_SERVER_QUERY
                        vUART_SendStr(DEBUG_UART_BASE, "\nS_logs");
#endif  //DEBUG_SERVER_QUERY
                    }
                    else
                    {
#ifdef DEBUG_SERVER_QUERY
                        vUART_SendStr(DEBUG_UART_BASE, "\nREQ_p=");
                        vUART_SendInt(DEBUG_UART_BASE, getServerReqType());
                        vUART_SendChr(DEBUG_UART_BASE, ',');
                        vUART_SendInt(DEBUG_UART_BASE, getClientMSGType());
                        vUART_SendChr(DEBUG_UART_BASE, ',');
                        vUART_SendInt(DEBUG_UART_BASE, getREQmode());
#endif  //DEBUG_SERVER_QUERY
                    }
                }   //if(++upload_time >= 30)
#ifdef FLASH_EN
                if(check_unsent_log())
                {
                    set_pending_request(true);
                    setServerReqType(NO_REQ);
                    setClientMSGType(UNSENT_LOGS);
                }

                if(++save_offline_time >= SAVE_OFFLINE_TIME)
                {
                    save_offline_time = 0;
                    // if(!System_mode)
                    {
                        save_OfflineTelecomData();
                    }
                }
#endif //FLASH_EN
            }   //if(get_system_state() != CONFIG_MODE)    
		}   
        tx_pending_dataPC();
	}
#endif	//if 0

	//return 0;
}

void update_ram_data(void)
{
	memset(&ram_data.ram_ADC, 0, sizeof(measurements_t));   //PP added on 23-02-24.
    memcpy(&ram_data.ram_ADC, &measurements, sizeof(measurements_t));

	memset(&ram_data.ram_time, 0, sizeof(time_stamp_t));    //PP added on 23-02-24.
#if RTC_SIMULATOR
		//update_date_time();
#elif defined(RTC_ENABLE)
		get_present_time(&ram_data.ram_time);
		// update_rtc(&dummyDateBuff[0], 0);
#endif
        ram_data.Latitude = gps_data.Latitude;
        ram_data.Longitude = gps_data.Longitude;

#ifdef DEBUG_EPOCHTIME
    // uint32_t ET = 0;
    // // int32_t ET = 0;
    // ET = asUnixTime(ram_data.ram_time.year, ram_data.ram_time.month, ram_data.ram_time.date, ram_data.ram_time.hour, ram_data.ram_time.min, ram_data.ram_time.sec);
    // // ET = (ET + UNIX_OFFSET - SECONDS_OF_LOCAL_TIME);
    // ET = (ET - SECONDS_OF_LOCAL_TIME);
    // // ET = convertToEpochTime(&ram_data.ram_time);
    // vUART_SendStr(DEBUG_UART_BASE, "\nET=");
    // vUART_SendInt(DEBUG_UART_BASE, ET);
#endif

//    //if(e2p_router_config.router1 != HOME)
//    {
//        // ram_data.supply_mode_R1 = (e2p_router_config.router1 == MODE_36V)? (e2p_router_config.router1 * 10) : (e2p_router_config.router1 * 12); //PP 14-03-24: this is to be uncommented if ckt successfully creates 30V
//        // ram_data.supply_mode_R1 = e2p_router_config.router1 * 12;
//    }
//    //if(e2p_router_config.router2 != HOME)
//    {
//        // ram_data.supply_mode_R2 = (e2p_router_config.router2 == MODE_36V)? (e2p_router_config.router2 * 10) : (e2p_router_config.router2 * 12); //PP 14-03-24: this is to be uncommented if ckt successfully creates 30V
//        // ram_data.supply_mode_R2 = e2p_router_config.router2 * 12;
//        ram_data.supply_mode_R2 = readBCD_SelectorSW();
//    }

        readBCD_SelectorSW();
        update_alarm_status();
        controlRelays();
#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\n2ACP,RC,ODUC,ACN,ODUV,BV,SV:");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    // vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router1);
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.NE_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Charger_voltage);
#endif

// #ifdef DEBUG_GET_LOC
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nGP1s:");
//     vUART_SendInt(DEBUG_UART_BASE, cnt_gps_1sec);
// #endif  //DEBUG_GET_LOC
    
    cnt_gps_1sec = 0;

}

// void setRAM_Alarm(uint8_t val, bool sts)
void setRAM_Alarm(Alarm_sts_bits_t val, bool sts)
{
    if(sts)
    {
        ram_data.ram_alarms |= (1 << val);
    }
    else
    {
        ram_data.ram_alarms &= ~(1 << val);
    }
}

uint32_t getRAM_Alarm(void)
{
    return ram_data.ram_alarms;
}

void setRAM_Status(uint8_t val, bool sts)
{
    if(sts)
    {
        ram_data.Status |= (1 << val);
    }
    else
    {
        ram_data.Status &= ~(1 << val);
    }
}

uint32_t getRAM_Status(void)
{
    return ram_data.Status;
}

void vPERIPH_ClockInit(void)
{
    // Setting MCU clock 16MHz from External Crystal //
    SysCtlClockSet(CLOCK_CONFIGURATION);
}

void vMAIN_InitClockPeripherals(void)
{
	vPERIPH_ClockInit();
	vGPIOPortEnable();
	//vPERIPH_SystickInit();
	vPERIPH_GPIOInit();
	vPERIPH_SystickInit();
	vPERIPH_UARTInit();
#ifdef  ADC_EN
	vADC0Init();
#endif  //ADC_EN

// #if defined(FLASH_EN)
// 	//PP (24-04-24) commenting this till I have'nt made my own structs for flash for Telecom_IoT.
// 	//PP (24-04-24) The flash test project is on hold. It will be resumed after a few more functionalities have been have been tested: LCD, RTC, E2P etc
// 	flashInit();
// #endif  //FLASH_EN
	vRTCI2CInit(); 
	vPERIPH_E2PInit();
#ifdef ETHERNET_EN
    vETHERNETSPIInit();
#endif  //ETHERNET_EN
#ifdef DEBUG_FLASH_INIT
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t *)"\nFL1");
#endif
#if defined(FLASH_EN)
	//PP (24-04-24) commenting this till I have'nt made my own structs for flash for Telecom_IoT.
	//PP (24-04-24) The flash test project is on hold. It will be resumed after a few more functionalities have been have been tested: LCD, RTC, E2P etc
	flashInit();
#endif  //FLASH_EN

	memset(&ram_data, 0, sizeof(ram_data_t));

}

void write_defaults(uint32_t addr)
{
    switch(addr)
    {
        case E2P_VOLTGE_CONFIG_ADDR:
        {
            memset(&e2p_router_config, 0, sizeof(e2p_router_config_t));
            e2p_router_config.router1=MODE_12V;  
            e2p_router_config.router2=MODE_12V;  
            e2p_write_voltage_config();  
			// PP commented on 25-04-24: will do this later:
            // SetRouterMode(MODE_12V, ROUTER_1); 
            // SetRouterMode(MODE_12V, ROUTER_2); 
        }
        break;

        case E2P_CLOUD_CFG_ADDR:
        {
            memset(&e2p_cloud_config, 0, sizeof(e2p_cloud_config_t));
            //memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10000/ws/?token=SYSKAIND222", strlen((const char*)"106.201.206.208:10000/ws/?token=SYSKAIND222"));
            // memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10001/ws/?token=TELECOM111", strlen((const char*)"106.201.206.208:10001/ws/?token=TELECOM111"));
            // memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10001/ws/?token=TELECOM222", strlen((const char*)"106.201.206.208:10001/ws/?token=TELECOM222"));
            memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10001/ws/?token=", strlen((const char*)"106.201.206.208:10001/ws/?token="));
            e2p_write_cloud_cfg();
        }

        case E2P_DEVICE_CFG_ADDR:
        {
            memset(&e2p_device_info, 0, sizeof(e2p_device_info_t));
            // memcpy(e2p_device_info.device_id, "TELECOM222", strlen((const char*)"TELECOM222"));
            // memcpy(e2p_device_info.device_id, "TELECOM111", strlen((const char*)"TELECOM111"));
            // memcpy(e2p_device_info.device_id, "TELECOM444", strlen((const char*)"TELECOM444"));
            memcpy(e2p_device_info.device_id, "TELECOM333", strlen((const char*)"TELECOM333"));
            e2p_write_device_cfg();
        }
        break;

        case E2P_DEVICE_LOC_ADDR:
        {
            memset(&e2p_location_info, 0, sizeof(e2p_location_info_t));
            e2p_location_info.latitude = 26263863;
            e2p_location_info.longitude = 73008957;
            e2p_write_location();
        }
        break;

        case E2P_CONFIG_TIME_ADDR:
        {
            memset(&e2p_config_time, 0, sizeof(e2p_config_time_t));
            e2p_config_time.config_time = 5;
            e2p_write_cfg_time();
        }
        break;

        default:
        {
            memset(&e2p_router_config, 0, sizeof(e2p_router_config_t));
////////////////////////////////////////////////////////////////////////////R1//////////////////////////////////////////////////////////////////////////////////////
            //12V
            e2p_router_config.router1 = MODE_12V;   

            //24V
            // e2p_router_config.router1 = MODE_24V; 

            //36V
            // e2p_router_config.router1 = MODE_36V;

            //48V
            // e2p_router_config.router1 = MODE_48V;

////////////////////////////////////////////////////////////////////////////R2////////////////////////////////////////////////////////////////////////////////
            //12V
            e2p_router_config.router2 = MODE_12V;

            //24V
            // e2p_router_config.router2 = MODE_24V;

            //36V
            // e2p_router_config.router2 = MODE_36V;

            //48V
            // e2p_router_config.router2 = MODE_48V;

            e2p_write_voltage_config(); 
			// PP commented on 25-04-24: will do this later:
            // SetRouterMode(e2p_router_config.router1, ROUTER_1);  //PP 09-02-24 commented for testing on BOARD A which has no relay ckt correction atm //PP 07-02-24: commented for testing
            // SetRouterMode(e2p_router_config.router2, ROUTER_2);
////////////////////////////////////////////////////////////////////////CloudCfg////////////////////////////////////////////////////////////////////////////////

            memset(&e2p_cloud_config, 0, sizeof(e2p_cloud_config_t));
            //memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10000/ws/?token=SYSKAIND222", strlen((const char*)"106.201.206.208:10000/ws/?token=SYSKAIND222"));
            //memcpy(e2p_cloud_config.cloud_url, "192.168.1.101:10001/ws/?token=TELECOM111", strlen((const char*)"192.168.1.101:10001/ws/?token=TELECOM111"));
            // memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10001/ws/?token=TELECOM111", strlen((const char*)"106.201.206.208:10001/ws/?token=TELECOM111"));
            // memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10001/ws/?token=TELECOM222", strlen((const char*)"106.201.206.208:10001/ws/?token=TELECOM222"));
            memcpy(e2p_cloud_config.cloud_url, "106.201.206.208:10001/ws/?token=", strlen((const char*)"106.201.206.208:10001/ws/?token="));
            e2p_write_cloud_cfg();
//////////////////////////////////////////////////////////////////////DeviceId//////////////////////////////////////////////////////////////////////////////////

            memset(&e2p_device_info, 0, sizeof(e2p_device_info_t));
            // memcpy(e2p_device_info.device_id, "TELECOM222", strlen((const char*)"TELECOM222"));
            // memcpy(e2p_device_info.device_id, "TELECOM111", strlen((const char*)"TELECOM111"));
            // memcpy(e2p_device_info.device_id, "TELECOM444", strlen((const char*)"TELECOM444"));
            memcpy(e2p_device_info.device_id, "TELECOM333", strlen((const char*)"TELECOM333"));
            e2p_write_device_cfg();
/////////////////////////////////////////////////////////////////////FixLocation/////////////////////////////////////////////////////////////////////////////////

            memset(&e2p_location_info, 0, sizeof(e2p_location_info_t));
            e2p_location_info.latitude = 26263863;
            e2p_location_info.longitude = 73008957;
            e2p_write_location();
/////////////////////////////////////////////////////////////////////LCD_UPD_TIME////////////////////////////////////////////////////////////////////////////////
            memset(&e2p_config_time, 0, sizeof(e2p_config_time_t));
            e2p_config_time.config_time = 5;
            e2p_write_cfg_time();
            
        }
        break;
    }    
}

void init_config(void)
{
    // HAL_GPIO_SetPin(PD, _BIT(4));
    // HAL_GPIO_SetPin(PD, _BIT(1));

#ifdef NEW_BOARD
    // DFMC_EraseChipTest(0);
    write_defaults(0xFF);
    cloud_config_data();

#ifdef FLASH_EN
    clear_logs();
    clear_flash();
#endif  //FLASH_EN

#else
    if(e2p_read_voltage_config()) //PP 09-02-24 commented for testing on BOARD A which has no relay ckt correction atm //PP 07-02-24: commented for testing  //PP commented on 03-02-24 for relay testing tempporarily
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRRK",DBG_UART);
		vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRRK");
#endif
        // HAL_GPIO_SetPin(PD, _BIT(4));
        // HAL_GPIO_SetPin(PD, _BIT(1));

        voltage_mode = e2p_router_config.router1;
	    // SetRouterMode(voltage_mode, ROUTER_1); // PP commented on 25-04-24: will do this later
	    voltage_mode = e2p_router_config.router2; 
	    // SetRouterMode(voltage_mode, ROUTER_2); // PP commented on 25-04-24: will do this later
    }
    else
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRRF",DBG_UART);
		vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRRF");
#endif
        // HAL_GPIO_SetPin(PD, _BIT(4));
        // HAL_GPIO_SetPin(PD, _BIT(1));

        write_defaults(E2P_VOLTGE_CONFIG_ADDR);
    }

    if(!e2p_read_cloud_cfg())
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRCCF1",DBG_UART);
		vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRCCF1");
#endif
        write_defaults(E2P_CLOUD_CFG_ADDR);
    }


    if(!e2p_read_device_cfg())
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRIDF",DBG_UART);
		vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRIDF");
#endif  
        write_defaults(E2P_DEVICE_CFG_ADDR);     
    }
    
    if(!cloud_config_data())
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRCCF2",DBG_UART);
		vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRCCF2");
#endif
        write_defaults(E2P_CLOUD_CFG_ADDR);
    }

    if(!e2p_read_location())
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRLF",DBG_UART);
		vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRLF");
#endif  
        write_defaults(E2P_DEVICE_LOC_ADDR);     
    }

    if(!e2p_read_cfg_time())
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRTF",DBG_UART);
		vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRTF");
#endif  
        write_defaults(E2P_CONFIG_TIME_ADDR);     
    }

#ifdef FLASH_EN  
    readFreqUpdData_flash();
#endif //FLASH_EN

    // memset(&ram_data, 0, sizeof(ram_data_t));

	// // PP commented on 25-04-24: will do this later:
    // memset(&Alarms, 0, sizeof(Alarms_t));

    // Alarms.Power_ON = true;
    // setRAM_Alarm(POWER_ON_BIT,Alarms.Power_ON);

#endif  //NEW_BOARD

    memset(&ram_data, 0, sizeof(ram_data_t));   //PP 22-05-24: shifted this down to make this independent of NEW_BOARD condition.

	// PP commented on 25-04-24: will do this later:
    memset(&Alarms, 0, sizeof(Alarms_t));

    Alarms.Power_ON = true;
    setRAM_Alarm(POWER_ON_BIT,Alarms.Power_ON);

}

relay_ctrl_sts_t getRelay_CtrlState(void)
{
    relay_ctrl_sts_t relay_ctrl_sts = MAINS_MODE;
    if((Alarms.ACEarth_fault) && (Alarms.MAINS_fault))
    {
        relay_ctrl_sts = BATT_MODE;
        /*
        if(Alarms.Batt_low)
        {
            relay_ctrl_sts = BATT_LOW;
            // return BATT_LOW;
        }
        else
        {
            relay_ctrl_sts = MAINS_OFF;
            // return MAINS_OFF;
        }
        */
    }
    else if((Alarms.ACEarth_fault) && (!Alarms.MAINS_fault))
    {
        relay_ctrl_sts = EARTH_FAULT;
        // return EARTH_FAULT;
    }
    else
    {
        relay_ctrl_sts = MAINS_MODE;
        // return STS_OK;
    }
    return relay_ctrl_sts;
}

void update_alarm_status(void)
{

    double R1C = 0.000, R2C = 0.000;

    // memset(&Alarms, 0, sizeof(Alarms_t));    //PP on 23-02-24, this will now be done in init_config(), in main() before while(1).

    Alarms.MAINS_OVF_fault = ((ram_data.ram_ADC.PN_AC_Voltage/1000 < ACV_RANGE_LOW) || (ram_data.ram_ADC.PN_AC_Voltage/1000 > ACV_RANGE_HIGH)) ? true : false;
    setRAM_Alarm(MAINS_OVF, Alarms.MAINS_OVF_fault);

    Alarms.ACPhase_fault = ((ram_data.ram_EXTI_cnt.freq_cnt < FREQ_RANGE_LOW) || (ram_data.ram_EXTI_cnt.freq_cnt > FREQ_RANGE_HIGH)) ? true : false;
    setRAM_Alarm(PHASE_FAULT, Alarms.ACPhase_fault);

    Alarms.ACEarth_fault = (!ram_data.ram_EXTI_cnt.earth_cnt) ? true : false;
    setRAM_Alarm(EARTHING_FAULT, Alarms.ACEarth_fault);
#ifdef  DEBUG_MAINS_FAULT
    if(Alarms.ACEarth_fault)
    {

        vUART_SendStr(UART_PC, "\n earth FAULT");
    }
    else
    {
        vUART_SendStr(UART_PC, "\n earth ok");
    }
#endif

    Alarms.Chg_fault = ((ram_data.ram_ADC.DC_Charger_voltage/1000 < CHG_RANGE_LOW) || (ram_data.ram_ADC.DC_Charger_voltage/1000 > CHG_RANGE_HIGH)) ? true : false;
    setRAM_Alarm(SMPS_FAULT, Alarms.Chg_fault);


    if((!ram_data.ram_EXTI_cnt.freq_cnt) || (ram_data.ram_EXTI_cnt.freq_cnt < 40) /*|| (Alarms.Chg_fault)*/)
    {
        ram_data.ram_ADC.PN_AC_Voltage = 0;
        Alarms.Supply_mode = true;
        setRAM_Alarm(SUPPLY_MODE, Alarms.Supply_mode);
        Alarms.MAINS_fault = true;
#ifdef  DEBUG_MAINS_FAULT
        vUART_SendStr(UART_PC, "\nMAINS FAULT");
#endif
        setRAM_Alarm(MAINS_FAULT, Alarms.Supply_mode);
    }
    else
    {
        Alarms.Supply_mode = false;
        setRAM_Alarm(SUPPLY_MODE, Alarms.Supply_mode);
        Alarms.MAINS_fault = false;
#ifdef  DEBUG_MAINS_FAULT
        vUART_SendStr(UART_PC, "\nMAINS OK");
#endif
        setRAM_Alarm(MAINS_FAULT, Alarms.Supply_mode);
    }

    if(Alarms.Supply_mode)
    {
#ifdef DEBUG_ALARMS
        // UWriteString((char*)"\nCHG_NC",DBG_UART);
#endif
        Alarms.Batt_low = ((ram_data.ram_ADC.DC_Battery_voltage/1000 < BATT_RANGE_LOW) || (ram_data.ram_ADC.DC_Battery_voltage/1000 > BATT_RANGE_HIGH)) ? true : false;
        setRAM_Alarm(BATT_FAULT, Alarms.Batt_low);
    }
    else
    {
#ifdef DEBUG_ALARMS
        // UWriteString((char*)"\nCHG_CONN",DBG_UART);
#endif
        Alarms.Batt_low = false;
        setRAM_Alarm(BATT_FAULT, Alarms.Batt_low);
    }

    R1C = ((double)ram_data.ram_ADC.DC_current_router1/1000);
    Alarms.Router1_NC = (R1C < R1C_RANGE_LOW) ? true : false;
    setRAM_Alarm(RTR_NC, Alarms.Router1_NC);

    R2C = ((double)ram_data.ram_ADC.DC_current_router2/1000);
    Alarms.Router2_NC = (R2C < R2C_RANGE_LOW) ? true : false;
    setRAM_Alarm(ODU_NC, Alarms.Router2_NC);

    //Alarms.Router1_V_fault = ((ram_data.ram_ADC.DC_Voltage_router1/1000 < (ram_data.supply_mode_R1 - 2)) || (ram_data.ram_ADC.DC_Voltage_router1/1000 > (ram_data.supply_mode_R1 + 2))) ? true : false;
    //setRAM_Alarm(RTR_V_FAULT, Alarms.Router1_V_fault);

    Alarms.Router2_V_fault = ((ram_data.ram_ADC.DC_Voltage_router2/1000 < (ram_data.supply_mode_R2 - 2)) || (ram_data.ram_ADC.DC_Voltage_router2/1000 > (ram_data.supply_mode_R2 + 2))) ? true : false;
    setRAM_Alarm(ODU_V_FAULT, Alarms.Router2_V_fault);


}


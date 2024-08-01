/*
 * E2P.cpp
 *
 *  Created on: Apr 25, 2024
 *      Author: ADMIN
 */

#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "driverlib/sysctl.h"

#include "driverlib/eeprom.h"
#include "E2P.h"
#include "_common.h"
#include "_debug.h"
#include "HW_pins.h"
#include "IO_cntrl.h"
#include "UartCore.h"
#include "gprs.h"
#include "gps.h"
#include "main.h"

e2p_router_config_t e2p_router_config;
e2p_cloud_config_t e2p_cloud_config;
e2p_device_info_t e2p_device_info;
e2p_location_info_t e2p_location_info;
e2p_config_time_t e2p_config_time;

cloud_config_t cloud_config;

#ifdef ENABLE_GPS
extern gps_info_t gps_info;
#endif  //ENABLE_GPS
extern ram_data_t ram_data;

void vPERIPH_E2PInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0); // EEPROM activate
    EEPROMInit(); // EEPROM start
}

void e2p_write_voltage_config()
{
    uint32_t addr = E2P_VOLTGE_CONFIG_ADDR;
    // __disable_irq();

    e2p_router_config.checksum =  getChecksum((unsigned char*)(&e2p_router_config), (sizeof(e2p_router_config) - 1));

    EEPROMProgram((uint32_t *)&e2p_router_config, addr, sizeof(e2p_router_config_t));

#ifdef DEBUG_E2P
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nWRR:");
    vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_router_config,sizeof(e2p_router_config_t));
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nsizeof(rcfg)=");
    vUART_SendInt(DEBUG_UART_BASE,sizeof(voltage_mode_t));
#endif

    // __enable_irq();
}

bool e2p_read_voltage_config()
{
    uint32_t addr = E2P_VOLTGE_CONFIG_ADDR;
    //int i=0;
    unsigned int i = 0;
    bool read_ok = true;

    // __disable_irq();

    for(i=0;i<3;i++)
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRR:",DBG_UART);
        // UWriteBytes((unsigned char*)&e2p_router_config, sizeof(e2p_router_config_t), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nRR:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_router_config,sizeof(e2p_router_config_t));
#endif
        EEPROMRead((uint32_t *)&e2p_router_config, addr, sizeof(e2p_router_config_t));
        if(e2p_router_config.checksum ==  getChecksum((unsigned char*)(&e2p_router_config), (sizeof(e2p_router_config) - 1)))
        {
#ifdef DEBUG_E2P
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRRK:");
            vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_router_config,sizeof(e2p_router_config_t));
#endif
            break;
        }
    }
    // __enable_irq();

    if(i>=3)
    {
#ifdef DEBUG_E2P
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRRF:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_router_config,sizeof(e2p_router_config_t));
#endif 
        read_ok = false;
    }
    return read_ok;
}

void e2p_write_cloud_cfg(void)
{
    // __disable_irq();

    uint32_t addr = E2P_CLOUD_CFG_ADDR;
    e2p_cloud_config.checksum =  getChecksum((unsigned char*)(&e2p_cloud_config), (sizeof(e2p_cloud_config_t) - 1));

    EEPROMProgram((uint32_t *)&e2p_cloud_config, addr, sizeof(e2p_cloud_config_t));

#ifdef DEBUG_E2P
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nWCC:");
    vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_cloud_config,sizeof(e2p_cloud_config_t));
#endif

    // __enable_irq();
}

bool e2p_read_cloud_cfg(void)
{
    bool read_ok = true;
    //int i=0;
    unsigned int i = 0;
	uint32_t addr = E2P_CLOUD_CFG_ADDR;

    // __disable_irq();

    for (i = 0; i < 3; i++)
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRCCR:",DBG_UART);
        // UWriteBytes((unsigned char*)&e2p_cloud_config, sizeof(e2p_cloud_config_t), DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nRCCR:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_cloud_config,sizeof(e2p_cloud_config_t));
#endif
        EEPROMRead((uint32_t *)&e2p_cloud_config, addr, sizeof(e2p_cloud_config_t));
        if(e2p_cloud_config.checksum ==  getChecksum((unsigned char*)(&e2p_cloud_config), (sizeof(e2p_cloud_config_t) - 1)))
        {
#ifdef DEBUG_E2P
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRCCK:");
            vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_cloud_config,sizeof(e2p_cloud_config_t));
#endif 
            break;
        }
    }
    // __enable_irq();

    if(i>=3)
	{
#ifdef DEBUG_E2P
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRCCF:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_cloud_config,sizeof(e2p_cloud_config_t));
#endif 
		read_ok = false;
	}
	return read_ok;
}

char * get_url()
{
    return (char *)&e2p_cloud_config.cloud_url;
}

void e2p_write_device_cfg()
{
    // __disable_irq();
    uint32_t addr = E2P_DEVICE_CFG_ADDR;

    e2p_device_info.checksum =  getChecksum((unsigned char*)(&e2p_device_info), (sizeof(e2p_device_info_t) - 1));
    EEPROMProgram((uint32_t *)&e2p_device_info, addr, sizeof(e2p_device_info_t));

#ifdef DEBUG_E2P
    // UWriteString((char*)"\nWDID:",DBG_UART);
    // UWriteBytes((unsigned char*)&e2p_device_info,sizeof(e2p_device_info_t),DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nwDID:");
    vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_device_info,sizeof(e2p_device_info_t));
#endif
    // __enable_irq();
}

bool e2p_read_device_cfg(void)
{
    bool read_ok = true;
    int i=0;
        uint32_t addr = E2P_DEVICE_CFG_ADDR;
    // __disable_irq();
    for (i = 0; i < 3; i++)
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRDID:",DBG_UART);
        // UWriteBytes((unsigned char*)&e2p_device_info,sizeof(e2p_device_info_t),DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nRDID:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_device_info,sizeof(e2p_device_info_t));
#endif
        EEPROMRead((uint32_t *)&e2p_device_info, addr, sizeof(e2p_device_info_t));
        if(e2p_device_info.checksum ==  getChecksum((unsigned char*)(&e2p_device_info), (sizeof(e2p_device_info_t) - 1)))
        {
#ifdef DEBUG_E2P
            // UWriteString((char*)"\nRDIDK:",DBG_UART);
            // UWriteBytes((unsigned char*)&e2p_device_info,sizeof(e2p_device_info_t),DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRDIDK");
            vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_device_info,sizeof(e2p_device_info_t));
#endif
            break;
        }
    }
    // __enable_irq();
    if(i>=3)
    {
        read_ok = false;
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRDIDF:",DBG_UART); //PP 03-02-24:To Vinay: why was this freely called?
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRDIDF");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_device_info,sizeof(e2p_device_info_t));
#endif 
    }
    return read_ok ;
}

void e2p_write_location()
{
    // memset(&e2p_location_info,0,sizeof(e2p_location_info_t));
    // e2p_location_info.latitude=26263863;
    // e2p_location_info.longitude=73008957;    //PP commented on 03-02-24, To vinay: Pls do not hardcode things if you need to test things, follow standard and use write_defaults() function.

    // __disable_irq();
    uint32_t addr = E2P_DEVICE_LOC_ADDR;

    e2p_location_info.checksum =  getChecksum((unsigned char*)(&e2p_location_info), (sizeof(e2p_location_info_t) - 1));
    EEPROMProgram((uint32_t *)&e2p_location_info, addr, sizeof(e2p_location_info_t));

#ifdef DEBUG_E2P
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nWL:");
    vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_location_info,sizeof(e2p_location_info_t));
#endif

    // __enable_irq();
}

bool e2p_read_location(void)
{
    bool read_ok = true;
    int i=0;
    uint32_t addr = E2P_DEVICE_LOC_ADDR;
    // __disable_irq();
    for (i = 0; i < 3; i++)
    {
        EEPROMRead((uint32_t *)&e2p_location_info, addr, sizeof(e2p_location_info_t));
#ifdef DEBUG_E2P
        vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nRLOC:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_location_info,sizeof(e2p_location_info_t));
#endif
        if(e2p_location_info.checksum ==  getChecksum((unsigned char*)(&e2p_location_info), (sizeof(e2p_location_info_t) - 1)))
        {
#ifdef DEBUG_E2P
            vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nRLK:");
            vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_location_info,sizeof(e2p_location_info_t));
#endif
            break;
        }
    }
    // __enable_irq();
    if(i>=3)
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRLF:",DBG_UART);
        // UWriteBytes((unsigned char*)&e2p_location_info,sizeof(e2p_location_info_t),DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRLF");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_location_info,sizeof(e2p_location_info_t));
#endif
        read_ok = false;
    }
// #ifdef DEBUG_E2P
//     // UWriteString((char*)"\nRLF:",DBG_UART);
//     // UWriteBytes((unsigned char*)&e2p_location_info,sizeof(e2p_location_info_t),DBG_UART);
//     vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRLF");
//     vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_location_info,sizeof(e2p_location_info_t));
// #endif
    return read_ok;
}

void e2p_write_cfg_time()
{
    // memset(&e2p_config_time,0,sizeof(e2p_config_time_t));
    // e2p_config_time.config_time = 5; //PP commented on 03-02-24, To vinay: Pls do not hardcode things if you need to test things, follow standard and use write_defaults() function.

    // __disable_irq();
    uint32_t addr = E2P_CONFIG_TIME_ADDR;
    e2p_config_time.checksum =  getChecksum((unsigned char*)(&e2p_config_time), (sizeof(e2p_config_time_t) - 1));

    EEPROMProgram((uint32_t *)&e2p_config_time, addr, sizeof(e2p_config_time_t));

#ifdef DEBUG_E2P
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nWT:");
    vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_config_time,sizeof(e2p_config_time_t));
#endif

    // __enable_irq();
}

bool e2p_read_cfg_time(void)
{
    bool read_ok = true;
    int i=0;
    uint32_t addr = E2P_CONFIG_TIME_ADDR;
    // __disable_irq();
    for (i = 0; i < 3; i++)
    {
#ifdef DEBUG_E2P
        // UWriteString((char*)"\nRT:",DBG_UART);
        // UWriteBytes((unsigned char*)&e2p_config_time,sizeof(e2p_config_time_t),DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nRT:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_config_time,sizeof(e2p_config_time_t));
#endif
        EEPROMRead((uint32_t *)&e2p_config_time, addr, sizeof(e2p_config_time_t));
        if(e2p_config_time.checksum ==  getChecksum((unsigned char*)(&e2p_config_time), (sizeof(e2p_config_time_t) - 1)))
        {
#ifdef DEBUG_E2P
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRTK:");
            vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_config_time,sizeof(e2p_config_time_t));
#endif 
            break;
        }
    }
    // __enable_irq();
    if(i>=3)
    {
#ifdef DEBUG_E2P
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRTF:");
        vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)&e2p_config_time,sizeof(e2p_config_time_t));
#endif 
        read_ok = false;
    }
// #ifdef DEBUG_E2P
//         UWriteString((char*)"\nread_time:",DBG_UART);
//         UWriteBytes((unsigned char*)&e2p_config_time,sizeof(e2p_config_time_t),DBG_UART);
// #endif
    return read_ok;
}

bool set_url(char *url)
{
	int i = 0;
    char temp_url[127];
    memset(temp_url, 0, sizeof(temp_url));
	//memset(&e2p_cloud_config.cloud_url,0,sizeof(e2p_cloud_config.cloud_url));

	while(url[i] != '\0')
	{
		//e2p_cloud_config.cloud_url[i] = url[i];
        temp_url[i] = url[i];

#ifdef DEBUG_PC_CMDS
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ni:");
        vUART_SendInt(DEBUG_UART_BASE, i);
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ncloud_url[j],url[i]:");
        vUART_SendChr(DEBUG_UART_BASE, temp_url[i]);
        vUART_SendChr(DEBUG_UART_BASE, ',');
        vUART_SendChr(DEBUG_UART_BASE, url[i]);
		//printf("\ncloud_url[%d]=%c,url[%d]=%c", i, e2p_cloud_config.cloud_url[i], i, url[i]);
#endif
		i++;
		if(i > CLOUD_URL_MAX_LEN)
		{
#ifdef  DEBUG_PC_CMDS
			// printf("\nSUF");
            // UWriteString((char*)"\nSUF",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSUF");
#endif
			return false;
            // break;
		}
	}

    memset(&e2p_cloud_config.cloud_url,0,sizeof(e2p_cloud_config.cloud_url));
    memcpy(e2p_cloud_config.cloud_url, temp_url, strlen((const char*)temp_url));
	e2p_write_cloud_cfg();
	return true;
}

bool set_device_id(char *id)
{
	int i = 0;
    char temp_device_id[15];
    memset(temp_device_id, 0, sizeof(temp_device_id));
	//memset(&e2p_device_info.device_id,0,sizeof(e2p_device_info.device_id));

	while(id[i] != '\0')
	{
		// e2p_device_info.device_id[i] = id[i];
        temp_device_id[i] = id[i];
#ifdef DEBUG_PC_CMDS
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ni:");
        vUART_SendInt(DEBUG_UART_BASE, i);
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ndID[i],ID[i]:");
        vUART_SendChr(DEBUG_UART_BASE, temp_device_id[i]);
        vUART_SendChr(DEBUG_UART_BASE, ',');
        vUART_SendChr(DEBUG_UART_BASE, id[i]);
		// printf("\ndevice_id[%d]=%c,id[%d]=%c", i, e2p_device_info.device_id[i], i, id[i]);
#endif
		i++;
		if(i >= DEVICE_ID_MAX_LEN)
		{
#ifdef  DEBUG_PC_CMDS
			// printf("\nSIF");
            // UWriteString((char*)"\nSIF",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSIF");
#endif
			return false;
            // break;
		}
	}
    memset(&e2p_device_info.device_id,0,sizeof(e2p_device_info.device_id));
    memcpy(e2p_device_info.device_id, temp_device_id, strlen((const char*)temp_device_id));
	e2p_write_device_cfg();
	return true;
}

char *get_device_id(void)
{
	return (char *)&e2p_device_info.device_id;
}

bool set_fixed_location(char *location)
{
	int i = 0, j = 0;//, cnt = 0;
	memset(&e2p_location_info,0,sizeof(e2p_location_info_t));
	char temp_lat[LAT_LEN];
	char temp_long[LONG_LEN];
	
	for(i = 0; location[i] != ','; i++)
	{
		temp_lat[j] = location[i];

#ifdef DEBUG_PC_CMDS
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nj,i:");
        vUART_SendInt(DEBUG_UART_BASE, j);
        vUART_SendChr(DEBUG_UART_BASE, ',');
        vUART_SendInt(DEBUG_UART_BASE, i);
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ntLat[j],Loc[i]:");
        vUART_SendChr(DEBUG_UART_BASE, temp_lat[j]);
        vUART_SendChr(DEBUG_UART_BASE, ',');
        vUART_SendChr(DEBUG_UART_BASE, location[i]);
		// printf("\ntemp_lat[%d]=%c,location[%d]=%c", j, temp_lat[j], i, location[i]);
#endif
		
		if(j >= LAT_LEN)
		{
#ifdef  DEBUG_PC_CMDS
			// printf("\nLAT_F");
            // UWriteString((char*)"\nLAT_F",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nLAT_F");
#endif
			//break;
			return false;
		}
		
		j++;
	}
	
#ifdef  DEBUG_PC_CMDS
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ne2p_Lat1=");
    vUART_SendInt(DEBUG_UART_BASE, atoi(temp_lat));
	// printf("\ne2p_Lat1=%d", atoi(temp_lat));
#endif

// 	e2p_location_info.latitude = atoi(temp_lat);    //PP shifting this down so that if it has to return false, it'll do so without disturbing e2p vars.

// #ifdef  DEBUG_PC_CMDS
//     UWriteString((char*)"\ne2p_Lat2=",DBG_UART);
//     UWriteInt(e2p_location_info.latitude,DBG_UART);
// 	// printf("\ne2p_Lat2=%d", e2p_location_info.latitude);
// #endif
	
	j = 0;
#ifdef  DEBUG_PC_CMDS
    // UWriteString((char*)"\ni,loc[i]=",DBG_UART);
    // UWriteInt(i,DBG_UART);
    // UWriteData(',',DBG_UART);
    // UWriteData(location[i],DBG_UART);
	// printf("\nlocation[%d]=%c",i, location[i]);
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ni,loc[i]=");
    vUART_SendInt(DEBUG_UART_BASE, i);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE, location[i]);
#endif 

	i++;
#ifdef  DEBUG_PC_CMDS
    // UWriteString((char*)"\ni,loc[i]=",DBG_UART);
    // UWriteInt(i,DBG_UART);
    // UWriteData(',',DBG_UART);
    // UWriteData(location[i],DBG_UART);
	// printf("\nlocation[%d]=%c",i, location[i]);
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ni,loc[i]=");
    vUART_SendInt(DEBUG_UART_BASE, i);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE, location[i]);
#endif
	
	for(; location[i] != '\0';)
	{
		temp_long[j] = location[i];

#ifdef DEBUG_PC_CMDS
        // UWriteString((char*)"\nj,i:",DBG_UART);
        // UWriteInt(j,DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteInt(i,DBG_UART);
        // UWriteString((char*)"\ntLong[j],Loc[i]:",DBG_UART);
        // UWriteData(temp_long[j],DBG_UART);
        // UWriteData(',',DBG_UART);
        // UWriteData(location[i],DBG_UART);
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nj,i:");
        vUART_SendInt(DEBUG_UART_BASE, j);
        vUART_SendChr(DEBUG_UART_BASE, ',');
        vUART_SendInt(DEBUG_UART_BASE, i);
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ntLong[j],Loc[i]:");
        vUART_SendChr(DEBUG_UART_BASE, temp_long[j]);
        vUART_SendChr(DEBUG_UART_BASE, ',');
        vUART_SendChr(DEBUG_UART_BASE, location[i]);
		// printf("\ntemp_long[%d]=%c,location[%d]=%c", j, temp_long[j], i, location[i]);
#endif
		
		if(j >= LONG_LEN)
		{
#ifdef  DEBUG_PC_CMDS
			// printf("\nLONG_F");
            // UWriteString((char*)"\nLONG_F",DBG_UART);
            vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nLONG_F");
#endif
			//break;
			return false;
		}
        	
		i++;
		j++;
	}
	
#ifdef  DEBUG_PC_CMDS
    // UWriteString((char*)"\ne2p_Long1=",DBG_UART);
    // UWriteInt(atoi(temp_long),DBG_UART);
	// printf("\ne2p_Long1=%d", atoi(temp_long));
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ne2p_Long1=");
    vUART_SendInt(DEBUG_UART_BASE, atoi(temp_long));
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE, i);
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ntLong[j],Loc[i]:");
    vUART_SendChr(DEBUG_UART_BASE, temp_long[j]);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendChr(DEBUG_UART_BASE, location[i]);
#endif

    e2p_location_info.latitude = atoi(temp_lat);    //PP shifted this down here so that if it has to return false, it'll do so without disturbing e2p vars.
	e2p_location_info.longitude = atoi(temp_long);

#ifdef  DEBUG_PC_CMDS
    // UWriteString((char*)"\ne2p_Lat2=",DBG_UART);
    // UWriteInt(e2p_location_info.latitude,DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ne2p_Lat2=");
    vUART_SendInt(DEBUG_UART_BASE, e2p_location_info.latitude);
	// printf("\ne2p_Lat2=%d", e2p_location_info.latitude);

    // UWriteString((char*)"\ne2p_Long2=",DBG_UART);
    // UWriteInt(e2p_location_info.longitude,DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\ne2p_Long2=");
    vUART_SendInt(DEBUG_UART_BASE, e2p_location_info.longitude);
	// printf("\ne2p_Long2=%d", e2p_location_info.longitude);
#endif

	
	e2p_write_location();
	return true;
}

char *get_fixed_location()
{
    return (char *)&e2p_location_info;
}

#if 0   //PP commented on 25-12-24: will uncomment this later when gps is implemented.
bool set_current_location()
{
    //if((gps_info.Latitude != '\0') && (gps_info.Longitude != '\0'))
    if(!get_location())
    {
        return false;
    }
    else
    {
        e2p_location_info.latitude = ram_data.Latitude;
        e2p_location_info.longitude = ram_data.Longitude;

        e2p_write_location();
    }
    
    return true;
}
#endif  // if 0

bool cloud_config_data(void)
{
    //int i=0,j=0;
    unsigned i=0,j=0;
    uint8_t temp_buff[128];//,buff[200];
    memset(temp_buff,0,sizeof(temp_buff));
    memset((void *)&cloud_config,0,sizeof(cloud_config_t));

    char *data =  get_url();

#ifdef DEBUG_CLOUD_CFG_DECODE
    UWriteString((char*)"\nurl=",DBG_UART);
    UWriteString(data,DBG_UART);
#endif

    for(;data[i] != ':';)
    {
        if(i >= 15)
        {
#ifdef DEBUG_CLOUD_CFG_DECODE
            UWriteString((char*)"\nipF",DBG_UART);
#endif
            return 0;
        }
        cloud_config.ip_addr[i] = data[i];
        i++;
    }

// #ifdef DEBUG_CLOUD_CFG_DECODE
#ifdef DEBUG_SERVER_QUERY
    // UWriteString((char*)"\nip=",DBG_UART);
    // UWriteString((char*)cloud_config.ip_addr,DBG_UART);
    vUART_SendStr(UART_PC, "\nip=");
    vUART_SendStr(UART_PC, cloud_config.ip_addr);
#endif

    i++;

    for(;data[i] != '/';)
    {
        if(j >= 5)
        {
#ifdef DEBUG_CLOUD_CFG_DECODE
            UWriteString((char*)"\npnF",DBG_UART);
#endif
            return 0;
        }
        temp_buff[j] = data[i];
        i++;
        j++;
    }

    cloud_config.port_num = atoi((const char *)temp_buff);

// #ifdef DEBUG_CLOUD_CFG_DECODE
#ifdef DEBUG_SERVER_QUERY
    // UWriteString((char*)"\npn=",DBG_UART);
    // UWriteInt(cloud_config.port_num,DBG_UART);
    vUART_SendStr(UART_PC, "\npn=");
    vUART_SendInt(UART_PC, cloud_config.port_num);
#endif

    memset(temp_buff,0,sizeof(temp_buff));
    j=0;

    for(;data[i] != '\0';)
    {
        if(i >= sizeof(e2p_cloud_config.cloud_url))
        //if(i >= 127)
        {
#ifdef DEBUG_CLOUD_CFG_DECODE
            UWriteString((char*)"\npaF",DBG_UART);
#endif
            return 0;
        }
        temp_buff[j] = data[i];
        i++;
        j++;
    }

    // my_sprintf((char *)buff,3,"%s%s",temp_buff,system_info.charge_box_id);
    // memcpy(&cloud_config.path,buff,sizeof(buff));
    memcpy(&cloud_config.path,temp_buff,sizeof(temp_buff));

    memcpy(&cloud_config.path[strlen((const char*)cloud_config.path)], e2p_device_info.device_id, strlen((const char*)e2p_device_info.device_id));

// #ifdef DEBUG_CLOUD_CFG_DECODE
#ifdef DEBUG_SERVER_QUERY
    // UWriteString((char*)"\npa=",DBG_UART);
    // UWriteString((char*)cloud_config.path,DBG_UART);
    vUART_SendStr(UART_PC, "\npa=");
    vUART_SendStr(UART_PC, cloud_config.path);
#endif

    return true;
}


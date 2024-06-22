/*
 * PC_Cmds.cpp
 *
 *  Created on: Apr 25, 2024
 *      Author: ADMIN
 */

#include <string.h>
#include <stdlib.h>
#include "stdint.h"
#include "stdbool.h"

#include "UartCore.h"
#include "_common.h"
#include "main.h"
#include "_config.h"
#include <driverlib/sysctl.h>
#include "gprs.h"
#include "gps.h"
#include "PC_Cmds.h"
#include "E2P.h"
#include "_debug.h"
#include "Clock.h"
#include "flashCore.h"
#include "flash_logger.h"

extern Rx_Buff_t  pc_uart_rx;
extern Tx_Buff_t  pc_uart_tx;

extern e2p_router_config_t e2p_router_config;
extern e2p_cloud_config_t e2p_cloud_config;
extern e2p_device_info_t e2p_device_info;
extern e2p_location_info_t e2p_location_info;
extern e2p_config_time_t e2p_config_time;

sys_mode_t System_mode;// = CONFIG_MODE;

void decodeMsgPC_Uart(void)
{
    if(pc_uart_rx.rx_state == READY)
    {
        response((cmd_t)pc_uart_rx.rx_buffer[CMD_INDX], UART_PC);
    }
    pc_uart_rx.rx_state = START;
    flushRxBuffer(UART_PC);
}

void preparePC_ResponsePacket(cmd_t cmd, char *data, unsigned char len, int sendAsIs, int immediateSend)
{
    flushTxBuffer(UART_PC);
	
	unsigned int i = 0;

    if (!sendAsIs)
	{
		pc_uart_tx.tx_indx = 0;
		
		pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx++] = 0xAA;
		
		pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx++] = len;
		
		pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx++] = (cmd | 0x80);		//Anand 21.05.2014
		while(i<(len-1))
		{
			pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx] = data[i];
			i++;
			pc_uart_tx.tx_indx++;
		}
		
		pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx++] = getChecksum((unsigned char*)&pc_uart_tx.tx_buffer[CMD_INDX], len);
		
		pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx++] = 0x55;
	}
	else
	{
		pc_uart_tx.tx_indx = 0;
		
		while(pc_uart_tx.tx_indx < len)
		{
			pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx] = data[pc_uart_tx.tx_indx];
			++pc_uart_tx.tx_indx;
		}
	}
	
	pc_uart_tx.tx_ready = TRUE;		//Anand 21.05.2014
	
	if(immediateSend)
	{
		pc_uart_tx.tx_ready = FALSE;
		Usendbuffer(UART_PC);
		flushTxBuffer(UART_PC);
	}
}

void response(cmd_t cmd, int uart_no)
{
    char resp[3];
	resp[0] = FALSE;

    sys_mode_t temp_sys_mode;

    switch (cmd)
    {
        case SET_MODE:
        {
            temp_sys_mode = (sys_mode_t)pc_uart_rx.rx_buffer[DATA_INDX];

            switch(temp_sys_mode)
            {
                case RUNNING_MODE:
				{
					// if(get_system_state() == RUNNING_MODE)   //copied this from syska iot, it's confusing as it sets opposite of that of the rx'ed enum value so this is commented
					// {
					// 	set_system_state(CONFIG_MODE);
					// 	resp[0] = ACK;
					// }
					// else
					// {
					// 	resp[0] = NACK;
					// }

                    if(get_system_state() == RUNNING_MODE)  //already in lcd mode
					{
#ifdef DEBUG_PC_CMDS
						// UWriteString((char*)"\nAlready_LCD_MODE",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nAlready_LCD_MODE");
#endif
						resp[0] = NACK;
					}
					else
					{
#ifdef DEBUG_PC_CMDS
						// UWriteString((char*)"\nSW_LCD_MODE",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSW_LCD_MODE");
#endif
                        set_system_state(RUNNING_MODE); //set to lcd mode if it's not in lcd mode
						resp[0] = ACK;
					}
					
				}
				break;
				
				case CONFIG_MODE:
				{
					// if(get_system_state() == CONFIG_MODE)    //copied this from syska iot, it's confusing as it sets opposite of that of the rx'ed enum value so this is commented
					// {
					// 	set_system_state(RUNNING_MODE);
					// 	resp[0] = ACK;
					// }
					// else
					// {
					// 	resp[0] = NACK;
					// }

                    if(get_system_state() == CONFIG_MODE)
					{
#ifdef DEBUG_PC_CMDS
						// UWriteString((char*)"\nAlready_CFG_MODE",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nAlready_CFG_MODE");
#endif
						resp[0] = NACK; //already in cfg mode
					}
					else
					{
#ifdef DEBUG_PC_CMDS
						// UWriteString((char*)"\nSW_CFG_MODE",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSW_CFG_MODE");
#endif
                        set_system_state(CONFIG_MODE);  //set to cfg mode if it's not in cfg mode
						resp[0] = ACK;
					}
				}
				break;
				
				default:
				break;
            }
        }
        break;

        case SET_GPRS_URL:
        {
            if(get_system_state() == CONFIG_MODE)
			{
#ifdef DEBUG_PC_CMDS
				// UWriteString((char*)"\nSU",DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSU");
#endif
				if(set_url((char *)&pc_uart_rx.rx_buffer[DATA_INDX]))
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSUK",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSUK");
#endif
					if((!e2p_read_cloud_cfg()) || (!cloud_config_data()))
    				{
#ifdef DEBUG_PC_CMDS
        				// UWriteString((char*)"\nSUF2",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSUF2");
#endif
						resp[0] = NACK;
					}
					else if((e2p_read_cloud_cfg()) && (cloud_config_data()))
					{
#ifdef DEBUG_PC_CMDS
        				// UWriteString((char*)"\nSUk2",DBG_UART);
                        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSUk2");
#endif						
						resp[0] = ACK;
					}
				}
				else
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSUF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSUF");
#endif
					resp[0] = NACK;
				}	
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
        }
        break;

        case GET_GPRS_URL:
		{
			if(get_system_state() == CONFIG_MODE)
			{
				char *data = get_url();
				//preparePC_ResponsePacket(uart_no, GET_GPRS_URL, data, sizeof(e2p_server_data.server_url) + 1);
                // preparePC_ResponsePacket(uart_no, GET_GPRS_URL, data, strlen(data), FALSE, TRUE);
				preparePC_ResponsePacket(GET_GPRS_URL, data, strlen(data) + 1, FALSE, TRUE);
            }
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}
		break;

        case SET_DEVICE_ID:
		{
			if(get_system_state() == CONFIG_MODE)
			{
				if(set_device_id((char *)&pc_uart_rx.rx_buffer[DATA_INDX]))
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSIK",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSIK");
#endif
					resp[0] = ACK;
				}
				else
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSIF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSIF");
#endif
					resp[0] = NACK;
				}
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}
		break;

        case GET_DEVICE_ID:
		{
			if(get_system_state() == CONFIG_MODE)
			{
				char *data = get_device_id();
				preparePC_ResponsePacket(GET_DEVICE_ID, data, sizeof(e2p_device_info.device_id) + 1, FALSE, TRUE);
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}
		break;

        case SET_FIXED_LOC:
        {
            if(get_system_state() == CONFIG_MODE)
			{
				if(set_fixed_location((char*)&pc_uart_rx.rx_buffer[DATA_INDX]))
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSLK",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSLK");
#endif
					resp[0] = ACK;
				}
				else
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSLF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSLF");
#endif
					resp[0] = NACK;
				}
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
            }
        }
        break;

        case GET_FIXED_LOC:
        {
            if(get_system_state() == CONFIG_MODE)
			{
                //char *data;
				char *data = get_fixed_location();
#ifdef DEBUG_PC_CMDS
				// UWriteString((char*)"\nGL:",DBG_UART);
				// UWriteInt(e2p_location_info.latitude, DBG_UART);
				// UWriteData(',',DBG_UART);
				// UWriteInt(e2p_location_info.longitude, DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nGL:");
                vUART_SendInt(DEBUG_UART_BASE, e2p_location_info.latitude);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, e2p_location_info.longitude);
#endif
				preparePC_ResponsePacket(GET_FIXED_LOC, data, sizeof(e2p_location_info) + 1, FALSE, TRUE);
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
        }
        break;
#if 0
        case SET_ROUTER_CFG:
        {
            if(get_system_state() == CONFIG_MODE)
			{
				int R1_mode = 0, ODU_mode = 0;

				R1_mode = pc_uart_rx.rx_buffer[DATA_INDX]/12;
				ODU_mode = pc_uart_rx.rx_buffer[DATA_INDX + 1]/12;

				if(((!R1_mode) || (R1_mode >= 5)) || ((!ODU_mode) || (ODU_mode >= 5)))
				{
					resp[0] = NACK;
					break;
				}

				// e2p_router_config.router1 = ((!R1_mode) || (R1_mode >= 5))? e2p_router_config.router1 : (voltage_mode_t)R1_mode;

				// e2p_router_config.router2 = ((!ODU_mode) || (ODU_mode >= 5))? e2p_router_config.router2 : (voltage_mode_t)ODU_mode;

				e2p_router_config.router1 = (voltage_mode_t)R1_mode;

				e2p_router_config.router2 = (voltage_mode_t)ODU_mode;

				e2p_write_voltage_config();

                // PP commented on 25-04-24: will do this later:
				// SetRouterMode(e2p_router_config.router1, ROUTER_1);
				// SetRouterMode(e2p_router_config.router2, ROUTER_2);

#ifdef DEBUG_PC_CMDS
				// UWriteString((char*)"\nSRM:",DBG_UART);
				// UWriteInt(R1_mode, DBG_UART);
				// UWriteData(',',DBG_UART);
				// UWriteInt(ODU_mode, DBG_UART);
                vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSRM:");
                vUART_SendInt(DEBUG_UART_BASE, R1_mode);
                vUART_SendChr(DEBUG_UART_BASE, ',');
                vUART_SendInt(DEBUG_UART_BASE, ODU_mode);
#endif				

				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
            }
        }
        break;

        case GET_ROUTER_CFG:
        {
            if(get_system_state() == CONFIG_MODE)
			{
				if(e2p_read_voltage_config())
				{
					char data[3];
					memset(data, 0, sizeof(data));
					data[0] = e2p_router_config.router1 * 12;
					data[1] = e2p_router_config.router2 * 12;
					// preparePC_ResponsePacket(uart_no, GET_ROUTER_CFG, (char*)&e2p_router_config, sizeof(e2p_router_config) + 1, FALSE, TRUE);
					preparePC_ResponsePacket(GET_ROUTER_CFG, data, sizeof(data) + 1, FALSE, TRUE);
				}
				else
				{
					resp[0] = NACK;
				}
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
        }
        break;
#endif	//if 0

		case SET_LCD_UPD_TIME:
		{
			if(get_system_state() == CONFIG_MODE)
			{
				if(pc_uart_rx.rx_buffer[DATA_INDX])
				{
					e2p_config_time.config_time = pc_uart_rx.rx_buffer[DATA_INDX];
					e2p_write_cfg_time();
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSLTK:",DBG_UART);
					// UWriteInt(e2p_config_time.config_time,DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSLTK:");
                    vUART_SendInt(DEBUG_UART_BASE, e2p_config_time.config_time);
#endif
					resp[0] = ACK;
				}
				else
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSLTF",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSLTF");
#endif
					resp[0] = NACK;
				}
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
            }
		}
		break;

		case GET_LCD_UPD_TIME:
		{
			if(get_system_state() == CONFIG_MODE)
			{
				preparePC_ResponsePacket(GET_LCD_UPD_TIME, (char*)&e2p_config_time, sizeof(e2p_config_time_t) + 1, FALSE, TRUE);
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}
		break;

#if 0
		case SET_CURRENT_LOC:
		{
			if(get_system_state() == CONFIG_MODE)
			{
				if(set_current_location())
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSCLK:",DBG_UART);
					// UWriteInt(e2p_location_info.latitude, DBG_UART);
					// UWriteData(',',DBG_UART);
					// UWriteInt(e2p_location_info.longitude, DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSCLK:");
                    vUART_SendInt(DEBUG_UART_BASE, e2p_location_info.latitude);
                    vUART_SendChr(DEBUG_UART_BASE, ',');
                    vUART_SendInt(DEBUG_UART_BASE, e2p_location_info.longitude);
#endif
					resp[0] = ACK;
				}
				else
				{
#ifdef DEBUG_PC_CMDS
					// UWriteString((char*)"\nSCLF:",DBG_UART);
                    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nSCLF:");
#endif					
					resp[0] = NACK;
				}
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}	
		break;	
#endif  //if 0

        case SET_RTC_DATETIME:
        {
            if(get_system_state() == CONFIG_MODE)
            {
                int indx = DATA_INDX;

                update_rtc((char *)&pc_uart_rx.rx_buffer[0], indx);

                resp[0] = ACK;
            }
            else
            {
                resp[0] = ERROR_INVALID_STATE;
            }
        } 
        break;

        case GET_RTC_DATETIME:
		{		
			if(get_system_state() == CONFIG_MODE)
			{				
				//char temp[6];
				time_stamp_t t;
				get_present_time(&t);
				preparePC_ResponsePacket(GET_RTC_DATETIME,(char *)&t, 7, FALSE, TRUE);
			}
			else
			{
				resp[0] = ERROR_INVALID_CMD;
			}
		}
		break;

		case FACTORY_RESET:
		{
			if(get_system_state() == CONFIG_MODE)
			{
#ifdef FLASH_EN
				clear_logs();
                clear_flash();
#endif
                // write_defaults(0xFF);
				write_defaults(E2P_VOLTGE_CONFIG_ADDR);
				write_defaults(E2P_CLOUD_CFG_ADDR);
				write_defaults(E2P_DEVICE_LOC_ADDR);
				write_defaults(E2P_CONFIG_TIME_ADDR);

				vUART_SendStr(UART_PC, "\nPress_RST_button");

				resp[0] = ACK;

				preparePC_ResponsePacket(FACTORY_RESET,resp, 2, FALSE, TRUE);

				while(1)
				{}
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}
		break;

#ifdef FLASH_EN
		case WRITE_FLASH:
		{
			if(get_system_state() == CONFIG_MODE)
			{
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\n1FLw:");
#endif
				unsigned long size,addr,index = DATA_INDX;
                char total_data[64];
                int i;
                addr = pc_uart_rx.rx_buffer[index++];
                addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 8);
                addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 16);
                addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 24);

                size = pc_uart_rx.rx_buffer[index++];
                size |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 8);

                for(i = 0; i < size; i++)
                {
                    total_data[i] = pc_uart_rx.rx_buffer[index++];
                }
#ifdef FLASH_WP_ENABLE
                remove_block_protection();
#endif
                WREN();
                flashPacketProgram((char*)total_data, size, addr);
                Wait_Busy();
                WRDI();             // HJ 29-12-2015    // Write DisableWRDI();
#ifdef FLASH_WP_ENABLE
                WBPR(0);
#endif
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\n2FLw:");
                vUART_SendInt(UART_PC,addr);
                vUART_SendChr(UART_PC,',');
                vUART_SendInt(UART_PC,size);
                vUART_SendChr(UART_PC,',');
                vUART_SendBytes(UART_PC,(const uint8_t *)total_data,size);
#endif
			}
			else
			{

			}
		}
		break;

		case READ_FLASH:
        {
            if((get_system_state() == CONFIG_MODE))
            {
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\n1FLr:");
#endif
                unsigned long size,addr,index = DATA_INDX;
                char total_data[64];
                addr = pc_uart_rx.rx_buffer[index++];
                addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 8);
                addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 16);
                addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 24);

                size = pc_uart_rx.rx_buffer[index++];
                size |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 8);

                WREN();
                readContToBuff(addr, size, (char *)total_data);
                WRDI();
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\nFLr:");
                vUART_SendInt(UART_PC,addr);
                vUART_SendChr(UART_PC,',');
                vUART_SendInt(UART_PC,size);
                vUART_SendChr(UART_PC,',');
                vUART_SendBytes(UART_PC,(const uint8_t *)total_data,size);
#endif
            }
            else
            {
                resp[0] = ERROR_INVALID_STATE;
            }
        }
		break;

		case ERASE_SECTOR_FLASH:
        {
            if((get_system_state() == CONFIG_MODE))
            {
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\n1erase:");
#endif
                unsigned long sector_addr;
                int index = DATA_INDX;
                sector_addr = pc_uart_rx.rx_buffer[index++];
                sector_addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 8);
                sector_addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 16);
                sector_addr |= ((unsigned long)pc_uart_rx.rx_buffer[index++] << 24);
#ifdef FLASH_WP_ENABLE
        		remove_block_protection();
#endif
				WREN();
				Sector_Erase(sector_addr);
				Wait_Busy();
				WRDI();             // HJ 29-12-2015    // Write Disable
#ifdef FLASH_WP_ENABLE
        		WBPR(0);
#endif
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\n2erase:");
#endif
            }
            else
            {
                resp[0] = ERROR_INVALID_STATE;
            }
        }
		break;

		case READ_FLASH_STATUS:
        {
            if((get_system_state() == CONFIG_MODE))
            {
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,(uint8_t *)"\n1fl_status:");
#endif
                unsigned char status = Read_Status_Register();
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,(uint8_t *)"\n2fl_status:");
                vUART_SendChr(UART_PC,status);
#endif
            }
            else
            {
                resp[0] = ERROR_INVALID_STATE;
            }
        }
		break;

		case FLASH_INIT:
        {
            if((get_system_state() == CONFIG_MODE))
            {
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\nfl_init:");
#endif
                flashInit();
            	updateFlashCurrAddr();          //komal
            }
            else
            {
                resp[0] = ERROR_INVALID_STATE;
            }
        }
		break;

		case CLEAR_LOGS:       //komal
		{
			if((get_system_state() == CONFIG_MODE))
			{
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\nfl_clr:");
#endif
				//clear_log_id();
				clear_logs();
				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}
    	break;

		case ADD_DUMMY_TELECOM_FL_LOGS:
		{
			if((get_system_state() == CONFIG_MODE))
			{
#ifdef DEBUG_PC_CMDS
                vUART_SendStr(UART_PC,"\ndummy_logs:");
#endif
				addDummyFL_TelecomLogs(pc_uart_rx.rx_buffer[CMD_INDX + 1] + (((unsigned long)pc_uart_rx.rx_buffer[CMD_INDX + 2]) << 8));
				resp[0] = ACK;
			}
			else
			{
				resp[0] = ERROR_INVALID_STATE;
			}
		}
		break;
#endif	//FLASH_EN
		case  SET_RTR_RELAY :
        {
            if(pc_uart_rx.rx_buffer[CMD_INDX+1])
            {
                GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR,RELAY_RTR);
                //set_router_state(RELAY_ON);
            }
            else
            {
                GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR,0);
                //set_router_state(RELAY_OFF);
            }
        }
		break;
		case  SET_ODU_RELAY :
        {
           // GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU,RELAY_ODU);
            if(pc_uart_rx.rx_buffer[CMD_INDX+1])
            {
                GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU,RELAY_ODU);
                //set_ODU_state(RELAY_ON);
            }
            else
            {
                GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU,0);
                //set_ODU_state(RELAY_OFF);
            }
        }
        break;
		case  SET_RTR_SEL_RELAY :
        {
            if(pc_uart_rx.rx_buffer[CMD_INDX+1])
            {
                GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR_SEL,RELAY_RTR_SEL);
                //set_router_selection_state(RELAY_ON);
            }
            else
            {
                GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR_SEL,0);
                //set_router_selection_state(RELAY_OFF);
            }
        }
        break;
        default:
        {
            resp[0] = ERROR_INVALID_CMD;
        }
        break;
    }
	if(resp[0] && (!pc_uart_tx.tx_ready))
	{
		preparePC_ResponsePacket(cmd, resp, 2, FALSE, FALSE);
	}
}

void tx_pending_dataPC(void)
{
	if(pc_uart_tx.tx_ready == TRUE)
	{
		pc_uart_tx.tx_ready = FALSE;
													
		Usendbuffer(UART_PC);		//Send data

		flushTxBuffer(UART_PC);	//Flush buffer
	}
}

void set_system_state(sys_mode_t state)
{
	System_mode = state;
}
sys_mode_t get_system_state(void)
{
	return System_mode;
}



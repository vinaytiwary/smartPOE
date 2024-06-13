/*
 * Display.cpp
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */

#include <string.h>
#include <stdlib.h> 
#include <stdint.h>
#include <stdio.h>
#include "_config.h"
#include "Display.h"
#include "HW_pins.h"
#include "IO_cntrl.h"
#include "ADC_Core.h"
#include "UartCore.h"
#include "main.h"
#include "_common.h"
#include "Clock.h"
#include "_debug.h"
#include "SysTick_Timer.h"

#if 0
extern measurements_t measurements;
extern volatile EXTI_cnt_t EXTI_cnt;
extern ram_data_t ram_data;

sys_config_t sys_config;
screen_state_t screen_state;

void PrepareLcdBuffer(unsigned char* buff, char col, char row)
{
    unsigned char lcd_buffer[32];
    memset(lcd_buffer,0,sizeof(lcd_buffer));
    unsigned int i = 0, j = 0;

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,1);
#endif

    char data_len =0;
    data_len = strlen((const char*)buff);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,2);
#endif

    lcd_buffer[i++] = 0xAA;
    lcd_buffer[i++] = 0X19;
    lcd_buffer[i++] = 0X03; 
    lcd_buffer[i++] = /*0X00;*/col;
    lcd_buffer[i++] = /*0X01;*/row;

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,3);
#endif   
    
   memcpy(lcd_buffer+5,buff,strlen((const char*)buff));

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,4);
#endif

   i +=strlen((const char*)buff);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,5);
#endif
   
   if(data_len < 21)
   {
#ifdef DEBUG_LCD_FREEZE
        vUART_SendInt(DEBUG_UART_BASE,6);
#endif
       for(j=0; j<(21-data_len); j++ )
       {
#ifdef DEBUG_LCD_FREEZE
            vUART_SendInt(DEBUG_UART_BASE,7);
#endif
            lcd_buffer[i++] = 0x20;
       }
    }
    lcd_buffer[i++] = 0x00;

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,8);
#endif

    lcd_buffer[i++] = getChecksum(&lcd_buffer[2],25);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,9);
#endif

    lcd_buffer[i++] = 0X55;

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,10);
#endif

    // UWriteBytes((unsigned char*)lcd_buffer,29,DBG_UART);
    vUART_SendBytes(DISPLAY_UART_BASE, (const uint8_t*)lcd_buffer, 29);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,11);
    // vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nlcd_buff=");
    // vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)lcd_buffer, 29);
#endif

#ifdef DEBUG_LCD_BUFF
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nlcd_buff=");
    vUART_SendBytes(DEBUG_UART_BASE, (const uint8_t*)lcd_buffer, 29);
#endif

    memset(lcd_buffer,0,sizeof(lcd_buffer)); 

#ifdef DEBUG_LCD_FREEZE
    vUART_SendInt(DEBUG_UART_BASE,12);
#endif
}

void clear_lcd(void)
{
    char clear_buff[8];
    char i=0;
    memset(clear_buff,0,sizeof(clear_buff));
    clear_buff[i++] =0XAA;
    clear_buff[i++] =0X01;
    clear_buff[i++] =0X04;
    clear_buff[i++] =0XFC;
    clear_buff[i++] =0X55;
    // UWriteBytes((unsigned char*)clear_buff,6,DBG_UART);
    // delay_millis(5);

    vUART_SendBytes(DISPLAY_UART_BASE, (const uint8_t*)clear_buff, 6);
}

void Data_Screen_lcd()
{
    char tmpstr[24];
    // if(sys_config.config_mode!=1 &&System_mode !=CONFIG_MODE)

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nA");
#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\n3ACV,RC,ODUC,RV,ODUV,BV,SV:");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Charger_voltage);
#endif
#endif

    memset(tmpstr,0,sizeof(tmpstr));
    my_sprintf(tmpstr,1,"EX GX NX       F:00Hz");
    PrepareLcdBuffer((unsigned char*)tmpstr,0,0);

    // memset(tmpstr,0,sizeof(tmpstr));
    // // sprintf(tmpstr,"%02d-%02d-%02d   %02d:%02d:%02d", ram_data.ram_time.date, ram_data.ram_time.month, ram_data.ram_time.year, ram_data.ram_time.hour, ram_data.ram_time.min, ram_data.ram_time.sec);
    // my_sprintf(tmpstr,7,"%02d-%02d-%02d     %02d:%02d:%02d", ram_data.ram_time.date, ram_data.ram_time.month, ram_data.ram_time.year, ram_data.ram_time.hour, ram_data.ram_time.min, ram_data.ram_time.sec);
    // PrepareLcdBuffer((unsigned char*)tmpstr,0,0);

    // vMAIN_DelayMS(5);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nB");
#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\n4ACV,RC,ODUC,RV,ODUV,BV,SV:");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Charger_voltage);
#endif
#endif

    memset(tmpstr,0,sizeof(tmpstr));
    memcpy(tmpstr,"---------------------",sizeof("---------------------"));
    PrepareLcdBuffer((unsigned char*)tmpstr,0,1);

    // vMAIN_DelayMS(5);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nC");
#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\n5ACV,RC,ODUC,RV,ODUV,BV,SV:");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Charger_voltage);
#endif
#endif

    memset(tmpstr,0,sizeof(tmpstr));
    //memcpy(tmpstr,"AC VOLTAGE:Val",strlen("AC VOLTAGE:Val"));
    // sprintf(tmpstr,"AC V:%03u.%03u",measurements.PN_AC_Voltage/1000,measurements.PN_AC_Voltage%1000);
    // sprintf(tmpstr,"ACV:%03u.%02u",ram_data.ram_ADC.PN_AC_Voltage/1000,ram_data.ram_ADC.PN_AC_Voltage%1000);
    // my_sprintf(tmpstr,3,"ACV:%03u.%02u",ram_data.ram_ADC.PN_AC_Voltage/1000,ram_data.ram_ADC.PN_AC_Voltage%1000);
    my_sprintf(tmpstr,3,"ACV:%03d.%02d",ram_data.ram_ADC.PN_AC_Voltage/1000,ram_data.ram_ADC.PN_AC_Voltage%1000);
    PrepareLcdBuffer((unsigned char*)tmpstr,0,2);
    //delay_millis(5);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nD");
#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\n6ACV,RC,ODUC,RV,ODUV,BV,SV:");
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.PN_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_current_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Voltage_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Battery_voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,ram_data.ram_ADC.DC_Charger_voltage);
#endif
#endif
    
    memset(tmpstr,0,sizeof(tmpstr));
    // sprintf(tmpstr,"R1C:%02u.%03u R1V:%02u.%03u",measurements.DC_current_router1/1000,measurements.DC_current_router1%1000,measurements.DC_Voltage_router1/1000,measurements.DC_Voltage_router1%1000);
    //sprintf(tmpstr,"R1C:%02u.%03u R1V:%02u.%03u",ram_data.ram_ADC.DC_current_router1/1000,ram_data.ram_ADC.DC_current_router1%1000,ram_data.ram_ADC.DC_Voltage_router1/1000,ram_data.ram_ADC.DC_Voltage_router1%1000);
    // sprintf(tmpstr,"RV :%02u.%02u  ODUV:%02u.%02u",ram_data.ram_ADC.DC_Voltage_router1/1000,(ram_data.ram_ADC.DC_Voltage_router1%1000)/10,ram_data.ram_ADC.DC_Voltage_router2/1000,(ram_data.ram_ADC.DC_Voltage_router2%1000)/10);
    my_sprintf(tmpstr,5,"RV :%02d.%02d  ODUV:%02d.%02d",ram_data.ram_ADC.DC_Voltage_router1/1000,(ram_data.ram_ADC.DC_Voltage_router1%1000)/10,ram_data.ram_ADC.DC_Voltage_router2/1000,(ram_data.ram_ADC.DC_Voltage_router2%1000)/10);
    PrepareLcdBuffer((unsigned char*)tmpstr,0,3);
    //delay_millis(5);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nE");
#endif

    memset(tmpstr,0,sizeof(tmpstr));
    // sprintf(tmpstr,"R2C:%02u.%03u R2V:%02u.%03u",measurements.DC_current_router2/1000,measurements.DC_current_router2%1000,measurements.DC_Voltage_router2/1000,measurements.DC_Voltage_router2%1000);
    //sprintf(tmpstr,"R2C:%02u.%03u R2V:%02u.%03u",ram_data.ram_ADC.DC_current_router2/1000,ram_data.ram_ADC.DC_current_router2%1000,ram_data.ram_ADC.DC_Voltage_router2/1000,ram_data.ram_ADC.DC_Voltage_router2%1000);
    // sprintf(tmpstr,"RC :%02u.%02u  ODUC:%02u.%02u",ram_data.ram_ADC.DC_current_router1/1000,(ram_data.ram_ADC.DC_current_router1%1000)/10,ram_data.ram_ADC.DC_current_router2/1000,(ram_data.ram_ADC.DC_current_router2%1000)/10);
    my_sprintf(tmpstr,5,"RC :%02d.%02d  ODUC:%02d.%02d",ram_data.ram_ADC.DC_current_router1/1000,(ram_data.ram_ADC.DC_current_router1%1000)/10,ram_data.ram_ADC.DC_current_router2/1000,(ram_data.ram_ADC.DC_current_router2%1000)/10);
    //memcpy(tmpstr,"VOLTAGE R1:Val R2:Val",sizeof("VOLTAGE R1:Val R2:Val"));
    PrepareLcdBuffer((unsigned char*)tmpstr,0,4);
    //delay_millis(5);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nF");
#endif
    
    memset(tmpstr,0,sizeof(tmpstr));
    //memcpy(tmpstr,"BATTERY   :Val",sizeof("BATTERY   :Val"));
    // sprintf(tmpstr,"B:%02u.%03u C:%02u.%03u",measurements.DC_Battery_voltage/1000,measurements.DC_Battery_voltage%1000,measurements.DC_Charger_voltage/1000,measurements.DC_Charger_voltage%1000);
    // sprintf(tmpstr,"BAT:%02u.%02u  SMPS:%02u.%02u",ram_data.ram_ADC.DC_Battery_voltage/1000,(ram_data.ram_ADC.DC_Battery_voltage%1000)/10,ram_data.ram_ADC.DC_Charger_voltage/1000,(ram_data.ram_ADC.DC_Charger_voltage%1000)/10);
    my_sprintf(tmpstr,5,"BAT:%02d.%02d  SMPS:%02d.%02d",ram_data.ram_ADC.DC_Battery_voltage/1000,(ram_data.ram_ADC.DC_Battery_voltage%1000)/10,ram_data.ram_ADC.DC_Charger_voltage/1000,(ram_data.ram_ADC.DC_Charger_voltage%1000)/10);
    PrepareLcdBuffer((unsigned char*)tmpstr,0,5);
    //delay_millis(5);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nG");
#endif    
        
    // memset(tmpstr,0,sizeof(tmpstr));
    // memcpy(tmpstr," ",sizeof(" "));
    // PrepareLcdBuffer((unsigned char*)tmpstr,0,6);
    // //delay_millis(5);

    memset(tmpstr,0,sizeof(tmpstr));
    memcpy(tmpstr,"---------------------",sizeof("---------------------"));
    PrepareLcdBuffer((unsigned char*)tmpstr,0,6);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nH");
#endif

    memset(tmpstr,0,sizeof(tmpstr));
    // sprintf(tmpstr,"%02d-%02d-%02d   %02d:%02d:%02d", ram_data.ram_time.date, ram_data.ram_time.month, ram_data.ram_time.year, ram_data.ram_time.hour, ram_data.ram_time.min, ram_data.ram_time.sec);
    my_sprintf(tmpstr,7,"%02d-%02d-%02d     %02d:%02d:%02d", ram_data.ram_time.date, ram_data.ram_time.month, ram_data.ram_time.year, ram_data.ram_time.hour, ram_data.ram_time.min, ram_data.ram_time.sec);
    PrepareLcdBuffer((unsigned char*)tmpstr,0,7);

#ifdef DEBUG_LCD_FREEZE
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nI");
#endif
}
#endif //if 0


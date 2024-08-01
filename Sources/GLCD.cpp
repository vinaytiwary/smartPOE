#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "GLCD.h"
#include "font5x8.h"
#include "HW_pins.h"
#include "delay.h"
#include "SysTick_Timer.h"
#include "IO_cntrl.h"
#include "main.h"
#include <string.h>
#include "_common.h"
#include <stdio.h>
#include "PC_Cmds.h"
#include "gprs.h"
#include "E2P.h"
#include "gps.h"
#include "Telecom_server_query.h"
#include "_config.h"

#ifdef ENABLE_GLCD

unsigned char screen_x = 0;
extern volatile EXTI_cnt_t EXTI_cnt;
sys_config_t sys_config;
extern ram_data_t ram_data;
extern Alarms_t Alarms;
extern sys_mode_t System_mode;
extern e2p_config_time_t e2p_config_time;

extern uint32_t GLCD_startup_time;

void GLCD_InitalizePorts(void)
{
    // Enable the GPIO ports
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

    // Wait for the GPIO modules to be ready
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) {}

    // Configure data pins as output
    GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, 0xFF);

    // Configure control pins as output
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, (KS0108_EN | KS0108_RS | KS0108_CS1 | KS0108_CS2));

    // Initialize control pins to default states
    GPIOPinWrite(GPIO_PORTK_BASE, (KS0108_EN | KS0108_RS | KS0108_CS1 | KS0108_CS2), (KS0108_EN | KS0108_RS | KS0108_CS1 | KS0108_CS2));
    //GPIOPinWrite(GPIO_PORTK_BASE, (KS0108_EN | KS0108_RS | KS0108_CS1), (KS0108_EN | KS0108_RS | KS0108_CS1));
}

void GLCD_EnableController(unsigned char controller)
{
    switch(controller) {
        case 0:
            GPIOPinWrite(KS0108_CTRL_PORT, KS0108_CS1, 0);
            break;
        case 1:
            GPIOPinWrite(KS0108_CTRL_PORT, KS0108_CS2, 0);
            break;
        default:
            // Handle unexpected values
            break;
    }
}


void GLCD_DisableController(unsigned char controller)
{
    switch(controller) {
           case 0:
               GPIOPinWrite(KS0108_CTRL_PORT, KS0108_CS1, KS0108_CS1);
               break;
           case 1:
               GPIOPinWrite(KS0108_CTRL_PORT, KS0108_CS2, KS0108_CS2);
               break;
           default:
               // Handle unexpected values
               break;
       }
}


void GLCD_WriteCommand(unsigned char commandToWrite, unsigned char controller)
{

    GPIOPinWrite(KS0108_CTRL_PORT, KS0108_RS, 0);
    GLCD_EnableController(controller);
    GPIOPinWrite(KS0108_DATA_PORT, 0xFF, commandToWrite);
    GPIOPinWrite(KS0108_CTRL_PORT, KS0108_EN, KS0108_EN);
    _delay_us(1);
    GPIOPinWrite(KS0108_CTRL_PORT, KS0108_EN, 0);

    GLCD_DisableController(controller);
    _delay_us(1);
}

void GLCD_WriteData(unsigned char dataToWrite)
{
    GPIOPinWrite(KS0108_CTRL_PORT, KS0108_RS, KS0108_RS);
    GLCD_EnableController(screen_x / 64);
    GPIOPinWrite(KS0108_DATA_PORT, 0xFF, dataToWrite);
    GPIOPinWrite(KS0108_CTRL_PORT, KS0108_EN, KS0108_EN);
    _delay_us(1);
    GPIOPinWrite(KS0108_CTRL_PORT, KS0108_EN, 0);
    GLCD_DisableController(screen_x / 64);

    screen_x++;
    if(screen_x >= KS0108_SCREEN_WIDTH)
        screen_x = 0;
    _delay_us(1);
}

void GLCD_Initalize(void)
{
    unsigned char i;
    GLCD_InitalizePorts();
    vMAIN_DelayMS(100);
    for(i = 0; i < 2; i++)
    {
        GLCD_WriteCommand((DISPLAY_ON_CMD | ON), i);
    }
}

void GLCD_GoTo(unsigned char x, unsigned char y)
{
    unsigned char i;
    screen_x = x;

    for(i = 0; i < (KS0108_SCREEN_WIDTH / 32); i++)//64
    {
        GLCD_WriteCommand((DISPLAY_SET_Y | 0), i);
        GLCD_WriteCommand((DISPLAY_SET_X | y), i);
        GLCD_WriteCommand((DISPLAY_START_LINE | 0), i);
    }
    GLCD_WriteCommand((DISPLAY_SET_Y | (x % 64)), (x / 64));
    GLCD_WriteCommand((DISPLAY_SET_X | y), (x / 64));

}

void GLCD_ClearScreen(void)
{
    unsigned char i, j;
    for(j = 0; j < (KS0108_SCREEN_HEIGHT / 8); j++)
    {
        GLCD_GoTo(0, j);
        for(i = 0; i < KS0108_SCREEN_WIDTH; i++)
          GLCD_WriteData(0x00);
    }
}

void GLCD_Clear_Line(unsigned char line_num)
{
    GLCD_GoTo(0, line_num);
    for(int i = 0; i < KS0108_SCREEN_WIDTH; i++)
        GLCD_WriteData(0x00);
}

void GLCD_WriteChar(char c)
{
    int index = (c - ' ') * 5;
    for (int i = 0; i < 5; i++)
    {
        GLCD_WriteData(font5x8[index + i]);
    }
    GLCD_WriteData(0x00);
}


void GLCD_WriteString(char *stringToWrite)
{
    while(*stringToWrite)
        GLCD_WriteChar(*stringToWrite++);
}

void GLCD_PrintBatterySts()
{
    GLCD_GoTo(115,0);
    uint8_t index,batt_percentage=0;
    if(ram_data.ram_ADC.DC_Battery_voltage <= BATT_RANGE_LOW)
    {
        batt_percentage =0;
    }
    else if(ram_data.ram_ADC.DC_Battery_voltage >= BATT_RANGE_HIGH)
    {
        batt_percentage =100;
    }
    else
    {
        batt_percentage = (((ram_data.ram_ADC.DC_Battery_voltage -BATTERY_MIN_RANGE)*100)/(BATTERY_MAX_RANGE-BATTERY_MIN_RANGE));
    }
    index = (batt_percentage/BATTERY_BASE_PERCENTAGE)*BATTERY_STS_WIDTH;
    for (int i = 0; i < BATTERY_STS_WIDTH; i++)
    {
        GLCD_WriteData(BattFont5x8[index + i]);
    }
}

void GLCD_PrintRightIcon()
{
    //GLCD_GoTo(80,0);
    GLCD_WriteData(0x10);
    GLCD_WriteData(0x20);
    GLCD_WriteData(0x40);
    GLCD_WriteData(0x20);
    GLCD_WriteData(0x10);
    GLCD_WriteData(0x08);
    GLCD_WriteData(0x04);
    GLCD_WriteData(0x02);

}

void GLCD_PrintWrongIcon()
{
    GLCD_WriteData(0x41);
    GLCD_WriteData(0x22);
    GLCD_WriteData(0x14);
    GLCD_WriteData(0x08);
    GLCD_WriteData(0x14);
    GLCD_WriteData(0x22);
    GLCD_WriteData(0x41);
}
void GLCD_PrintNetworkSts()
{
    GLCD_GoTo(0,0);
    // int index = getNW_status() *NETWORK_STS_WIDTH ;
    int index = get_network_status() *NETWORK_STS_WIDTH ;
    
    //index=10;
    for (int i = 0; i < NETWORK_STS_WIDTH; i++)
    {
        GLCD_WriteData(NetworkFont5x8[index + i]);
    }
}
void GLCD_PrintTopBoarder()
{
    for(int i=0;i<KS0108_SCREEN_WIDTH;i++)
    {
        GLCD_WriteData(0x04);
    }
}

void GLCD_PrintButtomBoarder()
{
    for(int i=0;i<KS0108_SCREEN_WIDTH;i++)
    {
        GLCD_WriteData(0x04);
    }
}

void Data_Screen_lcd()
{
  char tmpstr[24];
  unsigned char earth, gps,network;
  earth = Alarms.ACEarth_fault ? 'X' : ' ';
  //ram_data.ram_EXTI_cnt.freq_cnt =50;

    if(get_system_state() != CONFIG_MODE)
    {
      earth = Alarms.ACEarth_fault ? 'X' : ' ';
#ifdef ENABLE_GPS
      gps = get_gps_status() ? ' ' : 'X';
#else
    gps = 'X';
#endif  //ENABLE_GPS

      GLCD_PrintNetworkSts();
      GLCD_PrintBatterySts();
      memset(tmpstr,0,sizeof(tmpstr));
    //   my_sprintf(tmpstr,3,"E%cG%c  F:%02dHz",earth,gps,ram_data.ram_EXTI_cnt.freq_cnt);
      my_sprintf(tmpstr,3,"G%c   F:%02dHz   E%c",gps,ram_data.ram_EXTI_cnt.freq_cnt,earth);
      GLCD_GoTo(14,0);
      GLCD_WriteString((char*)tmpstr);

      GLCD_GoTo(0,1);
      GLCD_PrintTopBoarder();

      memset(tmpstr,0,sizeof(tmpstr));
      my_sprintf(tmpstr,3,"ACV:%03d.%02d",ram_data.ram_ADC.PN_AC_Voltage/1000,ram_data.ram_ADC.PN_AC_Voltage%1000);
      GLCD_GoTo(0,2);
      GLCD_WriteString((char*)tmpstr);

      memset(tmpstr,0,sizeof(tmpstr));
      my_sprintf(tmpstr,3,"ODUV:%02d.%02d      ",ram_data.ram_ADC.DC_Voltage_router2/1000,(ram_data.ram_ADC.DC_Voltage_router2%1000)/10);
      GLCD_GoTo(0,3);
      GLCD_WriteString((char*)tmpstr);

#ifdef RTR_HARDCODED
      memset(tmpstr,0,sizeof(tmpstr));
      if(ram_data.ram_ADC.DC_current_router1 < INVERTER_IDLE_CURRENT)
      {
          my_sprintf(tmpstr,3,"RC :00.00  ODUC:%02d.%02d",ram_data.ram_ADC.DC_current_router2/1000,(ram_data.ram_ADC.DC_current_router2%1000)/10);
      }
      else
      {
          my_sprintf(tmpstr,5,"RC :%02d.%02d  ODUC:%02d.%02d",(ram_data.ram_ADC.DC_current_router1-INVERTER_IDLE_CURRENT)/1000,((ram_data.ram_ADC.DC_current_router1-INVERTER_IDLE_CURRENT)%1000)/10,ram_data.ram_ADC.DC_current_router2/1000,(ram_data.ram_ADC.DC_current_router2%1000)/10);
      }
      GLCD_GoTo(0,4);
      GLCD_WriteString((char*)tmpstr);
#else
      memset(tmpstr,0,sizeof(tmpstr));
      my_sprintf(tmpstr,5,"RC :%02d.%02d  ODUC:%02d.%02d",ram_data.ram_ADC.DC_current_router1/1000,(ram_data.ram_ADC.DC_current_router1%1000)/10,ram_data.ram_ADC.DC_current_router2/1000,(ram_data.ram_ADC.DC_current_router2%1000)/10);
      GLCD_GoTo(0,4);
      GLCD_WriteString((char*)tmpstr);
#endif

#ifdef SMPS_HARDCODED
      memset(tmpstr,0,sizeof(tmpstr));
      if(Alarms.ACPhase_fault)
      {
          my_sprintf(tmpstr,3,"BAT:%02d.%02d  SMPS:00.00",ram_data.ram_ADC.DC_Battery_voltage/1000,(ram_data.ram_ADC.DC_Battery_voltage%1000)/10);
      }
      else
      {
          my_sprintf(tmpstr,5,"BAT:%02d.%02d  SMPS:%02d.%02d",ram_data.ram_ADC.DC_Battery_voltage/1000,(ram_data.ram_ADC.DC_Battery_voltage%1000)/10,ram_data.ram_ADC.DC_Charger_voltage/1000,(ram_data.ram_ADC.DC_Charger_voltage%1000)/10);
      }
      //sprintf(tmpstr,"BAT:%02u.%02u  SMPS:%02u.%02u",ram_data.ram_ADC.DC_Battery_voltage/1000,(ram_data.ram_ADC.DC_Battery_voltage%1000)/10,ram_data.ram_ADC.DC_Charger_voltage/1000,(ram_data.ram_ADC.DC_Charger_voltage%1000)/10);
      GLCD_GoTo(0,5);
      GLCD_WriteString((char*)tmpstr);
#else
      memset(tmpstr,0,sizeof(tmpstr));
      my_sprintf(tmpstr,5,"BAT:%02d.%02d  SMPS:%02d.%02d",ram_data.ram_ADC.DC_Battery_voltage/1000,(ram_data.ram_ADC.DC_Battery_voltage%1000)/10,ram_data.ram_ADC.DC_Charger_voltage/1000,(ram_data.ram_ADC.DC_Charger_voltage%1000)/10);
       //sprintf(tmpstr,"BAT:%02u.%02u  SMPS:%02u.%02u",ram_data.ram_ADC.DC_Battery_voltage/1000,(ram_data.ram_ADC.DC_Battery_voltage%1000)/10,ram_data.ram_ADC.DC_Charger_voltage/1000,(ram_data.ram_ADC.DC_Charger_voltage%1000)/10);
       GLCD_GoTo(0,5);
       GLCD_WriteString((char*)tmpstr);
#endif
/*
      memset(tmpstr,0,sizeof(tmpstr));
      memcpy(tmpstr," ",sizeof(" "));
      GLCD_GoTo(0,6);
      GLCD_WriteString((char*)tmpstr);
*/
      GLCD_GoTo(0,6);
      GLCD_PrintButtomBoarder();

      memset(tmpstr,0,sizeof(tmpstr));
      my_sprintf(tmpstr,7," %02d/%02d/20%02d %02d:%02d:%02d",ram_data.ram_time.date,ram_data.ram_time.month,ram_data.ram_time.year,ram_data.ram_time.hour,ram_data.ram_time.min,ram_data.ram_time.sec);
      //sprintf(tmpstr,"%02d/%02d20/%02d %02d:%02d:%02d",ram_data.ram_time.date,ram_data.ram_time.month,ram_data.ram_time.year,ram_data.ram_time.hour,ram_data.ram_time.min,ram_data.ram_time.sec);
      GLCD_GoTo(0,7);
      GLCD_WriteString((char*)tmpstr);
    }
   else
   {
       Display_CFGScreen();
   }
}

void AlarmDisplay()
{
     char tmpstr[24];

     Alarms_t localAlarm;
     localAlarm.MAINS_fault = Alarms.MAINS_fault ? 'Y' : 'N';
     localAlarm.MAINS_OVF_fault = Alarms.MAINS_OVF_fault ? 'Y' : 'N';
     localAlarm.ACPhase_fault = Alarms.ACPhase_fault ? 'Y' : 'N';
     localAlarm.ACEarth_fault = Alarms.ACEarth_fault ? 'Y' : 'N';
     localAlarm.Supply_mode = Alarms.Supply_mode ? 'Y' : 'N';
     localAlarm.Batt_low = Alarms.Batt_low ? 'Y' : 'N';

#ifdef RTR_HARDCODED
     localAlarm.Router1_NC = ram_data.ram_ADC.DC_current_router1 < 425 ? 'Y' : 'N';
#else
     localAlarm.Router1_NC = Alarms.Router1_NC ? 'Y' : 'N';
#endif  //RTR_HARDCODED

     localAlarm.Router2_NC = Alarms.Router2_NC ? 'Y' : 'N';
     localAlarm.Router1_V_fault = Alarms.Router1_V_fault ? 'Y' : 'N';
     localAlarm.Router2_V_fault = Alarms.Router2_V_fault ? 'Y' : 'N';
     localAlarm.Chg_fault = Alarms.Chg_fault ? 'Y' : 'N';

     if(get_system_state() != CONFIG_MODE)
     {
       if(Alarms.Supply_mode)
       {
         memset(tmpstr,0,sizeof(tmpstr));
         memcpy(tmpstr,"  ALARM    BAT",sizeof("  ALARM    BAT"));
         GLCD_Clear_Line(0);
         GLCD_GoTo(44,0);
         GLCD_WriteString((char*)tmpstr);
       }
       else
       {
         memset(tmpstr,0,sizeof(tmpstr));
         memcpy(tmpstr,"ALARM     MAIN",sizeof("ALARM     MAIN"));
         GLCD_Clear_Line(0);
         GLCD_GoTo(43,0);
         GLCD_WriteString((char*)tmpstr);
       }

       GLCD_GoTo(0,1);
       GLCD_PrintTopBoarder();

       memset(tmpstr,0,sizeof(tmpstr));
       my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(0,2);
       GLCD_WriteString((char*)tmpstr);

       memset(tmpstr,0,sizeof(tmpstr));
       my_sprintf(tmpstr,2,"EARTH F:%c  LOS     :--",localAlarm.ACEarth_fault/* ,localAlarm.Supply_mode */);
       GLCD_GoTo(0,3);
       GLCD_WriteString((char*)tmpstr);

       memset(tmpstr,0,sizeof(tmpstr));
       my_sprintf(tmpstr,2,"RTR  NC:%c  POE F   :--", localAlarm.Router1_NC/* , localAlarm.Router1_V_fault */);
       GLCD_GoTo(0,4);
       GLCD_WriteString((char*)tmpstr);

       memset(tmpstr,0,sizeof(tmpstr));
       my_sprintf(tmpstr,3,"ODU  NC:%c  BAT L   :%c",localAlarm.Router2_NC, localAlarm.Batt_low);
       GLCD_GoTo(0,5);
       GLCD_WriteString((char*)tmpstr);

       GLCD_GoTo(0,6);
       GLCD_PrintButtomBoarder();

       memset(tmpstr,0,sizeof(tmpstr));
	   my_sprintf(tmpstr,7," %02d/%02d/20%02d %02d:%02d:%02d",ram_data.ram_time.date,ram_data.ram_time.month,ram_data.ram_time.year,ram_data.ram_time.hour,ram_data.ram_time.min,ram_data.ram_time.sec);
	   GLCD_GoTo(0,7);
	   GLCD_WriteString((char*)tmpstr);
     }
     else
     {
       if(get_system_state() == CONFIG_MODE)
       {
           Display_CFGScreen();
       }
     }
}

void AlarmDisplayTest()
{
     char tmpstr[24];

     Alarms_t localAlarm;
     localAlarm.MAINS_fault = Alarms.MAINS_fault;
     localAlarm.MAINS_OVF_fault = Alarms.MAINS_OVF_fault ;
     localAlarm.ACPhase_fault = Alarms.ACPhase_fault;
     localAlarm.ACEarth_fault = Alarms.ACEarth_fault;
     localAlarm.Supply_mode = Alarms.Supply_mode;
     localAlarm.Batt_low = Alarms.Batt_low;

#ifdef RTR_HARDCODED
     localAlarm.Router1_NC = ram_data.ram_ADC.DC_current_router1 < INVERTER_IDLE_CURRENT ? 'Y' : 'N';
#else
     localAlarm.Router1_NC = Alarms.Router1_NC ? 'Y' : 'N';
#endif  //RTR_HARDCODED

     localAlarm.Router2_NC = Alarms.Router2_NC;
     localAlarm.Router1_V_fault = Alarms.Router1_V_fault;
     localAlarm.Router2_V_fault = Alarms.Router2_V_fault ;
     localAlarm.Chg_fault = Alarms.Chg_fault;

     if(get_system_state() != CONFIG_MODE)
     {
       if(Alarms.Supply_mode)
       {
         memset(tmpstr,0,sizeof(tmpstr));
         memcpy(tmpstr,"  ALARM    BAT",sizeof("  ALARM    BAT"));
         GLCD_Clear_Line(0);
         GLCD_GoTo(44,0);
         GLCD_WriteString((char*)tmpstr);
       }
       else
       {
         memset(tmpstr,0,sizeof(tmpstr));
         memcpy(tmpstr,"ALARM     MAIN",sizeof("ALARM     MAIN"));
         GLCD_Clear_Line(0);
         GLCD_GoTo(43,0);
         GLCD_WriteString((char*)tmpstr);
       }

       GLCD_GoTo(0,1);
       GLCD_PrintTopBoarder();

       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"MAINS:",sizeof("MAINS:"));
       //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(1,2);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(36,2);
       localAlarm.MAINS_fault ?GLCD_PrintWrongIcon() :GLCD_PrintRightIcon();


       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"EARTH:",sizeof("EARTH:"));
       //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(1,3);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(36,3);
       localAlarm.ACEarth_fault ?GLCD_PrintWrongIcon() :GLCD_PrintRightIcon();

       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"POE F:",sizeof("POE F:"));
       //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(1,4);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(36,4);
       print_dash();
       //localAlarm.Router1_V_fault ?GLCD_PrintWrongIcon() :GLCD_PrintRightIcon();

       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"LOS F:",sizeof("LOS F:"));
       //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(1,5);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(36,5);
       print_dash();
       //localAlarm.Supply_mode ?GLCD_PrintWrongIcon() :GLCD_PrintRightIcon();


       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"PWR CUT:",sizeof("PWR CUT:"));
       //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(71,2);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(118,2);
       localAlarm.MAINS_fault ?GLCD_PrintRightIcon() :GLCD_PrintWrongIcon();

       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"RTR CON:",sizeof("RTR CON:"));
       //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(71,3);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(118,3);
       ram_data.ram_ADC.DC_current_router1 < 550 ? GLCD_PrintWrongIcon() :GLCD_PrintRightIcon();

       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"ODU CON:",sizeof("ODU CON:"));
      //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(71,4);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(118,4);
       localAlarm.Router2_NC ? GLCD_PrintWrongIcon() :GLCD_PrintRightIcon();

       memset(tmpstr,0,sizeof(tmpstr));
       memcpy(tmpstr,"BAT LOW:",sizeof("BAT LOW:"));
      //my_sprintf(tmpstr,3,"MAINS F:%c  MAINSOVF:%c", localAlarm.MAINS_fault, localAlarm.MAINS_OVF_fault);
       GLCD_GoTo(71,5);
       GLCD_WriteString((char*)tmpstr);
       GLCD_GoTo(118,5);
       localAlarm.Batt_low ? GLCD_PrintRightIcon() :GLCD_PrintWrongIcon();

       GLCD_GoTo(0,6);
       GLCD_PrintButtomBoarder();

       memset(tmpstr,0,sizeof(tmpstr));
       my_sprintf(tmpstr,7," %02d/%02d/20%02d %02d:%02d:%02d",ram_data.ram_time.date,ram_data.ram_time.month,ram_data.ram_time.year,ram_data.ram_time.hour,ram_data.ram_time.min,ram_data.ram_time.sec);
       GLCD_GoTo(0,7);
       GLCD_WriteString((char*)tmpstr);
     }
     else
     {
       if(get_system_state() == CONFIG_MODE)
       {
           Display_CFGScreen();
       }
     }
}
void updateGlcd()
{
    if((my_millis() - GLCD_startup_time) > 1000)
    {
        static int count = 0;
        count++;
        if(count <= e2p_config_time.config_time * 10)
        {
            Data_Screen_lcd();
            if(count ==e2p_config_time.config_time * 10 )
            {
                for(int i=0;i<8;i++)
                {
                    GLCD_Clear_Line(i);
                }
            }
        }
        else if(count > e2p_config_time.config_time * 10)
        {
            //AlarmDisplay();
            AlarmDisplayTest();
            if(count >= e2p_config_time.config_time * 20)
            {
                count = 0;
                for(int i=0;i<8;i++)
                {
                    GLCD_Clear_Line(i);
                }

            }
        }
    }

}

void Display_CFGScreen(void)
{
  char tmpstr[24];
  memset(tmpstr,0,sizeof(tmpstr));
  memcpy(tmpstr,"     CONFIG MODE",sizeof("     CONFIG MODE"));
  GLCD_ClearScreen();
  GLCD_GoTo(0,4);
  GLCD_WriteString((char*)tmpstr);
}

void Display_BootUPscreen(void)
{
    GLCD_ClearScreen();
    GLCD_GoTo(36,3);
    GLCD_WriteString((char*)"Smart POE");
    GLCD_GoTo(48,4);
    GLCD_WriteString((char*)FIRMWARE_VERSION);
    GLCD_GoTo(23,5);
    GLCD_WriteString((char*)"Booting Up...");
    vMAIN_DelayMS(500);
}
void print_dash()
{
    for(int i=0;i<7;i++)
    {
        GLCD_WriteData(0x08);
    }
    
}
#endif  //ENABLE_GLCD

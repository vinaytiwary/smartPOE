/*
 * tiot_timer.cpp
 *
 *  Created on: 18-Jun-2024
 *      Author: Dell
 */
#include "stdint.h"
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <inc/hw_memmap.h>
#include <inc/tm4c1233e6pz.h>
#include <math.h>
#include <Sources/ADC_Core.h>
#include <Sources/tiot_timer.h>
#include <Sources/UartCore.h>
#include <Sources/IO_cntrl.h>

extern volatile double PN_ADC_RAW_MAX;
void vTimerInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT);
    // TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
    // TimerPrescaleSet(TIMER0_BASE, TIMER_B, 9);
    TimerPrescaleSet(TIMER0_BASE, TIMER_B, (8-1));  //0 to 7 = 8
//    TimerLoadSet(TIMER0_BASE, TIMER_B, 5000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, (((SysCtlClockGet() * 5)/ 1000)));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 5));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 15));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 14));    //AC ADC RAW range = 1218 to 1320
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 13));    //AC ADC RAW range = 1463 to 1513
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 12));    //AC ADC RAW range = 1052 to 1102
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 11));    //AC ADC RAW range = 1087 to 1193
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 10));    //AC ADC RAW range = 1228 to 1277
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 9));     //AC ADC RAW range = 1417 to 1493
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 8));     //AC ADC RAW range = 1074 to 1111
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 7));     //AC ADC RAW range = 1079 to 1196
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 6));     //AC ADC RAW range = 1217 to 1258
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 5));     //AC ADC RAW range = 1394 to 1473
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 4));     //AC ADC RAW range = 1083 to 1110
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 3));     //AC ADC RAW range = 1078 to 
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 2));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 1));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 15));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 16));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 17));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 18));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 19));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 20));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 21));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 22));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 23));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 24));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 25));
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 220799); //13.8ms
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 201599); //12.6ms
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 199999); //12.5ms
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 198399); //12.4ms
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 196799); //12.3ms     //till 1700
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 195199); //12.2ms    //problem
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 239999); //15ms       //problem
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 214399); //13.4ms  

    // TimerLoadSet(TIMER0_BASE, TIMER_B, 0xFFFF); 
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 10000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 2000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 4000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 6000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 8000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 3000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 30000);   //15ms
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 29600);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 29400);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 29200);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 29000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 28800);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 28600);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 28400);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 28200);
    TimerLoadSet(TIMER0_BASE, TIMER_B, 28000);      //14ms
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 27800);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 27600);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 27400);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 27200);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 27000);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 26800);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 26600);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 26400);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 26200);
    // TimerLoadSet(TIMER0_BASE, TIMER_B, 26000);

    // TimerLoadSet(TIMER0_BASE, TIMER_B, 24000);

    IntMasterEnable();
    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);

    // IntEnable(INT_TIMER0B);
    // TimerEnable(TIMER0_BASE, TIMER_B);
}

extern "C" void TimerIntHandler(void)
{
#ifdef ENABLE_CLI_SEI
    IntMasterDisable();
#endif  //ENABLE_CLI_SEI
    //
    // Clear the timer interrupt flag.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//    int32_t Vnow = 0;
//    static uint32_t measurements_count = 0;
//    static uint32_t Vsum = 0;
//    double readingVoltage = 0.0f;
//    if(++measurements_count >= 20)
//    {
//        PN_ADC_RAW_MAX = sqrt(Vsum / measurements_count) / 4095 * 3.3 * 500.0f;
//        PN_ADC_RAW_MAX -= 29;
//        vUART_SendChr(DEBUG_UART_BASE,'\n');
//        vUART_SendInt(DEBUG_UART_BASE,PN_ADC_RAW_MAX);
//        measurements_count = 0;
//        Vsum = 0;
//        TimerIntDisable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//        TimerDisable(TIMER0_BASE, TIMER_B);
//    }
//    else
//    {
//        Vnow = readADC(SIG_AC_VOLTAGE_ADC) - 2048;
//        Vsum += (Vnow * Vnow);
//    }

//    if(++measurements_count >= 20)
//    {
//        PN_ADC_RAW_MAX = Vsum;
//        //PN_ADC_RAW_MAX -= 29;
//        vUART_SendChr(DEBUG_UART_BASE,'\n');
//        vUART_SendInt(DEBUG_UART_BASE,PN_ADC_RAW_MAX);
//        measurements_count = 0;
//        Vsum = 0;
//        TimerIntDisable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//        TimerDisable(TIMER0_BASE, TIMER_B);
//    }
//    else
//    {
//        Vnow = readADC(SIG_AC_VOLTAGE_ADC);
//        //Vsum += (Vnow * Vnow);
//        if(Vnow > Vsum)
//        {
//            Vsum = Vnow;
//        }
//    }

    //
    // Set a flag to indicate that the interrupt occurred.
    //
    //g_bIntFlag = true;

    readACVoltage();

    // vGPIO_Toggle(LED_PORT_BASE, LED2_PIN, LED2_PIN );
#ifdef ENABLE_CLI_SEI
    IntMasterEnable();
#endif  //ENABLE_CLI_SEI
}


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

extern volatile double PN_ADC_RAW_MAX;
void vTimerInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT);
//    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_B, ((SysCtlClockGet() / 1000) * 5));
    IntMasterEnable();
    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
    //IntEnable(INT_TIMER0B);
//    TimerEnable(TIMER0_BASE, TIMER_B);




}

extern "C" void TimerIntHandler(void)
{
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
}


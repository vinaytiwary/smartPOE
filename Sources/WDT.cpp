/*
 * WDT.cpp
 *
 *  Created on: May 19, 2024
 *      Author: ADMIN
 */
#include "_config.h"

#ifdef ENABLE_WDT_RESET
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_watchdog.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/watchdog.h"

#include "WDT.h"
#include "SysTick_Timer.h"
#include "UartCore.h"
#include "_debug.h"

#ifdef WDT_IRQ_MODE
volatile WDT_t WDT;

//*****************************************************************************
//
// The interrupt handler for the watchdog.  This feeds the dog (so that the
// processor does not get reset) and blinks the blue LED.
//
//*****************************************************************************
void
WatchdogIntHandler(void)
{
#ifdef DEBUG_WDT
    // vUART_SendChr(UART_PC, 'W');
    vUART_SendStr(UART_PC, "WDT");
#endif
    //
    // If we have been told to stop feeding the watchdog, return immediately
    // without clearing the interrupt.  This will cause the system to reset
    // next time the watchdog interrupt fires.
    //
    if(getWDTreset_flg)
    {
        // return;  //If I just return from here, it will reset the ctller in double the time - if the WDT cntr is 2secs, this will make it reset in 4 secs.
        // If I just hardcode the cntr value to 0 here then it will reset in the exact time that was initially intended - if the WDT cntr is 2secs, this will make it reset in 2 secs.
        WatchdogUnlock(WATCHDOG0_BASE);
        WatchdogReloadSet(WATCHDOG0_BASE, 0);
        return;
    }
    else
    {
        //make it true here so that if it comes here the 2nd time, it won't feed 
        //the WDT and reset the controller. If it does not come here in the next 
        //WDT period (1sec), the flag should be resetted freely in main() -> while(1).
        setWDTreset_flg(true); 
        //
        // Clear the watchdog interrupt.
        //
        WatchdogIntClear(WATCHDOG0_BASE);
    }
}

// //*****************************************************************************
// //
// // The interrupt handler for the watchdog.  This feeds the dog (so that the
// // processor does not get reset) and blinks the blue LED.
// //
// //*****************************************************************************
// void
// WatchdogIntHandler(void)
// {
// #ifdef DEBUG_WDT
//     vUART_SendChr(UART_PC, 'W');
// #endif
//     //
//     // If we have been told to stop feeding the watchdog, return immediately
//     // without clearing the interrupt.  This will cause the system to reset
//     // next time the watchdog interrupt fires.
//     //
//     if(getWDTreset_flg)
//     {
//         return;
//     }
//     //make it true here so that if it comes here the 2nd time, it won't feed 
//     //the WDT and reset the controller. If it does not come here in the next 
//     //WDT period (1sec), the flag should be resetted freely in main() -> while(1).
//     setWDTreset_flg(true); 
//     //
//     // Clear the watchdog interrupt.
//     //
//     WatchdogIntClear(WATCHDOG0_BASE);
// }
#endif  //WDT_IRQ_MODE

void initWDT(void)
{
    // Enable the watchdog peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

    // Wait for the Watchdog 0 module to be ready.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_WDOG0)) {}

// #ifdef WDT_IRQ_MODE
    // Enable the watchdog interrupt
    IntEnable(INT_WATCHDOG_TM4C123);
// #endif  //WDT_IRQ_MODE

    // Check to see if the registers are locked, and if so, unlock them
    if(WatchdogLockState(WATCHDOG0_BASE) == true)
    {
        WatchdogUnlock(WATCHDOG0_BASE);
    }

// #ifdef WDT_IRQ_MODE
    // Enable Watchdog Interrupts
    WatchdogIntEnable(WATCHDOG0_BASE);
    WatchdogIntTypeSet(WATCHDOG0_BASE, WATCHDOG_INT_TYPE_INT);
// #endif  //WDT_IRQ_MODE

    // Set the period of the watchdog timer for 1 s (2 s to reset)
    // WatchdogReloadSet(WATCHDOG0_BASE, SysCtlClockGet()/3);
    // WatchdogReloadSet(WATCHDOG0_BASE, SysCtlClockGet());

#ifndef ETHERNET_EN
    WatchdogReloadSet(WATCHDOG0_BASE, (SysCtlClockGet() * 2));
#else
    WatchdogReloadSet(WATCHDOG0_BASE, (SysCtlClockGet() * 5));
#endif  //ETHERNET_EN

    // Enable the processor reset if watchdog is not fed (feeding is clearing interrupt)
    WatchdogResetEnable(WATCHDOG0_BASE);

    // Prevent changes to the setup values
    WatchdogLock(WATCHDOG0_BASE);

    // Enable Watchdog
    WatchdogEnable(WATCHDOG0_BASE);
}

void reset_controller(void)
{
#ifdef DEBUG_WDT
    vUART_SendStr(UART_PC, "\nS1=");
    vUART_SendInt(UART_PC, my_millis());
#endif  //DEBUG_WDT

#ifdef WDT_IRQ_MODE
    setWDTreset_flg(true); //We want to reset the controller
#endif  //WDT_IRQ_MODE
    while(1)
    {
#ifdef DEBUG_WDT
        vUART_SendStr(UART_PC, "\nS2=");
        // vUART_SendInt(UART_PC, my_millis()/1000);
        vUART_SendInt(UART_PC, my_millis());
#endif        
    }
}

void feedWDT(void)
{
#ifdef WDT_IRQ_MODE
    setWDTreset_flg(false);
#endif  //WDT_IRQ_MODE
    //
    // Clear the watchdog interrupt.
    //
    WatchdogIntClear(WATCHDOG0_BASE);
}

#ifdef WDT_IRQ_MODE
void setWDTreset_flg(bool val)
{
    WDT.resetWDT = val;
}

bool getWDTreset_flg(void)
{
    return WDT.resetWDT;
}
#endif  //WDT_IRQ_MODE

#endif  //ENABLE_WDT_RESET

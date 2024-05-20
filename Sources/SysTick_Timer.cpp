/*
 * SysTick_Timer.cpp
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"

#include "inc/hw_timer.h"

#include "inc/TM4C1233E6PZ.h"

#include "_config.h"
#include "SysTick_Timer.h"
#include "IO_cntrl.h"
#include "HW_pins.h"
#include "_common.h"
#include "ADC_Core.h"
#include "gprs.h"
#include "UartCore.h"
#include "main.h"

volatile uint32_t u32DelayCounter;
scheduler_t scheduler;
volatile uint32_t millis_cnt = 0;

extern volatile double ADC_RAW_MAX;
extern volatile EXTI_cnt_t EXTI_cnt;

extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;
extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern volatile Rx_Buff_t pc_uart_rx, display_uart_rx;

extern gprs_t gprs;
extern ram_data_t ram_data;

#ifdef ETHERNET_EN
extern volatile unsigned long  _millis;
#endif

void vPERIPH_SystickInit(void)
{
    //Systick Timer Configuration //
    SysTickPeriodSet((SYSTEM_CLOCK/SYSTICK_FREQ));
    SysTickIntEnable();
    SysTickEnable();
}

//*********************************************************************************//
// Function Name        :- void SysTickIntHandler(void)
// Input Parameters     :- void
// Output Parameters    :- void
// Description          :- Interrupt handler for systick interrupt.
//
// Note - This function mustbe added in vector table.
//*********************************************************************************//
void SysTickIntHandler(void)
{
    // static uint16_t ADC_RAW = 0;

    if(u32DelayCounter != 0U)
    {
        u32DelayCounter--;
    } //End of if

    millis_cnt++;   //unsigned variable. (uint32_t) should overflow to 0 again after reaching 0xFFFF_FFFF so no need of protection conditon?

#ifdef ETHERNET_EN
    if(_millis < 0xFFFFFFFF)
    {
        _millis++;
    }
#endif

    // ADC_RAW = readADC(MCU_VAC_ADC);
    // if(ADC_RAW > ADC_RAW_MAX)
    // {
    //     ADC_RAW_MAX = ADC_RAW;
    //     ADC_RAW = 0;
    // }

    scheduler.u16Cntr10ms++;
    scheduler.u16Cntr20ms++;
    scheduler.u16Cntr50ms++;
    scheduler.u16Cntr100ms++;
    scheduler.u16Cntr600ms++;
    scheduler.u16Cntr1s++;

    if(gprs_rx_isr_handler.elapsed < 0xFF)
    {
        gprs_rx_isr_handler.elapsed++;
    }
    if(display_uart_rx.elapsed < 0xFF)
    {
        display_uart_rx.elapsed++;
    }

//#if HW_VER == VER_1
//            vGPIO_Toggle(LED_PORT_BASE, (LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN), (LED1_PIN | LED3_PIN));
//#elif HW_VER == VER_2
//            MCP23017_pin_toggle(LED_PORT,STATUS_LED_PIN);
//#elif HW_VER == VER_3
//            // vGPIO_Toggle(LED_PORT_BASE, (LED1_PIN | LED2_PIN), (LED1_PIN | LED2_PIN));
//            vGPIO_Toggle(LED_PORT_BASE, LED1_PIN, LED1_PIN );
////#ifndef ETHERNET_EN
////            vGPIO_Toggle(ETHERNET_SPI_PORT_BASE, (ETHERNET_CLK_PIN | ETHERNET_MISO_PIN | ETHERNET_MOSI_PIN), (ETHERNET_CLK_PIN | ETHERNET_MISO_PIN | ETHERNET_MOSI_PIN));
////#endif
//#endif

    if(scheduler.u16Cntr10ms >= 10U)
    {
        scheduler.u16Cntr10ms = 0;
        scheduler.flg10ms = HIGH;
    }

    if(scheduler.u16Cntr20ms >= 20U)
    {
        scheduler.u16Cntr20ms = 0;
        scheduler.flg20ms = HIGH;

        // calculate_AC_ADC();
        vGPIO_Toggle(LED_PORT_BASE, LED2_PIN,  LED2_PIN);	//PP(24-04-24) for testing
    }

    if(scheduler.u16Cntr50ms >= 50U)
    {
        scheduler.u16Cntr50ms = 0;
        scheduler.flg50ms = HIGH;
    }

    if(scheduler.u16Cntr100ms >= 100U)
    {
        scheduler.u16Cntr100ms = 0;
        scheduler.flg100ms = HIGH;
        ToggleLEDs();
    }

    if(scheduler.u16Cntr600ms >= 600U)
    {
        scheduler.u16Cntr600ms = 0;
        scheduler.flg600ms = HIGH;
    }

    if(scheduler.u16Cntr1s >= 1000U)
    {
        scheduler.u16Cntr1s = 0;
        scheduler.flg1sec = HIGH;

        memcpy(&ram_data.ram_EXTI_cnt, (void*)&EXTI_cnt, sizeof(EXTI_cnt_t));
        memset((void*)&EXTI_cnt, 0, sizeof(EXTI_cnt_t));
    }
}   // End of SystickHandler

//*********************************************************************************//
// Function Name        :- void DelayMS(uint32_t u32Value)
// Input Parameters     :- delay value in mili seconds.
// Output Parameters    :- void
// Description          :- Provide delay for given time in mili seconds.
//*********************************************************************************//
void vMAIN_DelayMS(uint32_t u32Value)
{
    u32DelayCounter = u32Value;
    while(u32DelayCounter != 0U)
    {
        // wait till delay counter reaches to zero //
    }
} // End of DelayMS

uint32_t my_millis(void)
{
	uint32_t m;
    m = millis_cnt;
	// uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	// cli();
	// m = timer0_millis;
	// SREG = oldSREG;

	return m;
}

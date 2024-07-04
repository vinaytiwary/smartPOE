/*
 * IO_cntrl.cpp
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "inc/TM4C1233E6PZ.h"
#include "_config.h"
#include "HW_pins.h"
#include "IO_cntrl.h"
#include "UartCore.h"
#include "_debug.h"
#include "main.h"
#include "ADC_Core.h"
#include "E2P.h"
#include "_common.h"
#include "SysTick_Timer.h"

#ifdef ENABLE_GLCD
#include "GLCD.h"         //only for testing
#endif  //ENABLE_GLCD

voltage_mode_t voltage_mode;
volatile EXTI_cnt_t EXTI_cnt;
//router_mode_t router_mode;
extern ram_data_t ram_data;
BCD_MODE_t BCD_MODE;

relay_state_t relay_state;

relay_ctrl_state_t relay_ctrl_state;

extern uint32_t relay_startup_time;

//********************** GPIOD_IRQ_Handler *************************//
//extern "C" void GPIODIntHandler(void)
//{
//    uint8_t inputPinSts = 0;
//    // uint8_t EXTI_Sts = GPIOIntStatus(GPIO_PORTD_BASE, true);
//    uint32_t EXTI_Sts = GPIOIntStatus(GPIO_PORTD_BASE, true);
//    //uint32_t EXTI_Sts = GPIOIntStatus(GPIO_PORTD_BASE, false);
//
//    GPIOIntClear(GPIO_PORTD_BASE, EXTI_Sts);
//
//    if(EXTI_Sts & GPIO_PIN_1)
//    {
//        GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
//    }
//    else
//    {
//        GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_LOW);
//    }
//
////    if(EXTI_Sts)
////    {
////        EXTI_Sts = 0;
////        inputPinSts = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1)/GPIO_PIN_1;
////
////        if(inputPinSts)
////        {
////           // GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_HIGH);
////           GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
////
////        }
////        else
////        {
////            GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_LOW);
////           // GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
////        }
////    }
//}

extern "C" void GPIODIntHandler(void)
{
    uint8_t inputPinSts = 0;

//    vUART_SendChr(DEBUG_UART_BASE, 'A');

    // uint8_t EXTI_Sts = GPIOIntStatus(GPIO_PORTD_BASE, true);
    uint32_t EXTI_Sts = GPIOIntStatus(GPIO_PORTD_BASE, true);
    //uint32_t EXTI_Sts = GPIOIntStatus(GPIO_PORTD_BASE, false);

    GPIOIntClear(GPIO_PORTD_BASE, EXTI_Sts);

//    if(EXTI_Sts & GPIO_PIN_2)
//    {
//        GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
//    }
//    else
//    {
//        GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_LOW);
//    }

    if(EXTI_Sts)
    {
        EXTI_Sts = 0;
        inputPinSts = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)/GPIO_PIN_2;

        if(inputPinSts)
        {
           // GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_HIGH);
           GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);

        }
        else
        {
            GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_LOW);
           // GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
        }
    }
}

//********************** GPIO *************************//
void vGPIOPortEnable(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));

    vGPIO_UnlockGPIO(GPIO_PORTD_BASE,GPIO_PIN_7);
    vGPIO_UnlockGPIO(GPIO_PORTF_BASE,GPIO_PIN_0);
}

void vGPIO_UnlockGPIO(uint32_t ui32Port, uint8_t ui8Pins)
{
    HWREG(ui32Port + GPIO_O_LOCK) = GPIO_LOCK_KEY;      // Unlock the port
    HWREG(ui32Port + GPIO_O_CR) |= ui8Pins;             // Commit specified pins for next access
    HWREG(ui32Port + GPIO_O_AFSEL) &= ~ui8Pins;         // Function: GPIO (disable alternate function)
    HWREG(ui32Port + GPIO_O_DEN) &= ~ui8Pins;           // Disable digital driver
    HWREG(ui32Port + GPIO_O_LOCK) = 0;                  // Lock the port
}

void vLEDGPIOInit(void)
{
#if HW_VER == VER_1
    GPIOPinTypeGPIOOutput(LED_PORT_BASE, (LED1_PIN | LED2_PIN));
    GPIOPinWrite(LED_PORT_BASE, (LED1_PIN | LED2_PIN), GPIO_LOW);
#elif HW_VER == VER_2
#if 1
    //i2c_send_byte(MCP23017_ADDR,LED_DIR,~STATUS_LED_PIN);
    //i2c_send_byte(MCP23017_ADDR,LED_PORT,~STATUS_LED_PIN);
    MCP23017_set_dir(LED_DIR,STATUS_LED_PIN,LOW);
    MCP23017_pin_write(LED_PORT,STATUS_LED_PIN,LOW);
#endif
    GPIOPinTypeGPIOOutput(LED_PWM_PORT_BASE,RED_PWM_PIN|GREEN_PWM_PIN|BLUE_PWM_PIN);
    GPIOPinWrite(LED_PWM_PORT_BASE,RED_PWM_PIN|GREEN_PWM_PIN|BLUE_PWM_PIN,0);
    set_RGB_led(RGB_YELLOW);



#elif HW_VER == VER_3
    //for hardware version 3.0
    GPIOPinTypeGPIOOutput(LED_PORT_BASE, (LED1_PIN | LED2_PIN));
    // GPIOPinWrite(LED_PORT_BASE, (LED1_PIN | LED2_PIN), GPIO_LOW);
    GPIOPinWrite(LED_PORT_BASE, (LED1_PIN | LED2_PIN), (LED1_PIN | LED2_PIN));

//    GPIOPinTypeGPIOOutput(LED_PORT_BASE, LED2_PIN);
//    GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_LOW);

// GPIOPinTypeGPIOOutput(LED_PWM_PORT_BASE,RED_PWM_PIN|GREEN_PWM_PIN|BLUE_PWM_PIN);
// GPIOPinWrite(LED_PWM_PORT_BASE,RED_PWM_PIN|GREEN_PWM_PIN|BLUE_PWM_PIN,0);

// //set_RGB_led(RGB_YELLOW);          //PROBLEM: Program Hang
// // set_evse_status(CP_NOT_READY);
#endif	
}

void vPERIPH_GPIOInit(void)
{
    memset(&BCD_MODE, 0, sizeof(BCD_MODE_t));
    vLEDGPIOInit();
    // vInit_InputTestpins();   //commenting this as I'm using these pins for ADC testing.
    
#if HW_BOARD == TIOT_V2_00_BOARD
    init_ODU_Supplypins();
	// // vFreqDetectInit();
    vEarthCheckInit();
	
	GPIOPinTypeGPIOInput(BCD_SELECTOR_SW_BASE, (BCD_SELECTOR_S1|BCD_SELECTOR_S2|BCD_SELECTOR_S3|BCD_SELECTOR_S4));

    GPIOPinTypeGPIOOutput(RELAY_RTR_PORT, (RELAY_RTR | RELAY_RTR_SEL));
    GPIOPinTypeGPIOOutput(RELAY_ODU_PORT, RELAY_ODU );
    //GPIOPinWrite(RELAY_RTR_PORT, (RELAY_RTR),(RELAY_RTR));
   // GPIOPinWrite(RELAY_RTR_PORT, (RELAY_RTR_SEL),0);
    //GPIOPinTypeGPIOOutput(RELAY_RTR_PORT, RELAY_RTR );
    GPIOPinTypeGPIOOutput(BATT_CTRL_PORT, BATT_CTRL_PIN);
    GPIOPinWrite(BATT_CTRL_PORT, BATT_CTRL_PIN, BATT_CTRL_PIN);
    // GPIOPinWrite(BATT_CTRL_PORT, BATT_CTRL_PIN, LOW);
    //ControlODU_Relay(ON);

    GPIOPinTypeGPIOOutput(INVERTER_CTRL_PORT_BASE, INVERTER_CTRL_PIN);
    GPIOPinWrite(INVERTER_CTRL_PORT_BASE, INVERTER_CTRL_PIN, INVERTER_CTRL_PIN);
    // GPIOPinWrite(INVERTER_CTRL_PORT_BASE, INVERTER_CTRL_PIN, LOW);
#endif  //HW_BOARD == TIOT_V2_00_BOARD  
}

#if HW_BOARD == TIOT_V2_00_BOARD
void readBCD_SelectorSW(void)
{
    //uint8_t BCD_code = 0;
    //static uint8_t prev_BCDcode = 0;    //incase the roatry switch is >3 position.
    uint8_t BCD_pin0 = 0, BCD_pin1 = 0, BCD_pin2 = 0, BCD_pin3 = 0;

    BCD_pin0 = GPIOPinRead(BCD_SELECTOR_SW_BASE, BCD_SELECTOR_S1)/BCD_SELECTOR_S1;
    BCD_pin1 = GPIOPinRead(BCD_SELECTOR_SW_BASE, BCD_SELECTOR_S2)/BCD_SELECTOR_S2;
    BCD_pin2 = GPIOPinRead(BCD_SELECTOR_SW_BASE, BCD_SELECTOR_S3)/BCD_SELECTOR_S3;
    BCD_pin3 = GPIOPinRead(BCD_SELECTOR_SW_BASE, BCD_SELECTOR_S4)/BCD_SELECTOR_S4;

#ifdef  DEBUG_BCD_SEL_SW
    vUART_SendStr(UART_PC, "\nPB2,3,4,5:");
    vUART_SendInt(UART_PC, BCD_pin0);
    vUART_SendInt(UART_PC, BCD_pin1);
    vUART_SendInt(UART_PC, BCD_pin2);
    vUART_SendInt(UART_PC, BCD_pin3);
#endif  //DEBUG_BCD_SEL_SW

    BCD_MODE.BCD_code = ((BCD_pin0 << 0)|(BCD_pin1 << 1)|(BCD_pin2 << 2)|(BCD_pin3 << 3));
#ifdef  DEBUG_BCD_SEL_SW
    vUART_SendStr(UART_PC, "\n1BCD:");
    vUART_SendInt(UART_PC, BCD_MODE.BCD_code);
#endif  //DEBUG_BCD_SEL_SW

    if((BCD_MODE.BCD_code >= 0) && (BCD_MODE.BCD_code <= 4))
    {
        // // BCD_code = (BCD_code == 2)? ((BCD_code + 1) * 10): (BCD_code == 3)? (((BCD_code + 1) * 14)) : ((BCD_code + 1) * 12);
        // BCD_code = (BCD_code == 2)? 30 : (BCD_code == 4)? 56 : ((BCD_code + 1) * 12);
        // prev_BCDcode = BCD_code;
        BCD_MODE.BCD_code += 1;
        BCD_MODE.prev_BCDcode = BCD_MODE.BCD_code;
    }
    else
    {
#ifdef  DEBUG_BCD_SEL_SW
        vUART_SendStr(UART_PC, "\nINV_BCD:");
        vUART_SendInt(UART_PC, BCD_MODE.BCD_code);
#endif  //DEBUG_BCD_SEL_SW        
        BCD_MODE.BCD_code = BCD_MODE.prev_BCDcode;
    }
#ifdef  DEBUG_BCD_SEL_SW
    vUART_SendStr(UART_PC, "\n2BCD:");
    vUART_SendInt(UART_PC, BCD_MODE.BCD_code);
#endif  //DEBUG_BCD_SEL_SW  
    SetODU_Mode((voltage_mode_t)BCD_MODE.BCD_code);

    ram_data.supply_mode_R2 = (BCD_MODE.BCD_code == MODE_36V)? 30 : (BCD_MODE.BCD_code == MODE_56V)? 56 : ((BCD_MODE.BCD_code) * 12);
}
#endif  //#if HW_BOARD == TIOT_V2_00_BOARD

void vInit_InputTestpins(void)
{
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0);  //output pin to toggle and to be connected to input pin for input detection.
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_LOW);

//      GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);   //input pin
      GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);   //input pin
//      vEXTIpinInit();
}

void vGPIO_Toggle(uint32_t ui32Port, uint8_t ui8Pins, uint8_t ui8Val)
{
    ASSERT(_GPIOBaseValid(ui32Port));
    HWREG(ui32Port + (GPIO_O_DATA + (ui8Pins << 2))) ^= ui8Val;
}

void vInput_PollingRead(void)
{
    uint8_t inputPinSts = 0;
//    inputPinSts = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1)/GPIO_PIN_1;
    inputPinSts = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)/GPIO_PIN_2;

    if(inputPinSts)
    {
       // GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_HIGH);
       GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
        
    }
    else
    {
        GPIOPinWrite(LED_PORT_BASE, LED2_PIN, GPIO_LOW);
       // GPIOPinWrite(LED_PORT_BASE, LED2_PIN, LED2_PIN);
    }
}

//void vEXTIpinInit(void)
//{
////    GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_1);
////    GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_1);
////    GPIOIntRegister(GPIO_PORTD_BASE, &GPIODIntHandler);
//    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);   //input pin
//    GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_1 , GPIO_RISING_EDGE);
//    // GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_1 , GPIO_FALLING_EDGE);
//
////    GPIOIntRegister(GPIO_PORTD_BASE, &GPIODIntHandler);
//    GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_1);
//    GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_1);
//    IntEnable(INT_GPIOD);
//
//}

void vEXTIpinInit(void)
{
//    GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_2);
//    GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
//    GPIOIntRegister(GPIO_PORTD_BASE, &GPIODIntHandler);
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);   //input pin
    GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2 , GPIO_RISING_EDGE);
    // GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2 , GPIO_FALLING_EDGE);

//    GPIOIntRegister(GPIO_PORTD_BASE, &GPIODIntHandler);
    GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
    GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);
//    IntEnable(INT_GPIOD);

}

void ToggleLEDs(void)
{
#if HW_VER == VER_1
            vGPIO_Toggle(LED_PORT_BASE, (LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN), (LED1_PIN | LED3_PIN));
#elif HW_VER == VER_2
            MCP23017_pin_toggle(LED_PORT,STATUS_LED_PIN);
#elif HW_VER == VER_3
            // vGPIO_Toggle(LED_PORT_BASE, (LED1_PIN | LED2_PIN), (LED1_PIN | LED2_PIN));
            vGPIO_Toggle(LED_PORT_BASE, LED1_PIN, LED1_PIN );
//#ifndef ETHERNET_EN
//            vGPIO_Toggle(ETHERNET_SPI_PORT_BASE, (ETHERNET_CLK_PIN | ETHERNET_MISO_PIN | ETHERNET_MOSI_PIN), (ETHERNET_CLK_PIN | ETHERNET_MISO_PIN | ETHERNET_MOSI_PIN));
//#endif
#endif
}

#if HW_BOARD == TIOT_V2_00_BOARD

void init_ODU_Supplypins(void)
{
    GPIOPinTypeGPIOOutput(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN);
    GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, GPIO_LOW);    //Turn OFF ODU relay at startup
    // GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);

    GPIOPinTypeGPIOOutput(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN);
    GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, BUCK_BOOSTER_EN1_PIN);  //Turn OFF XL6019 A startup by setting 24V en pin HIGH.
    // GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, GPIO_LOW);

    GPIOPinTypeGPIOOutput(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN);
    GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, GPIO_LOW);    //Turn OFF 30V mode @ startup
    // GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, BUCK_BOOSTER_EN2_PIN);

    GPIOPinTypeGPIOOutput(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN);
    GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, GPIO_LOW);    //Turn OFF 48V mode @ startup
    // GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, BUCK_BOOSTER_EN3_PIN);

    GPIOPinTypeGPIOOutput(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN);
    GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, GPIO_LOW);    //Turn OFF 56V mode @ startup
    // GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, BUCK_BOOSTER_EN4_PIN);
}

void SetODU_Mode(voltage_mode_t BCD_SW)
{
    switch(BCD_SW)
    {
        case MODE_56V:
        {
#ifdef  DEBUG_GPIO
            vUART_SendStr(UART_PC, "\n56VM");
#endif  //DEBUG_GPIO
            GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, BUCK_BOOSTER_EN2_PIN);
            GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, BUCK_BOOSTER_EN3_PIN);
            GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, BUCK_BOOSTER_EN4_PIN);

            // GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;

        case MODE_48V:
        {
#ifdef  DEBUG_GPIO
            vUART_SendStr(UART_PC, "\n48VM");
#endif  //DEBUG_GPIO
            GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, BUCK_BOOSTER_EN2_PIN);
            GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, BUCK_BOOSTER_EN3_PIN);
            GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, GPIO_LOW);

            // GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;

        case MODE_36V:
        {
#ifdef  DEBUG_GPIO
            vUART_SendStr(UART_PC, "\n30VM");
#endif  //DEBUG_GPIO
            // GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, GPIO_LOW);
            // GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, GPIO_LOW);
            // GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, GPIO_LOW);
            // GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, BUCK_BOOSTER_EN4_PIN);

            GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, BUCK_BOOSTER_EN2_PIN);
            GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, GPIO_LOW);

            // GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;

        case MODE_24V:
        {
#ifdef  DEBUG_GPIO
            vUART_SendStr(UART_PC, "\n24VM");
#endif  //DEBUG_GPIO
            GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, GPIO_LOW);

            // GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;

        case MODE_12V:
        //case HOME:
        default:
        {
#ifdef  DEBUG_GPIO
            vUART_SendStr(UART_PC, "\n12VM");
#endif  //DEBUG_GPIO
            GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, BUCK_BOOSTER_EN1_PIN);
            GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, GPIO_LOW);

            // GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;
    }
}

#endif  // HW_BOARD == TIOT_V2_00_BOARD
void controlRelays(void)
{
    static bool mid_hysteresis_ODUrelay_state = false, mid_hysteresis_RTRrelay_state = false;
    switch(getRelay_CtrlState())
    {
        case EARTH_FAULT:
        {
            set_router_state(RELAY_OFF);
            set_router_selection_state(RELAY_OFF);
            //set_ODU_state(RELAY_OFF);

            if(my_millis() - relay_startup_time >= 10000)
            {
                ControlODU_Relay(OFF);
            }
#ifdef DEBUG_RELAY_STATE_LCD
            GLCD_GoTo(0,7);                      //ONLY FOR TEST
            GLCD_Clear_Line(7);                  //ONLY FOR TEST
            GLCD_WriteString((char*)"EARTH FAULT");                  //ONLY FOR TEST
#endif
#ifdef DEBUG_RELAY
            vUART_SendStr(UART_PC, "\nEF:");
            vUART_SendInt(UART_PC, get_router_state());
            vUART_SendChr(UART_PC, ',');
            vUART_SendInt(UART_PC, get_router_selection_state());
            vUART_SendChr(UART_PC, ',');
            vUART_SendInt(UART_PC, OFF);
#endif
            ram_data.ram_ADC.DC_current_router2 = 0;
            ram_data.ram_ADC.DC_Voltage_router2 = 0;
            ram_data.ram_ADC.DC_current_router1 = 0;
        }
        break;

        case MAINS_MODE:
        {
            set_router_selection_state(RELAY_OFF);
            set_router_state(RELAY_ON);
#ifdef DEBUG_RELAY_STATE_LCD
            GLCD_GoTo(0,7);                  //ONLY FOR TEST
            GLCD_Clear_Line(7);                  //ONLY FOR TEST
            GLCD_WriteString((char*)"MAINS MODE");                  //ONLY FOR TEST
#endif
#ifdef DEBUG_RELAY
            vUART_SendStr(UART_PC, "\nMM:");
            vUART_SendInt(UART_PC, get_router_state());
            vUART_SendChr(UART_PC, ',');
            vUART_SendInt(UART_PC, get_router_selection_state());
#endif
            if(isSupplyStable() && (my_millis() - relay_startup_time >= 10000))
            {
                //GPIOPinWrite(RELAY_ODU_PORT,RELAY_ODU, RELAY_ODU);
                //set_ODU_state(RELAY_ON);
                ControlODU_Relay(ON);
#ifdef DEBUG_RELAY
                vUART_SendStr(UART_PC, "\nMM:ODU_ON");
#endif
            }
            else
            {
                if(my_millis() - relay_startup_time >= 10000)
                {
                    //set_ODU_state(RELAY_OFF);
                    ControlODU_Relay(OFF);
                }
                ram_data.ram_ADC.DC_current_router2 = 0;
                ram_data.ram_ADC.DC_Voltage_router2 = 0;
#ifdef DEBUG_RELAY
                vUART_SendStr(UART_PC, "\nMMunst:");
                vUART_SendInt(UART_PC, ram_data.ram_ADC.DC_Voltage_router2);
#endif
            }
        }
        break;

        case BATT_MODE:
        {
            if(my_millis() - relay_startup_time >= 10000)
            {
                //set_router_selection_state(RELAY_ON);
                //set_router_state(RELAY_ON);
                ControlRouterSelection_Relay(RTR_INVERTER_SUPPLY);
                // ControlRouter_Relay(ON);    //PP: ONLY FOR TEST
            }
#ifdef DEBUG_RELAY_STATE_LCD
            GLCD_GoTo(0,7);                  //ONLY FOR TEST
            GLCD_Clear_Line(7);                  //ONLY FOR TEST
            GLCD_WriteString((char*)"BATT MODE");                  //ONLY FOR TEST
#endif
#ifdef DEBUG_RELAY
            vUART_SendStr(UART_PC, "\nBM");
#endif
#if 1
            if(ram_data.ram_ADC.DC_Battery_voltage < BATT_HYSTERESIS_LOW)
            {
#ifdef DEBUG_RELAY
                vUART_SendStr(UART_PC, "\nBL:");
#endif
                if((my_millis() - relay_startup_time >= 10000))
                {
                    //GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR, 0);
                    //GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU, 0);
                    ControlRouter_Relay(OFF);
                    ControlODU_Relay(OFF);
                    //set_router_state(RELAY_OFF);
                    //set_ODU_state(RELAY_OFF);
                }
                
                ram_data.ram_ADC.DC_current_router2 = 0;
                ram_data.ram_ADC.DC_Voltage_router2 = 0;
                ram_data.ram_ADC.DC_current_router1 = 0;

#ifdef DEBUG_RELAY
                vUART_SendInt(UART_PC, ram_data.ram_ADC.DC_Battery_voltage);
                vUART_SendChr(UART_PC, ',');
                vUART_SendInt(UART_PC, BATT_HYSTERESIS_LOW);
#endif
                mid_hysteresis_ODUrelay_state = false;
                mid_hysteresis_RTRrelay_state = false;
            }
            if(ram_data.ram_ADC.DC_Battery_voltage > BATT_HYSTERESIS_HIGH)
            {
#ifdef DEBUG_RELAY
                vUART_SendStr(UART_PC, "\n1BH:");
#endif
                if(my_millis() - relay_startup_time >= 10000)
                {
                    //GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR,RELAY_RTR);
                    //set_relay_state(RELAY_ON);
                    ControlRouter_Relay(ON);
                }

                if(isSupplyStable() && (my_millis() - relay_startup_time >= 10000))
                {
                    //GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU,RELAY_ODU);
                    ControlODU_Relay(ON);
                    //set_ODU_state(RELAY_ON);

                    mid_hysteresis_ODUrelay_state = true;
#ifdef DEBUG_RELAY
                    vUART_SendStr(UART_PC, "\n1BMH:RTR_ON,RS_INV,ODU_ON");
#endif
                }
                else
                {
#ifdef DEBUG_RELAY
                    vUART_SendStr(UART_PC, "\nBMunst:");
                    vUART_SendInt(UART_PC, ram_data.ram_ADC.DC_Voltage_router2);
#endif
                    if(my_millis() - relay_startup_time >= 10000)
                    {
                        //GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU, 0);
                        ControlODU_Relay(OFF);
                        //set_ODU_state(RELAY_OFF);
                    }

                    ram_data.ram_ADC.DC_current_router2 = 0;
                    ram_data.ram_ADC.DC_Voltage_router2 = 0;
                    ram_data.ram_ADC.DC_current_router1 = 0;

                    mid_hysteresis_ODUrelay_state = false;
#ifdef DEBUG_RELAY
                    vUART_SendStr(UART_PC, "\n2BMH:RTR_ON,RS_INV,ODU_OFF");
#endif
                }
#ifdef DEBUG_RELAY
                vUART_SendStr(UART_PC, "\n2BH:");
                vUART_SendInt(UART_PC, ram_data.ram_ADC.DC_Battery_voltage);
                vUART_SendChr(UART_PC, ',');
                vUART_SendInt(UART_PC, BATT_HYSTERESIS_HIGH);
#endif
                mid_hysteresis_RTRrelay_state = true;
            }

            if((ram_data.ram_ADC.DC_Battery_voltage >= BATT_HYSTERESIS_LOW) || (ram_data.ram_ADC.DC_Battery_voltage <= BATT_HYSTERESIS_HIGH))
            {
                if(!mid_hysteresis_ODUrelay_state)
                {
                    ram_data.ram_ADC.DC_current_router2 = 0;
                    ram_data.ram_ADC.DC_Voltage_router2 = 0;
                }

                ram_data.ram_ADC.DC_current_router1 = mid_hysteresis_RTRrelay_state? ram_data.ram_ADC.DC_current_router1 : 0;
            }
#endif  //if 0

#if 0
#ifdef DEBUG_RELAY
            vUART_SendStr(UART_PC, "\nBM_test_relaysON");
#endif
            ControlRouter_Relay(ON);
            ControlODU_Relay(ON);
#endif  // if 0
        }
        break;

        default:
        {

        }
        break;
    }
}

bool isSupplyStable(void)
{
    bool ret = false;
    uint32_t R2_mode = 0;
    if(BCD_MODE.BCD_code == MODE_12V)
    {
        R2_mode = ((((uint32_t)ram_data.supply_mode_R2 * 1000) + 1500));
#ifdef DEBUG_RELAY
        vUART_SendStr(UART_PC, "\nM_12:");
        vUART_SendInt(UART_PC, R2_mode);
#endif
        if((ram_data.ram_ADC.DC_Voltage_router2) <= R2_mode)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
    else
    {
        R2_mode = ((((uint32_t)ram_data.supply_mode_R2 * 1000) + 2500));
#ifdef DEBUG_RELAY
        vUART_SendStr(UART_PC, "\nM>12:");
        vUART_SendInt(UART_PC, R2_mode);
#endif
        if((ram_data.ram_ADC.DC_Voltage_router2) <= R2_mode)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
    return ret;
}

void vFreqDetectInit(void)
{
    GPIOIntRegister(FREQ_MEAS_PIN_BASE, FREQDetIntHandler);
    GPIOPinTypeGPIOInput(FREQ_MEAS_PIN_BASE,FREQ_MEAS_PIN);
    // GPIOIntTypeSet(FREQ_MEAS_PIN_BASE, FREQ_MEAS_PIN , GPIO_RISING_EDGE);
    GPIOIntTypeSet(FREQ_MEAS_PIN_BASE, FREQ_MEAS_PIN , GPIO_FALLING_EDGE);
//    GPIOIntTypeSet(ZERO_CROSS_DET_PIN_BASE, ZERO_CROSS_DET_PIN , GPIO_BOTH_EDGES);
    GPIOIntEnable(FREQ_MEAS_PIN_BASE, FREQ_MEAS_PIN);
}
void vEarthCheckInit(void)
{
    GPIOPinTypeGPIOInput(EARTH_DETECT_PORT_BASE, (EARTH_DETECT_PIN));
}
void FREQDetIntHandler(void)
{
    EXTI_cnt.freq_cnt++;
    GPIOIntClear(FREQ_MEAS_PIN_BASE, GPIOIntStatus(FREQ_MEAS_PIN_BASE, true));

   if((get_router_state()==RELAY_ON) && (my_millis() - relay_startup_time >= 10000))
   {
       set_router_state(RELAY_DEFAULT);
       ControlRouter_Relay(ON);
   }
   else if((get_router_state()==RELAY_OFF) && (my_millis() - relay_startup_time >= 10000))
   {
       set_router_state(RELAY_DEFAULT);
       ControlRouter_Relay(OFF);
   }

   if((get_router_selection_state()==RELAY_ON) && (my_millis() - relay_startup_time >= 10000))
   {
       set_router_selection_state(RELAY_DEFAULT);
       ControlRouterSelection_Relay(RTR_INVERTER_SUPPLY);
   }
   else if((get_router_selection_state()==RELAY_OFF) && (my_millis() - relay_startup_time >= 10000))
   {
       set_router_selection_state(RELAY_DEFAULT);
       ControlRouterSelection_Relay(RTR_MAIN_SUPPLY);
   }
   
   if(!(EXTI_cnt.freq_cnt%5))
    {
        IntEnable(INT_TIMER0B);
        TimerEnable(TIMER0_BASE, TIMER_B);
    }

}

uint8_t vEarthDetect(void)
{
    uint32_t value = 0;
    value = GPIOPinRead(EARTH_DETECT_PORT_BASE,EARTH_DETECT_PIN);

        //value /= WELDCHECK1_PIN;
#ifdef DEBUG_EARTHDETECT
//    vUART_SendStr(UART_PC,"\nEarth Detect:");
//    vUART_SendInt(UART_PC,value);
#endif

    return value;
}

void ControlODU_Relay(uint8_t val)
{
    if(val)
    {
        GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU, RELAY_ODU);
    }
    else
    {
        GPIOPinWrite(RELAY_ODU_PORT, RELAY_ODU, 0);
    }
}
void ControlRouter_Relay(uint8_t val)
{
    if(val)
    {
        GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR, RELAY_RTR);
    }
    else
    {
        GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR, 0);
    }
}

void ControlRouterSelection_Relay(uint8_t val)
{
    if(val)
    {
        GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR_SEL, RELAY_RTR_SEL);
    }
    else
    {
        GPIOPinWrite(RELAY_RTR_PORT, RELAY_RTR_SEL, 0);
    }
}
/*
void set_ODU_state(relay_ctrl_state_t state)
{
    relay_state.ODU = state;
}
relay_ctrl_state_t get_ODU_state(void)
{
    return  relay_state.ODU;
}
*/
void set_router_state(relay_ctrl_state_t state)
{
    relay_state.router = state;
}
relay_ctrl_state_t get_router_state(void)
{
    return relay_state.router;
}

void set_router_selection_state(relay_ctrl_state_t state)
{
    relay_state.router_selection = state;
}
relay_ctrl_state_t get_router_selection_state(void)
{
    return relay_state.router_selection;
}


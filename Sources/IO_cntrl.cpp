/*
 * IO_cntrl.cpp
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#include "stdint.h"
#include "stdbool.h"

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

volatile EXTI_cnt_t EXTI_cnt;
voltage_mode_t voltage_mode;

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
    vLEDGPIOInit();
    // vInit_InputTestpins();   //commenting this as I'm using these pins for ADC testing.
    
#if HW_BOARD == TIOT_V2_00_BOARD
    init_ODU_Supplypins();
#endif  //HW_BOARD == TIOT_V2_00_BOARD

    GPIOPinTypeGPIOInput(BCD_SELECTOR_SW_BASE, (BCD_SELECTOR_S1|BCD_SELECTOR_S2|BCD_SELECTOR_S3|BCD_SELECTOR_S4));
}

#if HW_BOARD == TIOT_V2_00_BOARD
void readBCD_SelectorSW(void)
{
    uint8_t BCD_code = 0;
    static uint8_t prev_BCDcode = 0;    //incase the roatry switch is >3 position.
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

    BCD_code = ((BCD_pin0 << 0)|(BCD_pin1 << 1)|(BCD_pin2 << 2)|(BCD_pin3 << 3));
#ifdef  DEBUG_BCD_SEL_SW
    vUART_SendStr(UART_PC, "\n1BCD:");
    vUART_SendInt(UART_PC, BCD_code);
#endif  //DEBUG_BCD_SEL_SW

    if((BCD_code >= 0) && (BCD_code <= 4))
    {
        // // BCD_code = (BCD_code == 2)? ((BCD_code + 1) * 10): (BCD_code == 3)? (((BCD_code + 1) * 14)) : ((BCD_code + 1) * 12);
        // BCD_code = (BCD_code == 2)? 30 : (BCD_code == 4)? 56 : ((BCD_code + 1) * 12);
        // prev_BCDcode = BCD_code;
        BCD_code += 1;
        prev_BCDcode = BCD_code;
    }
    else
    {
        BCD_code = prev_BCDcode;
#ifdef  DEBUG_BCD_SEL_SW
        vUART_SendStr(UART_PC, "\nINV_BCD:");
        vUART_SendInt(UART_PC, BCD_code);
#endif  //DEBUG_BCD_SEL_SW        
    }
#ifdef  DEBUG_BCD_SEL_SW
    vUART_SendStr(UART_PC, "\n2BCD:");
    vUART_SendInt(UART_PC, BCD_code);
#endif  //DEBUG_BCD_SEL_SW  
    SetODU_Mode((voltage_mode_t)BCD_code);
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

            GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
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

            GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;

        case MODE_36V:
        {
#ifdef  DEBUG_GPIO
            vUART_SendStr(UART_PC, "\n30VM");
#endif  //DEBUG_GPIO
            GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, BUCK_BOOSTER_EN4_PIN);

            GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
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

            GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;

        case MODE_12V:
        case HOME:
        default:
        {
#ifdef  DEBUG_GPIO
            vUART_SendStr(UART_PC, "\n12VM");
#endif  //DEBUG_GPIO
            GPIOPinWrite(BUCK_BOOSTER_EN1_BASE, BUCK_BOOSTER_EN1_PIN, BUCK_BOOSTER_EN1_PIN);
            GPIOPinWrite(BUCK_BOOSTER_EN2_BASE, BUCK_BOOSTER_EN2_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN3_BASE, BUCK_BOOSTER_EN3_PIN, GPIO_LOW);
            GPIOPinWrite(BUCK_BOOSTER_EN4_BASE, BUCK_BOOSTER_EN4_PIN, GPIO_LOW);

            GPIOPinWrite(ODU_SUPPLY_RELAY_BASE, ODU_SUPPLY_RELAY_PIN, ODU_SUPPLY_RELAY_PIN);
        }
        break;
    }
}

#endif  // HW_BOARD == TIOT_V2_00_BOARD

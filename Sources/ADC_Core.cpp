/*
 * ADC_Core.cpp
 *
 *  Created on: Apr 22, 2024
 *      Author: ADMIN
 */

#include <stdint.h>
#include <stdbool.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <math.h>
#include <Sources/SysTick_Timer.h>

#include "driverlib/adc.h"
#include "inc/TM4C1233E6PZ.h"

#include "_config.h"
#include "HW_pins.h"
#include "UartCore.h"
#include "ADC_Core.h"
#include "_debug.h"

#include "GLCD.h"

adc_t adc[TOTAL_ADC_SIGNALS];
measurements_t measurements;
adc_arr_t adc_arr;
volatile double PN_ADC_RAW_MAX = 0.0f;
volatile double NE_ADC_RAW_MAX = 0.000;
extern volatile uint32_t millis_cnt;

#ifdef ADC_EN

//******************************* ADC ******************************************//

/***************************************************************************//**
* @brief      ADC_PortInit
* @param      num : ADC peripheral selected, should be
*              - 0 to 21 : the number of ADC channel (AIN0 to AIN21)
* @return     None
*******************************************************************************/
void ADC_PortInit(int num)
{
    if(num == 0) 
    {
        // AIN0 (PE3)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    } 
    else if (num == 1) 
    {
        // AIN1 (PE2)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
    } 
    else if (num == 2) 
    {
        // AIN2 (PE1)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);
    } 
    else if (num == 3) 
    {
        // AIN3 (PE0)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
    } 
    else if (num == 4) 
    {  
        // AIN4 (PD7)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    } 
    else if (num == 5) 
    {
        // AIN5 (PD6)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_6);
    } 
    else if (num == 6) 
    {
        // AIN6 (PD5)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_5);
    } 
    else if (num == 7) 
    {
        // AIN7 (PD4)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_4);
    } 
    else if (num == 8) 
    {
        // AIN8 (PE5)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);
    } 
    else if (num == 9) 
    {  
        // AIN9 (PE4)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);
    } 
    else if (num == 10) 
    {
        // AIN10 (PB4)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_4);
    } 
    else if (num == 11) 
    {
        // AIN11 (PB5)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);
    } 
    else if (num == 12) 
    {
        // AIN12 (PD3)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);
    } 
    else if (num == 13) 
    {
        // AIN13 (PD2)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_2);
    } 
    else if (num == 14) 
    {
        // AIN14 (PD1)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);
    } 
    else if (num == 15) 
    { 
        // AIN15 (PD0)
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
       GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);
    } 
    else if (num == 16) 
    {
        // AIN16 (PH0)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
        GPIOPinTypeADC(GPIO_PORTH_BASE, GPIO_PIN_0);
    } 
    else if (num == 17) 
    {
        // AIN17 (PH1)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
        GPIOPinTypeADC(GPIO_PORTH_BASE, GPIO_PIN_1);
    } 
    else if (num == 18) 
    {
        // AIN18 (PH2)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
        GPIOPinTypeADC(GPIO_PORTH_BASE, GPIO_PIN_2);
    } 
    else if (num == 19) 
    { 
        // AIN19 (PH3)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
        GPIOPinTypeADC(GPIO_PORTH_BASE, GPIO_PIN_3);
    } 
    else if (num == 20) 
    {
        // AIN20 (PE7)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_7);
    } 
    else if (num == 21) 
    {
        // AIN21 (PE6)
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_6);
    }
    else
    {
        return;
    }
}

void vADC0Init(void)
{
    //
    // The ADC0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // //
    // // For this example ADC0 is used with AIN0/1 on port E7/E6.
    // // The actual port and pins used may be different on your part, consult
    // // the data sheet for more information.  GPIO port E needs to be enabled
    // // so these pins can be used.
    // // TODO: change this to whichever GPIO port you are using.
    // //
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // //
    // // Select the analog ADC function for these pins.
    // // Consult the data sheet to see which functions are allocated per pin.
    // // TODO: change this to select the port/pin you are using.
    // //
    // GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2);
    // GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_5);
    // //

    ADC_PortInit(SIG_AC_VOLTAGE_ADC);
    ADC_PortInit(SIG_ODU_CURRENT_ADC);
    ADC_PortInit(SIG_RTR_CURRENT_ADC);
#if 0
    ADC_PortInit(MCU_ADC_VCC_POE_1);
#endif  //if 0
    ADC_PortInit(SIG_ODU_VOLTAGE_ADC);
    ADC_PortInit(SIG_BATTERY_VOLT_ADC);
    ADC_PortInit(SIG_12V_IN_ADC);
#if HW_BOARD == TIOT_V2_00_BOARD
//    ADC_PortInit(SIG_EARTH_VTG_ADC);
#endif  //HW_BOARD == TIOT_V2_00_BOARD

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This example is arbitrarily using sequence 3.
    //
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);


    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // information on the ADC sequences and steps, reference the datasheet.
    //
#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nADC_CHS=");
    vUART_SendInt(DEBUG_UART_BASE,REQUIRED_ADC_CHANNELS);
   
#endif

    //ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CHANNEL_0, ADC_CTL_CH_START);     //komal_adc
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, REQUIRED_ADC_CHANNELS | ADC_CTL_IE |
                                ADC_CTL_END);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, 3);
}

uint32_t readADC(uint8_t seqno)
{
    uint32_t adc_value,pui32ADC0Value[1];
    ADC0_SSMUX3_R = seqno;

    //
    // Trigger the ADC conversion.
    //
    ADCProcessorTrigger(ADC0_BASE, 3);

    //
    // Wait for conversion to be completed.
    //
    while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

    //
    // Clear the ADC interrupt flag.
    //
    ADCIntClear(ADC0_BASE, 3);

    //
    // Read ADC Value.
    //
    ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
    adc_value = pui32ADC0Value[0];

#ifdef DEBUG_ADC
    // if((seqno == SIG_ODU_VOLTAGE_ADC)||(seqno == SIG_ODU_CURRENT_ADC))
    {
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRAW=");
        vUART_SendInt(DEBUG_UART_BASE,adc_value);
    }    
#endif
    return adc_value;
}

#ifdef ADC_FILTER_1
uint16_t updateADC(ADC_Channels_t ch, uint8_t indx)
{
    //int i = 0;
    unsigned int i = 0;
    adc[indx].av = 0;
    //adc[indx].max_adc = 0;

    // static int num_of_elements = 0;

// #ifdef DEBUG_ADC
//     // vUART_SendStr(UART_PC,(uint8_t*)"\narr:" );
//     // for(i = 0; i < (NUM_OF_SAMPLES - 1); i++)
//     // {
//     //     vUART_SendInt(UART_PC,adc[indx].arr[i] );
//     //     vUART_SendChr(UART_PC,',');
//     // }
// #endif

    for(i = 0; i < (NUM_OF_SAMPLES - 1); i++)
    {
        adc[indx].arr[NUM_OF_SAMPLES-i-1] = adc[indx].arr[NUM_OF_SAMPLES-i-2];
    }

    adc[indx].arr[0] = readADC(ch);

    adc[indx].num_of_elements += 1;

// #ifdef DEBUG_ADC
//         // vUART_SendStr(UART_PC,(uint8_t*)"\nRAWadc:");
// 	    // vUART_SendInt(UART_PC,adc.arr[0]);
// #endif

    if (adc[indx].num_of_elements > NUM_OF_SAMPLES)
	{
		adc[indx].num_of_elements = NUM_OF_SAMPLES;
	}

    if (adc[indx].num_of_elements == NUM_OF_SAMPLES)
    {
        adc[indx].max_adc = adc[indx].min_adc = adc[indx].arr[0];
    }
    
    for (i = 0; i < (NUM_OF_SAMPLES); i++)
	{
        if (adc[indx].num_of_elements == NUM_OF_SAMPLES)
        {
            adc[indx].max_adc = (adc[indx].arr[i] > adc[indx].max_adc)? adc[indx].arr[i]: adc[indx].max_adc;

            adc[indx].min_adc = (adc[indx].arr[i] < adc[indx].min_adc)? adc[indx].arr[i]: adc[indx].min_adc;
        }

		adc[indx].av += adc[indx].arr[i];

#ifdef DEBUG_ADC
        // if((indx == ADC_INDX_BATTV)||(indx == ADC_INDX_12VIN))
        // if((ch == SIG_ODU_VOLTAGE_ADC)||((ch == SIG_ODU_CURRENT_ADC)))
        {
            vUART_SendStr(UART_PC,(uint8_t*)"\nRAWadc:" );
            vUART_SendInt(UART_PC,adc[indx].arr[i]);
            vUART_SendChr(UART_PC,',');
            vUART_SendInt(UART_PC,adc[indx].max_adc);
            vUART_SendChr(UART_PC,',');
            vUART_SendInt(UART_PC,adc[indx].min_adc);
        }
#endif
	}

#ifdef DEBUG_ADC
        // if((indx == ADC_INDX_BATTV)||(indx == ADC_INDX_12VIN))
        // if((ch == SIG_ODU_VOLTAGE_ADC)||((ch == SIG_ODU_CURRENT_ADC)))
        {
            vUART_SendStr(UART_PC,(uint8_t*)"\nSUM:" );
            vUART_SendInt(UART_PC,adc[indx].av );
            vUART_SendStr(UART_PC,(uint8_t*)"\nNUM:" );
            vUART_SendInt(UART_PC,adc[indx].num_of_elements );

            if (adc[indx].num_of_elements == NUM_OF_SAMPLES)
            {
                vUART_SendStr(UART_PC,(uint8_t*)"\nSUM-max-min:" );
                vUART_SendInt(UART_PC,(adc[indx].av - adc[indx].max_adc - adc[indx].min_adc));
                vUART_SendChr(UART_PC,',');
                vUART_SendInt(UART_PC,(adc[indx].num_of_elements - 2) );
            }
        }
#endif

    if (adc[indx].num_of_elements == NUM_OF_SAMPLES)
    {
        adc[indx].av = (adc[indx].av - adc[indx].max_adc - adc[indx].min_adc);
    }

    adc[indx].av = (adc[indx].num_of_elements == NUM_OF_SAMPLES)? (adc[indx].av/(adc[indx].num_of_elements - 2)) : (adc[indx].av/adc[indx].num_of_elements);

    return adc[indx].av;
}
#else
#endif

uint32_t getODUCurrent(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(SIG_ODU_CURRENT_ADC, ADC_INDX_ODUC);

    analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * GAIN_OPAMP_U3) * OPAMP_INPUT_SHUNT_RESISTOR;   //PP (24-04-24) commented until HW is finalized
    // analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION));
    analog_vtg *= 1000;

    measurements.DC_current_router2 = analog_vtg;

// #ifdef DEBUG_ADC
//     // UWriteString((char*)"\nAVGadc:", DBG_UART);
//     // UWriteInt(adc_avg, DBG_UART);
//     // UWriteString((char*)"\nV@ PA1=",DBG_UART);
//     // UWriteInt((unsigned long)analog_vtg, DBG_UART);   
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
//     vUART_SendInt(DEBUG_UART_BASE,adc_avg);
//     // vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE0=");
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE3=");
//     vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);
// #endif

    return measurements.DC_current_router2;
}

uint32_t getRouter1_DC_current(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(SIG_RTR_CURRENT_ADC, ADC_INDX_RTRC);

    analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * GAIN_OPAMP_U3) * OPAMP_INPUT_SHUNT_RESISTOR;   //PP (24-04-24) commented until HW is finalized
    // analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION));
    analog_vtg *= 1000;

    measurements.DC_current_router1 = analog_vtg;

// #ifdef DEBUG_ADC
//     // UWriteString((char*)"\nAVGadc:", DBG_UART);
//     // UWriteInt(adc_avg, DBG_UART);
//     // UWriteString((char*)"\nV@ PA2=",DBG_UART);
//     // UWriteInt((unsigned long)analog_vtg, DBG_UART); 
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
//     vUART_SendInt(DEBUG_UART_BASE,adc_avg);
//     // vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE5=");
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD5=");
//     vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);   
// #endif

    return measurements.DC_current_router1;
}

#if 0
uint32_t getRouter1_DCvoltage(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(MCU_ADC_VCC_POE_1, DC_VADC1);

    // analog_vtg = adc_avg * (ADC_REFV/ADC_RESOLUTION) * ROUTER1_RESISTOR_RATIO;   //PP (24-04-24) commented until HW is finalized
    analog_vtg = adc_avg * (ADC_REFV/ADC_RESOLUTION);
    analog_vtg *= 1000;

    //measurements.DC_Voltage_router1 = analog_vtg*10215;
    measurements.DC_Voltage_router1 = analog_vtg;

#ifdef DEBUG_ADC
    // UWriteString((char*)"\nAVGadc:", DBG_UART);
    // UWriteInt(adc_avg, DBG_UART);
    // UWriteString((char*)"\nV@ PA5=",DBG_UART);
    // UWriteInt((unsigned long)analog_vtg, DBG_UART); 
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
    vUART_SendInt(DEBUG_UART_BASE,adc_avg);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE4=");
    vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);   
#endif

    return measurements.DC_Voltage_router1;
}
#endif //if 0

uint32_t getODUVoltage(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(SIG_ODU_VOLTAGE_ADC, ADC_INDX_ODUV);

    analog_vtg = adc_avg * (ADC_REFV/ADC_RESOLUTION) * ODUV_RESISTOR_RATIO;   //PP (24-04-24) commented until HW is finalized
    // analog_vtg = adc_avg * (ADC_REFV/ADC_RESOLUTION);
    analog_vtg *= 1000;

    //measurements.DC_Voltage_router2 = analog_vtg*10215;
    measurements.DC_Voltage_router2 = analog_vtg;

#ifdef DEBUG_ADC
    // UWriteString((char*)"\nAVGadc:", DBG_UART);
    // UWriteInt(adc_avg, DBG_UART);
    // UWriteString((char*)"\nV@ PB0=",DBG_UART);
    // UWriteInt((unsigned long)analog_vtg, DBG_UART); 
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
    vUART_SendInt(DEBUG_UART_BASE,adc_avg);
    // vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD2=");
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE2=");
    vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);   
#endif

    return measurements.DC_Voltage_router2;
}

uint32_t getBatteryVoltage(void)
{
    GPIOPinWrite(BATT_CTRL_PORT, BATT_CTRL_PIN, GPIO_LOW);  //PP added on 20-06-24

    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(SIG_BATTERY_VOLT_ADC, ADC_INDX_BATTV);

    _delay_us(50);

    //analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98097) * BATTERY_RESISTOR_RATIO;
    // analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98223) * BATTERY_RESISTOR_RATIO;
     analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION)) * BATTERY_RESISTOR_RATIO; //PP (24-04-24) commented until HW is finalized
//    analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION)) * ((10000+3300)/3300); //PP (24-04-24) commented until HW is finalized
//    analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION));

    analog_vtg *= 1000;
    //analog_vtg *= DC_BATTERY_RESISTOR_RATIO;

    //measurements.DC_Battery_voltage = analog_vtg*2866; //(analog_vtg * 1000);;
    measurements.DC_Battery_voltage = analog_vtg; 

// #ifdef DEBUG_ADC
//     // UWriteString((char*)"\nAVGadc:", DBG_UART);
//     // UWriteInt(adc_avg, DBG_UART);
//     // UWriteString((char*)"\nV@ PB1=",DBG_UART);
//     // UWriteInt((unsigned long)analog_vtg, DBG_UART);  
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
//     vUART_SendInt(DEBUG_UART_BASE,adc_avg);
//     // vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD1=");
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE0=");
//     vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);  
// #endif

    GPIOPinWrite(BATT_CTRL_PORT, BATT_CTRL_PIN, BATT_CTRL_PIN); //PP added on 20-06-24
    return measurements.DC_Battery_voltage;
}

uint32_t getSMPSVoltage(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(SIG_12V_IN_ADC, ADC_INDX_12VIN);

    //analog_vtg = (((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98189) * SMPS_12VIN_RESISTOR_RATIO);  //multiplying by offset 0.98189 (1/1.0184415335) calculated from the slope between measurement and adc values on PB2 for the range 0-5v
    // analog_vtg = (((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98223) * SMPS_12VIN_RESISTOR_RATIO);
    analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * SMPS_12VIN_RESISTOR_RATIO);
    // analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION)); //PP (24-04-24) commented until HW is finalized
    
    analog_vtg *= 1000;
    //analog_vtg *= DC_CHARGER_RESISTOR_RATIO;

    measurements.DC_Charger_voltage = analog_vtg; //(analog_vtg * 1000);

// #ifdef DEBUG_ADC
//     // UWriteString((char*)"\nAVGadc:", DBG_UART);
//     // UWriteInt(adc_avg, DBG_UART);
//     // UWriteString((char*)"\nV@ PB2=",DBG_UART);
//     // UWriteInt((unsigned long)analog_vtg, DBG_UART);  
//     // UWriteData('\n',DBG_UART);
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
//     vUART_SendInt(DEBUG_UART_BASE,adc_avg);
//     // vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD0=");
//     vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE1=");
//     vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);
// #endif

    return measurements.DC_Charger_voltage;
}

uint32_t calculate_PN_AC_ADC(void)
{
//    int x1 = 3045, y1 = 237 , x2 = 3055, y2 = 250;
//    measurements.PN_AC_Voltage = (PN_ADC_RAW_MAX * 85.3112033)/1000; //75585284280
    //measurements.PN_AC_Voltage = (PN_ADC_RAW_MAX * 81.6393442)/1000; // //75.5852842  //77.7150448
    measurements.PN_AC_Voltage = (PN_ADC_RAW_MAX * 80.894442); // //75.5852842  //77.7150448
    //0.0816393442622950819672131147541
//    measurements.PN_AC_Voltage = (PN_ADC_RAW_MAX * 83.871)/1000;
//    measurements.PN_AC_Voltage = sqrt() / 4095 * 5 * 500.0;
    /****************************************************************************
    ADC_VALUE : 2999,3006,2979 -> MAIN_VOLTAGE = 228
    ADC_VALUE : 3044,3045,3042 -> MAIN_VOLTAGE = 246

    calculating voltage by line equation of two points.

    (y-y1) = ((y2-y1)/(x2-x1)) * (x-x1) where y is voltage and x is adc value

    if y1 = 228 , y2 is 246 and x1 = 2999 , x2 = 3044
    y = 0.4x - 971.6 ( y = kx +c)

    measurements.PN_AC_Voltage = (((246-228)/(3044-2999)) * (PN_ADC_RAW_MAX - 2999)) + 246;
    ********************************************************************************/
//    measurements.PN_AC_Voltage = (2.983561644 * PN_ADC_RAW_MAX) - 8849.478082;
//    measurements.PN_AC_Voltage = (0.461 * PN_ADC_RAW_MAX) - 1159.26;
//    measurements.PN_AC_Voltage = ((float)((y2-y1)/(x2-x1)) * (PN_ADC_RAW_MAX - x1)) + y1;

    PN_ADC_RAW_MAX = 0;

    // measurements.PN_AC_Voltage = (measurements.PN_AC_Voltage < 200)? 0 : measurements.PN_AC_Voltage;

//     return (measurements.PN_AC_Voltage * 1000);
//    measurements.PN_AC_Voltage *= 1000;
#ifdef DEBUG_MAIN_ADC
            vUART_SendStr(DEBUG_UART_BASE,", ");
            vUART_SendInt(DEBUG_UART_BASE,measurements.PN_AC_Voltage);
#endif
    return measurements.PN_AC_Voltage;
}

uint32_t calculate_NE_AC_ADC(void)
{
    measurements.NE_AC_Voltage = (NE_ADC_RAW_MAX * 83.8709677)/1000;
    //measurements.NE_AC_Voltage = (NE_ADC_RAW_MAX * 83.871)/1000;
    NE_ADC_RAW_MAX = 0;

    // measurements.NE_AC_Voltage = (measurements.NE_AC_Voltage < 200)? 0 : measurements.NE_AC_Voltage;

    // return (measurements.NE_AC_Voltage * 1000);
    measurements.NE_AC_Voltage *= 1000;

    return measurements.NE_AC_Voltage;
}
void readACVoltage()
{
    static uint16_t PN_ADC_RAW = 0;
//    int32_t Vnow = 0;
//    uint32_t Vsum = 0;
//    uint32_t measurements_count = 0;
//    uint32_t t_start = my_millis();
//    double readingVoltage = 0.0f;
//#if HW_BOARD == TIOT_V2_00_BOARD

//    if(my_millis() - ACVoltReadMillis >= 5)
    //if(g_bIntFlag)
//    {
//        vUART_SendStr(DEBUG_UART_BASE,"\nFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
//        g_bIntFlag = false;
        PN_ADC_RAW = readADC(SIG_AC_VOLTAGE_ADC);

//        while (my_millis() - t_start < 20)
//        {
//            Vnow = readADC(SIG_AC_VOLTAGE_ADC) - 2048;
//            Vsum += (Vnow * Vnow);
//            measurements_count++;
//        }
//
//        readingVoltage = sqrt(Vsum / measurements_count) / 4095 * 3.3 * 500.0f;
//        readingVoltage -= 29;

        if(adc_arr.index < 10)
        {
            // adc_arr.collectSamples[adc_arr.index++] = PN_ADC_RAW;

            adc_arr.collectSamples[adc_arr.index] = PN_ADC_RAW;

            adc_arr.index++;
        }
        

#ifdef DEBUG_MAIN_ADC
        // vUART_SendStr(DEBUG_UART_BASE, "\nRAW=");
        // vUART_SendInt(DEBUG_UART_BASE,PN_ADC_RAW);
        // vUART_SendChr(DEBUG_UART_BASE, ',');
        // vUART_SendInt(DEBUG_UART_BASE, adc_arr.index);
#endif  //DEBUG_MAIN_ADC

        // if(adc_arr.index > 9)
        // {
        //     adc_arr.index = 0;
        // }
        // else
        // {
        //     adc_arr.index++;
        // }
// #ifdef DEBUG_MAIN_ADC
//         vUART_SendStr(DEBUG_UART_BASE, "\nRAW=");
//         vUART_SendInt(DEBUG_UART_BASE,PN_ADC_RAW);
//         vUART_SendChr(DEBUG_UART_BASE, ',');
//         vUART_SendInt(DEBUG_UART_BASE, adc_arr.index - 1);
// #endif  //DEBUG_MAIN_ADC

////        if (PN_ADC_RAW > current_adc_val)
//        {
//            PN_ADC_RAW_MAX = PN_ADC_RAW;
//#if HW_BOARD == TIOT_V2_00_BOARD
//         calculate_PN_AC_ADC();
//        // calculate_NE_AC_ADC();
//#endif  //HW_BOARD == TIOT_V2_00_BOARD
//#ifdef DEBUG_MAIN_ADC
//            vUART_SendStr(DEBUG_UART_BASE,"\nV:");
//            vUART_SendInt(DEBUG_UART_BASE,readingVoltage);
////            vUART_SendChr(DEBUG_UART_BASE,',');
////            vUART_SendInt(DEBUG_UART_BASE,measurements_count);
////            vUART_SendChr(DEBUG_UART_BASE,',');
////            vUART_SendInt(DEBUG_UART_BASE,Vsum);
//
//#endif
//        }
//        IntDisable(INT_TIMER0B);
//        TimerIntDisable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//        ACVoltReadMillis = my_millis();
        /*PN_ADC_RAW_MAX */


//    }
//    if(millis_cnt % 2 == 0)
//    {
//         PN_ADC_RAW = readADC(SIG_AC_VOLTAGE_ADC);
//    PN_ADC_RAW_MAX = readADC(SIG_AC_VOLTAGE_ADC);
//         if(PN_ADC_RAW > PN_ADC_RAW_MAX)
//         {
//             PN_ADC_RAW_MAX = PN_ADC_RAW;
//             PN_ADC_RAW = 0;
//         }
//    }
//    else
//    {
        // NE_ADC_RAW = readADC(SIG_EARTH_VTG_ADC);
        // if(NE_ADC_RAW > NE_ADC_RAW_MAX)
        // {
        //     NE_ADC_RAW_MAX = NE_ADC_RAW;
        //     NE_ADC_RAW = 0;
        // }
//    }
//#endif  //HW_BOARD == TIOT_V2_00_BOARD
//    calculate_PN_AC_ADC();
}
void GetAdcData(void)
{
   //getACvoltage();

#ifdef DEBUG_AC_ADC
    // vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE1=");
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD7=");
    vUART_SendInt(DEBUG_UART_BASE,(int32_t)measurements.PN_AC_Voltage);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD6=");
    vUART_SendInt(DEBUG_UART_BASE,(int32_t)measurements.NE_AC_Voltage);
#endif
//    measurements.PN_AC_Voltage = 0;

   getRouter1_DC_current();

   getODUCurrent();

#if 0
   getRouter1_DCvoltage();
#endif  // if 0

   getODUVoltage();

   getBatteryVoltage();

   getSMPSVoltage();

#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\n1ACP,RC,ODUC,ACN,ODUV,BV,SV:");
    vUART_SendInt(DEBUG_UART_BASE,measurements.PN_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_current_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_current_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    // vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Voltage_router1);
    vUART_SendInt(DEBUG_UART_BASE,measurements.NE_AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Voltage_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Battery_voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Charger_voltage);
#endif
}

#if HW_BOARD == TIOT_V2_00_BOARD
#ifdef DEBUG_ADC_SIG
void get_ADC_SIGarray(ADC_Channels_t ch, uint8_t indx)
{
    vUART_SendStr(UART_PC, "\nASIG:");
    vUART_SendInt(UART_PC, ch);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, indx);

    vUART_SendStr(UART_PC, "\nASIG_arr");

    for(uint8_t i = 0; i < (NUM_OF_SAMPLES - 1); i++)
    {
        vUART_SendChr(UART_PC, ',');
        vUART_SendInt(UART_PC, adc[indx].arr[i]);
    }

    vUART_SendStr(UART_PC, "\nAMAX,MIN:");
    vUART_SendInt(UART_PC, adc[indx].max_adc);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, adc[indx].min_adc);

    vUART_SendStr(UART_PC, "\nASIG_av:");
    vUART_SendInt(UART_PC, adc[indx].av);

    vUART_SendStr(UART_PC, "#ele,#ele-max-min:");
    vUART_SendInt(UART_PC, adc[indx].num_of_elements);
    vUART_SendChr(UART_PC, ',');
    vUART_SendInt(UART_PC, (adc[indx].num_of_elements-2));

    vUART_SendStr(UART_PC, "\nAFinal:");
    if(ch == SIG_ODU_CURRENT_ADC)
    {
        vUART_SendInt(UART_PC, measurements.DC_current_router2);
    }
    else if(ch == SIG_ODU_VOLTAGE_ADC)
    {
        vUART_SendInt(UART_PC, measurements.DC_Voltage_router2);
    }
    else if(ch == SIG_12V_IN_ADC)
    {
        vUART_SendInt(UART_PC, measurements.DC_Charger_voltage);
    }
    else if(ch == SIG_BATTERY_VOLT_ADC)
    {
        vUART_SendInt(UART_PC, measurements.DC_Battery_voltage);
    }
    else if(ch == SIG_AC_VOLTAGE_ADC)
    {
        vUART_SendInt(UART_PC, measurements.PN_AC_Voltage);
    }
    else if(ch == SIG_EARTH_VTG_ADC)
    {
        vUART_SendInt(UART_PC, measurements.NE_AC_Voltage);
    }
    else if(ch == SIG_RTR_CURRENT_ADC)
    {
        vUART_SendInt(UART_PC, measurements.DC_current_router1);
    }
}
#endif  //DEBUG_ADC_SIG
#endif  //HW_BOARD == TIOT_V2_00_BOARD

#endif  //ADC_EN



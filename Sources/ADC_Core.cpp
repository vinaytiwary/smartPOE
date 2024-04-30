/*
 * ADC_Core.cpp
 *
 *  Created on: Apr 22, 2024
 *      Author: ADMIN
 */

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/adc.h"
#include "inc/TM4C1233E6PZ.h"

#include "HW_pins.h"
#include "UartCore.h"
#include "ADC_Core.h"
#include "_debug.h"
#include "_config.h"

adc_t adc[TOTAL_ADC_SIGNALS];
measurements_t measurements;
volatile double ADC_RAW_MAX = 0.000;

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

    ADC_PortInit(MCU_VAC_ADC);
    ADC_PortInit(ADC_current_router2);
    ADC_PortInit(ADC_current_router1);
    ADC_PortInit(MCU_ADC_VCC_POE_1);
    ADC_PortInit(MCU_ADC_VCC_POE_2);
    ADC_PortInit(MCU_Battery_VADC);
    ADC_PortInit(MCU_VADC);

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
        vUART_SendStr(DEBUG_UART_BASE, (uint8_t*)"\nRAW=");
        vUART_SendInt(DEBUG_UART_BASE,adc_value);
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

#ifdef DEBUG_ADC
    // vUART_SendStr(UART_PC,(uint8_t*)"\narr:" );
    // for(i = 0; i < (NUM_OF_SAMPLES - 1); i++)
    // {
    //     vUART_SendInt(UART_PC,adc[indx].arr[i] );
    //     vUART_SendChr(UART_PC,',');
    // }
#endif

    for(i = 0; i < (NUM_OF_SAMPLES - 1); i++)
    {
        adc[indx].arr[NUM_OF_SAMPLES-i-1] = adc[indx].arr[NUM_OF_SAMPLES-i-2];
    }

    adc[indx].arr[0] = readADC(ch);

    adc[indx].num_of_elements += 1;

#ifdef DEBUG_ADC
        // vUART_SendStr(UART_PC,(uint8_t*)"\nRAWadc:");
	    // vUART_SendInt(UART_PC,adc.arr[0]);
#endif

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
        // if((indx == BATT_ADC)||(indx == CHARGER_ADC))
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
        // if((indx == BATT_ADC)||(indx == CHARGER_ADC))
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

uint32_t getRouter2_DC_current(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(ADC_current_router2, DC_CADC2);

    // analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * GAIN_OPAMP_U4) * OPAMP_INPUT_SHUNT_RESISTOR;   //PP (24-04-24) commented until HW is finalized
    analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION));
    analog_vtg *= 1000;

    measurements.DC_current_router2 = analog_vtg;

#ifdef DEBUG_ADC
    // UWriteString((char*)"\nAVGadc:", DBG_UART);
    // UWriteInt(adc_avg, DBG_UART);
    // UWriteString((char*)"\nV@ PA1=",DBG_UART);
    // UWriteInt((unsigned long)analog_vtg, DBG_UART);   
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
    vUART_SendInt(DEBUG_UART_BASE,adc_avg);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE0=");
    vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);
#endif

    return measurements.DC_current_router2;
}

uint32_t getRouter1_DC_current(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(ADC_current_router1, DC_CADC1);

    // analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * GAIN_OPAMP_U4) * OPAMP_INPUT_SHUNT_RESISTOR;   //PP (24-04-24) commented until HW is finalized
    analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION));
    analog_vtg *= 1000;

    measurements.DC_current_router1 = analog_vtg;

#ifdef DEBUG_ADC
    // UWriteString((char*)"\nAVGadc:", DBG_UART);
    // UWriteInt(adc_avg, DBG_UART);
    // UWriteString((char*)"\nV@ PA2=",DBG_UART);
    // UWriteInt((unsigned long)analog_vtg, DBG_UART); 
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
    vUART_SendInt(DEBUG_UART_BASE,adc_avg);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE5=");
    vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);   
#endif

    return measurements.DC_current_router1;
}

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

uint32_t getRouter2_DCvoltage(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(MCU_ADC_VCC_POE_2, DC_VADC2);

    // analog_vtg = adc_avg * (ADC_REFV/ADC_RESOLUTION) * ROUTER2_RESISTOR_RATIO;   //PP (24-04-24) commented until HW is finalized
    analog_vtg = adc_avg * (ADC_REFV/ADC_RESOLUTION);
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
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD2=");
    vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);   
#endif

    return measurements.DC_Voltage_router2;
}

uint32_t getBatteryVoltage(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(MCU_Battery_VADC, BATT_ADC);

    //analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98097) * BATTERY_RESISTOR_RATIO;
    // analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98223) * BATTERY_RESISTOR_RATIO;
    // analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION)) * BATTERY_RESISTOR_RATIO; //PP (24-04-24) commented until HW is finalized
    analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION));

    analog_vtg *= 1000;
    //analog_vtg *= DC_BATTERY_RESISTOR_RATIO;

    //measurements.DC_Battery_voltage = analog_vtg*2866; //(analog_vtg * 1000);;
    measurements.DC_Battery_voltage = analog_vtg; 

#ifdef DEBUG_ADC
    // UWriteString((char*)"\nAVGadc:", DBG_UART);
    // UWriteInt(adc_avg, DBG_UART);
    // UWriteString((char*)"\nV@ PB1=",DBG_UART);
    // UWriteInt((unsigned long)analog_vtg, DBG_UART);  
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
    vUART_SendInt(DEBUG_UART_BASE,adc_avg);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD1=");
    vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);  
#endif

    return measurements.DC_Battery_voltage;
}

uint32_t getChargerVoltage(void)
{
    uint32_t adc_avg = 0;
    double analog_vtg = 0.0;

    adc_avg = updateADC(MCU_VADC, CHARGER_ADC);

    //analog_vtg = (((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98189) * CHARGER_RESISTOR_RATIO);  //multiplying by offset 0.98189 (1/1.0184415335) calculated from the slope between measurement and adc values on PB2 for the range 0-5v
    // analog_vtg = (((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * 0.98223) * CHARGER_RESISTOR_RATIO);
    // analog_vtg = ((adc_avg * (ADC_REFV/ADC_RESOLUTION)) * CHARGER_RESISTOR_RATIO);
    analog_vtg = (adc_avg * (ADC_REFV/ADC_RESOLUTION)); //PP (24-04-24) commented until HW is finalized
    
    analog_vtg *= 1000;
    //analog_vtg *= DC_CHARGER_RESISTOR_RATIO;

    measurements.DC_Charger_voltage = analog_vtg; //(analog_vtg * 1000);

#ifdef DEBUG_ADC
    // UWriteString((char*)"\nAVGadc:", DBG_UART);
    // UWriteInt(adc_avg, DBG_UART);
    // UWriteString((char*)"\nV@ PB2=",DBG_UART);
    // UWriteInt((unsigned long)analog_vtg, DBG_UART);  
    // UWriteData('\n',DBG_UART);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nAVGadc:");
    vUART_SendInt(DEBUG_UART_BASE,adc_avg);
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PD0=");
    vUART_SendInt(DEBUG_UART_BASE,/* (int32_t) */analog_vtg);
#endif

    return measurements.DC_Charger_voltage;
}

uint32_t calculate_AC_ADC(void)
{
    measurements.AC_Voltage = (ADC_RAW_MAX * 83.8709677)/1000;
    //measurements.AC_Voltage = (ADC_RAW_MAX * 83.871)/1000;
    ADC_RAW_MAX = 0;

    // measurements.AC_Voltage = (measurements.AC_Voltage < 200)? 0 : measurements.AC_Voltage;

    // return (measurements.AC_Voltage * 1000);
    measurements.AC_Voltage *= 1000;

    return measurements.AC_Voltage;
}

void GetAdcData(void)
{
   //getACvoltage();

#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\nV@ PE1=");
    vUART_SendInt(DEBUG_UART_BASE,(int32_t)measurements.AC_Voltage);
#endif
//    measurements.AC_Voltage = 0;

   getRouter1_DC_current();

   getRouter2_DC_current();

   getRouter1_DCvoltage();

   getRouter2_DCvoltage();

   getBatteryVoltage();

   getChargerVoltage();

#ifdef DEBUG_ADC
    vUART_SendStr(DEBUG_UART_BASE,(uint8_t*)"\n1ACV,RC,ODUC,RV,ODUV,BV,SV:");
    vUART_SendInt(DEBUG_UART_BASE,measurements.AC_Voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_current_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_current_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Voltage_router1);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Voltage_router2);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Battery_voltage);
    vUART_SendChr(DEBUG_UART_BASE, ',');
    vUART_SendInt(DEBUG_UART_BASE,measurements.DC_Charger_voltage);
#endif
}



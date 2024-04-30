/*
 * ADC_Core.h
 *
 *  Created on: Apr 22, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_ADC_CORE_H_
#define SOURCES_ADC_CORE_H_

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/adc.h"
#include "inc/TM4C1233E6PZ.h"

#include "_config.h"

#define ADC_REFV		(3.3)
#define ADC_RESOLUTION	(4095)

#define NUM_OF_SAMPLES		(10)				//10

#define TOTAL_ADC_SIGNALS	(7)

#define REQUIRED_ADC_CHANNELS   (ADC_CTL_CH2|ADC_CTL_CH3|ADC_CTL_CH8|ADC_CTL_CH9|ADC_CTL_CH13|ADC_CTL_CH14|ADC_CTL_CH15)

enum	//ADC indexes Used for adc raw array queue sampling
{
	AC_VADC = 0,	//0
	DC_CADC2,		//1
	DC_CADC1,		//2
	DC_VADC1,		//3
	DC_VADC2,		//4	
	BATT_ADC,		//5
	CHARGER_ADC		//6
};

typedef enum	//used for adc channel number of the gpio pin
{
	MCU_VAC_ADC	= 2,			    //ADC_AIN2
	ADC_current_router2,		    //ADC_AIN3
	ADC_current_router1 = 8,		//ADC_AIN8
	MCU_ADC_VCC_POE_1,		        //ADC_AIN9
	MCU_ADC_VCC_POE_2 = 13,		    //ADC_AIN13
	MCU_Battery_VADC,			    //ADC_AIN14
	MCU_VADC					    //ADC_AIN15
}ADC_Channels_t;

typedef struct
{
	uint32_t arr[NUM_OF_SAMPLES];
	uint32_t av;
	uint16_t num_of_elements;
#ifdef ADC_FILTER_1
	uint32_t max_adc;
	uint32_t min_adc;
#endif	//ADC_FILTER_1
}__attribute__((packed))adc_t;

typedef struct 
{
	uint32_t AC_Voltage;
	uint32_t DC_current_router1;
	uint32_t DC_current_router2;
	uint32_t DC_Voltage_router1;
	uint32_t DC_Voltage_router2;
	uint32_t DC_Battery_voltage;
	uint32_t DC_Charger_voltage;	
}__attribute__((packed))measurements_t;

void ADC_PortInit(int num);

void vADC0Init(void);

uint32_t readADC(uint8_t seqno);

uint16_t updateADC(ADC_Channels_t ch, uint8_t indx);

uint32_t getRouter2_DC_current(void);

uint32_t getRouter1_DC_current(void);

uint32_t getRouter1_DCvoltage(void);

uint32_t getRouter2_DCvoltage(void);

uint32_t getBatteryVoltage(void);

uint32_t getChargerVoltage(void);

uint32_t calculate_AC_ADC(void);

void GetAdcData(void);

#endif /* SOURCES_ADC_CORE_H_ */


/*
 * main.h
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_MAIN_H_
#define SOURCES_MAIN_H_

#include "stdint.h"
#include "stdbool.h"

#include "IO_cntrl.h"
#include "ADC_Core.h"
#include "Clock.h"

#define MINUTE		          (60)

#define SAVE_OFFLINE_TIME		(30 * MINUTE)
// #define SAVE_OFFLINE_TIME		(1 * MINUTE)

typedef enum
{
  EARTH_FAULT,
  BATT_MODE,
  MAINS_MODE,
}relay_ctrl_sts_t;

typedef enum
{
  MAINS_FAULT,    //BIT 0
  MAINS_OVF,      //BIT 1
  PHASE_FAULT,    //BIT 2
  EARTHING_FAULT, //BIT 3
  SUPPLY_MODE,    //BIT 4
  SMPS_FAULT,     //BIT 5
  BATT_FAULT,     //BIT 6
  RTR_NC,         //BIT 7
  ODU_NC,         //BIT 8
  RTR_V_FAULT,    //BIT 9
  ODU_V_FAULT,    //BIT 10
  POWER_ON_BIT,   //BIT 11
}Alarm_sts_bits_t;

typedef struct 
{
  unsigned char MAINS_fault;            //0: AC_V avbl, 1: AC_
  unsigned char MAINS_OVF_fault;           //0: OK, 1: 220V > ACV > 240V
  unsigned char ACPhase_fault;       //0: OK, 1: 48Hz > Freq > 52Hz
  unsigned char ACEarth_fault;       //0: OK, 1: EC = 0
  unsigned char Supply_mode;         //0: CHG, 1: BATT
  unsigned char Chg_fault;           //0: OK, 1: 10V > CHG > 13V
  unsigned char Batt_low;            //0: 10V < BATT > 13V or Supply_mode = 0(CHG), 1: 10V > BATT > 13V and Supply_mode = 1(BATT)
  unsigned char Router1_NC;          //0: OK, 1: Router1_C < 100mA
  unsigned char Router2_NC;          //0: OK, 1: Router2_C < 100mA
  unsigned char Router1_V_fault;     //0: OK, 1: 10V > R1V > 13V, or 22V > R1V > 26V, or 34V > R1V > 38V, or 46V > R1V > 50V
  unsigned char Router2_V_fault;     //0: OK, 1: 10V > R2V > 13V, or 22V > R2V > 26V, or 34V > R2V > 38V, or 46V > R2V > 50V
  unsigned char Power_ON;
}__attribute__((packed))Alarms_t;

typedef struct
{
    EXTI_cnt_t ram_EXTI_cnt;
    measurements_t ram_ADC;
    time_stamp_t ram_time;
    uint8_t supply_mode_R1;
    uint8_t supply_mode_R2;
    int32_t Latitude;
    int32_t Longitude;
    uint32_t Status;
    uint32_t ram_alarms;
}__attribute__((packed))ram_data_t;

void vPERIPH_ClockInit(void);
void vMAIN_InitClockPeripherals(void);

void write_defaults(uint32_t addr);

void init_config(void);

void update_ram_data(void);

void update_alarm_status(void);

uint32_t getRSTreason(void);

// void setRAM_Alarm(uint8_t val, bool sts);
void setRAM_Alarm(Alarm_sts_bits_t val, bool sts);

uint32_t getRAM_Alarm(void);

void setRAM_Status(uint8_t val, bool sts);

uint32_t getRAM_Status(void);
relay_ctrl_sts_t getRelay_CtrlState(void);
#endif /* SOURCES_MAIN_H_ */

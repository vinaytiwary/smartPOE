/*
 * IO_cntrl.h
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_IO_CNTRL_H_
#define SOURCES_IO_CNTRL_H_

#include "stdint.h"
#include <stdbool.h>
#include "_config.h"

typedef enum
{
  ROUTER_1,
  ROUTER_2
}__attribute__((packed))router_num_t;

typedef enum
{
  HOME,
  MODE_12V,
  MODE_24V,
  MODE_36V,
  MODE_48V,
  MODE_56V
}__attribute__((packed))voltage_mode_t;

typedef struct
{
  uint8_t freq_cnt;
  uint8_t earth_cnt;
}__attribute__((packed))EXTI_cnt_t;

#ifdef __cplusplus
extern "C" {
#endif

void GPIODIntHandler(void);

#ifdef __cplusplus
}
#endif

void vGPIOPortEnable(void);
void vGPIO_UnlockGPIO(uint32_t ui32Port, uint8_t ui8Pins);
void vLEDGPIOInit(void);
void vPERIPH_GPIOInit(void);
#if HW_BOARD == TIOT_V2_00_BOARD
void readBCD_SelectorSW(void);
#endif  //HW_BOARD == TIOT_V2_00_BOARD
void vInit_InputTestpins(void);
void vGPIO_Toggle(uint32_t ui32Port, uint8_t ui8Pins, uint8_t ui8Val);
void vInput_PollingRead(void);
void vEXTIpinInit(void);
void ToggleLEDs(void);

#if HW_BOARD == TIOT_V2_00_BOARD
void init_ODU_Supplypins(void);
#endif  //HW_BOARD == TIOT_V2_00_BOARD

void SetODU_Mode(voltage_mode_t BCD_SW);

#endif /* SOURCES_IO_CNTRL_H_ */

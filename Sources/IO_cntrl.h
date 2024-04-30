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
  MODE_48V
}__attribute__((packed))router_mode_t;

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
void vInit_InputTestpins(void);
void vGPIO_Toggle(uint32_t ui32Port, uint8_t ui8Pins, uint8_t ui8Val);
void vInput_PollingRead(void);
void vEXTIpinInit(void);
void ToggleLEDs(void);

#endif /* SOURCES_IO_CNTRL_H_ */

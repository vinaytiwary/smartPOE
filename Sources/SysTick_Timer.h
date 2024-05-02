/*
 * SysTick_Timer.h
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_SYSTICK_TIMER_H_
#define SOURCES_SYSTICK_TIMER_H_

#include "stdint.h"
#include "stdbool.h"

typedef struct
{
    uint16_t u16Cntr10ms;
    uint16_t u16Cntr20ms;
    uint16_t u16Cntr50ms;
    uint16_t u16Cntr100ms;
    uint16_t u16Cntr600ms;
    uint16_t u16Cntr1s;
    uint8_t flg10ms     :1;
    uint8_t flg20ms     :1;
    uint8_t flg50ms     :1;
    uint8_t flg100ms    :1;
    uint8_t flg600ms    :1;
    uint8_t flg1sec     :1;
    uint8_t reserved    :3;
}__attribute__((packed)) scheduler_t;

void vPERIPH_SystickInit(void);
void vMAIN_DelayMS(uint32_t u32Value);

uint32_t my_millis(void);

#ifdef __cplusplus
extern "C" {
#endif

void SysTickIntHandler(void);

#ifdef __cplusplus
}
#endif

#endif  //SOURCES_SYSTICK_TIMER_H_

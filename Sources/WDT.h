/*
 * WDT.h
 *
 *  Created on: May 19, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_WDT_H_
#define SOURCES_WDT_H_

#include "_config.h"
#ifdef ENABLE_WDT_RESET

#ifdef WDT_IRQ_MODE
typedef struct
{
    volatile bool resetWDT;
}WDT_t;

#ifdef __cplusplus
extern "C" {
#endif

void WatchdogIntHandler(void);

#ifdef __cplusplus
}
#endif
#endif  //WDT_IRQ_MODE

void initWDT(void);

void reset_controller(void);

void feedWDT(void);

#ifdef WDT_IRQ_MODE
void setWDTreset_flg(bool val);

bool getWDTreset_flg(void);
#endif  //WDT_IRQ_MODE

#endif  //ENABLE_WDT_RESET

#endif /* SOURCES_WDT_H_ */

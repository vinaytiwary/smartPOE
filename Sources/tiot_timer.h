/*
 * tiot_timer.h
 *
 *  Created on: 18-Jun-2024
 *      Author: Dell
 */

#ifndef SOURCES_TIOT_TIMER_H_
#define SOURCES_TIOT_TIMER_H_
//#include <Sources/tiot_timer.h>

void vTimerInit(void);
#ifdef __cplusplus
extern "C" {
#endif
void TimerIntHandler(void);
#ifdef __cplusplus
}
#endif

#endif /* SOURCES_TIOT_TIMER_H_ */

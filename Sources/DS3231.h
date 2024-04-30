/*
 * DS3231.h
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_DS3231_H_
#define SOURCES_DS3231_H_


#define RTC_ADDR        (0x68)              //(0xD0)              // For DS3231
#define RTC_DS3231

void vRTCI2CInit(void);
uint32_t I2CReceive(void);
void Read_RTC(uint8_t *, uint8_t);
void Write_RTC(uint8_t *, uint8_t);


#endif /* SOURCES_DS3231_H_ */

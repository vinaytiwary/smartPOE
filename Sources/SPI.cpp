/*
 * SPI.cpp
 *
 *  Created on: Apr 23, 2024
 *      Author: ADMIN
 */

#include "stdint.h"
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"

#include "inc/TM4C1233E6PZ.h"
#include "_config.h"

#include "HW_pins.h"
#include "SPI.h"
#include "delay.h"

uint8_t SPI_transrecv(uint32_t base,uint8_t data)
{

    uint32_t rxtxData;
#if 1
    rxtxData = data;

    SSIDataPutNonBlocking(base, (uint8_t) rxtxData);

    //while(SSIBusy(RFID_SPI_BASE));
    sleep_us(1);  //need to update function for 1us


    SSIDataGetNonBlocking(base, &rxtxData);

    return (uint8_t) rxtxData;
#endif
    /*uint32_t rxtxData;

    rxtxData = data;

    SSIDataPutNonBlocking(RFID_SPI_BASE, (uint8_t) rxtxData);

    //while(SSIBusy(RFID_SPI_BASE));
    sleep_us(1);

    SSIDataGetNonBlocking(RFID_SPI_BASE, &rxtxData);

    return (uint8_t) rxtxData;*/
}



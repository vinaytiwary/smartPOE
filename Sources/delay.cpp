/*
 * delay.cpp
 *
 *  Created on: Apr 23, 2024
 *      Author: ADMIN
 */

#include <stdbool.h>
#include <stdint.h>
#include "HW_pins.h"
#include "delay.h"

#define US_TIME    SYSTEM_CLOCK/1000000    /* 10 uS */

void sleep_us(int us)
{
    volatile int    i;
    while (us--)
    {
        for (i = 0; i < US_TIME; i++)
        {
            ;  //   Burn cycles.
        }
    }
}



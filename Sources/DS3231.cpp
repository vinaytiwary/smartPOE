/*
 * DS3231.cpp
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */
#include "stdint.h"
#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "driverlib\i2c.h"
#include "driverlib\sysctl.h"
#include "driverlib\pin_map.h"
#include "DS3231.h"
#include "HW_pins.h"

void vRTCI2CInit(void)
{
    SysCtlPeripheralDisable(DS3231_I2C_PERIPH);
    //reset module
    SysCtlPeripheralReset(DS3231_I2C_PERIPH);
    SysCtlPeripheralEnable(DS3231_I2C_PERIPH);

    while(!SysCtlPeripheralReady(DS3231_I2C_PERIPH));

    GPIOPinConfigure(DS3231_SCL_GPIO_CFG);
    GPIOPinConfigure(DS3231_SDA_GPIO_CFG);

    GPIOPinTypeI2CSCL(DS3231_I2C_PORT_BASE, DS3231_SCL_PIN);
    GPIOPinTypeI2C(DS3231_I2C_PORT_BASE, DS3231_SDA_PIN);

    I2CMasterInitExpClk(DS3231_I2C_BASE, SysCtlClockGet(), false);

    I2CRxFIFOFlush(DS3231_I2C_BASE);
    I2CTxFIFOFlush(DS3231_I2C_BASE);

    //HWREG(DS3231_I2C_BASE + I2C_O_FIFOCTL) = 80008000;
}

//read specified register on slave device
uint32_t I2CReceive(void)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(DS3231_I2C_BASE, RTC_ADDR, false);

    //specify register to be read
    I2CMasterDataPut(DS3231_I2C_BASE, 00);

    //send control byte and register address byte to slave device
    I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(DS3231_I2C_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(DS3231_I2C_BASE, RTC_ADDR, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(DS3231_I2C_BASE));

    //return data pulled from the specified register
    return I2CMasterDataGet(DS3231_I2C_BASE);
}
void Read_RTC(uint8_t *rtc_data, uint8_t len)
{
    uint8_t index = 0;
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(DS3231_I2C_BASE, RTC_ADDR, false);

    //specify register to be read
    I2CMasterDataPut(DS3231_I2C_BASE, 00);

    //send control byte and register address byte to slave device
    I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(DS3231_I2C_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(DS3231_I2C_BASE, RTC_ADDR, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(DS3231_I2C_BASE));

    //return data pulled from the specified register
    rtc_data[index++] = I2CMasterDataGet(DS3231_I2C_BASE);

    if(len >= 2)
    {
        for(;index < (len - 1);)
        {
            I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

            //wait for MCU to finish transaction
            while(I2CMasterBusy(DS3231_I2C_BASE));

            //return data pulled from the specified register
            rtc_data[index++] = I2CMasterDataGet(DS3231_I2C_BASE);
        }
        I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

        //wait for MCU to finish transaction
        while(I2CMasterBusy(DS3231_I2C_BASE));

        //return data pulled from the specified register
        rtc_data[index++] = I2CMasterDataGet(DS3231_I2C_BASE);
    }
}
void Write_RTC(uint8_t *rtc_data, uint8_t len)
{
    uint8_t index = 0;
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(DS3231_I2C_BASE, RTC_ADDR, false);

    //specify register to be read
    I2CMasterDataPut(DS3231_I2C_BASE, 00);

    //send control byte and register address byte to slave device
    I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(DS3231_I2C_BASE));

    if(len >= 2)
    {
        for(;index < (len - 1);)
        {
            I2CMasterDataPut(DS3231_I2C_BASE, rtc_data[index++]);
            I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

            //wait for MCU to finish transaction
            while(I2CMasterBusy(DS3231_I2C_BASE));
        }
        I2CMasterDataPut(DS3231_I2C_BASE, rtc_data[index++]);
        I2CMasterControl(DS3231_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

        //wait for MCU to finish transaction
        while(I2CMasterBusy(DS3231_I2C_BASE));
    }
}




/*
 * HW_pins.h
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_HW_PINS_H_
#define SOURCES_HW_PINS_H_

#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "driverlib\gpio.h"
#include "driverlib\sysctl.h"
#include "driverlib\timer.h"
#include "driverlib\i2c.h"
#include "driverlib\pin_map.h"

#include "_config.h"

#define GPIO_LOW    0
// #define GPIO_HIGH    1

#if HW_BOARD == TIOT_V2_00_BOARD
//*********************** LEDs ************************//
#define LED_PWM_PORT_BASE           GPIO_PORTH_BASE
#define RED_PWM_PIN                 GPIO_PIN_1
#define GREEN_PWM_PIN               GPIO_PIN_2
#define BLUE_PWM_PIN                GPIO_PIN_3

#define LED_PORT_BASE               GPIO_PORTF_BASE
#define LED1_PIN                    GPIO_PIN_4
#define LED2_PIN                    GPIO_PIN_5

//*********************** UARTs ************************//
#define LTE_UART_PERIPH             SYSCTL_PERIPH_UART3
#define LTE_UART_BASE               UART3_BASE
// #define LTE_BAUD_RATE               115200
#define LTE_BAUD_RATE               38400
#define LTE_UART_CONFIG             (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)
#define LTE_UART_INT                INT_UART3
#define LTE_UART_INTEN_FLAG         (UART_INT_RX | UART_INT_RT)
#define LTE_UART_PORT_BASE          GPIO_PORTC_BASE
#define LTE_RX_PIN                  GPIO_PIN_6
#define LTE_TX_PIN                  GPIO_PIN_7
#define LTE_RX_GPIO_CFG             GPIO_PC6_U3RX
#define LTE_TX_GPIO_CFG             GPIO_PC7_U3TX

// #define LTE_PWRKEY_PORT_BASE        GPIO_PORTB_BASE      //EVSE pins
// #define LTE_PWRKEY_PIN              GPIO_PIN_3           //EVSE pins

#define LTE_PWRKEY_PORT_BASE        GPIO_PORTC_BASE
#define LTE_PWRKEY_PIN              GPIO_PIN_5


#define DEBUG_UART_PERIPH           SYSCTL_PERIPH_UART0
#define DEBUG_UART_BASE             UART0_BASE
#define DEBUG_BAUD_RATE             38400
#define DEBUG_UART_CONFIG           (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)
#define DEBUG_UART_INT              INT_UART0
#define DEBUG_UART_INTEN_FLAG       (UART_INT_RX | UART_INT_RT)
#define DEBUG_UART_PORT_BASE        GPIO_PORTA_BASE
#define DEBUG_RX_PIN                GPIO_PIN_0
#define DEBUG_TX_PIN                GPIO_PIN_1
#define DEBUG_RX_GPIO_CFG           GPIO_PA0_U0RX
#define DEBUG_TX_GPIO_CFG           GPIO_PA1_U0TX

//*********** FLASH  *************//
#define FLASH_SPI_PERIPH             SYSCTL_PERIPH_SSI0
#define FLASH_SPI_BASE               SSI0_BASE
#define FLASH_SPI_FRAME_FORMAT       SSI_FRF_MOTO_MODE_0
#define FLASH_SPI_MODE               SSI_MODE_MASTER
#define FLASH_SPI_CLOCK              4000000//800000             //Hz
#define FLASH_SPI_DATA_WIDTH         8                   //Bits

#define FLASH_SPI_PORT_BASE          GPIO_PORTA_BASE
#define FLASH_CLK_PIN                GPIO_PIN_2
//#define FLASH_CS_PIN                 GPIO_PIN_3
#define FLASH_MISO_PIN               GPIO_PIN_4
#define FLASH_MOSI_PIN               GPIO_PIN_5
#define FLASH_SCK_GPIO_CFG           GPIO_PA2_SSI0CLK
//#define FLASH_CS_GPIO_CFG            GPIO_PA3_SSI0FSS
#define FLASH_MISO_GPIO_CFG          GPIO_PA4_SSI0RX
#define FLASH_MOSI_GPIO_CFG          GPIO_PA5_SSI0TX


#define FLASH_PORT_BASE              GPIO_PORTG_BASE
#define FLASH_CS_PIN                 GPIO_PIN_4
#define FLASH_WP_PIN                 GPIO_PIN_5
#define FLASH_RST_PIN                GPIO_PIN_6

/******************RELAYS PINS**********************************/
#define RELAY_RTR_PORT               GPIO_PORTG_BASE
#define RELAY_ODU_PORT               GPIO_PORTE_BASE

#define RELAY_ODU                    GPIO_PIN_7
#define RELAY_RTR                    GPIO_PIN_0
#define RELAY_RTR_SEL                GPIO_PIN_3



#define BATT_CTRL_PORT               GPIO_PORTD_BASE
#define BATT_CTRL_PIN                GPIO_PIN_3
//************ DS3231 (RTC IC) **************//
#define DS3231_I2C_PERIPH         SYSCTL_PERIPH_I2C1
#define DS3231_I2C_BASE           I2C1_BASE

#define DS3231_I2C_PORT_BASE      GPIO_PORTA_BASE
#define DS3231_SCL_PIN            GPIO_PIN_6
#define DS3231_SDA_PIN            GPIO_PIN_7

#define DS3231_SCL_GPIO_CFG       GPIO_PA6_I2C1SCL
#define DS3231_SDA_GPIO_CFG       GPIO_PA7_I2C1SDA

//************ ENC28J60 (ETHERNET IC) **************//

#define ETHERNET_SPI_FRAME_FORMAT   SSI_FRF_MOTO_MODE_0
#define ETHERNET_SPI_MODE           SSI_MODE_MASTER
#define ETHERNET_SPI_CLOCK          4000000             //Hz
#define ETHERNET_SPI_DATA_WIDTH     8                   //Bits

#define ETHERNET_SPI_PERIPH         SYSCTL_PERIPH_SSI1
#define ETHERNET_SPI_BASE           SSI1_BASE
#define ETHERNET_PHYINT_PORT_BASE   GPIO_PORTG_BASE
#define ETHERNET_PHYINT_PIN         GPIO_PIN_2
#define ETHERNET_RST_PORT_BASE      GPIO_PORTG_BASE
#define ETHERNET_RST_PIN            GPIO_PIN_1
#define ETHERNET_SPI_PORT_BASE      GPIO_PORTF_BASE
#define ETHERNET_CLK_PIN            GPIO_PIN_2
#define ETHERNET_CS_PIN             GPIO_PIN_3
#define ETHERNET_MISO_PIN           GPIO_PIN_0
#define ETHERNET_MOSI_PIN           GPIO_PIN_1
#define ETHERNET_SCK_GPIO_CFG       GPIO_PF2_SSI1CLK
#define ETHERNET_CS_GPIO_CFG        GPIO_PF3_SSI1FSS
#define ETHERNET_MISO_GPIO_CFG      GPIO_PF0_SSI1RX
#define ETHERNET_MOSI_GPIO_CFG      GPIO_PF1_SSI1TX

/*******************10 POS BCD Selector switch*****************/

#define BCD_SELECTOR_SW_BASE        GPIO_PORTB_BASE
#define BCD_SELECTOR_S1             GPIO_PIN_2
#define BCD_SELECTOR_S2             GPIO_PIN_3
#define BCD_SELECTOR_S3             GPIO_PIN_4
#define BCD_SELECTOR_S4             GPIO_PIN_5

#ifdef ENABLE_GLCD
// Define the control and data ports and pins
#define KS0108_DATA_PORT GPIO_PORTH_BASE
#define KS0108_CTRL_PORT GPIO_PORTK_BASE
#define KS0108_RS GPIO_PIN_1    // Register select pin
#define KS0108_RW 0             // No RW pin, using write-only interface
#define KS0108_EN GPIO_PIN_0    // Enable pin
#define KS0108_CS1 GPIO_PIN_2   // Chip Select 1
#define KS0108_CS2 GPIO_PIN_3   // Chip Select 2
#endif  //ENABLE_GLCD

/**************************GPIO************************/
#define FREQ_MEAS_PIN_BASE            GPIO_PORTJ_BASE
#define FREQ_MEAS_PIN                 GPIO_PIN_0

#define EARTH_DETECT_PORT_BASE        GPIO_PORTJ_BASE
#define EARTH_DETECT_PIN              GPIO_PIN_1
/*******************ODU Supply Enable PINS *****************/

#define ODU_SUPPLY_RELAY_BASE         GPIO_PORTE_BASE
#define ODU_SUPPLY_RELAY_PIN          GPIO_PIN_7

#define BUCK_BOOSTER_EN1_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN1_PIN          GPIO_PIN_4

#define BUCK_BOOSTER_EN2_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN2_PIN          GPIO_PIN_5

#define BUCK_BOOSTER_EN3_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN3_PIN          GPIO_PIN_6

#define BUCK_BOOSTER_EN4_BASE         GPIO_PORTB_BASE
#define BUCK_BOOSTER_EN4_PIN          GPIO_PIN_0

/**************************GPIO************************/
#define FREQ_MEAS_PIN_BASE            GPIO_PORTJ_BASE
#define FREQ_MEAS_PIN                 GPIO_PIN_0

#define EARTH_DETECT_PORT_BASE        GPIO_PORTJ_BASE
#define EARTH_DETECT_PIN              GPIO_PIN_1

/*************************INVERTER CTRL*************************/
#define INVERTER_CTRL_PORT_BASE         GPIO_PORTD_BASE
#define INVERTER_CTRL_PIN               GPIO_PIN_0

#ifdef LEDS_ON_GLCD_PINS
#define LED_ODUMODE_BASE                GPIO_PORTH_BASE

#define LED_0VM_BASE                    GPIO_PORTH_BASE
#define LED_0VM_PIN                     GPIO_PIN_0

#define LED_12VM_BASE                   GPIO_PORTH_BASE
#define LED_12VM_PIN                    GPIO_PIN_1

#define LED_24VM_BASE                   GPIO_PORTH_BASE
#define LED_24VM_PIN                    GPIO_PIN_2

#define LED_30VM_BASE                   GPIO_PORTH_BASE
#define LED_30VM_PIN                    GPIO_PIN_3

#define LED_48VM_BASE                   GPIO_PORTH_BASE
#define LED_48VM_PIN                    GPIO_PIN_4

#define LED_56VM_BASE                   GPIO_PORTH_BASE
#define LED_56VM_PIN                    GPIO_PIN_5

#define LED_ETH_DET_BASE                GPIO_PORTH_BASE
#define LED_ETH_DET_PIN                 GPIO_PIN_6

#define LED_LOW_BATT_BASE               GPIO_PORTH_BASE
#define LED_LOW_BATT_PIN                GPIO_PIN_7

#if ODUVTG_SEL_SW == PUSH_BUTTON_TYPE

#define SW_ODUVTG_SEL_A_BASE              GPIO_PORTK_BASE
#define SW_ODUVTG_SEL_A_PIN               GPIO_PIN_0     

#endif  //ODUVTG_SEL_SW == PUSH_BUTTON_TYPE
#endif  //LEDS_ON_GLCD_PINS

#elif   HW_BOARD == TIOT_V3_00_BOARD

//*********************** UARTs ************************//
#define LTE_UART_PERIPH             SYSCTL_PERIPH_UART3
#define LTE_UART_BASE               UART3_BASE
// #define LTE_BAUD_RATE               115200
#define LTE_BAUD_RATE               38400
#define LTE_UART_CONFIG             (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)
#define LTE_UART_INT                INT_UART3
#define LTE_UART_INTEN_FLAG         (UART_INT_RX | UART_INT_RT)
#define LTE_UART_PORT_BASE          GPIO_PORTC_BASE
#define LTE_RX_PIN                  GPIO_PIN_6
#define LTE_TX_PIN                  GPIO_PIN_7
#define LTE_RX_GPIO_CFG             GPIO_PC6_U3RX
#define LTE_TX_GPIO_CFG             GPIO_PC7_U3TX

// #define LTE_PWRKEY_PORT_BASE        GPIO_PORTB_BASE      //EVSE pins
// #define LTE_PWRKEY_PIN              GPIO_PIN_3           //EVSE pins

#define LTE_PWRKEY_PORT_BASE        GPIO_PORTC_BASE
#define LTE_PWRKEY_PIN              GPIO_PIN_5


#define DEBUG_UART_PERIPH           SYSCTL_PERIPH_UART0
#define DEBUG_UART_BASE             UART0_BASE
#define DEBUG_BAUD_RATE             38400
#define DEBUG_UART_CONFIG           (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)
#define DEBUG_UART_INT              INT_UART0
#define DEBUG_UART_INTEN_FLAG       (UART_INT_RX | UART_INT_RT)
#define DEBUG_UART_PORT_BASE        GPIO_PORTA_BASE
#define DEBUG_RX_PIN                GPIO_PIN_0
#define DEBUG_TX_PIN                GPIO_PIN_1
#define DEBUG_RX_GPIO_CFG           GPIO_PA0_U0RX
#define DEBUG_TX_GPIO_CFG           GPIO_PA1_U0TX

//*********** FLASH  *************//
#define FLASH_SPI_PERIPH             SYSCTL_PERIPH_SSI0
#define FLASH_SPI_BASE               SSI0_BASE
#define FLASH_SPI_FRAME_FORMAT       SSI_FRF_MOTO_MODE_0
#define FLASH_SPI_MODE               SSI_MODE_MASTER
#define FLASH_SPI_CLOCK              4000000//800000             //Hz
#define FLASH_SPI_DATA_WIDTH         8                   //Bits

#define FLASH_SPI_PORT_BASE          GPIO_PORTA_BASE
#define FLASH_CLK_PIN                GPIO_PIN_2
//#define FLASH_CS_PIN                 GPIO_PIN_3
#define FLASH_MISO_PIN               GPIO_PIN_4
#define FLASH_MOSI_PIN               GPIO_PIN_5
#define FLASH_SCK_GPIO_CFG           GPIO_PA2_SSI0CLK
//#define FLASH_CS_GPIO_CFG            GPIO_PA3_SSI0FSS
#define FLASH_MISO_GPIO_CFG          GPIO_PA4_SSI0RX
#define FLASH_MOSI_GPIO_CFG          GPIO_PA5_SSI0TX


#define FLASH_PORT_BASE              GPIO_PORTG_BASE
#define FLASH_CS_PIN                 GPIO_PIN_4
#define FLASH_WP_PIN                 GPIO_PIN_5
#define FLASH_RST_PIN                GPIO_PIN_6

//************ DS3231 (RTC IC) **************//
#define DS3231_I2C_PERIPH         SYSCTL_PERIPH_I2C1
#define DS3231_I2C_BASE           I2C1_BASE

#define DS3231_I2C_PORT_BASE      GPIO_PORTA_BASE
#define DS3231_SCL_PIN            GPIO_PIN_6
#define DS3231_SDA_PIN            GPIO_PIN_7

#define DS3231_SCL_GPIO_CFG       GPIO_PA6_I2C1SCL
#define DS3231_SDA_GPIO_CFG       GPIO_PA7_I2C1SDA

//********************** GLCD **********************//
#ifdef ENABLE_GLCD
// Define the control and data ports and pins
#define KS0108_DATA_PORT GPIO_PORTH_BASE
#define KS0108_CTRL_PORT GPIO_PORTK_BASE
#define KS0108_RS GPIO_PIN_1    // Register select pin
#define KS0108_RW 0             // No RW pin, using write-only interface
#define KS0108_EN GPIO_PIN_0    // Enable pin
#define KS0108_CS1 GPIO_PIN_2   // Chip Select 1
#define KS0108_CS2 GPIO_PIN_3   // Chip Select 2
#endif  //ENABLE_GLCD

/**************************GPIO************************/
#define FREQ_MEAS_PIN_BASE            GPIO_PORTJ_BASE
#define FREQ_MEAS_PIN                 GPIO_PIN_0

#define EARTH_DETECT_PORT_BASE        GPIO_PORTJ_BASE
#define EARTH_DETECT_PIN              GPIO_PIN_1

/**************************LEDs************************/

#ifdef LEDS_ON_GLCD_PINS
#define LED_ODUMODE_BASE                GPIO_PORTH_BASE

#define LED_0VM_BASE                    GPIO_PORTH_BASE
#define LED_0VM_PIN                     GPIO_PIN_0

#define LED_12VM_BASE                   GPIO_PORTH_BASE
#define LED_12VM_PIN                    GPIO_PIN_1

#define LED_24VM_BASE                   GPIO_PORTH_BASE
#define LED_24VM_PIN                    GPIO_PIN_2

#define LED_30VM_BASE                   GPIO_PORTH_BASE
#define LED_30VM_PIN                    GPIO_PIN_3

#define LED_48VM_BASE                   GPIO_PORTH_BASE
#define LED_48VM_PIN                    GPIO_PIN_4

#define LED_56VM_BASE                   GPIO_PORTH_BASE
#define LED_56VM_PIN                    GPIO_PIN_5

#define LED_ETH_DET_BASE                GPIO_PORTH_BASE
#define LED_ETH_DET_PIN                 GPIO_PIN_6

#define LED_LOW_BATT_BASE               GPIO_PORTH_BASE
#define LED_LOW_BATT_PIN                GPIO_PIN_7

#define LED_NW_STS_BASE                 GPIO_PORTK_BASE
#define LED_NW_STS_PIN                  GPIO_PIN_0

#define LED_BATT_MODE_BASE              GPIO_PORTK_BASE
#define LED_BATT_MODE_PIN               GPIO_PIN_1

#define LED_SYS_STSA_BASE               GPIO_PORTK_BASE
#define LED_SYS_STSA_PIN                GPIO_PIN_2

#define LED_SYS_STSB_BASE               GPIO_PORTK_BASE
#define LED_SYS_STSB_PIN                GPIO_PIN_3

/**************************SWITCHES************************/
#if ODUVTG_SEL_SW == PUSH_BUTTON_TYPE

#define SW_ODUVTG_SEL_A_BASE              GPIO_PORTF_BASE
#define SW_ODUVTG_SEL_A_PIN               GPIO_PIN_4    

#define SW_ODUVTG_SEL_B_BASE              GPIO_PORTF_BASE
#define SW_ODUVTG_SEL_B_PIN               GPIO_PIN_5  

#define SW_ODUVTG_SEL_C_BASE              GPIO_PORTF_BASE
#define SW_ODUVTG_SEL_C_PIN               GPIO_PIN_6  

#define SW_ODUVTG_SEL_D_BASE              GPIO_PORTF_BASE
#define SW_ODUVTG_SEL_D_PIN               GPIO_PIN_7  

#endif  //ODUVTG_SEL_SW == PUSH_BUTTON_TYPE
#endif  //LEDS_ON_GLCD_PINS

/*******************ODU Supply Enable PINS *****************/

#define ODU_SUPPLY_RELAY_BASE         GPIO_PORTE_BASE
#define ODU_SUPPLY_RELAY_PIN          GPIO_PIN_1

#define BUCK_BOOSTER_EN1_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN1_PIN          GPIO_PIN_3

#define BUCK_BOOSTER_EN2_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN2_PIN          GPIO_PIN_4

#define BUCK_BOOSTER_EN3_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN3_PIN          GPIO_PIN_5

#define BUCK_BOOSTER_EN4_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN4_PIN          GPIO_PIN_6

#define BUCK_BOOSTER_EN5_BASE         GPIO_PORTE_BASE
#define BUCK_BOOSTER_EN5_PIN          GPIO_PIN_7

/*******************BATT CTL PIN *****************/

#define BATT_CTRL_PORT               GPIO_PORTD_BASE
#define BATT_CTRL_PIN                GPIO_PIN_3

#endif  //HW_BOARD

#endif /* SOURCES_HW_PINS_H_ */

/*
 * UartCore.h
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_UARTCORE_H_
#define SOURCES_UARTCORE_H_

#include "HW_pins.h"

#define UART0   (0)
#define UART1   (1)
#define UART2   (2)
#define UART3   (3)
#define UART4   (4)
#define UART5   (5)
#define UART6   (6)
#define UART7   (7)

#define TX_BUFFER_MAX   (64)
// #define RX_BUFFER_MAX    (64)
#define RX_BUFFER_MAX   (150)

#define DISPLAY_TX_MAX_LEN          (300)


#define NO_OF_UART_USED (4)
#define ESP_UART_FRAMETIMEOUT       (30)
#define DISPLAY_UART_FRAMETIMEOUT   (30)
#define RS485_UART_FRAMETIMEOUT     (30)
#define LTE_UART_FRAMETIMEOUT       (30)
#define ESP_UART        (UART2)
#define DISPLAY_UART    (UART7)
#define RS485_UART      (UART1)
#define LTE_UART        (UART3)
#define UART_PC         (DEBUG_UART_BASE)

#define LEN_INDX    1
#define CMD_INDX    2
#define DATA_INDX   3

#define ACK     (0x03)      //R
#define NACK    (0x04)      //R
#define ERROR_INVALID_CMD (0x05)

#define ERROR_INVALID_STATE (0xA1)

typedef enum
{
    DISPLAY_TX_START,
    DISPLAY_TX_READY,
}display_tx_state_t;

typedef enum
{
    START,
    PROCESS,
    READY,
    READ_LEN,
    DATA_READ,
    EOP,
    LRC_READ,
}ISR_rx_state_t;

typedef struct
{
    unsigned char rx_buffer[RX_BUFFER_MAX];
    int rx_indx;
    //rx_msg_state_t rx_state;
    uint8_t elapsed;
    ISR_rx_state_t rx_state;
}Rx_Buff_t;

typedef struct
{
    char tx_buffer[TX_BUFFER_MAX];
    int tx_indx;
    char tx_ready;
    char pending_command;
    unsigned long curr_pck_num;
    char wait_for_ack;
}Tx_Buff_t;

typedef struct
{
    uint8_t tx_buff[DISPLAY_TX_MAX_LEN];
    uint16_t tx_index;
    display_tx_state_t state;
    uint8_t pending_cmd;
}display_tx_t;

#ifdef __cplusplus
extern "C" {
#endif

void DEBUGUARTIntHandler(void);

void LTEUARTIntHandler(void);

void DISPLAYUARTIntHandler(void);

#ifdef __cplusplus
}
#endif

void vDEBUGUARTInit(void);

void vLTEUARTInit(void);

void vPERIPH_UARTInit(void);

static void U4_RxIntHandler(void);

static void U4_TxIntHandler(void);

static void U3_RxIntHandler(void);

static void U3_TxIntHandler(void);

static void U0_RxIntHandler(void);

static void U0_TxIntHandler(void);

void vUART_SendChr(uint32_t ui32Base, uint8_t u8Val);

void vUART_SendStr(uint32_t ui32Base, const uint8_t *pui8Buffer);

#ifdef UART_TX_IRQ_EN
void vUART_SendStr_INT(uint32_t ui32Base, const uint8_t *pui8Buffer);
#endif  //UART_TX_IRQ_EN

void vUART_SendBytes(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count);

void vUART_SendInt(uint32_t ui32Base, int32_t i32Val);

void Usendbuffer(uint32_t ui32Base);

void vUART_CheckFrameTimeout(int uart_no);

void flushRxBuffer(int uart_no);

void flushTxBuffer(int uart_no);

void prepare_graphical_tx_buff(unsigned int cmd, char *data, unsigned char len);

#endif /* SOURCES_UARTCORE_H_ */


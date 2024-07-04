/*
 * UartCore.cpp
 *
 *  Created on: Apr 18, 2024
 *      Author: ADMIN
 */

#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include <Sources/ADC_Core.h>
#include "stdlib.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "inc/hw_uart.h"
#include "inc/hw_types.h"

#include "Sources/HW_pins.h"
#include "UartCore.h"

#include "inc/TM4C1233E6PZ.h"
#include "_config.h"
#include "gprs.h"
#include "_common.h"
#include "PC_Cmds.h"
#include "Telecom_server_query.h"
#include "_debug.h"

Rx_Buff_t pc_uart_rx, display_uart_rx;
Tx_Buff_t pc_uart_tx;//, display_uart_tx;

display_tx_t display_tx;
volatile uint8_t g_bIntFlag = false;

extern volatile gprs_rx_data_buff_t gprs_rx_buff;
extern volatile gprs_tx_data_buff_t gprs_tx_buff;
extern volatile gprs_rx_isr_handler_t gprs_rx_isr_handler;

extern gprs_t gprs;

void vDEBUGUARTInit(void)
{
    SysCtlPeripheralEnable(DEBUG_UART_PERIPH);
    GPIOPinConfigure(DEBUG_RX_GPIO_CFG);
    GPIOPinConfigure(DEBUG_TX_GPIO_CFG);
    GPIOPinTypeUART(DEBUG_UART_PORT_BASE, (DEBUG_RX_PIN | DEBUG_TX_PIN));
    UARTConfigSetExpClk(DEBUG_UART_BASE, SYSTEM_CLOCK, DEBUG_BAUD_RATE, DEBUG_UART_CONFIG);
    IntEnable(DEBUG_UART_INT);
    UARTIntEnable(DEBUG_UART_BASE, DEBUG_UART_INTEN_FLAG);
    // UARTIntEnable(DEBUG_UART_BASE, UART_INT_RX);
    // UARTIntEnable(DEBUG_UART_BASE, UART_INT_RT);

    flushRxBuffer(UART_PC);  //PP on 19-04-24: donot forget to uncomment this later when u've made a function for this.
    flushTxBuffer(UART_PC);
}

void vLTEUARTInit(void)
{
    SysCtlPeripheralEnable(LTE_UART_PERIPH);
    GPIOPinConfigure(LTE_RX_GPIO_CFG);
    GPIOPinConfigure(LTE_TX_GPIO_CFG);
    GPIOPinTypeUART(LTE_UART_PORT_BASE, (LTE_RX_PIN | LTE_TX_PIN));
    UARTConfigSetExpClk(LTE_UART_BASE, SYSTEM_CLOCK, LTE_BAUD_RATE, LTE_UART_CONFIG);
    IntEnable(LTE_UART_INT);
    UARTIntEnable(LTE_UART_BASE, LTE_UART_INTEN_FLAG);

    GPIOPinTypeGPIOOutput(LTE_PWRKEY_PORT_BASE, LTE_PWRKEY_PIN);
    GPIOPinWrite(LTE_PWRKEY_PORT_BASE, LTE_PWRKEY_PIN, GPIO_LOW);

    flushRxBuffer(LTE_UART);  //PP on 19-04-24: donot forget to uncomment this later when u've made a function for this.
    flushTxBuffer(LTE_UART);
}

#if 0
void vDISPLAYUARTInit(void)
{
    SysCtlPeripheralEnable(DISPLAY_UART_PERIPH);
    GPIOPinConfigure(DISPLAY_TX_GPIO_CFG);
    GPIOPinConfigure(DISPLAY_RX_GPIO_CFG);
    GPIOPinTypeUART(DISPLAY_UART_PORT_BASE, (DISPLAY_RX_PIN | DISPLAY_TX_PIN));
    UARTConfigSetExpClk(DISPLAY_UART_BASE, SYSTEM_CLOCK, DISPLAY_BAUD_RATE, DISPLAY_UART_CONFIG);
    IntEnable(DISPLAY_UART_INT);
    // UARTTxIntModeSet(DISPLAY_UART_BASE, UART_TXINT_MODE_EOT);
    UARTIntEnable(DISPLAY_UART_BASE, DISPLAY_UART_INTEN_FLAG);
    // UARTIntEnable(DISPLAY_UART_BASE, UART_INT_RT);  //PP on 20-4-24: this one works too. Wonder if DISPLAY_UART_INTEN_FLAG aka (UART_INT_RX | UART_INT_RT) is really necessary?

    flushTxBuffer(DISPLAY_UART);
    flushRxBuffer(DISPLAY_UART);  //PP on 19-04-24: donot forget to uncomment this later when u've made a function for this.
}
#endif  //if 0

void vPERIPH_UARTInit(void)
{
    vDEBUGUARTInit();
    vLTEUARTInit();
#if 0
    vDISPLAYUARTInit();
#endif//if 0
}

//*********************************************************************************//
// Function Name        :- void DEBUGUARTIntHandler(void)
// Input Parameters     :- void
// Output Parameters    :- void
// Description          :- Interrupt handler for DEBUG UART interrupt.
//
// Note - This function must be added in vector table.
//*********************************************************************************//

// extern "C" void DEBUGUARTIntHandler(void)
// {
//     uint32_t ui32Status, u32Data;
//
//     // Get the interrrupt status. //
//     ui32Status = UARTIntStatus(DEBUG_UART_BASE, true);
//
//     // Clear the asserted interrupts. //
//     UARTIntClear(DEBUG_UART_BASE, ui32Status);
//
//     // Loop while there are characters in the receive FIFO. //
//     while(UARTCharsAvail(DEBUG_UART_BASE))
//     {
//         // Read the next character from the UART and write it back to the UART. //
//         //UARTCharPutNonBlocking(DEBUG_UART_BASE, UARTCharGetNonBlocking(DEBUG_UART_BASE));
//         u32Data = UARTCharGetNonBlocking(DEBUG_UART_BASE);
//
//         UARTCharPutNonBlocking(DEBUG_UART_BASE, u32Data);
//     }
// }
//extern "C" void TimerIntHandler(void)
//{
//    //
//    // Clear the timer interrupt flag.
//    //
//    TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//
//    //
//    // Set a flag to indicate that the interrupt occurred.
//    //
//    //g_bIntFlag = true;
////    readACVoltage();
//}

extern "C" void DEBUGUARTIntHandler(void)
{
    // IntMasterDisable(); //PP added on 11-06-24
    uint32_t ui32Status;//, u32Data;

    // Get the interrrupt status. //
    ui32Status = UARTIntStatus(DEBUG_UART_BASE, true);

#ifdef UART_TX_IRQ_EN
    // Clear the asserted interrupts. //
    // UARTIntClear(DEBUG_UART_BASE, ui32Status);

    // if(ui32Status & UART_INT_RX)
    // if(ui32Status & UART_INT_RT)
    // if(ui32Status & DEBUG_UART_INTEN_FLAG)
    if((ui32Status & UART_INT_RX) || (ui32Status & UART_INT_RT))
    {
        // UARTIntClear(DEBUG_UART_BASE, UART_INT_RX);
        UARTIntClear(DEBUG_UART_BASE, DEBUG_UART_INTEN_FLAG);
        // UARTIntClear(DEBUG_UART_BASE, ui32Status);
        // U4_RxIntHandler();
        U0_RxIntHandler();
    }

    if(ui32Status & UART_INT_TX)
    {
        // PP on 20-04-24: This (UARTIntClear for tx intr) should only be done when the whole buffer is sentout.
        // otherwise it breaks the packets. Especially if we're using non irq transmits along with irq transmits in the code.
        // UARTIntClear(DEBUG_UART_BASE, UART_INT_TX);
        // U4_TxIntHandler();
        U0_TxIntHandler();
    }
#else

    static int msglen=0,datacnt=0;

    char tmpudr;

    // Clear the asserted interrupts. //
    UARTIntClear(DEBUG_UART_BASE, ui32Status);

    // Loop while there are characters in the receive FIFO. //
    while(UARTCharsAvail(DEBUG_UART_BASE))
    {
        // Read the next character from the UART and write it back to the UART. //
        //UARTCharPutNonBlocking(DEBUG_UART_BASE, UARTCharGetNonBlocking(DEBUG_UART_BASE));
        tmpudr = UARTCharGetNonBlocking(DEBUG_UART_BASE);

        UARTCharPutNonBlocking(DEBUG_UART_BASE, tmpudr);

        pc_uart_rx.elapsed = 0;

        char rsp=0;
        switch(pc_uart_rx.rx_state)
        {
            case START :
            {
                if(tmpudr == 0xAA)        //if(udata =='#')
                {
                    flushRxBuffer(UART_PC);
                    pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;
                    pc_uart_rx.rx_state = READ_LEN;
                    datacnt = 0;
                }
            }
            break;

            case READ_LEN :
            {
                msglen = tmpudr;
                pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;
                pc_uart_rx.rx_state = DATA_READ;
            }
            break;

            case DATA_READ :
            {
                pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;
                datacnt++;
                if (datacnt>=RX_BUFFER_MAX)
                {
                    flushRxBuffer(UART_PC);
                    pc_uart_rx.rx_state = START;
                }
                if(datacnt>msglen)      //R changed >= -> >
                {
                    pc_uart_rx.rx_state = EOP;
                }
            }
            break;

            case EOP :
            {
                if(tmpudr==0x55)     //if(udata=='$')        //Anand
                {
                    pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;      //R

                    if(getChecksum((unsigned char*)&pc_uart_rx.rx_buffer[LEN_INDX + 1], msglen)==pc_uart_rx.rx_buffer[msglen + LEN_INDX + 1])       //R
                    {
                        rsp = ACK;
                        pc_uart_rx.rx_state = READY;
                    }
                    else
                    {
                        rsp = NACK;
                        pc_uart_rx.rx_state = START;
                    }
                }
                else
                {
                    pc_uart_rx.rx_state = START;
                    rsp=NACK;
                }
                if(rsp) //will be true for ACK/NACK
                {
                    //uart_packet_resp(rsp,UART0);
                    if((!pc_uart_tx.wait_for_ack) || (rsp==NACK))
                        preparePC_ResponsePacket((cmd_t)pc_uart_rx.rx_buffer[CMD_INDX], &rsp, 2, FALSE, FALSE);       //Anand 21.05.2014

                    if(rsp == NACK)
                    {
                        flushRxBuffer(UART_PC);     //Anand 22.04.2014
                    }
                    //Let the main program know
                    pc_uart_tx.tx_ready = TRUE;
                }
            }
            break;

            default:
            {}
            break;
        }
    }
#endif
    // IntMasterEnable(); //PP added on 11-06-24
}

#ifdef UART_TX_IRQ_EN
// static inline void U4_RxIntHandler(void)
static inline void U0_RxIntHandler(void)
{
    static int msglen=0,datacnt=0;

    char tmpudr;

    tmpudr = UARTCharGetNonBlocking(DEBUG_UART_BASE);

    UARTCharPutNonBlocking(DEBUG_UART_BASE, tmpudr);

    pc_uart_rx.elapsed = 0;

    char rsp=0;
    switch(pc_uart_rx.rx_state)
    {
        case START :
            {
                if(tmpudr == 0xAA)        //if(udata =='#')
                {
                    flushRxBuffer(UART_PC);
                    pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;
                    pc_uart_rx.rx_state = READ_LEN;
                    datacnt = 0;
                }
            }
            break;

            case READ_LEN :
            {
                msglen = tmpudr;
                pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;
                pc_uart_rx.rx_state = DATA_READ;
            }
            break;

            case DATA_READ :
            {
                pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;
                datacnt++;
                if (datacnt>=RX_BUFFER_MAX)
                {
                    flushRxBuffer(UART_PC);
                    pc_uart_rx.rx_state = START;
                }
                if(datacnt>msglen)      //R changed >= -> >
                {
                    pc_uart_rx.rx_state = EOP;
                }
            }
            break;

            case EOP :
            {
                if(tmpudr==0x55)     //if(udata=='$')        //Anand
                {
                    pc_uart_rx.rx_buffer[pc_uart_rx.rx_indx++]= tmpudr;      //R

                    if(getChecksum((unsigned char*)&pc_uart_rx.rx_buffer[LEN_INDX + 1], msglen)==pc_uart_rx.rx_buffer[msglen + LEN_INDX + 1])       //R
                    {
                        rsp = ACK;
                        pc_uart_rx.rx_state = READY;
                    }
                    else
                    {
                        rsp = NACK;
                        pc_uart_rx.rx_state = START;
                    }
                }
                else
                {
                    pc_uart_rx.rx_state = START;
                    rsp=NACK;
                }
                if(rsp) //will be true for ACK/NACK
                {
                    //uart_packet_resp(rsp,UART0);
                    if((!pc_uart_tx.wait_for_ack) || (rsp==NACK))
                        preparePC_ResponsePacket((cmd_t)pc_uart_rx.rx_buffer[CMD_INDX], &rsp, 2, FALSE, FALSE);       //Anand 21.05.2014

                    if(rsp == NACK)
                    {
                        flushRxBuffer(UART_PC);     //Anand 22.04.2014
                    }
                    //Let the main program know
                    pc_uart_tx.tx_ready = TRUE;
                }
            }
            break;

            default:
            {}
            break;
    }

}

// static inline void U4_TxIntHandler(void)
static inline void U0_TxIntHandler(void)
{
    pc_uart_tx.tx_indx++;
    
    if((pc_uart_tx.tx_indx >= TX_BUFFER_MAX)||(pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx] == '\0'))
    {
        UARTIntClear(DEBUG_UART_BASE, UART_INT_TX);
        UARTIntDisable(DEBUG_UART_BASE, UART_INT_TX);
    }
    else
    {
        // UARTIntClear(DEBUG_UART_BASE, UART_INT_TX);
        UARTCharPutNonBlocking(DEBUG_UART_BASE, pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx]);
        // HWREG(DEBUG_UART_BASE + UART_O_DR) = pc_uart_tx.tx_buffer[pc_uart_tx.tx_indx];
    }
}
#endif  //UART_TX_IRQ_EN

//*********************************************************************************//
// Function Name        :- void LTEUARTIntHandler(void)
// Input Parameters     :- void
// Output Parameters    :- void
// Description          :- Interrupt handler for LTE UART interrupt.
//
// Note - This function must be added in vector table.
//*********************************************************************************//
extern "C" void LTEUARTIntHandler(void)
{
    // IntMasterDisable(); //PP added on 11-06-24
    uint32_t ui32Status;//, u32Data;

    // Get the interrrupt status. //
    ui32Status = UARTIntStatus(LTE_UART_BASE, true);

#ifdef UART_TX_IRQ_EN
    // // Clear the asserted interrupts. //
    // UARTIntClear(LTE_UART_BASE, ui32Status);

    if((ui32Status & UART_INT_RX) || (ui32Status & UART_INT_RT))
    {
        // UARTIntClear(LTE_UART_BASE, UART_INT_RX);
        UARTIntClear(LTE_UART_BASE, DEBUG_UART_INTEN_FLAG);
        // UARTIntClear(LTE_UART_BASE, ui32Status);
        U3_RxIntHandler();
    }

    if(ui32Status & UART_INT_TX)
    {
        // PP on 20-04-24: This (UARTIntClear for tx intr) should only be done when the whole buffer is sentout.
        // otherwise it breaks the packets. Especially if we're using non irq transmits along with irq transmits in the code.
        // UARTIntClear(LTE_UART_BASE, UART_INT_TX);
        U3_TxIntHandler();
    }
#else

    char tmpudr;

    // Clear the asserted interrupts. //
    UARTIntClear(LTE_UART_BASE, ui32Status);

    // Loop while there are characters in the receive FIFO. //
    while(UARTCharsAvail(LTE_UART_BASE))
    {
        tmpudr = UARTCharGetNonBlocking(LTE_UART_BASE);
        
#ifdef ECHO_LTE_TO_DBG
        // UARTCharPutNonBlocking(LTE_UART_BASE, tmpudr);
        UARTCharPutNonBlocking(DEBUG_UART_BASE, tmpudr);
#endif  //ECHO_LTE_TO_DBG

        gprs_rx_isr_handler.state = GPRS_RX_INPROG;
        
        gprs_rx_isr_handler.elapsed = 0;
        if ((gprs_rx_buff.index < (GPRS_RX_BUFFER_MAX - 1)) && (tmpudr != 0x00))
        {
            gprs_rx_buff.buffer[gprs_rx_buff.index++] = tmpudr;
        }
    }
#endif  //UART_TX_IRQ_EN
    // IntMasterEnable(); //PP added on 11-06-24
}

#ifdef UART_TX_IRQ_EN
static inline void U3_RxIntHandler(void)
{
    char tmpudr;

    tmpudr = UARTCharGetNonBlocking(LTE_UART_BASE);

    // UARTCharPutNonBlocking(LTE_UART_BASE, tmpudr);
    UARTCharPutNonBlocking(DEBUG_UART_BASE, tmpudr);

    gprs_rx_isr_handler.state = GPRS_RX_INPROG;
	
	gprs_rx_isr_handler.elapsed = 0;
    if ((gprs_rx_buff.index < (GPRS_RX_BUFFER_MAX - 1)) && (tmpudr != 0x00))
    {
        gprs_rx_buff.buffer[gprs_rx_buff.index++] = tmpudr;
    }
}

static inline void U3_TxIntHandler(void)
{
    gprs_tx_buff.index++;

    if ((gprs_tx_buff.index >= GPRS_TX_BUFFER_MAX) || (gprs_tx_buff.buffer[gprs_tx_buff.index] == '\0')) 
    {
        unlock(gprs_tx_buff.locked);
        UARTIntClear(LTE_UART_BASE, UART_INT_TX);
        UARTIntDisable(LTE_UART_BASE, UART_INT_TX);
    }
    else
    {
        // UARTIntClear(LTE_UART_BASE, UART_INT_TX);
        UARTCharPutNonBlocking(LTE_UART_BASE, gprs_tx_buff.buffer[gprs_tx_buff.index]);
        // HWREG(LTE_UART_BASE + UART_O_DR) = gprs_tx_buff.buffer[gprs_tx_buff.index];
    }
}
#endif	//UART_TX_IRQ_EN

#if 0
//*********************************************************************************//
// Function Name        :- void UART7IntHandler(void)
// Input Parameters     :- void
// Output Parameters    :- void
// Description          :- Interrupt handler for UART7 interrupt.
//
// Note - This function must be added in vector table.
//*********************************************************************************//
extern "C" void DISPLAYUARTIntHandler(void)
{
    uint32_t ui32Status;//, u32Data;

    // Get the interrrupt status. //
    ui32Status = UARTIntStatus(DISPLAY_UART_BASE, true);

#ifdef UART_TX_IRQ_EN
    // // Clear the asserted interrupts. //
    // UARTIntClear(DISPLAY_UART_BASE, ui32Status);

    if((ui32Status & UART_INT_RX) || (ui32Status & UART_INT_RT))
    // if(/* (ui32Status & UART_INT_RX) ||  */(ui32Status & UART_INT_RT))
    {
        // UARTIntClear(DISPLAY_UART_BASE, UART_INT_RX);
        // UARTIntClear(DISPLAY_UART_BASE, UART_INT_RT);
        UARTIntClear(DISPLAY_UART_BASE, DISPLAY_UART_INTEN_FLAG);
        // UARTIntClear(DISPLAY_UART_BASE, ui32Status);

        U0_RxIntHandler();
    }

    if(ui32Status & UART_INT_TX)
    {
        // PP on 20-04-24: This (UARTIntClear for tx intr) should only be done when the whole buffer is sentout.
        // otherwise it breaks the packets. Especially if we're using non irq transmits along with irq transmits in the code.
        // UARTIntClear(DISPLAY_UART_BASE, UART_INT_TX);    
        U0_TxIntHandler();
    }
#else
    char tmpudr;

    // Clear the asserted interrupts. //
    UARTIntClear(DISPLAY_UART_BASE, ui32Status);

    // Loop while there are characters in the receive FIFO. //

    while(UARTCharsAvail(DISPLAY_UART_BASE))
    {
        tmpudr = UARTCharGetNonBlocking(DISPLAY_UART_BASE);

        // UARTCharPutNonBlocking(DISPLAY_UART_BASE, tmpudr);
        // UARTCharPutNonBlocking(DEBUG_UART_BASE, tmpudr);

        display_uart_rx.elapsed = 0;

        // // if(display_uart_rx.rx_indx < (RX_BUFFER_MAX - 1))    //PP on 20-04-24: Commenting this as this not there in EVSE DISPLAY RX ISR
        // {
        //     // PP on 25-04-24: commenting this as LCD MCU code is sending ACK inconsistently, 
        //     // and we donot currently have the code to handle it even if we recieved it consistently.
        //     // this is just gonna overflow the rx buff and hang our MCU unless we handle that response.
        //     display_uart_rx.rx_buffer[display_uart_rx.rx_indx++] = tmpudr;   
        // }
    }

#endif  //UART_TX_IRQ_EN
}

#ifdef UART_TX_IRQ_EN
static inline void U0_RxIntHandler(void)
{
    char tmpudr;
    tmpudr = UARTCharGetNonBlocking(DISPLAY_UART_BASE);

    // UARTCharPutNonBlocking(DISPLAY_UART_BASE, tmpudr);
    // UARTCharPutNonBlocking(DEBUG_UART_BASE, tmpudr);

    display_uart_rx.elapsed = 0;

    // // if(display_uart_rx.rx_indx < (RX_BUFFER_MAX - 1))    //PP on 20-04-24: Commenting this as this not there in EVSE DISPLAY RX ISR
    // {
    //     // PP on 25-04-24: commenting this as LCD MCU code is sending ACK inconsistently, 
    //     // and we donot currently have the code to handle it even if we recieved it consistently.
    //     // this is just gonna overflow the rx buff and hang our MCU unless we handle that response.
    //     display_uart_rx.rx_buffer[display_uart_rx.rx_indx++] = tmpudr;   
    // }
}

static inline void U0_TxIntHandler(void)
{
    display_tx.tx_index++;
    
    if((display_tx.tx_index >= TX_BUFFER_MAX)||(display_tx.tx_buff[display_tx.tx_index] == '\0'))
    {
        UARTIntClear(DISPLAY_UART_BASE, UART_INT_TX);
        UARTIntDisable(DISPLAY_UART_BASE, UART_INT_TX);
    }
    else
    {
        // UARTIntClear(DISPLAY_UART_BASE, UART_INT_TX);
        UARTCharPutNonBlocking(DISPLAY_UART_BASE, display_tx.tx_buff[display_tx.tx_index]);
        // HWREG(DISPLAY_UART_BASE + UART_O_DR) = display_tx.tx_buff[display_tx.tx_index];
    }
}
#endif  //UART_TX_IRQ_EN
#endif  //if 0

void vUART_SendChr(uint32_t ui32Base, uint8_t u8Val)
{
    UARTCharPutNonBlocking(ui32Base, u8Val);
    while(UARTBusy(ui32Base)){};    
}

void vUART_SendStr(uint32_t ui32Base, const uint8_t *pui8Buffer)
{
    while(*pui8Buffer != '\0')
    {
        // Write the next character to the UART. //
        UARTCharPutNonBlocking(ui32Base, *pui8Buffer++);
        while(UARTBusy(ui32Base));

    }
}

#ifdef UART_TX_IRQ_EN
void vUART_SendStr_INT(uint32_t ui32Base, const uint8_t *pui8Buffer)
{
    if(ui32Base == DEBUG_UART_BASE)
    {
        if (pui8Buffer[0] == '\0') return;
		else
        {
            //
            // See if there is space in the transmit FIFO.
            //
            if(!(HWREG(ui32Base + UART_O_FR) & UART_FR_TXFF))
            {
                if(strlen((const char*)pui8Buffer) < TX_BUFFER_MAX)
				{
					strcpy((char*)pc_uart_tx.tx_buffer, (const char*)pui8Buffer);
				}
				else
				{
					strcpy((char*)pc_uart_tx.tx_buffer, (const char*)"BUFFER SIZE ERROR");
				}

				pc_uart_tx.tx_indx = 0;

                //
                // Write this character to the transmit FIFO.
                //
                HWREG(ui32Base + UART_O_DR) = pc_uart_tx.tx_buffer[0];

                UARTTxIntModeSet(DEBUG_UART_BASE, UART_TXINT_MODE_EOT);

				UARTIntEnable(DEBUG_UART_BASE, UART_INT_TX);

                // //
                // // Write this character to the transmit FIFO.
                // //
                // HWREG(ui32Base + UART_O_DR) = pc_uart_tx.tx_buffer[0];

                // UARTIntEnable(DEBUG_UART_BASE, UART_INT_TX);
            }
        }
    }

    if(ui32Base == LTE_UART_BASE)
    {
        if (pui8Buffer[0] == '\0') return;
        else
        {
            //
            // See if there is space in the transmit FIFO.
            //
            if(!(HWREG(ui32Base + UART_O_FR) & UART_FR_TXFF))
            {
                if(strlen((const char*)pui8Buffer) < GPRS_TX_BUFFER_MAX)
				{
					strcpy((char*)gprs_tx_buff.buffer, (const char*)pui8Buffer);
				}
				else
				{
					strcpy((char*)gprs_tx_buff.buffer, (const char*)"BUFFER SIZE ERROR");
				}

				gprs_tx_buff.index = 0;

                //
                // Write this character to the transmit FIFO.
                //
                HWREG(ui32Base + UART_O_DR) = gprs_tx_buff.buffer[0];

                UARTTxIntModeSet(LTE_UART_BASE, UART_TXINT_MODE_EOT);

				UARTIntEnable(LTE_UART_BASE, UART_INT_TX);
            }
        }
    }

#if 0
    if(ui32Base == DISPLAY_UART_BASE)
    {
        if (pui8Buffer[0] == '\0') return;
		else
        {
            //
            // See if there is space in the transmit FIFO.
            //
            if(!(HWREG(ui32Base + UART_O_FR) & UART_FR_TXFF))
            {
                if(strlen((const char*)pui8Buffer) < TX_BUFFER_MAX)
				{
					strcpy((char*)display_tx.tx_buff, (const char*)pui8Buffer);
				}
				else
				{
					strcpy((char*)display_tx.tx_buff, (const char*)"BUFFER SIZE ERROR");
				}

				display_tx.tx_index = 0;

                //
                // Write this character to the transmit FIFO.
                //
                HWREG(ui32Base + UART_O_DR) = display_tx.tx_buff[0];

                UARTTxIntModeSet(DISPLAY_UART_BASE, UART_TXINT_MODE_EOT);

				UARTIntEnable(DISPLAY_UART_BASE, UART_INT_TX);
            }
        }
    }
#endif  //if 0
}
#endif  //UART_TX_IRQ_EN

void vUART_SendBytes(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    while(ui32Count--)
    {
        // Write the next character to the UART. //
        UARTCharPutNonBlocking(ui32Base, *pui8Buffer++);
        while(UARTBusy(ui32Base));
        /*{
            cnt_test++;
        }*/
    }
}

void vUART_SendInt(uint32_t ui32Base, int32_t i32Val)
{
    uint8_t buff[20];
    ltoa(i32Val,(char *)buff,10);
    vUART_SendStr(ui32Base,buff);
}

void Usendbuffer(uint32_t ui32Base)
{
    // vUART_SendInt(DISPLAY_UART_BASE, 10);
    int i = 0;
    // if(ui32Base == UART_PC)
    if(ui32Base == DEBUG_UART_BASE)
    {
        for (i=0; i < pc_uart_tx.tx_indx; i++)
        {
            vUART_SendChr(ui32Base,pc_uart_tx.tx_buffer[i]);
        }
    }
    // else if(ui32Base == LTE_UART)
    else if(ui32Base == LTE_UART_BASE)
    {
        for (i=0; i < gprs_tx_buff.index; i++)
        {
            vUART_SendChr(ui32Base,gprs_tx_buff.buffer[i]);
        }
    }
#if 0
    // else if(ui32Base == DISPLAY_UART)
    else if(ui32Base == DISPLAY_UART_BASE)
    {
        // vUART_SendInt(DISPLAY_UART_BASE, 11);
        for(i=0; i < display_tx.tx_index; i++)
        {
            // vUART_SendInt(DISPLAY_UART_BASE, 12);
            vUART_SendChr(ui32Base,display_tx.tx_buff[i]);
        }

    }
#endif //if 0
}

void vUART_CheckFrameTimeout(int uart_no)
{
    if(uart_no == LTE_UART)
    {
        if(gprs_rx_isr_handler.elapsed >= LTE_UART_FRAMETIMEOUT)
        {
            gprs_rx_isr_handler.elapsed = 0;


            if(count_doubleSquareBracket((char *)gprs_rx_buff.buffer,gprs_rx_buff.index) == 2)
            {
//#ifdef DEBUG_RANDOM_
//            vUART_SendStr(UART_PC,"\nfcount2");
//#endif
                if((gprs_rx_buff.index > 0) && (gprs_rx_buff.locked != LOCKED))
                {
                    gprs_rx_buff.locked = LOCKED;

                    gprs_rx_isr_handler.state = GPRS_RX_IDLE;
#ifdef DEBUG_GPRS_RX
                    // vUART_SendStr(UART_PC,"\ngprs_rx_buf:");
                    // vUART_SendStr(UART_PC,(const uint8_t *)gprs_rx_buff.buffer);
#endif
                    gprs.gprs_rx_buff_len = gprs_rx_buff.index;
                    //memset(gprs_resp_rx_buff,0,sizeof(gprs_rx_data_buff_t));
                    //memcpy(&gprs_resp_rx_buff,&gprs_rx_buff,sizeof(gprs_rx_data_buff_t));
                    // PP commented on 27-04-24: EVSE relevant function:
#ifndef ETHERNET_EN
                    // GPRS_Server_Request();
                    bool gotReq = server_query();
                    if(gotReq)
                    {
                        HandleQueryStates();
                    }
#endif  // ETHERNET_EN
                }
            }
            else if((gprs_rx_buff.index > 0) && (gprs_rx_buff.locked != LOCKED))
            {
//#ifdef DEBUG_RANDOM_
//            vUART_SendStr(UART_PC,"\nnfcount");
//#endif
                gprs_rx_buff.locked = LOCKED;

                gprs_rx_isr_handler.state = GPRS_RX_IDLE;
#ifdef DEBUG_GPRS_RX
                    // vUART_SendStr(UART_PC,"\ngprs_rx_buf:");
                    // vUART_SendStr(UART_PC,(const uint8_t *)gprs_rx_buff.buffer);
#endif
                gprs.gprs_rx_buff_len = gprs_rx_buff.index;
                //memset(gprs_resp_rx_buff,0,sizeof(gprs_rx_data_buff_t));
                //memcpy(&gprs_resp_rx_buff,&gprs_rx_buff,sizeof(gprs_rx_data_buff_t));

                // PP commented on 27-04-24: EVSE relevant function:
#ifndef ETHERNET_EN
                //  GPRS_Server_Request();
                bool gotReq = server_query();
                if(gotReq)
                {
                    HandleQueryStates();
                }
#endif  // ETHERNET_EN
            }
// #ifdef DEBUG_GPRS_RX
//             vUART_SendStr(UART_PC,"\ngrb:");
//             vUART_SendInt(UART_PC, gprs_rx_buff.index);
//             vUART_SendChr(UART_PC, ',');
//             vUART_SendBytes(UART_PC, (const uint8_t*)gprs_rx_buff.buffer, gprs_rx_buff.index);
// #endif
        }
    }
}

void flushRxBuffer(int uart_no)
{
#if 0
    if(uart_no == DISPLAY_UART)
    {
        memset(&display_uart_rx,0,sizeof(Rx_Buff_t));
    }
    else 
#endif  //if 0
    if(uart_no == UART_PC)
    {
        memset(&pc_uart_rx,0,sizeof(Rx_Buff_t));
    }
    else if(uart_no == LTE_UART)
    {
        memset((void *)&gprs_rx_buff,0,sizeof(gprs_rx_data_buff_t));
    }
}

void flushTxBuffer(int uart_no)
{
#if 0
    if(uart_no == DISPLAY_UART)
    {
        memset(&display_tx,0,sizeof(Tx_Buff_t));
    }
    else 
#endif  //if 0
    if(uart_no == UART_PC)
    {
        unsigned int last_pck_num = pc_uart_tx.curr_pck_num;
        char last_pending_cmd = pc_uart_tx.pending_command, last_wait_for_ack = pc_uart_tx.wait_for_ack;

        memset(&pc_uart_tx,0,sizeof(Tx_Buff_t));

        pc_uart_tx.pending_command = last_pending_cmd;
        pc_uart_tx.curr_pck_num = last_pck_num;
        pc_uart_tx.wait_for_ack = last_wait_for_ack;
        //memset(&pc_uart_tx,0,sizeof(Tx_Buff_t));
    }
    else if(uart_no == LTE_UART)
    {
        memset((void *)&gprs_tx_buff,0,sizeof(gprs_tx_data_buff_t));
    }
}

void prepare_graphical_tx_buff(unsigned int cmd,char * data,unsigned char len)
{
    int i = 0;
    display_tx.tx_index = 0;
    display_tx.tx_buff[display_tx.tx_index++] = 0xAA;
    display_tx.tx_buff[display_tx.tx_index++] = len;
    display_tx.tx_buff[display_tx.tx_index++] = cmd;
    while (i<(len-1))
    {
        display_tx.tx_buff[display_tx.tx_index] = data[i];
        i++;
        display_tx.tx_index++;
    }

    display_tx.tx_buff[display_tx.tx_index++] = getChecksum((unsigned char *)&display_tx.tx_buff[CMD_INDX],len);
    display_tx.tx_buff[display_tx.tx_index++]= 0x55;
}

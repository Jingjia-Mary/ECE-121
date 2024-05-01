/*
 * File:   uart_note.c
 * Author: Petersen, ECE121 W2023
 */
// Be sure this include is added to your uart.c code.
#include <stdio.h> // to test buffer
#include "uart.h" // The header file for this source file. 
#include "BOARD.h"
#include <xc.h>
#include "sys/attribs.h"
#include <string.h> 


/*******************************************************************************
 * PRIVATE FUNCTIONS to add to your Uart.c file                                                         *
 ******************************************************************************/
#define UART_BAUDRATE 115200
#define UART_BUFFER_SIZE 256

typedef struct {
    uint8_t *buffer;
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} circular_buffer;

// Define UART buffer instances
static circular_buffer rxBuffer;
static circular_buffer txBuffer;
static int Transmit = 0; // Initialize to FALSE
static int Collision = 0; // Initialize to FALSE
/**
 * Refer to ...\docs\MPLAB C32 Libraries.pdf: 32-Bit Language Tools Library.
 * In sec. 2.13.2 helper function _mon_putc() is noted as normally using
 * UART2 for STDOUT character data flow. Adding a custom version of your own
 * can redirect this to UART1 by calling your putchar() function.   
 */
void _mon_putc(char c) {
    //your code goes here
}

/****************************************************************************
 * Function: IntUart1Handler
 * Parameters: None.
 * Returns: None.
 * The PIC32 architecture calls a single interrupt vector for both the 
 * TX and RX state machines. Each IRQ is persistent and can only be cleared
 * after "removing the condition that caused it". This function is declared in
 * sys/attribs.h. 
 ****************************************************************************/

void CircularBuffer_Init(circular_buffer *buffer, uint16_t size) {
    buffer->buffer = (uint8_t *)malloc(size * sizeof(uint8_t));
    buffer->size = size;
    buffer->head = 0;
    buffer->tail = 0;
}

int CircularBuffer_IsEmpty(const circular_buffer *buffer) {
    return buffer->head == buffer->tail;
}

int CircularBuffer_IsFull(const circular_buffer *buffer) {
    return ((buffer->head + 1) % buffer->size) == buffer->tail;
}

void CircularBuffer_Enqueue(circular_buffer *buffer, uint8_t data) {
    if (!CircularBuffer_IsFull(buffer)) {
        buffer->buffer[buffer->head] = data;
        buffer->head = (buffer->head + 1) % buffer->size;
    }
}

int CircularBuffer_Dequeue(circular_buffer *buffer, uint8_t *data) {
    if (!CircularBuffer_IsEmpty(buffer)) {
        *data = buffer->buffer[buffer->tail];
        buffer->tail = (buffer->tail + 1) % buffer->size;
        return 1; // Success
    }
    return 0; // Buffer is empty
}

void UART_Init(void) {
    // Initialize circular buffers
    CircularBuffer_Init(&rxBuffer, UART_BUFFER_SIZE);
    CircularBuffer_Init(&txBuffer, UART_BUFFER_SIZE);
    
    // Configure UART
        U1MODE = 0; //pic zip section 21
    U1STA = 0;
    U1TXREG = 0;
    U1RXREG = 0;
    U1BRG = (PB_CLOCK / (16 * UART_BAUDRATE)) - 1; //board.c to board.h
    U1MODEbits.PDSEL = 00;
    U1MODEbits.STSEL = 0;
    U1MODEbits.ON = 1;
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    IPC6bits.U1IP = 6;
    IPC6bits.U1IS = 0;
    IEC0bits.U1RXIE = 1;
    IEC0bits.U1TXIE = 1;
    U1STAbits.URXISEL = 0;
    U1STAbits.UTXISEL = 1;
}

int UART_Transmit(char data) {
    if (!CircularBuffer_IsFull(&txBuffer)) {
        CircularBuffer_Enqueue(&txBuffer, data);
        // Enable UART transmit interrupt if not already enabled
        return 1; // Success
    }
    return 0; // Buffer is full
}

int UART_Receive(char *data) {
    if (!CircularBuffer_IsEmpty(&rxBuffer)) {
        CircularBuffer_Dequeue(&rxBuffer, (uint8_t *)data);
        return 1; // Success
    }
    return 0; // Buffer is empty
}
//void enqueue(struct circular_buffer *buffer, char x) { //insert characters if needed
//    if (!isfull(buffer)) {
//        buffer->data[buffer->tail] = x;
//        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
//    }
//}
//
//char dequeue(struct circular_buffer *buffer) {
//    char x = '\0'; // Default value if buffer is empty
//    if (!isempty(buffer)) {
//        x = buffer->data[buffer->head];
//        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
//    }
//    return x; //Return the dequeued value
//}

void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
    // Check if receive interrupt flag is set
    if (IFS0bits.U1RXIF) {
        // Clear the receive interrupt flag
        IFS0bits.U1RXIF = 0;
        // Process received characters while data is available
        while (U1STAbits.URXDA == 1) {
            // Add received character to the receive buffer
            CircularBuffer_Enqueue(&rxBuffer, U1RXREG);
        }
        // Handle Receive Buffer Overrun Error
        if (U1STAbits.OERR == 1) {
            // Clear the overrun error flag
            U1STAbits.OERR = 0;
        }
    }
    // Check if transmit interrupt flag is set
    if (IFS0bits.U1TXIF) {
        // Clear the transmit interrupt flag
        IFS0bits.U1TXIF = 0;
        // If data is not currently being transmitted
        if (Transmit == TRUE) {
            Collision = TRUE;
        } else {
            unsigned char txData;
            while (!U1STAbits.UTXBF && !CircularBuffer_IsEmpty(&txBuffer)) {
                CircularBuffer_Dequeue(&txBuffer, &txData);
                U1TXREG = txData;
            }
        }
    }
}
// Function to get a character from the receive buffer

//int PutChar(char ch) {
//    // Check if the TX buffer is not full
//    if (!isfull(&txBuffer)) {
//        // Enqueue the character into the TX buffer
//        enqueue(&txBuffer, ch); // Optionally, enable UART transmit interrupt if it's not already enabled
//        IEC0bits.U1TXIE = 1; // Enable UART transmit interrupt
//        return 1; // Return true if successful
//    } else {
//        return 0; // Return false if buffer is full
//    }
//}
//
//unsigned char GetChar(unsigned char *ch) {
//    // Check if the RX buffer is not empty
//    if (!isempty(&rxBuffer)) {
//        *ch = dequeue(&rxBuffer);// Dequeue a character from the RX buffer
//        return 1; // Return the dequeued character
//    } else {
//        *ch = '\0'; // Set the output character to NULL for error
//        return 0; // Return 0 for error (buffer is empty)
//    }
//}




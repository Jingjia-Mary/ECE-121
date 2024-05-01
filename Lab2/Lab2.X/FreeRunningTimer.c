/* 
 * File:   FreeRunningTimer.c
 * Author: maryx
 *
 * Created on March 17, 2024, 6:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attribs.h>
#include "BOARD.h"
#include "uart.h"
#include "Protocol2.h"
#include "FreeRunningTimer.h" 

// Constants
#define SYSTEM_CLOCK 80000000L
#define PB_CLOCK SYSTEM_CLOCK/2
#define BAUDRATE 115200

// Function prototypes
static void initializeTimer(void);
static void timerInterruptHandler(void);

// Variables
static unsigned int milliSecondsCounter;
static unsigned int microSecondsCounter;

// Function to initialize the free-running timer
static void initializeTimer(void) {
    BOARD_Init();
    TMR5 = 0x0;
    PR5 = 0x1387; // Load period register with 16-bit value
    T5CONbits.ON = 0;
    T5CONbits.TCKPS = 0b011; // Set prescaler to 1:8
    IFS0bits.T5IF = 0;
    IPC5bits.T5IP = 0b011; // Set interrupt priority to 3
    IPC5bits.T5IS = 0b0; // Set interrupt subpriority to 0
    IEC0bits.T5IE = 1; // Enable timer interrupt
    T5CONbits.ON = 1; // Turn on Timer 5
}

// Function to get milliseconds from the free-running timer
unsigned int FreeRunningTimer_GetMilliSeconds(void){     
    return milliSecondsCounter;
}

// Function to get microseconds from the free-running timer
unsigned int FreeRunningTimer_GetMicroSeconds(void) {
    microSecondsCounter = milliSecondsCounter + (TMR5 / 5);
    return microSecondsCounter;
}

// Timer 5 interrupt handler
void __ISR(_TIMER_5_VECTOR) Timer5IntHandler(void) {
    if (IFS0bits.T5IF == 1) {
        milliSecondsCounter++;
        IFS0bits.T5IF = 0; // Clear Timer 5 interrupt flag
    }
}

int main(void) {
    FreeRunningTimer_Init();
    Protocol_Init(BAUDRATE);
    while(1){
        
    }
    return (SUCCESS);
}
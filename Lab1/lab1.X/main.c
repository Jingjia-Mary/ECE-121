/* 
 * File:   main.c
 * Author: maryx
 *
 * Created on February 5, 2024, 2:03 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include <xc.h>
#include "Protocol2.h"
#include "BOARD.h"
#include "MessageIDs.h"

#define baudRate 115200

/*
 * 
 */ //this main is for part 1 check when part 2 need a new main fuction to check

//void test_UART_loopback(void) {
//    unsigned char received_char;
//    unsigned char char_to_send;
//
//    // Loop indefinitely
//    while (1) {
//        // Check if a character is received
//        if (GetChar(&received_char)) {
//            // Echo back the received character
//            PutChar(received_char);
//        }
//    }
//}
//
//int main(void) {
//    BOARD_Init();
//    Uart_Init(115200); //lab manual sign
//
//    test_UART_loopback();
//
//    return 0;
//}

int main() {
    BOARD_Init();
    UART_Init();
    Protocol_Init(baudRate);
    while(1) {  
        runProtocol();
    }
    BOARD_End();
}
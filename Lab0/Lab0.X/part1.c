/* 
 * File:   part1.c
 * Author: maryx
 *
 * Created on January 16, 2024, 1:31 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "BOARD.h"
#include "Leds.h" //set a h file for the led lights to call out in c file
#include "xc.h"

/*
 * 
 */
int main(void) {
    BOARD_Init(); //set h files so start it
    LEDS_INIT();
    
    while (1){
        if (PORTFbits.RF1){ //has different ports because each button belongs to different port
            LATE = 0b00010001;  //it is binary
        }else if (PORTDbits.RD5){
            LATE = 0b00100010;
        }else if (PORTDbits.RD6){
            LATE = 0b01000100;
        }else if (PORTDbits.RD7){
            LATE = 0b10001000;
        }else{
            LATE = 0;  //set to zero when there is nothing
        }
//        LATE = 0xFF;
    }
    
    
}


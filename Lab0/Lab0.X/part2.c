/* 
 * File:   part2.c
 * Author: maryx
 *
 * Created on January 17, 2024, 12:01 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "BOARD.h"
#include "Leds.h"
#include "xc.h"

/*
 * 
 */
int main(void) {
    BOARD_Init(); //set h files so start it
    LEDS_INIT();
    
    while (1){
        int i;
        LATE ++; 
        for (i = 0; i < 115000; i++){ //set the dely loop, the number comes out of try outs
            asm("nop");
        }
        if (PORTFbits.RF1 || PORTDbits.RD5 || PORTDbits.RD6 || PORTDbits.RD7){ //different button pressed use or to simplify
            LATE = 0x00;
        }
    }
    
}


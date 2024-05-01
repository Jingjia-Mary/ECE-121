/* 
 * File:   FreeRunningTimer.h
 * Author: Petersen
 * written for ECE121 W2024
 */

#ifndef FREERUNNINGTIMER_H
#define	FREERUNNINGTIMER_H

#include "BOARD.h"
#include "xc.h" 
#include <sys/attribs.h> 
/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/**
 * @Function FreeRunningTimer_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module */
void FreeRunningTimer_Init(void);

/**
 * Function: FreeRunningTimer_GetMilliSeconds
 * @param None
 * @return the current MilliSecond Count
   */
unsigned int FreeRunningTimer_GetMilliSeconds(void);

/**
 * Function: FreeRunningTimer_GetMicroSeconds
 * @param None
 * @return the current MicroSecond Count
   */
unsigned int FreeRunningTimer_GetMicroSeconds(void);
#endif	/* FREERUNNINGTIMER_H */


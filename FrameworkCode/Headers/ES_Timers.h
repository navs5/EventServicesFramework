/****************************************************************************
 Module
         ES_Timers.h

 Revision
         1.0.1

 Description
         Header File for the ME218 Timer Module

 Notes


 History
 When           Who	What/Why
 -------------- ---	--------
 10/13/15 20:48 jec  removed prototype for IsTimerActive, I had removed the code
                     a couple of years ago
 08/13/13 12:03 jec  added prototype for ES_Timer_Tick_Resp as part of
                     moving all of the hardware specific code to ES_Port.c
 01/15/12 16:43 jec  converted for Gen2 of the Events & Services Framework
 10/21/11 18:33 jec  Begin conversion for use with the new Event Framework
 09/01/05 12:29 jec  Converted rates and return values to enumerated constants
 06/15/04 09:58 jec  converted all prefixes to EF_Timer
 05/27/04 15:04 jec  revised TMR_RATE macros for the S12
 11/24/00 13:17 jec  revised TMR_RATE macros for the HC12
 02/20/99 14:11 jec  added #defines for TMR_ERR, TMR_ACTIVE, TMR_EXPIRED
 02/24/97 14:28 jec  Began Coding
****************************************************************************/

#ifndef ES_Timers_H
#define ES_Timers_H

#include "ES_Port.h"
#include "ES_Types.h"

typedef enum
{
  ES_Timer_ERR        = -1,
  ES_Timer_ACTIVE     = 1,
  ES_Timer_OK         = 0,
  ES_Timer_NOT_ACTIVE = 0
}ES_TimerReturn_t;

void ES_Timer_Init(TimerRate_t Rate);
void ES_Timer_Tick_Resp(void);
ES_TimerReturn_t ES_Timer_InitTimer(uint8_t Num, uint16_t NewTime);
ES_TimerReturn_t ES_Timer_SetTimer(uint8_t Num, uint16_t NewTime);
ES_TimerReturn_t ES_Timer_StartTimer(uint8_t Num);
ES_TimerReturn_t ES_Timer_StopTimer(uint8_t Num);
uint16_t ES_Timer_GetTime(void);

#endif   /* ES_Timers_H */
/*------------------------------ End of file ------------------------------*/


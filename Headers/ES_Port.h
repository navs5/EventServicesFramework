/****************************************************************************
 Module
     ES_Port.h
 Description
     header file to collect all of the hardware/platform dependent info for
     a particular port of the ES framework
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/14/15 21:50 jec     added prototype for ES_Timer_GetTime
 01/18/15 13:24 jec     clean up and adapt to use TI driver lib functions
                        for implementing EnterCritical & ExitCritical
 03/13/14		joa		      Updated files to use with Cortex M4 processor core.
 	 	 	 	 	 	            Specifically, this was tested on a TI TM4C123G mcu.
 08/13/13 12:12 jec     moved the timer rate constants from ES_Timers.h here
 08/05/13 14:24 jec     new version replacing port.h and adding define to
                        capture the C99 compliant behavior of the compiler
*****************************************************************************/
#ifndef ES_PORT_H
#define ES_PORT_H

// pull in the hardware header files that we need

#include <stdio.h>
#include <stdint.h>
#include "termio.h"
#include "bitdefs.h"       /* generic bit defs (BIT0HI, BIT0LO,...) */
#include "Bin_Const.h"     /* macros to specify binary constants in C */
#include "ES_Types.h"

// macro to control the use of C99 data types (or simulations in case you don't
// have a C99 compiler).

// Codewarrior V5, is not C99 so keep this commented out for C32 & E128
#define COMPILER_IS_C99

// The macro 'ES_FLASH' is needed on some compilers to allocate const data to 
// ROM. The macro must be defined, even if it evaluates to nothing.
// for the 'C32 & E128 this is not needed so it evaluates to nothing
#define ES_FLASH 

// the macro 'ES_READ_FLASH_BYTE' is needed on some Harvard machines to generate
// code to read a byte from the program space (Flash)
// The macro must be defined, even if it evaluates to nothing.
// for the 'C32 & E128 we don't need special instructions so it evaluates to a
// simple reference to the variable
#define ES_READ_FLASH_BYTE(_flash_var_)    (_flash_var_)                  

// these macros provide the wrappers for critical regions, where ints will be off
// but the state of the interrupt enable prior to entry will be restored.
// allocation of temp var for saving interrupt enable status should be defined
// in ES_Port.c

// Cortex M-series processors 
// The Interrupt Program Status Register (IPSR) contains the exception type number
// of the current interrupt service routine (ISR)
// Using TivaWare, CPUcpsid() - IntMasterDisable() calls this. Equivalent to __diable_irq()?
extern uint32_t _PRIMASK_temp;
uint32_t CPUgetPRIMASK_cpsid(void);
void CPUsetPRIMASK(uint32_t newPRIMASK);


#define EnterCritical()	{ _PRIMASK_temp = CPUgetPRIMASK_cpsid(); }
#define ExitCritical() { CPUsetPRIMASK(_PRIMASK_temp); }


/* Rate constants for programming the SysTick Period to generate tick interrupts.
   These assume an 40MHz configuration, they are the values to be used to program
   the SysTick Reload Value (STRELOAD) register. STRELOAD is 24-bits wide and so
   the highest value is 0xFFFFFF (16,777,216) which equates to
   16777216*1000/40000000 = 419.4 mS.
   They are all listed as -1 because the actual cycle time includes 1 cycle to 
   reset from the max count back to zero, so, for example, to achieve a 4000 
   count cycle time, you load the register with 4000-1
 */
typedef enum {	ES_Timer_RATE_OFF  	=   (0),
				ES_Timer_RATE_100uS = 4000-1,
				ES_Timer_RATE_500uS = 20000-1,
				ES_Timer_RATE_1mS	= 40000-1,
				ES_Timer_RATE_2mS	= 80000-1,
				ES_Timer_RATE_4mS	= 160000-1,
				ES_Timer_RATE_5mS	= 200000-1,
				ES_Timer_RATE_8mS	= 320000-1,
				ES_Timer_RATE_10mS	= 400000-1,
				ES_Timer_RATE_16mS	= 640000-1,
				ES_Timer_RATE_32mS	= 1280000-1
} TimerRate_t;

// map the generic functions for testing the serial port to actual functions 
// for this platform. If the C compiler does not provide functions to test
// and retrieve serial characters, you should write them in ES_Port.c
#define IsNewKeyReady()  ( kbhit() != 0 )
#define GetNewKey()      getchar()

// prototypes for the hardware specific routines
void _HW_Timer_Init(TimerRate_t Rate);
bool _HW_Process_Pending_Ints( void );
uint16_t _HW_GetTickCount(void);
void ConsoleInit(void);
// and the one Framework function that we define here
uint16_t ES_Timer_GetTime(void);

#endif

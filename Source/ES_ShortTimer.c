//#define DEBUG
/****************************************************************************
 Module
   ES_ShortTimer.c

 Revision
   1.0.1

 Description
   This is a library to provide for the creation of short time-outs
   (shorter than the resolution of the ES_Timer library).

 Notes
   This module uses the Tiva Peripheral Driver Library functions and
   the ability that it provides to 'hook' a function into an interrupt
   response routine without modifying the vector table directly.
   Uses timers A & B on 16/32 bit Timer Module 5

 History
 When           Who     What/Why
 -------------- ---     --------
 10/11/15 10:30 jec     first pass
 10/11/15 18:10 jec     converted to post events to the framework

****************************************************************************/
// the common headers for I/O, C99 types
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "BITDEFS.H"

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_ints.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

// the header to get the timing functions
#include "ES_ShortTimer.h"

// the framework headers
#include "ES_Framework.h"
#include "ES_Configure.h"

// module level functions

// define for the timer pre-scaler. this sets the resolution of the
// timing functions
// the current values are based on a 40mHz clock rate to give 1uS resolution
#define PRE_1uS 40

void ShortTimerAHandler(void);
void ShortTimerBHandler(void);

// module level variables

static uint8_t  Timer_A_Priority  = SHORT_TIMER_UNUSED;
static uint8_t  Timer_B_Priority  = SHORT_TIMER_UNUSED;

//******************************
// ES_ShortTimerInit()
// Initialize the timer subsystem and log the services to which the timeout
// messages will be posted
//******************************
void ES_ShortTimerInit(uint8_t TimeAPrio, uint8_t TimeBPrio)
{
#ifdef DEBUG
// set up I/O lines for debugging
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
// start with the lines low
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, BIT0LO & BIT1LO);
#endif

// enable the clock to the timer module
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
// configure as 2 16 bit timers
  TimerConfigure(TIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT |
      TIMER_CFG_B_ONE_SHOT);
// set prescale to get 1uS resolution
  TimerPrescaleSet(TIMER5_BASE, TIMER_BOTH, PRE_1uS);
// log the service to which the timeout will be posted
  Timer_A_Priority  = TimeAPrio;
  Timer_B_Priority  = TimeBPrio;
}

void ES_ShortTimerStart(uint32_t Which, uint16_t TimeoutValue)
{
  if ((Which != TIMER_A) && (Which != TIMER_B))
  {
    return;
  }
  // for very short delays. just immediatly post. There is 10us of overhead
  if (TimeoutValue < 11)
  {
    if (Which == TIMER_A)
    {
      ShortTimerAHandler();
    }
    else
    {
      ShortTimerBHandler();
    }
  }
  TimerLoadSet(TIMER5_BASE, Which, TimeoutValue);

  if (Which == TIMER_A) // local enable
  {
    TimerIntEnable(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    // NVIC Enable
    IntEnable(INT_TIMER5A_TM4C123);
  }
  else
  {
    // local enable
    TimerIntEnable(TIMER5_BASE, TIMER_TIMB_TIMEOUT);
    // NVIC Enable
    IntEnable(INT_TIMER5B_TM4C123);
  }

  TimerEnable(TIMER5_BASE, Which);

#ifdef DEBUG
// raise I/O line to show we started
  if (Which == TIMER_A)
  {
    GPIOPinWrite(GPIO_PORTB_BASE, BIT0HI, BIT0HI);
  }
  else
  {
    GPIOPinWrite(GPIO_PORTB_BASE, BIT1HI, BIT1HI);
  }
#endif
  return;
}

void ShortTimerAHandler(void)
{
  ES_Event_t ThisEvent;

// start by clearing the source of the interrupt
  TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
#ifdef DEBUG
// lower I/O line to show we arrived
  GPIOPinWrite(GPIO_PORTB_BASE, BIT0HI, BIT0LO);
#endif

// post the timeout for this timer
  ThisEvent.EventType   = ES_SHORT_TIMEOUT;
  ThisEvent.EventParam  = TIMER_A;
// protect against timer that was not correctly initialized
  if (Timer_A_Priority != SHORT_TIMER_UNUSED)
  {
    ES_PostToService(Timer_A_Priority, ThisEvent);
  }
}

void ShortTimerBHandler(void)
{
  ES_Event_t ThisEvent;

// start by clearing the source of the interrupt
  TimerIntClear(TIMER5_BASE, TIMER_TIMB_TIMEOUT);
#ifdef DEBUG
// lower I/O line to show we arrived
  GPIOPinWrite(GPIO_PORTB_BASE, BIT1HI, BIT1LO);
#endif

// post the timeout for this timer
  ThisEvent.EventType   = ES_SHORT_TIMEOUT;
  ThisEvent.EventParam  = TIMER_B;
// protect against timer that was not correctly initialized
  if (Timer_B_Priority != SHORT_TIMER_UNUSED)
  {
    ES_PostToService(Timer_B_Priority, ThisEvent);
  }
}


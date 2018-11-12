//#define TEST
/****************************************************************************
 Module
     ES_LookupTables.c
 Description
     This is the home for a set of constant lookup tables that are used in
     multiple places in the framework and beyond.

 Notes
     As a rule, I don't approve of global variables for a host of reasons.
     In this case I decided to make them global in the interests of
     efficiency. These tables will be references very often in the timer
     functions (as often as 8 times/ms) and on every pass through the event
     scheduler/dispatcher. As a result I decided to simply make them global.
     Since they are constant, they are not subject to the multiple access
     point issues associated with modifiable global variables.

 History
 When           Who     What/Why
 -------------- ---     --------
 10/20/13 17:03 jec      converted Byte2MSBitNum array to a Nybble sized array
                         (15 entries) and made function GetMSBitSet() to figure
                         out the MSB set. This was done to facilitate moving to
                         16 timers & services, which would take too big of a
                         table to look up directly.
 01/15/12 09:07 jec      change of heart: converted to global vars for the
                         tables and removed the access functions
 01/12/12 10:25 jec      adding the Ready variable and management functions.
 01/03/12 11:16 jec      started coding
*****************************************************************************/
/*----------------------------- Include Files -----------------------------*/

#include "ES_Types.h"
#include "ES_General.h"
#include "ES_Timers.h"
#include "bitdefs.h"

/*----------------------------- Module Defines ----------------------------*/
#define ISOLATE_LS_NYBBLE 0x0F

/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/

/*
  this table is used to go from a bit number (0-7) to the mask used to clear
  that bit in a byte. Technically, this is not necessary (as you could always
  complement the SetMask) but it will save the complement operation every
  time it is used to clear a bit. If we clear a bit with it in more than 8
  places, then it is a win on code size and speed.

uint16_t const BitNum2ClrMask[] = {
  BIT0LO, BIT1LO, BIT2LO, BIT3LO, BIT4LO, BIT5LO, BIT6LO, BIT7LO, BIT8LO, BIT9LO,
  BIT10LO, BIT11LO, BIT12LO, BIT13LO, BIT14LO, BIT15LO
};
*/

/*
  this table is used to go from a bit number (0-15) to the mask used to set
  that bit in a byte.
*/
uint16_t const BitNum2SetMask[] = {
  BIT0HI, BIT1HI, BIT2HI, BIT3HI, BIT4HI, BIT5HI, BIT6HI, BIT7HI, BIT8HI, BIT9HI,
  BIT10HI, BIT11HI, BIT12HI, BIT13HI, BIT14HI, BIT15HI
};

/*
  this table is used to go from an unsigned 4bit value to the most significant
  bit that is set in that nybble. It is used in the determination of priorities
  from the Ready variable and in determining active timers in
  the timer interrupt response. Index into the array with (ByteVal-1) to get
  the correct MS Bit num.
*/
uint8_t const Nybble2MSBitNum[15] = {
  0U, 1U, 1U, 2U, 2U, 2U, 2U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U
};

/*------------------------------ Module Code ------------------------------*/
uint8_t ES_GetMSBitSet(uint16_t Val2Check)
{
  int8_t  LoopCntr;
  uint8_t Nybble2Test;
  uint8_t ReturnVal = 128; // this is the error return value

  // loop through the parameter, nybble by nybble
  for (LoopCntr = sizeof(Val2Check) * (BITS_PER_BYTE / BITS_PER_NYBBLE) - 1;
      LoopCntr >= 0; LoopCntr--)
  {
    // move a nybble into the 4 LSB positions for lookup
    Nybble2Test = (uint8_t)
        ((Val2Check >> (uint8_t)(LoopCntr * BITS_PER_NYBBLE)) &
        ISOLATE_LS_NYBBLE);
    if (Nybble2Test != 0)
    {
      // lookup the bit num & adjust for the number of shifts to get there
      ReturnVal = Nybble2MSBitNum[Nybble2Test - 1] +
          (LoopCntr * BITS_PER_NYBBLE);
      break;
    }
  }
  return ReturnVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/
#ifdef TEST
#include <stdio.h>

void main(void)
{
  uint16_t  Counter = 0;
  uint8_t   MSBit;

  puts( "Testing the MSB Look-up function\n\r");
  puts( __TIME__ " " __DATE__);
  puts( "\n\r");
  MSBit = ES_GetMSBitSet(Counter);
    printf("the MSB set in %u is bit %d\n\r", Counter, MSBit);

  for (Counter = 1; Counter != 0; Counter++)
  {
    MSBit = ES_GetMSBitSet(Counter);
    printf("the MSB set in %u is bit %d\n\r", Counter, MSBit);
  }
}

#endif
/*------------------------------ End of File ------------------------------*/

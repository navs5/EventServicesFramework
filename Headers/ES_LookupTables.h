/****************************************************************************
 Module
     ES_LookupTables.h
 Description
     Extern declarations for a set of constant lookup tables that are used in
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
 10/20/13 21:19 jec      got rid of BitNum2ClrMask and replaced with #define
                         replaced Byte2MSBNum with function ES_GetMSBSet
                         replaced Byte2MSBNum array with Nybble2MSBNum
 08/05/13 15:45 jec      added #include for ES_Types.h since we depend on it
 01/15/12 13:03 jec      started coding
*****************************************************************************/
#include "ES_Types.h"
/*
  Since we moved up to 16 timers & services, this table got too big to justify
  having a separate table for the clear and set masks, so just #define the
  tilde operator in to keep the readability
*/
#define BitNum2ClrMask ~BitNum2SetMask

/*
  this table is used to go from a bit number (0-7) to the mask used to set
  that bit in a byte.
*/
extern uint16_t const BitNum2SetMask[];

/*
  this table is used to go from an unsigned 4bit value to the most significant
  bit that is set in that nybble. It is used in the determination of priorities
  from the Ready variable and in determining active timers in
  the timer interrupt response. Index into the array with (ByteVal-1) to get
  the correct MS Bit num.
*/
extern uint8_t const Nybble2MSBitNum[15];

/****************************************************************************
 Function
   ES_GetMSBSet
 Parameters
   uint16_t  Val2Check The number to find the MSB in
 Returns
   bit number of the MSB that is set in Val2Check, 128 if Val2Check = 0
 Description
   find the MSB that is set in Val2Check and returns that bit number
 Notes

 Author
   J. Edward Carryer, 10/20/13, 17:03
****************************************************************************/
uint8_t ES_GetMSBitSet(uint16_t Val2Check);

/****************************************************************************
 Module
     EF_Framework.h
 Description
     header file for use with the top level functions of the EF Event Framework
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/02/13 17:06 jec      added ES_PostToServiceLIFO prototype
 08/05/13 15:00 jec      added #include for ES_Port.h to get portability stuff
 10/17/06 07:41 jec      started coding
*****************************************************************************/

#ifndef ES_Framework_H
#define ES_Framework_H

#include "ES_Types.h"
#include "ES_Port.h"
#include "ES_Events.h"

// These includes are not strictly necessary for the framework, but simplify
// the use of the framework by requiring only 2 include files
// ES_Configure.h and ES_Framework.h
// The balance of the framework headers are included here

#include "ES_PostList.h"
#include "ES_General.h"
#include "ES_Timers.h"

typedef enum
{
  Success     = 0,
  FailedPost  = 1,
  FailedRun,
  FailedPointer,
  FailedIndex,
  FailedInit
}ES_Return_t;

ES_Return_t ES_Initialize(TimerRate_t NewRate);
ES_Return_t ES_Run(void);
bool ES_PostAll(ES_Event_t ThisEvent);
bool ES_PostToService(uint8_t WhichService, ES_Event_t ThisEvent);
bool ES_PostToServiceLIFO(uint8_t WhichService, ES_Event_t TheEvent);

#endif   // ES_Framework_H

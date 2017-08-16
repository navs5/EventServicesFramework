/****************************************************************************
 Module
     ES_CheckEvents.c
 Description
     source file for the module to call the User event checking routines
 Notes
     Users should not modify the contents of this file.
 History
 When           Who     What/Why
 -------------- ---     --------
                jec     out all user modifications into ES_Configure
 10/16/11 12:32 jec      started coding
*****************************************************************************/

#include "ES_Configure.h"
#include "ES_Events.h"
#include "ES_General.h"
#include "ES_CheckEvents.h"

// Include the header files for the module(s) with your event checkers. 
// This gets you the prototypes for the event checking functions.

#include EVENT_CHECK_HEADER

// Fill in this array with the names of your event checking functions

static CheckFunc * const ES_EventList[]={EVENT_CHECK_LIST };


// Implementation for public functions

/****************************************************************************
 Function
   ES_CheckUserEvents
 Parameters
   None
 Returns
   bool: true if any of the user event checkers returned true, false otherwise
 Description
   loop through the EF_EventList array executing the event checking functions
 Notes
   
 Author
   J. Edward Carryer, 10/25/11, 08:55
****************************************************************************/
bool ES_CheckUserEvents( void ) 
{
  uint8_t i;
  // loop through the array executing the event checking functions
  for ( i=0; i< ARRAY_SIZE(ES_EventList); i++) {
    if ( ES_EventList[i]() == true )
      break; // found a new event, so process it first
  }
  if ( i == ARRAY_SIZE(ES_EventList) ) // if no new events
    return (false);
  else
    return(true);
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

/****************************************************************************
 Module
     ES_DeferRecall.c

 Description
     This is a module implementing  the management of event deferal and recall
      queues
 Notes


 History
 When           Who     What/Why
 -------------- ---     --------

 10/11/14 14:58 jec     converted RecallEvent to RecallEvents to pull all
                        deferred events off the deferral queue
 11/02/13 16:38 jec      Began Coding
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_General.h"
#include "ES_Events.h"
#include "ES_DeferRecall.h"

/*--------------------------- External Variables --------------------------*/

/*----------------------------- Module Defines ----------------------------*/

/*------------------------------ Module Types -----------------------------*/

/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     ES_RecallEvents
 Parameters
      uint8_t WhichService, number of the service to post Recalled event to
      ES_Event * pBlock, pointer to the block of memory that implements the
        Defer/Recall queue
 Returns
     bool true if an event was recalled, false if no event was left in queue
 Description
     pulls all events off the deferral queue if any are available. If there was
     something in the queue, then it posts it LIFO fashion to the queue
     indicated by WhichService
 Notes
     None.
 Author
     J. Edward Carryer, 11/20/13 16:49
****************************************************************************/
bool ES_RecallEvents(uint8_t WhichService, ES_Event_t *pBlock)
{
  ES_Event_t  RecalledEvent;
  bool      WereEventsPulled = false;
  // recall any events from the queue
  do
  {
    ES_DeQueue(pBlock, &RecalledEvent);
    if (RecalledEvent.EventType != ES_NO_EVENT)
    {
      ES_PostToServiceLIFO(WhichService, RecalledEvent);
      WereEventsPulled = true;
    }
  } while (RecalledEvent.EventType != ES_NO_EVENT);
  return WereEventsPulled;
}

/*------------------------------- Footnotes -------------------------------*/

/*------------------------------ End of file ------------------------------*/


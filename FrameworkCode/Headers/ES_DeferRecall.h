#ifndef DEFER_RECALL_H
#define DEFER_RECALL_H

#include "ES_Queue.h"
#include "ES_Events.h"

/****************************************************************************
 Function
   ES_InitDeferralQueueWith  (wrapper for ES_InitQueue )
   this is a straight re-naming to aid readability
 Parameters
   EF_Event * pBlock : pointer to the block of memory to use for the Queue
   unsigned char BlockSize: size of the block pointed to by pBlock
 Returns
   max number of entries in the created queue
 Description
   Initializes a queue structure at the beginning of the block of memory
 Notes
   you should pass it a block that is at least sizeof(ES_Queue_t) larger than
   the number of entries that you want in the queue. Since the size of an
   ES_Event (at 4 bytes; 2 enum, 2 param) is greater than the
   sizeof(ES_Queue_t), you only need to declare an array of ES_Event
   with 1 more element than you need for the actual queue.
****************************************************************************/
#define ES_InitDeferralQueueWith(a, b) ES_InitQueue(a, b)

/****************************************************************************
 Function
   ES_DeferEvent  (wrapper for ES_EnQueueLIFO)
   this is a straight re-naming to aid readability
 Parameters
   ES_Event * pBlock : pointer to the block of memory in use as the Queue
   ES_Event Event2Add : event to be added to the Queue
 Returns
   bool : true if the add was successful, false if not
 Description
   if it will fit, adds Event2Add to the Queue
 ***************************************************************************/
#define ES_DeferEvent(a, b) ES_EnQueueLIFO(a, b)

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
     pulls an event off the deferral queue if one is available. It there was
     something in the queue, then it posts it LIFO fashion to the queue
     indicated by WhichService
 Notes
     None.
 Author
     J. Edward Carryer, 11/20/13 16:49
****************************************************************************/
bool ES_RecallEvents(uint8_t WhichService, ES_Event_t *pBlock);

#endif

//#define TEST
/****************************************************************************
 Module
     ES_Queue.c
 Description
     Implements a FIFO circular buffer of EF_Event in a block of memory
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 09:34 jec      converted to use the new C99 types from types.h
 08/09/11 18:16 jec      started coding
*****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Queue.h"
#include "ES_Port.h"

/*----------------------------- Module Defines ----------------------------*/
unsigned int _PRIMASK_temp;
//unsigned int _FAULTMASK_temp;
// QueueSize is max number of entries in the queue
// CurrentIndex is the 'read-from' index,
// actually CurrentIndex + sizeof(EF_Queue_t)
// entries are made to CurrentIndex + NumEntries + sizeof(ES_Queue_t)
typedef struct {  uint8_t QueueSize;
                  uint8_t CurrentIndex;
                  uint8_t NumEntries;
} ES_Queue_t;

typedef ES_Queue_t * pQueue_t;

/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
   ES_InitQueue
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
 Author
   J. Edward Carryer, 08/09/11, 18:40
****************************************************************************/
uint8_t ES_InitQueue( ES_Event * pBlock, uint8_t BlockSize )
{
   pQueue_t pThisQueue;
   // initialize the Queue by setting up initial values for elements
   pThisQueue = (pQueue_t)pBlock;
   // use all but the structure overhead as the Queue
   pThisQueue->QueueSize = BlockSize - 1;
   pThisQueue->CurrentIndex = 0;
   pThisQueue->NumEntries = 0;
   return(pThisQueue->QueueSize);
}

/****************************************************************************
 Function
   ES_EnQueueFIFO
 Parameters
   ES_Event * pBlock : pointer to the block of memory in use as the Queue
   ES_Event Event2Add : event to be added to the Queue
 Returns
   bool : true if the add was successful, false if not
 Description
   if it will fit, adds Event2Add to the Queue
 Notes

  Author
   J. Edward Carryer, 08/09/11, 18:59
****************************************************************************/
bool ES_EnQueueFIFO( ES_Event * pBlock, ES_Event Event2Add )
{
   pQueue_t pThisQueue;
   pThisQueue = (pQueue_t)pBlock;
   // index will go from 0 to QueueSize-1 so use '<' to test if there is space
   if ( pThisQueue->NumEntries < pThisQueue->QueueSize)
   {  // save the new event, use % to create circular buffer in block
      // 1+ to step past the Queue struct at the beginning of the
      // block
      EnterCritical();   // save interrupt state, turn ints off
      pBlock[ 1 + ((pThisQueue->CurrentIndex + pThisQueue->NumEntries)
               % pThisQueue->QueueSize)] = Event2Add;
      pThisQueue->NumEntries++;          // inc number of entries
      ExitCritical();  // restore saved interrupt state
      
      return(true);
   }else
      return(false);
}

/****************************************************************************
 Function
   ES_EnQueueLIFO
 Parameters
   ES_Event * pBlock : pointer to the block of memory in use as the Queue
   ES_Event Event2Add : event to be added to the Queue
 Returns
   bool : true if the add was successful, false if not
 Description
   if it will fit, adds Event2Add to the Queue at the extraction point, making
   it the next event to be removed by a DeQueue operation, that is a 
   Last In First Out operation.
 Notes

  Author
   J. Edward Carryer, 11/02/13, 14:30
****************************************************************************/
bool ES_EnQueueLIFO( ES_Event * pBlock, ES_Event Event2Add )
{
   pQueue_t pThisQueue;
   pThisQueue = (pQueue_t)pBlock;
   // index will go from 0 to QueueSize-1 so use '<' to test if there is space
    if ( pThisQueue->NumEntries < pThisQueue->QueueSize){
      EnterCritical();   // save interrupt state, turn ints off
    // OK, there is space note that the queue now has 1 more entry
      pThisQueue->NumEntries++;
    // Check to see if we need to wrap around as we back up index
      if (pThisQueue->CurrentIndex == 0){
       pThisQueue->CurrentIndex = pThisQueue->QueueSize -1;
      }
      else{
        pThisQueue->CurrentIndex--;
      }  
      pBlock[ 1 + pThisQueue->CurrentIndex ] = Event2Add;
      ExitCritical();  // restore saved interrupt state      
      return(true);
    }else // in case no room on the queue
      return(false);
}


/****************************************************************************
 Function
   ES_DeQueue
 Parameters
   unsigned char * pBlock : pointer to the block of memory in use as the Queue
   ES_Event * pReturnEvent : used to return the event pulled from the queue
 Returns
   The number of entries remaining in the Queue
 Description
   pulls next available entry from Queue, EF_NO_EVENT if Queue was empty and
   copies it to *pReturnEvent.
 Notes

 Author
   J. Edward Carryer, 08/09/11, 19:11
****************************************************************************/
uint8_t ES_DeQueue( ES_Event * pBlock, ES_Event * pReturnEvent )
{
   pQueue_t pThisQueue;
   uint8_t NumLeft;

   pThisQueue = (pQueue_t)pBlock;
   if ( pThisQueue->NumEntries > 0)
   {
      EnterCritical();   // save interrupt state, turn ints off
      *pReturnEvent = pBlock[ 1 + pThisQueue->CurrentIndex ];
      // inc the index
      pThisQueue->CurrentIndex++;
      // this way we only do the modulo operation when we really need to
      if (pThisQueue->CurrentIndex >=  pThisQueue->QueueSize)
         pThisQueue->CurrentIndex = (uint8_t)(pThisQueue->CurrentIndex % pThisQueue->QueueSize);
      //dec number of elements since we took 1 out
      NumLeft = --pThisQueue->NumEntries; 
      ExitCritical();  // restore saved interrupt state
   }else { // no items left in the queue
      (*pReturnEvent).EventType = ES_NO_EVENT;
      (*pReturnEvent).EventParam = 0;
      NumLeft = 0;
   }
   return NumLeft;
}

/****************************************************************************
 Function
   ES_IsQueueEmpty
 Parameters
   unsigned char * pBlock : pointer to the block of memory in use as the Queue
 Returns
   bool : true if Queue is empty
 Description
   see above
 Notes

 Author
   J. Edward Carryer, 08/10/11, 13:29
****************************************************************************/
bool ES_IsQueueEmpty( ES_Event * pBlock )
{
   pQueue_t pThisQueue;

   pThisQueue = (pQueue_t)pBlock;
   return(pThisQueue->NumEntries == 0);
}

#if 0
/****************************************************************************
 Function
   QueueFlushQueue
 Parameters
   unsigned char * pBlock : pointer to the block of memory in use as the Queue
 Returns
   nothing
 Description
   flushes the Queue by reinitializing the indecies
 Notes

 Author
   J. Edward Carryer, 08/12/06, 19:24
****************************************************************************/
void QueueFlushQueue( uint8_t * pBlock )
{
   pQueue_t pThisQueue;
   // doing this with a Queue structure is not strictly necessary
   // but makes it clearer what is going on.
   pThisQueue = (pQueue_t)pBlock;
   pThisQueue->CurrentIndex = 0;
   pThisQueue->NumEntries = 0;
   return;
}


#endif
/***************************************************************************
 private functions
 ***************************************************************************/
#ifdef TEST

#include <stdio.h>
#include "ES_General.h"

static ES_Event TestQueue[3+1];
volatile  uint8_t NumLeft; // for debugging visibility

void main(void){
  ES_Event MyEvent;
  bool bReturn;
  
  ES_InitQueue( TestQueue, ARRAY_SIZE(TestQueue) );
  MyEvent.EventType = 0;
  MyEvent.EventParam = 1;
  bReturn = ES_EnQueueFIFO( TestQueue, MyEvent );
  bReturn +=1; // keep that sily optimizer away
  
    // Try stuffing one on using the LIFO rule
  MyEvent.EventType = 10;
  MyEvent.EventParam = 11;
  bReturn = ES_EnQueueLIFO( TestQueue, MyEvent );
  bReturn +=1; // keep that sily optimizer away
  
  // at this point, the events in the queue should be 11,0
  // so pull off the 11, leaving 1 entry
  NumLeft = ES_DeQueue( TestQueue, &MyEvent);
  if ( NumLeft != 1)
    bReturn = 0;  

  MyEvent.EventType = 2;
  MyEvent.EventParam = 3;
  bReturn = ES_EnQueueFIFO( TestQueue, MyEvent );
  bReturn +=1; // keep that sily optimizer away
  
  MyEvent.EventType = 4;
  MyEvent.EventParam = 5;
  bReturn = ES_EnQueueFIFO( TestQueue, MyEvent );
  bReturn +=1; // keep that sily optimizer away
  
  // queue is now full so this one should fail
  MyEvent.EventType = 6;
  MyEvent.EventParam = 7;
  bReturn = ES_EnQueueFIFO( TestQueue, MyEvent );
  bReturn +=1; // keep that sily optimizer away
  
  // at this point, the events in the queue should be 0,2,4
  // so pull off the 0, leaving 2 entries
  NumLeft = ES_DeQueue( TestQueue, &MyEvent);
  if ( NumLeft != 2)
    bReturn = 0;  
  // Try stuffing one on using the LIFO rule
  MyEvent.EventType = 8;
  MyEvent.EventParam = 9;
  bReturn = ES_EnQueueLIFO( TestQueue, MyEvent );
  bReturn +=1; // keep that sily optimizer away
  
  // at this point, the events in the queue should be 8,2,4
  // so pull off the 8, leaving 2 entries
  NumLeft = ES_DeQueue( TestQueue, &MyEvent);
  NumLeft += 3; //to keep the compiler from optimizing away the last save
  
  while(1)
    ;
}

#endif
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

























































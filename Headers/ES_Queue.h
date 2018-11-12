/****************************************************************************
 Module
     ES_Queue.h
 Description
     header file for use with the Queue functions of the Events  & Services
     Framework
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 08/05/13 15:19 jec      modifications to suit new portable type definitions
 01/15/12 09:36 jec      converted to use new types from ES_Types.h
 10/17/11 07:49 jec      new header to match the rest of the framework
 08/09/11 09:30 jec      started coding
*****************************************************************************/
#ifndef ES_Queue_H
#define ES_Queue_H

#include "ES_Types.h"
#include "ES_Events.h"

/* prototypes for public functions */

uint8_t ES_InitQueue(ES_Event_t *pBlock, uint8_t BlockSize);
bool ES_EnQueueFIFO(ES_Event_t *pBlock, ES_Event_t Event2Add);
bool ES_EnQueueLIFO(ES_Event_t *pBlock, ES_Event_t Event2Add);
uint8_t ES_DeQueue(ES_Event_t *pBlock, ES_Event_t *pReturnEvent);
//void EF_FlushQueue( unsigned char * pBlock );
bool ES_IsQueueEmpty(ES_Event_t *pBlock);

#endif /*ES_Queue_H */


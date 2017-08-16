/****************************************************************************
 Module
     EF_Framework.c
 Description
     source file for the core functions of the Events & Services framework
 Notes
     
 History
 When           Who     What/Why
 -------------- ---     --------
 11/02/13 17:05 jec      added PostToServiceLIFO function
 10/21/13 17:50 jec      added entries to expand number of possible services to 
                         16
 08/13/13 12:59 jec      re-structured ES_Run() slightly to call hardware
                         specific routine to process pending interrupts. This
                         removes the need for re-entrant code and critial
                         regions in the queue code.
 08/06/13 14:50 jec      removed references to PostKeyFunc. We moved that out
                         of the framework into the EventCheckers module
 01/30/12 19:31 jec      moved call to ES_InitTimers into the ES_Initialize
                         this required adding a parameter to ES_Initialize.
 01/15/12 12:15 jec      major revision for Gen2
 10/17/11 12:24 jec      started coding
*****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Queue.h"
#include "ES_LookupTables.h"
#include <stdio.h>

// Include the header files for the Service modules.
// This gets you the prototypes for the public service functions.

#include "ES_ServiceHeaders.h"


/*----------------------------- Module Defines ----------------------------*/
typedef bool InitFunc_t( uint8_t Priority );
typedef ES_Event RunFunc_t( ES_Event ThisEvent );

typedef InitFunc_t * pInitFunc;
typedef RunFunc_t * pRunFunc;

#define NULL_INIT_FUNC ((pInitFunc)0)

typedef struct {
    InitFunc_t *InitFunc;    // Service Initialization function
    RunFunc_t *RunFunc;      // Service Run function
}ES_ServDesc_t;

typedef struct {
    ES_Event *pMem;       // pointer to the memory
    uint8_t Size;      // how big is it
}ES_QueueDesc_t;

/*---------------------------- Module Functions ---------------------------*/
//static bool CheckSystemEvents( void );

/*---------------------------- Module Variables ---------------------------*/
/****************************************************************************/
// You fill in this array with the names of the service init & run functions
// for each service that you use.
// The order is: InitFunction, RunFunction
// The first entry, at index 0, is the lowest priority, with increasing
// priority with higher indices

static ES_ServDesc_t const ServDescList[] =
{ {SERV_0_INIT, SERV_0_RUN } /* lowest priority  always present */
#if NUM_SERVICES > 1
  ,{SERV_1_INIT, SERV_1_RUN }
#endif
#if NUM_SERVICES > 2
  ,{SERV_2_INIT, SERV_2_RUN }
#endif
#if NUM_SERVICES > 3
  ,{SERV_3_INIT, SERV_3_RUN }
#endif
#if NUM_SERVICES > 4
  ,{SERV_4_INIT, SERV_4_RUN }
#endif
#if NUM_SERVICES > 5
  ,{SERV_5_INIT, SERV_5_RUN }
#endif
#if NUM_SERVICES > 6
  ,{SERV_6_INIT, SERV_6_RUN }
#endif
#if NUM_SERVICES > 7
  ,{SERV_7_INIT, SERV_7_RUN }
#endif
#if NUM_SERVICES > 8
  ,{SERV_8_INIT, SERV_8_RUN }
#endif
#if NUM_SERVICES > 9
  ,{SERV_9_INIT, SERV_9_RUN }
#endif
#if NUM_SERVICES > 10
  ,{SERV_10_INIT, SERV_10_RUN }
#endif
#if NUM_SERVICES > 11
  ,{SERV_11_INIT, SERV_11_RUN }
#endif
#if NUM_SERVICES > 12
  ,{SERV_12_INIT, SERV_12_RUN }
#endif
#if NUM_SERVICES > 13
  ,{SERV_13_INIT, SERV_13_RUN }
#endif
#if NUM_SERVICES > 14
  ,{SERV_14_INIT, SERV_14_RUN }
#endif
#if NUM_SERVICES > 15
  ,{SERV_15_INIT, SERV_15_RUN }
#endif

};


/****************************************************************************/
// The queues for the services

static ES_Event Queue0[SERV_0_QUEUE_SIZE+1];
#if NUM_SERVICES > 1
static ES_Event Queue1[SERV_1_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 2
static ES_Event Queue2[SERV_2_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 3
static ES_Event Queue3[SERV_3_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 4
static ES_Event Queue4[SERV_4_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 5
static ES_Event Queue5[SERV_5_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 6
static ES_Event Queue6[SERV_6_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 7
static ES_Event Queue7[SERV_7_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 8
static ES_Event Queue8[SERV_8_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 9
static ES_Event Queue9[SERV_9_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 10
static ES_Event Queue10[SERV_10_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 11
static ES_Event Queue11[SERV_11_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 12
static ES_Event Queue12[SERV_12_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 13
static ES_Event Queue13[SERV_13_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 14
static ES_Event Queue14[SERV_14_QUEUE_SIZE+1];
#endif
#if NUM_SERVICES > 15
static ES_Event Queue15[SERV_15_QUEUE_SIZE+1];
#endif

/****************************************************************************/
// array of queue descriptors for posting by priority level

static ES_QueueDesc_t const EventQueues[NUM_SERVICES] = { 
  { Queue0, ARRAY_SIZE(Queue0) } 
#if NUM_SERVICES > 1
, { Queue1, ARRAY_SIZE(Queue1) }
#endif
#if NUM_SERVICES > 2
, { Queue2, ARRAY_SIZE(Queue2) }
#endif
#if NUM_SERVICES > 3
, { Queue3, ARRAY_SIZE(Queue3) }
#endif
#if NUM_SERVICES > 4
, { Queue4, ARRAY_SIZE(Queue4) }
#endif
#if NUM_SERVICES > 5
, { Queue5, ARRAY_SIZE(Queue5) }
#endif
#if NUM_SERVICES > 6
, { Queue6, ARRAY_SIZE(Queue6) }
#endif
#if NUM_SERVICES > 7
, { Queue7, ARRAY_SIZE(Queue7) }
#endif
#if NUM_SERVICES > 8
, { Queue8, ARRAY_SIZE(Queue8) }
#endif
#if NUM_SERVICES > 9
, { Queue9, ARRAY_SIZE(Queue9) }
#endif
#if NUM_SERVICES > 10
, { Queue10, ARRAY_SIZE(Queue10) }
#endif
#if NUM_SERVICES > 11
, { Queue11, ARRAY_SIZE(Queue11) }
#endif
#if NUM_SERVICES > 12
, { Queue12, ARRAY_SIZE(Queue12) }
#endif
#if NUM_SERVICES > 13
, { Queue13, ARRAY_SIZE(Queue13) }
#endif
#if NUM_SERVICES > 14
, { Queue14, ARRAY_SIZE(Queue14) }
#endif
#if NUM_SERVICES > 15
, { Queue15, ARRAY_SIZE(Queue15) }
#endif
};

/****************************************************************************/
// Variable used to keep track of which queues have events in them

uint16_t Ready;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
   ES_Initialize
 Parameters
   TimerRate_t the rate at which to initialize the framework timer subsystem
 Returns
   ES_Return_t : FailedPointer if any of the function pointers are NULL
                 FailedInit if any of the initialization functions failed
 Description
   Initialize all the services and tests for NULL pointers in the array
 Notes

 Author
   J. Edward Carryer, 10/23/11,
****************************************************************************/
ES_Return_t ES_Initialize( TimerRate_t NewRate ){
  uint8_t i;
  ES_Timer_Init( NewRate); // start up the timer subsystem
  // loop through the list testing for NULL pointers and
  for ( i=0; i< ARRAY_SIZE(ServDescList); i++) {
    if ( (ServDescList[i].InitFunc == (pInitFunc)0) ||
         (ServDescList[i].RunFunc == (pRunFunc)0) )
      return FailedPointer; // protect against NULL pointers
    // and initializing the event queues (must happen before running inits)  
    ES_InitQueue( EventQueues[i].pMem, EventQueues[i].Size );
   // executing the init functions
    if ( ServDescList[i].InitFunc(i) != true )
      return FailedInit; // this is a failed initialization
  }
  return Success;
}

/****************************************************************************
 Function
   ES_Run
 Parameters
   None
 Returns
   ES_Return_t : FailedRun is any of the run functions failed during execution
 Description
   This is the main framework function. It searches through the state
   machines to find one with a non-empty queue and then executes the
   state machine to process the event in its queue.
   while all the queues are empty, it searches for system generated or
   user generated events.
 Notes
   this function only returns in case of an error
 Author
   J. Edward Carryer, 10/23/11,
****************************************************************************/
ES_Return_t ES_Run( void ){
  // make these static to improve speed
  uint8_t HighestPrior;
  static ES_Event ThisEvent;
  
  while(1){ // stay here unless we detect an error condition

    // loop through the list executing the run functions for services
    // with a non-empty queue. Process any pending ints before testing
    // Ready
    while( (_HW_Process_Pending_Ints()) && (Ready != 0)){
      HighestPrior =  ES_GetMSBitSet(Ready);
      if ( ES_DeQueue( EventQueues[HighestPrior].pMem, &ThisEvent ) == 0 ){
        Ready &= BitNum2ClrMask[HighestPrior]; // mark queue as now empty
      }
      if( ServDescList[HighestPrior].RunFunc(ThisEvent).EventType != 
                                                              ES_NO_EVENT) {
              return FailedRun;
      }
    }

    // all the queues are empty, so look for new user detected events
    ES_CheckUserEvents();
  }
}

/****************************************************************************
 Function
   ES_PostAll
 Parameters
   ES_Event : The Event to be posted
 Returns
   boolean : False if any of the post functions failed during execution
 Description
   posts to all of the services' queues 
 Notes

 Author
   J. Edward Carryer, 01/15/12,
****************************************************************************/
bool ES_PostAll( ES_Event ThisEvent){

  uint8_t i;
  // loop through the list executing the post functions
  for ( i=0; i< ARRAY_SIZE(EventQueues); i++) {
    if ( ES_EnQueueFIFO( EventQueues[i].pMem, ThisEvent ) != true ){
      break; // this is a failed post
    }else{
      Ready |= BitNum2SetMask[i]; // show queue as non-empty
    }
  }
  if ( i == ARRAY_SIZE(EventQueues) ){ // if no failures
    return (true);
  }else{
    return(false);
  }
}

/****************************************************************************
 Function
   ES_PostToService
 Parameters
   uint8_t : Which service to post to (index into ServDescList)
   ES_Event : The Event to be posted
 Returns
   boolean : False if the post function failed during execution
 Description
   posts to one of the services' queues
 Notes
   used by the timer library to associate a timer with a state machine
 Author
   J. Edward Carryer, 01/16/12,
****************************************************************************/
bool ES_PostToService( uint8_t WhichService, ES_Event TheEvent){
  if ((WhichService < ARRAY_SIZE(EventQueues)) &&
      (ES_EnQueueFIFO( EventQueues[WhichService].pMem, TheEvent) == 
                                                                true )){
    Ready |= BitNum2SetMask[WhichService]; // show queue as non-empty
    return true;
  } else
    return false;
}

/****************************************************************************
 Function
   ES_PostToServiceLIFO
 Parameters
   uint8_t : Which service to post to (index into ServDescList)
   ES_Event : The Event to be posted
 Returns
   boolean : False if the post function failed during execution
 Description
   Posts, using LIFO strategy, to one of the services' queues
 Notes
   used by the Defer/Recall event capability
 Author
   J. Edward Carryer, 11/02/13
****************************************************************************/
bool ES_PostToServiceLIFO( uint8_t WhichService, ES_Event TheEvent){
  if ((WhichService < ARRAY_SIZE(EventQueues)) &&
      (ES_EnQueueLIFO( EventQueues[WhichService].pMem, TheEvent) == 
                                                                true )){
    Ready |= BitNum2SetMask[WhichService]; // show queue as non-empty
    return true;
  } else
    return false;
}

//*********************************
// private functions
//*********************************
#if 0
/****************************************************************************
 Function
   CheckSystemEvents
 Parameters
   None
 Returns
   boolean : True if a system event was detected
 Description
   check for system generated events and uses pPostKeyFunc to post to one
   of the state machine's queues
 Notes
   currently only tests for incoming keystrokes
 Author
   J. Edward Carryer, 10/23/11, 
****************************************************************************/
static bool CheckSystemEvents( void ){
  
  if ( kbhit() != 0 ) // new key waiting?
  {
    ES_Event ThisEvent;
    ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = getchar();
    (*pPostKeyFunc)( ThisEvent );
    return true;
  }
  return false;
}
#endif
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

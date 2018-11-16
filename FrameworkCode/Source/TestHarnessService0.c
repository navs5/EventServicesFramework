/****************************************************************************
 Module
   TestHarnessService0.c

 Revision
   1.0.1

 Description
   This is the first service for the Test Harness under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/26/17 18:26 jec     moves definition of ALL_BITS to ES_Port.h
 10/19/17 21:28 jec     meaningless change to test updating
 10/19/17 18:42 jec     removed referennces to driverlib and programmed the
                        ports directly
 08/21/17 21:44 jec     modified LED blink routine to only modify bit 3 so that
                        I can test the new new framework debugging lines on PF1-2
 08/16/17 14:13 jec      corrected ONE_SEC constant to match Tiva tick rate
 11/02/13 17:21 jec      added exercise of the event deferral/recall module
 08/05/13 20:33 jec      converted to test harness service
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// This module
#include "TestHarnessService0.h"

// Hardware
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "ES_Port.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)

#define ENTER_POST     ((MyPriority<<3)|0)
#define ENTER_RUN      ((MyPriority<<3)|1)
#define ENTER_TIMEOUT  ((MyPriority<<3)|2)

// #define ALL_BITS (0xff<<2)   Moved to ES_Port.h
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

static void InitLED(void);
static void BlinkLED(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
// add a deferral queue for up to 3 pending deferrals +1 to allow for ovehead
static ES_Event_t DeferralQueue[3 + 1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTestHarnessService0

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitTestHarnessService0(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  // initialize deferral queue for testing Deferal function
  ES_InitDeferralQueueWith(DeferralQueue, ARRAY_SIZE(DeferralQueue));
  // initialize LED drive for testing/debug output
  InitLED();
  // initialize the Short timer system for channel A
  ES_ShortTimerInit(MyPriority, SHORT_TIMER_UNUSED);

#ifdef _INCLUDE_BYTE_DEBUG_
  // initialize the byte-wide debugging
  _HW_ByteDebug_Init();
#endif  

  // set up I/O lines for debugging
  // enable the clock to Port B
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
  // kill a few cycles to let the peripheral clock get going
  while ((HWREG(SYSCTL_PRGPIO) & BIT1HI) != BIT1HI)
  {}
  // Enable pins for digital I/O
  HWREG(GPIO_PORTB_BASE + GPIO_O_DEN) |= (BIT2HI);

  // make pin 2 on Port B into outputs
  HWREG(GPIO_PORTB_BASE + GPIO_O_DIR) |= (BIT2HI);
  // start with the lines low
  HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT2LO;

  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/****************************************************************************
 Function
     PostTestHarnessService0

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostTestHarnessService0(ES_Event_t ThisEvent)
{
#ifdef _INCLUDE_BYTE_DEBUG_
  _HW_ByteDebug_SetValueWithStrobe( ENTER_POST );
  _HW_ByteDebug_SetValueWithStrobe( END_SERVICE );
#endif  
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTestHarnessService0

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunTestHarnessService0(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  static char DeferredChar = '1';

#ifdef _INCLUDE_BYTE_DEBUG_
  _HW_ByteDebug_SetValueWithStrobe( ENTER_RUN );
#endif  
  switch (ThisEvent.EventType)
  {
    case ES_INIT:
    {
      ES_Timer_InitTimer(SERVICE0_TIMER, HALF_SEC);
      puts("Service 00:");
      printf("\rES_INIT received in Service %d\r\n", MyPriority);
    }
    break;
    case ES_TIMEOUT:   // re-start timer & announce
    {
#ifdef _INCLUDE_BYTE_DEBUG_
      _HW_ByteDebug_SetValueWithStrobe( ENTER_TIMEOUT );
#endif  
      ES_Timer_InitTimer(SERVICE0_TIMER, FIVE_SEC);
     printf("ES_TIMEOUT received from Timer %d in Service %d\r\n",
          ThisEvent.EventParam, MyPriority);
      BlinkLED();
    }
    break;
    case ES_SHORT_TIMEOUT:   // lower the line & announce
    {
      HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT2LO;
      puts("ES_SHORT_TIMEOUT received");
    }
    break;
    case ES_NEW_KEY:   // announce
    {
      printf("ES_NEW_KEY received with -> %c <- in Service 0\r\n",
          (char)ThisEvent.EventParam);
      if ('d' == ThisEvent.EventParam)
      {
        ThisEvent.EventParam = DeferredChar++;   //
        if (ES_DeferEvent(DeferralQueue, ThisEvent))
        {
          puts("ES_NEW_KEY deferred in Service 0\r");
        }
      }
      if ('r' == ThisEvent.EventParam)
      {
        ThisEvent.EventParam = 'Q';   // This one gets posted normally
        ES_PostToService(MyPriority, ThisEvent);
        // but we slide the deferred events under it so it(they) should come out first
        if (true == ES_RecallEvents(MyPriority, DeferralQueue))
        {
          puts("ES_NEW_KEY(s) recalled in Service 0\r");
          DeferredChar = '1';
        }
      }
      if ('p' == ThisEvent.EventParam)
      {
        ES_ShortTimerStart(TIMER_A, 10);
        // raise the line to show we started
        HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT2HI;
        //puts("Pulsed!\r");
      }
    }
    break;
    default:
    {}
     break;
  }
#ifdef _INCLUDE_BYTE_DEBUG_
  _HW_ByteDebug_SetValueWithStrobe( END_SERVICE );
#endif  

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void InitLED(void)
{
  // enable the clock to Port F
        HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;
  // kill a few cycles to let the peripheral clock get going
  while ((HWREG(SYSCTL_PRGPIO) & BIT5HI) != BIT5HI)
  {}
  // Enable pins for digital I/O
        HWREG(GPIO_PORTF_BASE + GPIO_O_DEN) |= (BIT3HI);

  // make pin 3 on Port F into outputs
        HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) |= (BIT3HI);
}

static void BlinkLED(void)
{
  static uint8_t LEDvalue = 8;

  // toggle state of LED
        HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) ^= LEDvalue;

  //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDvalue);
  // Cycle through Red, Green and Blue LEDs
//	if (LEDvalue == 8)
//		{LEDvalue = 2;}
//	else
//		{LEDvalue = LEDvalue*2;}
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


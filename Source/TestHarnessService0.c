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
 11/02/13 17:21 jec      added exercise of the event deferral/recall module
 08/05/13 20:33 jec      converted to test harness service
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "TestHarnessService0.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
#include "ES_ShortTimer.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

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
static ES_Event DeferralQueue[3+1];

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
bool InitTestHarnessService0 ( uint8_t Priority )
{
  ES_Event ThisEvent;
  
  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
	// initialize deferral queue for testing Deferal function
  ES_InitDeferralQueueWith( DeferralQueue, ARRAY_SIZE(DeferralQueue) );
	// initialize LED drive for testing/debug output
	InitLED();
  // initialize the Short timer system for channel A
  ES_ShortTimerInit(MyPriority, SHORT_TIMER_UNUSED);

  // set up I/O lines for debugging
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);
  // start with the lines low
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, BIT2LO);  

  
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
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
bool PostTestHarnessService0( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
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
ES_Event RunTestHarnessService0( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	static char DeferredChar = '1';
  
  switch (ThisEvent.EventType){
    case ES_INIT :
      ES_Timer_InitTimer(SERVICE0_TIMER, HALF_SEC);
      puts("Service 00:");
      printf("\rES_INIT received in Service %d\r\n", MyPriority);
      break;
    case ES_TIMEOUT :  // re-start timer & announce
      ES_Timer_InitTimer(SERVICE0_TIMER, FIVE_SEC);
      printf("ES_TIMEOUT received from Timer %d in Service %d\r\n", 
              ThisEvent.EventParam, MyPriority);
			BlinkLED();
      break;
    case ES_SHORT_TIMEOUT :  // lower the line & announce
      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, BIT2LO);  
      puts("ES_SHORT_TIMEOUT received");
      break;
    case ES_NEW_KEY :  // announce
      printf("ES_NEW_KEY received with -> %c <- in Service 0\r\n", 
              (char)ThisEvent.EventParam);
      if( 'd' == ThisEvent.EventParam ){
          ThisEvent.EventParam = DeferredChar++; //
          if (ES_DeferEvent( DeferralQueue, ThisEvent )){
            puts("ES_NEW_KEY deferred in Service 0\r");
          }
      }
      if( 'r' == ThisEvent.EventParam ){
          ThisEvent.EventParam = 'Q'; // This one gets posted normally
          ES_PostToService( MyPriority, ThisEvent);
          // but we slide the deferred events under it so it(they) should come out first
          if ( true == ES_RecallEvents( MyPriority, DeferralQueue )){
            puts("ES_NEW_KEY(s) recalled in Service 0\r");
					DeferredChar = '1';
          }
      }
     if( 'p' == ThisEvent.EventParam ){
          ES_ShortTimerStart(TIMER_A, 10);
          // raise the line to show we started
          GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, BIT2HI);  
          //puts("Pulsed!\r");
      }
      break;
    default :
      break;
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void InitLED(void)
{
	volatile uint32_t Dummy;
	// enable the clock to Port F
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;
	// kill a few cycles to let the peripheral clock get going
	Dummy = HWREG(SYSCTL_RCGCGPIO);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	
	// Enable pins for digital I/O
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	
	// make pins 1,2 & 3 on Port F into outputs
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	//GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}


static void BlinkLED(void)
{
	static uint8_t LEDvalue = 2;
	
	// Turn off all of the LEDs
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + (0xff<<2))) &= ~(GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	// Turn on the new LEDs
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + (0xff<<2))) |= LEDvalue;
	
	//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LEDvalue);
	// Cycle through Red, Green and Blue LEDs
	if (LEDvalue == 8) 
		{LEDvalue = 2;} 
	else 
		{LEDvalue = LEDvalue*2;}

}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


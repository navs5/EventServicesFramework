/****************************************************************************
 Module
   ES_Port.c

 Revision
   1.0.1

 Description
   This is the sample file to demonstrate adding the hardware specific
   functions to the Events & Services Framework. This sample is also used
   as the file for the port to the Freescale MC9S12C32 processor.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 08/21/17 13:47 jec     added functions to init 2 lines for debugging the framework
                        and functions to set & clear those lines.
 03/13/14 10:30	joa		  Updated files to use with Cortex M4 processor core.
                        Specifically, this was tested on a TI TM4C123G mcu.
 03/05/14 13:20	joa		  Began port for TM4C123G
 08/13/13 12:42 jec     moved the hardware specific aspects of the timer here
 08/06/13 13:17 jec     Began moving the stuff from the V2 framework files
 ***************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "driverlib/rom.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ssi.h"
#include "inc/hw_sysctl.h"
#include "inc\tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"  // Define PART_TM4C123GH6PM in project
#include "driverlib/systick.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "utils/uartstdio.h"

#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Timers.h"

#define UART_PORT 0
#define UART_BAUD 115200UL
#define SRC_CLK_FREQ 16000000UL
#define CLK_FREQ 40000000UL

// change the base address for the debug lines here
#define DEBUG_PORT GPIO_PORTF_BASE

// bit to use when enabling the port for debugging,
// must match base address from DEBUG_PORT definition above
// BIT0HI = PortA, BIT1HI = PortB, BIT2HI = PortC, BIT3HI = PortD,
// BIT4HI = PortE, BIT5HI = PortF
#define DEBUG_PORT_ENABLE_BIT BIT5HI

// which bits on the port are to be used for debugging
#define DEBUG_PORT_WHICH_BITS (BIT1HI | BIT2HI)

// define wihch bit corresponds to which debugging line
#define DEBUG_LINE_1 BIT1HI
#define DEBUG_LINE_2 BIT2HI

// used to set CPSDVSR on SSI1, large, even value for debugging, 2 for production
#define BYTE_DEBUG_SSI1__DIVISOR 2

// TickCount is used to track the number of timer ints that have occurred
// since the last check. It should really never be more than 1, but just to
// be sure, we increment it in the interrupt response rather than simply
// setting a flag. Using this variable and checking approach we remove the
// need to post events from the interrupt response routine. This is necessary
// for compilers like HTC for the midrange PICs which do not produce re-entrant
// code so cannot post directly to the queues from within the interrupt resp.
static volatile uint8_t TickCount;

// Global tick count to monitor number of SysTick Interrupts
// make uint16_t to maintain backwards compatibility and not overly burden
// 8 and 16 bit processors
static volatile uint16_t SysTickCounter = 0;

// This variable is used to store the state of the interrupt mask when
// doing EnterCritical/ExitCritical pairs
uint32_t _PRIMASK_temp;

// Shadow of Debug port expander contents to allow setting & clearing bits
// since we can not read it back ('595 is write only)

static uint8_t ByteDebugPortShadow = 0;

/****************************************************************************
 Function
     _HW_Timer_Init
 Parameters
     unsigned char Rate set to one of the TMR_RATE_XX values to set the
     Tick rate
 Returns
     None.
 Description
     Initializes the timer module by setting up the SysTick Interrupt with the requested
    rate
 Notes
     modify as required to port to other timer hardware
 Author
     J. Edward Carryer, 02/24/97 14:23
     John Alabi, 03/05/14 13:22
****************************************************************************/
void _HW_Timer_Init(TimerRate_t Rate)
{
  ROM_SysTickPeriodSet(Rate); /* Set the SysTick Interrupt Rate */
  ROM_SysTickIntEnable();     /* Enable the SysTick Interrupt */
  ROM_SysTickEnable();        /* Enable SysTick */
  ROM_IntMasterEnable();      /* Make sure interrupts are enabled */
}

/****************************************************************************
 Function
     SysTickIntHandler
 Parameters
     none
 Returns
     None.
 Description
     interrupt response routine for the tick interrupt that will allow the
     framework timers to run.
 Notes
     As currently (8/13/13) implemented this does not actually post events
     but simply sets a flag to indicate that the interrupt has occurred.
     the framework response is handled below in _HW_Process_Pending_Ints
 Author
    John Alabi, 03/05/14 13:50
****************************************************************************/
void SysTickIntHandler(void)
{
  /* Interrupt automatically cleared by hardware */
  ++TickCount;          /* flag that it occurred and needs a response */
  ++SysTickCounter;     // keep the free running time going
#ifdef LED_DEBUG
  BlinkLED();
#endif
}

/****************************************************************************
 Function
    _HW_GetTickCount()
 Parameters
    none
 Returns
    uint16_t   count of number of system ticks that have occurred.
 Description
    wrapper for access to SysTickCounter, needed to move increment of tick
    counter to this module to keep the timer ticking during blocking code
 Notes

 Author
    Ed Carryer, 10/27/14 13:55
****************************************************************************/
uint16_t _HW_GetTickCount(void)
{
  return SysTickCounter;
}

/****************************************************************************
 Function
     _HW_Process_Pending_Ints
 Parameters
     none
 Returns
     always true.
 Description
     processes any pending interrupts (actually the hardware interrupt already
     occurred and simply set a flag to tell this routine to execute the non-
     hardware response)
 Notes
     While this routine technically does not need a return value, we always
     return true so that it can be used in the conditional while() loop in
     ES_Run. This way the test for pending interrupts get processed after every
     run function is called and even when there are no queues with events.
     This routine could be expanded to process any other interrupt sources
     that you would like to use to post events to the framework services.
 Author
     J. Edward Carryer, 08/13/13 13:27
****************************************************************************/
bool _HW_Process_Pending_Ints(void)
{
  while (TickCount > 0)
  {
    /* call the framework tick response to actually run the timers */
    ES_Timer_Tick_Resp();
    TickCount--;
  }
  return true;  // always return true to allow loop test in ES_Run to proceed
}

/****************************************************************************
 Function
     ConsoleInit
 Parameters
     none
 Returns
     none.
 Description
  TBD
 Notes
  TBD
 Author
     John Alabi, 03/05/14 15:07
 ****************************************************************************/
void ConsoleInit(void)
{
  // Enable designated port that will be used for the UART
  ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOA);

  // Enable the designated UART
  ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_UART0);

  // Configure pins for UART including setup for alternate (UART) functionality
  ROM_GPIOPinConfigure( GPIO_PA0_U0RX);
  ROM_GPIOPinConfigure( GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Select the clock source
  ROM_UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

  // Initialize the UART for console I/O
  UARTStdioConfig(UART_PORT, UART_BAUD, SRC_CLK_FREQ);
}

#if defined(rvmdk) || defined(__ARMCC_VERSION)
uint32_t CPUgetPRIMASK_cpsid(void)
{
  register uint32_t r0;
  __asm
  {
    mrs   r0, PRIMASK;      // Store PRIMASK in r0
    cpsid i;                // Disable interrupts
  }
  return r0;
}

void CPUsetPRIMASK(uint32_t newPRIMASK)
{
  __asm
  {
    msr PRIMASK, newPRIMASK         // Store newPRIMASK in PRIMASK
  }
}

#endif

/****************************************************************************
 Function
     _HW_DebugLines_Init
 Parameters
     none
 Returns
     None.
 Description
     Initializes the port lines for framework/application debugging
 Notes

 Author
     J. Edward Carryer, 08/21/17 11:51
****************************************************************************/
void _HW_DebugLines_Init(void)
{
  // enable clock to the debug port
  HWREG(SYSCTL_RCGCGPIO) |= DEBUG_PORT_ENABLE_BIT;
  while ((HWREG(SYSCTL_PRGPIO) & DEBUG_PORT_ENABLE_BIT) != DEBUG_PORT_ENABLE_BIT)
  {
    ; // wait for port to be ready
  }
  // set the debug pins as digital
  HWREG(DEBUG_PORT + GPIO_O_DEN) |= DEBUG_PORT_WHICH_BITS;
  // set pins as outputs
  HWREG(DEBUG_PORT + GPIO_O_DIR) |= DEBUG_PORT_WHICH_BITS;
}

/****************************************************************************
 Function
     _HW_DebugLines_SetLine1
 Parameters
     none
 Returns
     None.
 Description
     Sets debug Line1 to be a 1
 Notes

 Author
     J. Edward Carryer, 08/21/17 13:23
****************************************************************************/
void _HW_DebugLines_SetLine1(void)
{
  HWREG(DEBUG_PORT + (GPIO_O_DATA + ALL_BITS)) |= DEBUG_LINE_1;
}

/****************************************************************************
 Function
     _HW_DebugLines_ClearLine1
 Parameters
     none
 Returns
     None.
 Description
     Sets debug Line1 to be a 0
 Notes

 Author
     J. Edward Carryer, 08/21/17 13:23
****************************************************************************/
void _HW_DebugLines_ClearLine1(void)
{
  HWREG(DEBUG_PORT + (GPIO_O_DATA + ALL_BITS)) &= ~DEBUG_LINE_1;
}

/****************************************************************************
 Function
     _HW_DebugLines_SetLine2
 Parameters
     none
 Returns
     None.
 Description
     Sets debug Line1 to be a 1
 Notes

 Author
     J. Edward Carryer, 08/21/17 13:25
****************************************************************************/
void _HW_DebugLines_SetLine2(void)
{
  HWREG(DEBUG_PORT + (GPIO_O_DATA + ALL_BITS)) |= DEBUG_LINE_2;
}

/****************************************************************************
 Function
     _HW_DebugLines_ClearLine2
 Parameters
     none
 Returns
     None.
 Description
     Sets debug Line1 to be a 0
 Notes

 Author
     J. Edward Carryer, 08/21/17 13:26
****************************************************************************/
void _HW_DebugLines_ClearLine2(void)
{
  HWREG(DEBUG_PORT + (GPIO_O_DATA + ALL_BITS)) &= ~DEBUG_LINE_2;
}

/****************************************************************************
 Function
     _HW_ByteDebug_Init
 Parameters
     none
 Returns
     None.
 Description
     Initializes SSI1 on PortF to send bytes to an 'HC595 connected there
 Notes
    based on code from SSIDemo.c in project GIT_FrameworkWithSPIDemo
    Does not use the Rx line on SSI1 so PF0 is still free
 Author
     J. Edward Carryer, 07/29/18 15:03
****************************************************************************/
void _HW_ByteDebug_Init( void){
// enable the clock to the GPIO port (PortF for SSI1)
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;

// enable the clock to SSI1
	HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R1;

// wait for the GPIO port to be ready
  while( (HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R5) != SYSCTL_PRGPIO_R5 )
    ;    

// program the GPIO to use the alternate functions on the SSI1 pins(1,2 & 3)
  HWREG(GPIO_PORTF_BASE + GPIO_O_AFSEL) |= 
                     (BIT3HI | BIT2HI | BIT1HI );
  
// select the SSI1 functions on those pins. first clear the fields, then
// set mux position 2 for each of pins 3,2 & 1 to get the SSI functions
  HWREG(GPIO_PORTF_BASE + GPIO_O_PCTL) = (HWREG(GPIO_PORTF_BASE + GPIO_O_PCTL) &
    ~(GPIO_PCTL_PF3_M | GPIO_PCTL_PF2_M | GPIO_PCTL_PF1_M)) | 
      ( GPIO_PCTL_PF3_SSI1FSS | GPIO_PCTL_PF2_SSI1CLK | GPIO_PCTL_PF1_SSI1TX);
  
// program those lines for digital I/O
  HWREG(GPIO_PORTF_BASE + GPIO_O_DEN) |= (BIT3HI | BIT2HI | BIT1HI );
  
// program the required data directions on the port pins 
// Tx(1), Fs(3) & Clk(2) are all outputs  
  HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) = (HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) |
    (BIT3HI | BIT2HI | BIT1HI ));

// Since we are using SPI mode 3, enable the pull-up on the clock line (bit 2)
  HWREG(GPIO_PORTF_BASE + GPIO_O_PUR) |= BIT2HI;
  
// wait for the SSI1 to be ready, it really should be ready by now
  while( (HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R1) != SYSCTL_PRSSI_R1 )
    ;    
  
// make sure the the SSI is disabled before programming mode bits
  HWREG(SSI1_BASE + SSI_O_CR1) &= ~SSI_CR1_SSE;
  
// select master mode (MS=0) with TXRIS indicating End of Transmit (EOT=1)
  HWREG(SSI1_BASE + SSI_O_CR1) = (HWREG(SSI1_BASE + SSI_O_CR1) & 
    ~SSI_CR1_MS) | SSI_CR1_EOT ;

// configure the SSI clock source to the system clock
  HWREG(SSI1_BASE + SSI_O_CC) = 0;
  
// configure the clock pre-scaler
  HWREG(SSI1_BASE + SSI_O_CPSR) = BYTE_DEBUG_SSI1__DIVISOR;
  
// Configure clock rate (SCR=0), Phase & polarity (SPH=0, SPO=0), 
// SPI mode (FRF=0), data size to 8 bits (DSS)
// SPH=0, SPO=0 is required to get the FS line to pulse on every byte
// and have the rising edge of the clock in the center of the valid data
// for latching by the '595
  HWREG(SSI1_BASE + SSI_O_CR0) =  (HWREG(SSI1_BASE + SSI_O_CR0) & 
    ~(SSI_CR0_SCR_M | SSI_CR0_FRF_M | SSI_CR0_DSS_M | SSI_CR0_SPH | 
      SSI_CR0_SPO )) | (SSI_CR0_DSS_8);

// make sure the the SSI is enabled for operation
  HWREG(SSI1_BASE + SSI_O_CR1) |= SSI_CR1_SSE;
  
// set all of the output lines lo
  HWREG(SSI1_BASE + SSI_O_DR) = 0;

}

/****************************************************************************
 Function
     _HW_ByteDebug_ClearBit( uint8_t WhichBit )
 Parameters
     uint8_t WhichBit, specifies which bit to clear
 Returns
     None.
 Description
     Sets specified bit position on the debug port expander to be a 0 and
     updates the contents of the Shadow copy of the port contents
 Notes

 Author
     J. Edward Carryer, 07/29/18 15:23
****************************************************************************/
void _HW_ByteDebug_ClearBit( uint8_t WhichBit ){
  
// update the shadow register contents and write the new data to the SSI 
// data register
    ByteDebugPortShadow &= ~(BIT0HI << WhichBit);
  // make sure that there is room in the FIFO, then write new data
  while(0 == (HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_TNF))
    ;
  HWREG(SSI1_BASE + SSI_O_DR) = ByteDebugPortShadow;
}

/****************************************************************************
 Function
     _HW_ByteDebug_SetBit( uint8_t WhichBit )
 Parameters
     uint8_t WhichBit, specifies which bit to set
 Returns
     None.
 Description
     Sets specified bit position on the debug port expander to be a 1 and
     updates the contents of the Shadow copy of the port contents
 Notes

 Author
     J. Edward Carryer, 07/29/18 15:33
****************************************************************************/
void _HW_ByteDebug_SetBit( uint8_t WhichBit ){
  
// update the shadow register contents and write the new data to the SSI 
// data register
    ByteDebugPortShadow |= (BIT0HI << WhichBit);
  // make sure that there is room in the FIFO, then write new data
  while(0 == (HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_TNF))
    ;
  HWREG(SSI1_BASE + SSI_O_DR) = ByteDebugPortShadow;
}

/****************************************************************************
 Function
     _HW_ByteDebug_SetValueWithStrobe( uint8_t NewValue )
 Parameters
     uint8_t NewValue, specifies the new value (7 bits) to be placed on the
                       debug expansion port
 Returns
     None.
 Description
     Places a 7-bit value on bits 0-6 of the debug port expander and
     pulses bit 7 to synchronize the update.
 Notes

 Author
     J. Edward Carryer, 07/30/18 09:50
****************************************************************************/
void _HW_ByteDebug_SetValueWithStrobe( uint8_t NewValue ){
  
// update the shadow register contents and write the new data to the SSI 
// data register first with bit 7 hi, then with bit 7 lo
    ByteDebugPortShadow = NewValue;
  // make sure that there is room in the FIFO, then write new data with B7 hi
  while(0 == (HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_TNF))
      ;
  HWREG(SSI1_BASE + SSI_O_DR) = ByteDebugPortShadow | BIT7HI;
  
  // make sure that there is room in the FIFO, then write new data with B7 lo
  while(0 == (HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_TNF))
      ;
  HWREG(SSI1_BASE + SSI_O_DR) = ByteDebugPortShadow & BIT7LO;
  
}

/****************************************************************************
 Function
     _HW_ByteDebug_SetValue( uint8_t NewValue )
 Parameters
     uint8_t NewValue, specifies the new value (8 bits) to be placed on the
                       debug expansion port
 Returns
     None.
 Description
     Places an 8-bit value on bits 0-7 of the debug port expander.
 Notes

 Author
     J. Edward Carryer, 11/12/18 18:59
****************************************************************************/
void _HW_ByteDebug_SetValue( uint8_t NewValue ){
  
// update the shadow register contents and write the new data to the SSI 
// data register
    ByteDebugPortShadow = NewValue;
  // make sure that there is room in the FIFO, then write new data
  while(0 == (HWREG(SSI1_BASE + SSI_O_SR) & SSI_SR_TNF))
      ;
  HWREG(SSI1_BASE + SSI_O_DR) = ByteDebugPortShadow;
    
}

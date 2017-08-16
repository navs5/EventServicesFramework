#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "termio.h"
#include "uartstdio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"

#define PORT_NUM			0
#define UART_BASE			UART0_BASE
#define SYSCTL_PERIPH_UART	SYSCTL_PERIPH_UART0
#define SYSCTL_PERIPH_GPIO	SYSCTL_PERIPH_GPIOA
#define GPIO_RX_PIN_ADDR 	GPIO_PA0_U0RX
#define GPIO_TX_PIN_ADDR 	GPIO_PA1_U0TX
#define GPIO_PORT_BASE		GPIO_PORTA_BASE
#define GPIO_RX_PIN			GPIO_PIN_0
#define	GPIO_TX_PIN			GPIO_PIN_1

#define UART_BAUD		115200UL
#define SRC_CLK_FREQ	16000000UL
#define CLK_FREQ		40000000UL

unsigned char TERMIO_GetChar(void) {
	// (unsigned char)UARTCharGet(uint32_t ui32Base);
	return UARTgetc();
}

void TERMIO_PutChar(unsigned char ch) {
	/* sends a character to the terminal channel */
	UARTCharPut(UART_BASE, ch);
}

void TERMIO_Init(void) {

	// Enable designated port that will be used for the UART
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOA );

	// Enable the designated UART
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	// Configure pins for UART including setup for alternate (UART) functionality
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Select the clock source
	UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

	// Initialize the UART for console I/O
	UARTStdioConfig(PORT_NUM, UART_BAUD, SRC_CLK_FREQ);

	// Retarget I/O to UART
 #if defined(ccs)
	mapStdioToUart();
 #endif

}

int kbhit(void) {
	/* checks for a character from the terminal channel */
	if(!(HWREG(UART_BASE + UART_O_FR) & UART_FR_RXFE))
		return 1;
	else
		return 0;
}

#if defined(ccs)

#include <file.h>

#define PORT_NUM			0
#define UART_BASE			UART0_BASE

int uart_open(const char *path, unsigned flags, int llv_fd)
{
	return llv_fd;
}

int uart_close(int dev_fd)
{
	return dev_fd;
}

off_t uart_lseek(int dev_fd, off_t offset, int origin)
{
	return -1;
}

int uart_unlink(const char *path)
{
	return -1;
}

int uart_rename(const char * old_name, const char * new_name)
{
	return -1;
}


int uart_write(int dev_fd, const char *buf, unsigned count)
{
	int retVal = count;
	const char *pch = buf;
	if (buf == NULL)
		return -1;
	while(count) {
		UARTCharPut(UART_BASE, *pch++);
		count--;
//		if (UARTCharsAvail(UART_BASE)) {
//			UARTCharPutNonBlocking(UART_BASE, *pch++);
//			count--;
//		}
	}
	return retVal;
}

int uart_read(int dev_fd, char *buf, unsigned count)
{
	if (!count)
		return 0;
	if (buf == NULL)
		return -1;
	char *pch = buf;
	unsigned numRead = 0;
	int retVal = 0;
	while (count) {
		retVal = UARTCharGet(UART_BASE);
		if (retVal == -1)
			return 0;
		*pch++ = retVal;
		numRead++;
		count--;
//		if (UARTCharsAvail(UART_BASE)) {
//			retVal = UARTCharGetNonBlocking(UART_BASE);
//			// if character available is EOF, return 0
//			if (retVal == -1)
//				return 0;
//			*pch++ = retVal;
//			numRead++;
//		}
	}

	return numRead;
}



int mapStdioToUart(void)
{
	// _MSA indicates device supports multiple streams open at a time
	// _SSA indicates device supports only one open stream at a time
	int retVal = add_device("uart", _MSA, uart_open, uart_close, uart_read, uart_write,
				uart_lseek, uart_unlink, uart_rename);
	FILE *fid = fopen("uart", "w");

	if (fid == NULL) {
		puts(" Failed top open uart for C I/O.");
		return -1;
	}

	// Reopen stdout as a uart file
	if (!freopen("uart:", "w", stdout)) {
		puts("Failed to freopen stdout");
		return -2;
	}

	// Turn off buffering for stdout by setting to 0
	// TIRTOS uses line buffering IOLBF (typical for stdout) at 128
    //freopen("UART:0", "w", stdout);
    //setvbuf(stdout, NULL, _IOLBF, 128);
	if (setvbuf(stdout, NULL, _IONBF, 0)) {
		puts("Failed to setvbuf stdout");
		return -3;
	}

	printf("This goes to stdout\n\r");
	puts("puts test to stdout");

	if (!freopen("uart:", "r", stdin)) {
		puts("Failed to freopen stdout");
		return -4;
	}

	if (setvbuf(stdin, NULL, _IONBF, 0)) {
		puts("Failed to setvbuf stdout");
		return -5;
	}

	return 0;
}

#endif

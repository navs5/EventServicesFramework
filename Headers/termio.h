/*
 * termio.h
 *
 *  Created on: Feb 25, 2014
 *      Author: John Alabi
 */

#ifndef TERMIO_H_
#define TERMIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "utils/uartstdio.h"

//#if defined(ccs)
//#define printf	UARTprintf
//#define puts	UARTprintf
//#endif

/* receives character from the terminal channel - BLOCKING */
unsigned char TERMIO_GetChar(void);

/* sends a character to the terminal channel
   wait for output buffer empty */
void TERMIO_PutChar(unsigned char ch);

/* initializes the communication channel */
/* set baud rate to 115.2 kbaud and turn on Rx and Tx */
void TERMIO_Init(void);
/* checks for a character from the terminal channel */
int kbhit(void);

#if defined(ccs)
#include <file.h>

int uart_open(const char *path, unsigned flags, int llv_fd);
int uart_close(int dev_fd);
int uart_read(int dev_fd, char *buf, unsigned count);
int uart_write(int dev_fd, const char *buf, unsigned count);
off_t uart_lseek(int dev_fd, off_t offset, int origin);
int uart_unlink(const char *path);
int uart_rename(const char * old_name, const char * new_name);

int mapStdioToUart(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* TERMIO_H_ */

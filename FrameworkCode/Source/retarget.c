/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/

#include <stdio.h>
#include <time.h>

#if defined(rvmdk)
#include <rt_misc.h>
#pragma import(__use_no_semihosting_swi)

extern void TERMIO_PutChar(unsigned char ch);
extern unsigned char TERMIO_GetChar(void);

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  TERMIO_PutChar(ch);
  return (ch);
}

int fgetc(FILE *f) {
  return (int)TERMIO_GetChar();
}

int ferror( FILE *f ) {
	/* preferred implementation of ferror */
	return EOF;
}

void _ttywrch(int ch) {
	TERMIO_PutChar(ch);
}

void _sys_exit(int return_code) {
	label: goto label; /* Forever loop */
}
#endif

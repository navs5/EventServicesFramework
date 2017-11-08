/****************************************************************************
 Module
     ES_Types.h
 Description
     header file to provide generally used types for the Events  & Services
     Framework. Most of these come from stdint.h, and stdbool.h on compilers
     that support C99
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 08/05/13 14:24 jec      converted to take advantage of C99 compilers if avail
                         and if not to define the subset that we use.
 10/17/11 07:49 jec      new header to match the rest of the framework
 08/09/11 09:30 jec      started coding
*****************************************************************************/
#ifndef ES_TYPES_H
#define ES_TYPES_H

/* Standard ANSI  99 C types for exact integer sizes*/
#ifdef COMPILER_IS_C99
#include <stdint.h>
#else  /* use the open source version included with ES framework */
#include "stdint.h"
#endif

/* Standard ANSI  99 C types for booleans, true & false*/
/* must come after stdint.h because the emulation uses uint8_t */
#ifdef COMPILER_IS_C99
#include <stdbool.h>
#else   // provide C99 compliant definitions
#include "stdbool.h"
#endif

#endif /* ES_TYPES_H */

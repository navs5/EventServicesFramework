#ifndef BIN_CONST_H
#define BIN_CONST_H

/* Binary constant generator macro
   By Tom Torfs - donated to the public domain
*/

/* All macro's evaluate to compile-time constants */

/* *** helper macros *** */

/* turn a numeric literal into a hex constant
   (avoids problems with leading zeroes)
   8-bit constants max value 0x11111111, always fits in unsigned long
*/

#define HEX__(n) 0x ## n ## UL

/* 8-bit conversion function */
#define B8__(x) ((x & 0x0000000FUL) ? 1 : 0) \
  + ((x & 0x000000F0UL) ? 2 : 0) \
  + ((x & 0x00000F00UL) ? 4 : 0) \
  + ((x & 0x0000F000UL) ? 8 : 0) \
  + ((x & 0x000F0000UL) ? 16 : 0) \
  + ((x & 0x00F00000UL) ? 32 : 0) \
  + ((x & 0x0F000000UL) ? 64 : 0) \
  + ((x & 0xF0000000UL) ? 128 : 0)

/* *** user macros *** */
/* for upto 8-bit binary constants */

#define B8(d) ((uint8_t)B8__(HEX__(d)))

/* for upto 16-bit binary constants, MSB first */

#define B16(dmsb, dlsb) (((unsigned short)B8(dmsb) << 8) \
      + B8(dlsb))

/* for upto 32-bit binary constants, MSB first */

#define B32(dmsb, db2, db3, dlsb) (((unsigned long)B8(dmsb) << 24) \
      + ((unsigned long)B8(db2) << 16) \
      + ((unsigned long)B8(db3) << 8) \
      + B8(dlsb))

#endif                       /* BIN_CONST_H */

/* Sample usage:
      B8(01010101) = 85
      B16(10101010,01010101) = 43605
      B32(10000000,11111111,10101010,01010101) = 2164238933
*/

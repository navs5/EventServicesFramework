// header file for the ES_ShortTimer library

#ifndef ES_ShortTimer_H
#define ES_ShortTimer_H
#include <stdint.h>
#include "driverlib/timer.h"
#include "ES_Configure.h"

#define SHORT_TIMER_UNUSED MAX_NUM_SERVICES

void ES_ShortTimerInit(uint8_t TimeAPrio, uint8_t TimeBPrio);
void ES_ShortTimerStart( uint32_t Which, uint16_t TimeoutValue);

#endif //ES_ShortTimer_H

/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef TestHarnessService0_H
#define TestHarnessService0_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"

// Public Function Prototypes

bool InitTestHarnessService0 ( uint8_t Priority );
bool PostTestHarnessService0( ES_Event ThisEvent );
ES_Event RunTestHarnessService0( ES_Event ThisEvent );


#endif /* ServTemplate_H */


/****************************************************************************
 Module
     ES_Events.h
 Description
     header file with definitions for the event structure
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/19/17 14:22 jec      changed include to ES_Cpnfigre to get definition of
                         ES_EventTyp_t
 08/05/13 15:19 jec      modifications to suit new portable type definitions
 01/15/12 11:46 jec      moved event enum to config file, changed prefixes to ES
 10/23/11 22:01 jec      customized for Remote Lock problem
 10/17/11 08:47 jec      changed event prefixes to EF_ to match new framework
 08/09/06 14:30 jec      started coding
*****************************************************************************/

#ifndef ES_Events_H
#define ES_Events_H

#include <stdint.h>

#include "ES_Configure.h"

typedef struct ES_Event
{
  ES_EventType_t EventType;      // what kind of event?
  uint16_t EventParam;          // parameter value for use w/ this event
}ES_Event_t;

#endif /* ES_Events_H */

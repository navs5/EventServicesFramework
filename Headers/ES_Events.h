/****************************************************************************
 Module
     ES_Events.h
 Description
     header file with definitions for the event structure 
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 08/05/13 15:19 jec      modifications to suit new portable type definitions
 01/15/12 11:46 jec      moved event enum to config file, changed prefixes to ES
 10/23/11 22:01 jec      customized for Remote Lock problem
 10/17/11 08:47 jec      changed event prefixes to EF_ to match new framework
 08/09/06 14:30 jec      started coding
*****************************************************************************/

#ifndef ES_Events_H
#define ES_Events_H

#include "ES_Types.h"

typedef struct ES_Event_t {
    ES_EventTyp_t EventType;    // what kind of event?
    uint16_t   EventParam;      // parameter value for use w/ this event
}ES_Event;


#endif /* ES_Events_H */

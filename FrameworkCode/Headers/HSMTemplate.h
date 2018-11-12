/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef HSMTemplate_H
#define HSMTemplate_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { STATE_ZERO, STATE_ONE, STATE_TWO } TemplateState_t ;


// Public Function Prototypes

ES_Event_t RunTemplateSM( ES_Event_t CurrentEvent );
void StartTemplateSM ( ES_Event_t CurrentEvent );
TemplateState_t QueryTemplateSM ( void );

#endif /*SHMTemplate_H */


/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef TopHSMTemplate_H
#define TopHSMTemplate_H

// State definitions for use with the query function
typedef enum { STATE_ONE, STATE_TWO, STATE_THREE } MasterState_t ;

// Public Function Prototypes

ES_Event_t RunMasterSM( ES_Event_t CurrentEvent );
void StartMasterSM ( ES_Event_t CurrentEvent );
bool PostMasterSM( ES_Event_t ThisEvent );
bool InitMasterSM ( uint8_t Priority );

#endif /*TopHSMTemplate_H */


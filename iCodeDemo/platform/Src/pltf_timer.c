/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2018 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*! \file pltf_timer.c
 *
 *  \brief SW Timer implementation
 *
 *  \author Gustavo Patricio
 *
 *   This module makes use of a System Tick in millisconds and provides
 *   an abstraction for SW timers.
 *   Modified to implement timers on Linux platform.	
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include <time.h>
#include "pltf_timer.h"
#include "platform.h"

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL AND HELPER FUNCTIONS
******************************************************************************
*/

/*****************************************************************************/

uint32_t ts2milisec(struct timespec* ts) {
	return  ((ts->tv_sec * (uint32_t)1000) + (ts->tv_nsec/1000000));
}


/****************************************************************************/

uint32_t platformGetSysTick_linux() {
	struct timespec cur_ts;
	clock_gettime(CLOCK_REALTIME, &cur_ts);
	return ts2milisec(&cur_ts); 
}


/*******************************************************************************/
uint32_t timerCalculateTimer( uint16_t time )
{
 
  return (platformGetSysTick() + time);
}


/*******************************************************************************/
bool timerIsExpired( uint32_t timer )
{
  uint32_t uDiff;
  int32_t sDiff;
  
  uDiff = (timer - platformGetSysTick());   /* Calculate the diff between the timers */
  sDiff = uDiff;                            /* Convert the diff to a signed var      */
  
  /* Check if the given timer has expired already */
  if( sDiff < 0 )
  {
    return true;
  }
  
  return false;
}


/*******************************************************************************/
void timerDelay( uint16_t tOut )
{
  uint32_t t;
  
  /* Calculate the timer and wait blocking until is running */
  t = timerCalculateTimer( tOut );
  while( timerIsRunning(t) );
}


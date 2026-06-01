
/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
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

/*
 *      PROJECT:   ST25R391x firmware
 *      $Revision: $
 *      LANGUAGE:  ISO C99
 */
 
/*! \file rfal_dynamicPower.c
 *
 *  \author Martin Zechleitner
 *
 *  \brief Functions to manage and set dynamic power settings.
 *  
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_dynamicPowerTbl.h"
#include "rfal_dynamicPower.h"
#include "platform.h"
#include "rfal_rf.h"
#include "utils.h"


/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */

#ifndef RFAL_FEATURE_DYNAMIC_POWER
    #error " RFAL: Module configuration missing. Please enable/disable Dynamic Power module by setting: RFAL_FEATURE_DYNAMIC_POWER "
#endif

#if RFAL_FEATURE_DYNAMIC_POWER

/*
 ******************************************************************************
 * LOCAL DATA TYPES
 ******************************************************************************
 */

static uint8_t* gRfalCurrentDynamicPower;
static uint8_t  gRfalDynamicPowerTableEntries;
static uint8_t  gRfalDynamicPower[RFAL_DYNAMIC_POWER_TABLE_SIZE_MAX];
static uint8_t  gRfalTableEntry;

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */
void rfalDynamicPowerInitialize( void )
{
    /* Use the default Dynamic Power values */
    gRfalCurrentDynamicPower = (uint8_t*) rfalDynamicPowerDefaultSettings;
    gRfalDynamicPowerTableEntries = (sizeof(rfalDynamicPowerDefaultSettings) / RFAL_DYNAMIC_POWER_TABLE_PAPAMETER);
    
    ST_MEMCPY( gRfalDynamicPower, gRfalCurrentDynamicPower, sizeof(rfalDynamicPowerDefaultSettings) );
    
    gRfalTableEntry = 0;
}

/*******************************************************************************/
ReturnCode rfalDynamicPowerTableLoad( rfalDynamicPowerEntry* powerTbl, uint8_t powerTblEntries )
{
    uint8_t entry = 0;
    
    // check if the table size parameter is too big
    if( (powerTblEntries * RFAL_DYNAMIC_POWER_TABLE_PAPAMETER) > RFAL_DYNAMIC_POWER_TABLE_SIZE_MAX)
    {
        return ERR_NOMEM;
    }
    
    // check if the first increase entry is 0xFF
    if( (powerTblEntries == 0) || (powerTbl == NULL) )
    {
        return ERR_PARAM;
    }
                
    // check if the entries of the dynamic power table are valid
    for (entry = 0; entry < powerTblEntries; entry++)
    {
        if(powerTbl[entry].inc < powerTbl[entry].dec)
        {
            return ERR_PARAM;
        }
    }
    
    ST_MEMCPY( gRfalDynamicPower, powerTbl, (powerTblEntries * RFAL_DYNAMIC_POWER_TABLE_PAPAMETER) );
    gRfalCurrentDynamicPower = gRfalDynamicPower;
    gRfalDynamicPowerTableEntries = powerTblEntries;
    
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalDynamicPowerTableRead( rfalDynamicPowerEntry* tblBuf, uint8_t tblBufEntries, uint8_t* tableEntries )
{
    if( (tblBuf == NULL) || (tblBufEntries < gRfalDynamicPowerTableEntries) || (tableEntries == NULL) )
    {
        return ERR_PARAM;
    }
        
    /* Copy the whole Table to the given buffer */
    ST_MEMCPY( tblBuf, gRfalCurrentDynamicPower, (tblBufEntries * RFAL_DYNAMIC_POWER_TABLE_PAPAMETER) );
    *tableEntries = gRfalDynamicPowerTableEntries;
    
    return ERR_NONE;
}

/*******************************************************************************/
void rfalDynamicPowerAdjust( void )
{
    uint8_t amplitude = 0;
    rfalDynamicPowerEntry* dynamicPowerTable = (rfalDynamicPowerEntry*) gRfalCurrentDynamicPower;
    
    /* Check if the Power Adjustment is disabled */
    if( gRfalCurrentDynamicPower == NULL )
    {
        return;
    }
    
    /* measure the RF amplitude */
    rfalMeasureRF( &amplitude );
    
    /* increase the output power */
    if( amplitude >= dynamicPowerTable[gRfalTableEntry].inc )
    {
        
        /* the top of the table represents the highest amplitude value*/
        if( gRfalTableEntry == 0 )
        {
            /* check if the maximum driver value has been reached */
            return;
        }
        /* go up in the table to decrease the driver resistance */
        gRfalTableEntry--;
    }
    else
    {
        /* decrease the output power */
        if(amplitude <= dynamicPowerTable[gRfalTableEntry].dec)
        {
            /* The bottom is the highest possible value */
            if( gRfalTableEntry == gRfalDynamicPowerTableEntries)
            {
                /* check if the minimum driver value has been reached */
                return;
            }
            /* go down in the table to increase the driver resistance */
            gRfalTableEntry++;
        }
        else
        {
            // do not write the driver again with the same value
            return;
        }
    }
    
    /* get the new value for RFO resistance form the table and apply the new RFO resistance setting */ 
    rfalSetModulatedRFO( dynamicPowerTable[gRfalTableEntry].rfoRes );
}


/*******************************************************************************/
void rfalDynamicPowerEnable( void )
{
    gRfalCurrentDynamicPower = gRfalDynamicPower;
}


/*******************************************************************************/
void rfalDynamicPowerDisable( void )
{
    gRfalCurrentDynamicPower = NULL;
}

#endif /* RFAL_FEATURE_DYNAMIC_POWER */

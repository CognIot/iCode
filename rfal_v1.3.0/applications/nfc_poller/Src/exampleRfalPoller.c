
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


/*
 *      PROJECT:   ST25R391x firmware
 *      LANGUAGE:  ISO C99
 */

/*  \file exampleRfalPoller.c
 * 
 *  \example exampleRfalPoller.c
 *
 *  \author Gustavo Patricio 
 *
 *  \brief  NFC Poller/Reader device (PCD) example
 *  
 *   This example shows how to use the different RFAL modules to perform all 
 *   procedures required as a NFC Poller / Reader device (PCD).
 *   
 *   It performs all the necessary steps to detect and activate a nearby device
 *   (Tags/Cards and P2P enabled devices such as phones). Once the device has 
 *   been activated a presence check is performed in a loop until the device 
 *   has been removed. 
 *   
 *   This example follows the guidelines described on NFC Forum Activity spec
 *   but it does not fully implement all its requirements.
 *      
 *   
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "example_poller.h"
#include "logger.h"
#include "st_errno.h"
#include "utils.h"
#include "platform.h"
#include "rfal_nfca.h"
#include "rfal_nfcb.h"
#include "rfal_nfcf.h"
#include "rfal_nfcv.h"
#include "rfal_isoDep.h"
#include "rfal_nfcDep.h"
#include "rfal_analogConfig.h"

const char* LOG_HEADER = "\r\nDemo Software provided by Bostin Technology\n\rScanning for NFC technologies \n\r";



#define LOG_BUFFER_SIZE 4096
static char logBuffer[LOG_BUFFER_SIZE];
static uint32_t logCnt = 0;

static int platformLog(const char* format, ...);
static void platformLogCreateHeader(char* buf);

#define platformLogClear()              system("clear")
#define platformLog2Screen(buf)         printf(buf); platformLogCreateHeader(buf);//buf[0] = 0;



/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define EXAMPLE_RFAL_POLLER_DEVICES      10    /* Number of devices supported */
#define EXAMPLE_RFAL_POLLER_RF_BUF_LEN   255   /* RF buffer length            */

#define EXAMPLE_RFAL_POLLER_FOUND_NONE   0x00  /* No device found Flag        */
#define EXAMPLE_RFAL_POLLER_FOUND_A      0x01  /* NFC-A device found Flag     */
#define EXAMPLE_RFAL_POLLER_FOUND_B      0x02  /* NFC-B device found Flag     */
#define EXAMPLE_RFAL_POLLER_FOUND_F      0x04  /* NFC-F device found Flag     */
#define EXAMPLE_RFAL_POLLER_FOUND_V      0x08  /* NFC-V device Flag           */


/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! Main state                                                                          */
typedef enum{
    EXAMPLE_RFAL_POLLER_STATE_INIT                =  0,  /* Initialize state            */
    EXAMPLE_RFAL_POLLER_STATE_TECHDETECT          =  1,  /* Technology Detection state  */
    EXAMPLE_RFAL_POLLER_STATE_COLAVOIDANCE        =  2,  /* Collision Avoidance state   */
    EXAMPLE_RFAL_POLLER_STATE_ACTIVATION          =  3,  /* Activation state            */
    EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_START  =  4,  /* Data Exchange Start state   */
    EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_CHECK  =  5,  /* Data Exchange Check state   */
    EXAMPLE_RFAL_POLLER_STATE_DEACTIVATION        =  9   /* Deactivation state          */
}exampleRfalPollerState;


/*! Device type                                                                         */
typedef enum{
    EXAMPLE_RFAL_POLLER_TYPE_NFCA  =  0,                 /* NFC-A device type           */
    EXAMPLE_RFAL_POLLER_TYPE_NFCB  =  1,                 /* NFC-B device type           */
    EXAMPLE_RFAL_POLLER_TYPE_NFCF  =  2,                 /* NFC-F device type           */
    EXAMPLE_RFAL_POLLER_TYPE_NFCV  =  3                  /* NFC-V device type           */
}exampleRfalPollerDevType;


/*! Device interface                                                                    */
typedef enum{
    EXAMPLE_RFAL_POLLER_INTERFACE_RF     = 0,            /* RF Frame interface          */
    EXAMPLE_RFAL_POLLER_INTERFACE_ISODEP = 1,            /* ISO-DEP interface           */
    EXAMPLE_RFAL_POLLER_INTERFACE_NFCDEP = 2             /* NFC-DEP interface           */
}exampleRfalPollerRfInterface;


/*! Device struct containing all its details                                            */
typedef struct{
    exampleRfalPollerDevType type;                      /* Device's type                */
    union{
        rfalNfcaListenDevice nfca;                      /* NFC-A Listen Device instance */
        rfalNfcbListenDevice nfcb;                      /* NFC-B Listen Device instance */
        rfalNfcfListenDevice nfcf;                      /* NFC-F Listen Device instance */
        rfalNfcvListenDevice nfcv;                      /* NFC-V Listen Device instance */
    }dev;                                               /* Device's instance            */
    
    exampleRfalPollerRfInterface rfInterface;           /* Device's interface           */
    union{
        rfalIsoDepDevice isoDep;                        /* ISO-DEP instance             */
        rfalNfcDepDevice nfcDep;                        /* NFC-DEP instance             */
    }proto;                                             /* Device's protocol            */
    
}exampleRfalPollerDevice;


/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */
static uint8_t                 t1tReadReq[]    = { 0x01, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44 };                                                   /* T1T READ Block:0 Byte:0 */
static uint8_t                 t2tReadReq[]    = { 0x30, 0x00 };                                                                                 /* T2T READ Block:0 */
static uint8_t                 t3tCheckReq[]   = { 0x06, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x01, 0x09, 0x00, 0x01, 0x80, 0x00 };   /* T3T Check/Read command */
static uint8_t                 t4tSelectReq[]  = { 0x00, 0xA4, 0x00, 0x00, 0x00 };                                                               /* T4T Select MF, DF or EF APDU  */
static uint8_t                 t5tSysInfoReq[] = { 0x02, 0x2B };                                                                                 /* NFC-V Get SYstem Information command*/
static uint8_t                 nfcbReq[]       = { 0x00 };                                                                                       /* NFC-B proprietary command */
static uint8_t                 llcpSymm[]      = { 0x00, 0x00 };                                                                                 /* LLCP SYMM command */

static uint8_t                 gNfcid3[]       = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A };                                  /* NFCID3 used for ATR_REQ */
static uint8_t                 gGenBytes[]     = { 0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03 }; /* P2P General Bytes: LCCP Connect */

/*******************************************************************************/

static uint8_t                 gDevCnt;                                 /* Number of devices found                         */
static exampleRfalPollerDevice gDevList[EXAMPLE_RFAL_POLLER_DEVICES];   /* Device List                                     */
static exampleRfalPollerState  gState;                                  /* Main state                                      */
static uint8_t                 gTechsFound;                             /* Technologies found bitmask                      */
exampleRfalPollerDevice        *gActiveDev;                             /* Active device pointer                           */
static uint16_t                gRcvLen;                                 /* Received length                                 */
static bool                    gRxChaining;                             /* Rx chaining flag                                */

/*! Transmit buffers union, only one interface is used at a time                                                           */
static union{
    uint8_t                 rfTxBuf[EXAMPLE_RFAL_POLLER_RF_BUF_LEN];    /* RF Tx buffer (not used on this demo)            */
    rfalIsoDepBufFormat     isoDepTxBuf;                                /* ISO-DEP Tx buffer format (with header/prologue) */
    rfalNfcDepBufFormat     nfcDepTxBuf;                                /* NFC-DEP Rx buffer format (with header/prologue) */
}gTxBuf;


/*! Receive buffers union, only one interface is used at a time                                                            */
static union {
    uint8_t                 rfRxBuf[EXAMPLE_RFAL_POLLER_RF_BUF_LEN];    /* RF Rx buffer                                    */
    rfalIsoDepBufFormat     isoDepRxBuf;                                /* ISO-DEP Rx buffer format (with header/prologue) */
    rfalNfcDepBufFormat     nfcDepRxBuf;                                /* NFC-DEP Rx buffer format (with header/prologue) */
}gRxBuf;


/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static bool exampleRfalPollerTechDetetection( void );
static bool exampleRfalPollerCollResolution( void );
static bool exampleRfalPollerActivation( uint8_t devIt );
static bool exampleRfalPollerNfcDepActivate( exampleRfalPollerDevice *device );
static ReturnCode exampleRfalPollerDataExchange( void );
static bool exampleRfalPollerDeactivate( void );


/*
******************************************************************************
* INITIAL SCREEN
******************************************************************************
*/
int splashscreen(void)
{
    platformLogClear();
    printf("\n***********************************************\n");
    printf("*             Bostin Technology               *\n");
    printf("*                                             *\n");
    printf("*                NFC2 Reader                  *\n");
    printf("*                                             *\n");
    printf("*           Based on the ST25R3911B           *\n");
    printf("*             demo provided by ST             *\n");
    printf("*                                             *\n");
    printf("*        for more info www.cognIoT.eu         *\n");
    printf("***********************************************\n");
    platformDelay(2000);
    return (0);
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*!
 ******************************************************************************
 * \brief Passive Poller Run
 * 
 * This method implements the main state machine going thought all the 
 * different activities that a Reader/Poller device (PCD) needs to perform.
 * 
 * 
 ******************************************************************************
 */
extern void exampleRfalPollerRun( void )
{
    ReturnCode err;
    uint8_t    i;
    
    splashscreen();
    
    rfalAnalogConfigInitialize();                                                     /* Initialize RFAL's Analog Configs */
	rfalInitialize();                                                                 /* Initialize RFAL */
	//platformLog("\n\rExample RFAL Poller started \r\n");

   
	for(;;)
	{
	    rfalWorker();                                                                 /* Execute RFAL process */

	    /* switchoff all the leds at start */
	    //platformLedOff(LED_NFCA_PORT, LED_NFCA_PIN);
	    //platformLedOff(LED_NFCB_PORT, LED_NFCB_PIN);
	    //platformLedOff(LED_NFCF_PORT, LED_NFCF_PIN);
	    //platformLedOff(LED_NFCV_PORT, LED_NFCV_PIN);    
	    platformLedOff(LED_TAG_READ_PORT, LED_TAG_READ_PIN);                        /* Added by MB to switch LED off */

        platformLedOn(PLATFORM_LED_FIELD_PORT,PLATFORM_LED_FIELD_PIN);
        platformLedOff(LED_TAG_READ_PORT, LED_TAG_READ_PIN); 
		platformDelay(20);

	    switch( gState )
	    {
	        /*******************************************************************************/
	        case EXAMPLE_RFAL_POLLER_STATE_INIT:                                     
	            
	            gTechsFound = EXAMPLE_RFAL_POLLER_FOUND_NONE; 
	            gActiveDev  = NULL;
	            gDevCnt     = 0;
	            
	            gState = EXAMPLE_RFAL_POLLER_STATE_TECHDETECT;
	            break;
	            
	            
            /*******************************************************************************/
	        case EXAMPLE_RFAL_POLLER_STATE_TECHDETECT:
	            
	            if( !exampleRfalPollerTechDetetection() )                             /* Poll for nearby devices in different technologies */
	            {
	                gState = EXAMPLE_RFAL_POLLER_STATE_DEACTIVATION;                  /* If no device was found, restart loop */
	                break;
	            }
	            
	            gState = EXAMPLE_RFAL_POLLER_STATE_COLAVOIDANCE;                      /* One or more devices found, go to Collision Avoidance */
	            break;
	            
	            
            /*******************************************************************************/
	        case EXAMPLE_RFAL_POLLER_STATE_COLAVOIDANCE:
	            
	            if( !exampleRfalPollerCollResolution() )                              /* Resolve any eventual collision */
                {
                    gState = EXAMPLE_RFAL_POLLER_STATE_DEACTIVATION;                  /* If Collision Resolution was unable to retrieve any device, restart loop */
                    break;
                }
	            
	            platformLog("Device(s) found: %d \r\n", gDevCnt);
	            
	            for(i=0; i<gDevCnt; i++)
	            {
	                switch( gDevList[i].type )
	                {
	                    case EXAMPLE_RFAL_POLLER_TYPE_NFCA:
	                        platformLog( " NFC-A device UID: %s \r\n", hex2str(gDevList[i].dev.nfca.nfcId1, gDevList[i].dev.nfca.nfcId1Len) );
	                        //platformLedOn( LED_NFCA_PORT, LED_NFCA_PIN  );
                            platformLedOn(LED_TAG_READ_PORT, LED_TAG_READ_PIN); 
	                        break;
	                        
	                    case EXAMPLE_RFAL_POLLER_TYPE_NFCB:
	                        platformLog( " NFC-B device UID: %s \r\n", hex2str(gDevList[i].dev.nfcb.sensbRes.nfcid0, RFAL_NFCB_NFCID0_LEN) );
	                        //platformLedOn( LED_NFCB_PORT, LED_NFCB_PIN  );
                            platformLedOn(LED_TAG_READ_PORT, LED_TAG_READ_PIN); 
                            break;
                            
	                    case EXAMPLE_RFAL_POLLER_TYPE_NFCF:
	                        platformLog( " NFC-F device UID: %s \r\n", hex2str(gDevList[i].dev.nfcf.sensfRes.NFCID2, RFAL_NFCF_NFCID2_LEN) );
                            //platformLedOn( LED_NFCF_PORT, LED_NFCF_PIN  );
                            platformLedOn(LED_TAG_READ_PORT, LED_TAG_READ_PIN); 
                            break;
                            
	                    case EXAMPLE_RFAL_POLLER_TYPE_NFCV:
	                        platformLog( " NFC-V device UID: %s \r\n", hex2str(gDevList[i].dev.nfcv.InvRes.UID, RFAL_NFCV_UID_LEN) );
                            //platformLedOn( LED_NFCV_PORT, LED_NFCV_PIN  );
                            platformLedOn(LED_TAG_READ_PORT, LED_TAG_READ_PIN); 
                            break;
	                }
	            }
	            //platformDelay(200);
	            gState = EXAMPLE_RFAL_POLLER_STATE_ACTIVATION;                        /* Device(s) have been identified, go to Activation */
                break;
	        
                
            /*******************************************************************************/
	        case EXAMPLE_RFAL_POLLER_STATE_ACTIVATION:
#if 0
	            if( !exampleRfalPollerActivation( 0 ) )                               /* Any device previous identified can be Activated, on this example will select the firt on the list */
	            {
                    gState = EXAMPLE_RFAL_POLLER_STATE_DEACTIVATION;                  /* If Activation failed, restart loop */
                    break;
                }
	            
	            //gState = EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_START;                /* Device has been properly activated, go to Data Exchange */
	            gState = EXAMPLE_RFAL_POLLER_STATE_DEACTIVATION;
		    break;
#endif	            
	            
            /*******************************************************************************/
#if 0	        
		case EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_START:                       
	        case EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_CHECK:
	                
	            err = exampleRfalPollerDataExchange();                                /* Perform Data Exchange, in this example a simple transfer will executed in order to do device's presence check */
                switch( err )
                {
                    case ERR_NONE:                                                    /* Data exchange successful  */
                        platformDelay(300);                                           /* Wait a bit */
                        gState = EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_START;        /* Trigger new exchange with device */
                        break;
                        
                    case ERR_BUSY:                                                    /* Data exchange ongoing  */
                        gState = EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_CHECK;        /* Once triggered/started the Data Exchange only do check until is completed */
                        break;
                        
                    default:                                                          /* Data exchange not successful, card removed or other transmission error */
                        platformLog("Data exchange terminated with error: %d \r\n", err);
                        gState = EXAMPLE_RFAL_POLLER_STATE_DEACTIVATION;              /* Restart loop */
                        break;
                }
                break;
                
#endif	            
            /*******************************************************************************/
	        case EXAMPLE_RFAL_POLLER_STATE_DEACTIVATION:
#if 0	            
	            exampleRfalPollerDeactivate();                                        /* If a card has been activated, properly deactivate the device */
#endif	            
	            rfalFieldOff();                                                       /* Turn the Field Off powering down any device nearby */
	            platformDelay(2);                                                     /* Remain a certain period with field off */
	            gState = EXAMPLE_RFAL_POLLER_STATE_INIT;                              /* Restart the loop */

                platformLogClear();
                platformLog2Screen(logBuffer);

	            break;
	        
	        
            /*******************************************************************************/
	        default:
	            return;
	    }
	}
}
	

/*!
 ******************************************************************************
 * \brief Poller Technology Detection
 * 
 * This method implements the Technology Detection / Poll for different 
 * device technologies.
 * 
 * \return true         : One or more devices have been detected
 * \return false         : No device have been detected
 * 
 ******************************************************************************
 */
static bool exampleRfalPollerTechDetetection( void )
{
    ReturnCode           err;
    rfalNfcaSensRes      sensRes;
    rfalNfcbSensbRes     sensbRes;
    rfalNfcvInventoryRes invRes;
    uint8_t              sensbResLen;
    
    gTechsFound = EXAMPLE_RFAL_POLLER_FOUND_NONE;
    
    /*******************************************************************************/
    /* NFC-A Technology Detection                                                  */
    /*******************************************************************************/
    
    rfalNfcaPollerInitialize();                                                       /* Initialize RFAL for NFC-A */
    rfalFieldOnAndStartGT();                                                          /* Turns the Field On and starts GT timer */
    
    err = rfalNfcaPollerTechnologyDetection( RFAL_COMPLIANCE_MODE_NFC, &sensRes ); /* Poll for NFC-A devices */
    if( err == ERR_NONE )
    {
        gTechsFound |= EXAMPLE_RFAL_POLLER_FOUND_A;
    }
    
    
    /*******************************************************************************/
    /* NFC-B Technology Detection                                                  */
    /*******************************************************************************/
    
    rfalNfcbPollerInitialize();                                                       /* Initialize RFAL for NFC-B */
    rfalFieldOnAndStartGT();                                                          /* As field is already On only starts GT timer */
    
    err = rfalNfcbPollerTechnologyDetection( RFAL_COMPLIANCE_MODE_NFC, &sensbRes, &sensbResLen ); /* Poll for NFC-B devices */
    if( err == ERR_NONE )
    {
        gTechsFound |= EXAMPLE_RFAL_POLLER_FOUND_B;
    }
    
    
    /*******************************************************************************/
    /* NFC-F Technology Detection                                                  */
    /*******************************************************************************/
    
    rfalNfcfPollerInitialize( RFAL_BR_212 );                                          /* Initialize RFAL for NFC-F */
    rfalFieldOnAndStartGT();                                                          /* As field is already On only starts GT timer */
    
    err = rfalNfcfPollerCheckPresence();                                              /* Poll for NFC-F devices */
    if( err == ERR_NONE )
    {
        gTechsFound |= EXAMPLE_RFAL_POLLER_FOUND_F;
    }
    
    
    /*******************************************************************************/
    /* NFC-V Technology Detection                                                  */
    /*******************************************************************************/
    
    rfalNfcvPollerInitialize();                                                       /* Initialize RFAL for NFC-V */
    rfalFieldOnAndStartGT();                                                          /* As field is already On only starts GT timer */
    
    err = rfalNfcvPollerCheckPresence( &invRes );                                     /* Poll for NFC-V devices */
    if( err == ERR_NONE )
    {
        gTechsFound |= EXAMPLE_RFAL_POLLER_FOUND_V;
    }
    
    return (gTechsFound != EXAMPLE_RFAL_POLLER_FOUND_NONE);
}

/*!
 ******************************************************************************
 * \brief Poller Collision Resolution
 * 
 * This method implements the Collision Resolution on all technologies that
 * have been detected before.
 * 
 * \return true         : One or more devices identified 
 * \return false        : No device have been identified
 * 
 ******************************************************************************
 */
static bool exampleRfalPollerCollResolution( void )
{
    uint8_t    i;
    uint8_t    devCnt;
    ReturnCode err;
    
    
    /*******************************************************************************/
    /* NFC-A Collision Resolution                                                  */
    /*******************************************************************************/
    if( gTechsFound & EXAMPLE_RFAL_POLLER_FOUND_A )                                   /* If a NFC-A device was found/detected, perform Collision Resolution */
    {
        rfalNfcaListenDevice nfcaDevList[EXAMPLE_RFAL_POLLER_DEVICES];
        
        rfalNfcaPollerInitialize();
        rfalFieldOnAndStartGT();                                                      /* Ensure GT again as other technologies have also been polled */
        err = rfalNfcaPollerFullCollisionResolution( RFAL_COMPLIANCE_MODE_NFC, (EXAMPLE_RFAL_POLLER_DEVICES - gDevCnt), nfcaDevList, &devCnt );
        if( (err == ERR_NONE) && (devCnt != 0) )
        {
            for( i=0; i<devCnt; i++ )                                                 /* Copy devices found form local Nfca list into global device list */
            {
                gDevList[gDevCnt].type     = EXAMPLE_RFAL_POLLER_TYPE_NFCA;
                gDevList[gDevCnt].dev.nfca = nfcaDevList[i];
                gDevCnt++;
            }
        }
    }
    
    /*******************************************************************************/
    /* NFC-B Collision Resolution                                                  */
    /*******************************************************************************/
    if( gTechsFound & EXAMPLE_RFAL_POLLER_FOUND_B )                                   /* If a NFC-A device was found/detected, perform Collision Resolution */
    {
        rfalNfcbListenDevice nfcbDevList[EXAMPLE_RFAL_POLLER_DEVICES];
        
        rfalNfcbPollerInitialize();
        rfalFieldOnAndStartGT();                                                      /* Ensure GT again as other technologies have also been polled */
        err = rfalNfcbPollerCollisionResolution( RFAL_COMPLIANCE_MODE_NFC, (EXAMPLE_RFAL_POLLER_DEVICES - gDevCnt), nfcbDevList, &devCnt );
        if( (err == ERR_NONE) && (devCnt != 0) )
        {
            for( i=0; i<devCnt; i++ )                                                 /* Copy devices found form local Nfcb list into global device list */
            {
                gDevList[gDevCnt].type     = EXAMPLE_RFAL_POLLER_TYPE_NFCB;
                gDevList[gDevCnt].dev.nfcb = nfcbDevList[i];
                gDevCnt++;
            }
        }
    }
    
    
    /*******************************************************************************/
    /* NFC-F Collision Resolution                                                  */
    /*******************************************************************************/
    if( gTechsFound & EXAMPLE_RFAL_POLLER_FOUND_F )                                   /* If a NFC-F device was found/detected, perform Collision Resolution */
    {
        rfalNfcfListenDevice nfcfDevList[EXAMPLE_RFAL_POLLER_DEVICES];
        
        rfalNfcfPollerInitialize( RFAL_BR_212 );
        rfalFieldOnAndStartGT();                                                      /* Ensure GT again as other technologies have also been polled */
        err = rfalNfcfPollerCollisionResolution( RFAL_COMPLIANCE_MODE_NFC, (EXAMPLE_RFAL_POLLER_DEVICES - gDevCnt), nfcfDevList, &devCnt );
        if( (err == ERR_NONE) && (devCnt != 0) )
        {
            for( i=0; i<devCnt; i++ )                                                 /* Copy devices found form local Nfcf list into global device list */
            {
                gDevList[gDevCnt].type     = EXAMPLE_RFAL_POLLER_TYPE_NFCF;
                gDevList[gDevCnt].dev.nfcf = nfcfDevList[i];
                gDevCnt++;
            }
        }
    }
    
    /*******************************************************************************/
    /* NFC-V Collision Resolution                                                  */
    /*******************************************************************************/
    if( gTechsFound & EXAMPLE_RFAL_POLLER_FOUND_V )                                   /* If a NFC-F device was found/detected, perform Collision Resolution */
    {
        rfalNfcvListenDevice nfcvDevList[EXAMPLE_RFAL_POLLER_DEVICES];
        
        rfalNfcvPollerInitialize();
        rfalFieldOnAndStartGT();                                                      /* Ensure GT again as other technologies have also been polled */
        err = rfalNfcvPollerCollisionResolution( (EXAMPLE_RFAL_POLLER_DEVICES - gDevCnt), nfcvDevList, &devCnt );
        if( (err == ERR_NONE) && (devCnt != 0) )
        {
            for( i=0; i<devCnt; i++ )                                                /* Copy devices found form local Nfcf list into global device list */
            {
                gDevList[gDevCnt].type     = EXAMPLE_RFAL_POLLER_TYPE_NFCV;
                gDevList[gDevCnt].dev.nfcv = nfcvDevList[i];
                gDevCnt++;
            }
        }
    }
    
    return (gDevCnt > 0);
}


/*!
 ******************************************************************************
 * \brief Poller Activation
 * 
 * This method Activates a given device according to it's type and 
 * protocols supported
 *  
 * \param[in]  devIt : device's position on the list to be activated 
 * 
 * \return true         : Activation successful 
 * \return false        : Activation failed
 * 
 ******************************************************************************
 */
static bool exampleRfalPollerActivation( uint8_t devIt )
{
    ReturnCode           err;
    rfalNfcaSensRes      sensRes;
    rfalNfcaSelRes       selRes;
    rfalNfcbSensbRes     sensbRes;
    uint8_t              sensbResLen;
    
    if( devIt > gDevCnt )
    {
        return false;
    }
    
    switch( gDevList[devIt].type )
    {
        /*******************************************************************************/
        /* NFC-A Activation                                                            */
        /*******************************************************************************/
        case EXAMPLE_RFAL_POLLER_TYPE_NFCA:
            
            rfalNfcaPollerInitialize();
            if( gDevList[devIt].dev.nfca.isSleep )                                    /* Check if desired device is in Sleep      */
            {
                err = rfalNfcaPollerCheckPresence( RFAL_14443A_SHORTFRAME_CMD_WUPA, &sensRes ); /* Wake up all cards  */
                if( err != ERR_NONE )
                {
                    return false;
                }
                
                err = rfalNfcaPollerSelect( gDevList[devIt].dev.nfca.nfcId1, gDevList[devIt].dev.nfca.nfcId1Len, &selRes ); /* Select specific device  */
                if( err != ERR_NONE )
                {
                    return false;
                }
            }
            
            /*******************************************************************************/
            /* Perform protocol specific activation                                        */
            switch( gDevList[devIt].dev.nfca.type )
            {
                /*******************************************************************************/
                case RFAL_NFCA_T1T:
                    
                    /* No further activation needed for a T1T (RID already performed)*/
                    platformLog("NFC-A T1T device activated \r\n");                   /* NFC-A T1T device activated */
                    
                    gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_RF;
                    break;
                    
                
                /*******************************************************************************/
                case RFAL_NFCA_T2T:
                  
                    /* No specific activation needed for a T2T */    
                    platformLog("NFC-A T2T device activated \r\n");                   /* NFC-A T2T device activated */
                    
                    gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_RF;
                    break;
                
                
                /*******************************************************************************/
                case RFAL_NFCA_T4T:
                
                    /* Perform ISO-DEP (ISO14443-4) activation: RATS and PPS if supported */
                    err = rfalIsoDepPollAHandleActivation( (rfalIsoDepFSxI)RFAL_ISODEP_FSDI_DEFAULT, RFAL_ISODEP_NO_DID, RFAL_BR_424, &gDevList[devIt].proto.isoDep );
                    if( err != ERR_NONE )
                    {
                        return false;
                    }
                    
                    platformLog("NFC-A T4T (ISO-DEP) device activated \r\n");         /* NFC-A T4T device activated */
                    
                    gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_ISODEP;
                    break;
                  
                  
                /*******************************************************************************/
                case RFAL_NFCA_T4T_NFCDEP:                                              /* Device supports both T4T and NFC-DEP */
                case RFAL_NFCA_NFCDEP:                                                  /* Device supports NFC-DEP */
                  
                    /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
                    if( !exampleRfalPollerNfcDepActivate( &gDevList[devIt] ) )
                    {
                      return false;
                    }
                    
                    platformLog("NFC-A P2P (NFC-DEP) device activated \r\n");         /* NFC-A P2P device activated */
                    gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_NFCDEP;
                    break;
            }
            
            break;
        
        /*******************************************************************************/
        /* NFC-B Activation                                                            */
        /*******************************************************************************/
        case EXAMPLE_RFAL_POLLER_TYPE_NFCB:
            
            rfalNfcbPollerInitialize();
            if( gDevList[devIt].dev.nfcb.isSleep )                                    /* Check if desired device is in Sleep */
            {
                /* Wake up all cards. SENSB_RES may return collision but the NFCID0 is available to explicitly select NFC-B card via ATTRIB; so error will be ignored here */
                rfalNfcbPollerCheckPresence( RFAL_NFCB_SENS_CMD_ALLB_REQ, RFAL_NFCB_SLOT_NUM_1, &sensbRes, &sensbResLen );
            }
            
            
            /*******************************************************************************/
            /* Perform ISO-DEP (ISO14443-4) activation: RATS and PPS if supported          */
            err = rfalIsoDepPollBHandleActivation( (rfalIsoDepFSxI)RFAL_ISODEP_FSDI_DEFAULT, RFAL_ISODEP_NO_DID, RFAL_BR_424, 0x00, &gDevList[devIt].dev.nfcb, NULL, 0, &gDevList[devIt].proto.isoDep );
            if( err == ERR_NONE )
            {
                platformLog("NFC-B T4T (ISO-DEP) device activated \r\n");             /* NFC-B T4T device activated */
                
                gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_ISODEP ;
                break;
            }
            
            platformLog("NFC-B device activated \r\n");                               /* NFC-B  device activated */
            gDevList[devIt].rfInterface =  EXAMPLE_RFAL_POLLER_INTERFACE_RF;
            break;
            
        /*******************************************************************************/
        /* NFC-F Activation                                                            */
        /*******************************************************************************/
        case EXAMPLE_RFAL_POLLER_TYPE_NFCF:
            
            rfalNfcfPollerInitialize( RFAL_BR_212 );
            if( rfalNfcfIsNfcDepSupported( &gDevList[devIt].dev.nfcf ) )
            {
                /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
                if( !exampleRfalPollerNfcDepActivate( &gDevList[devIt] ) )
                {
                    return false;
                }
                
                platformLog("NFC-F P2P (NFC-DEP) device activated \r\n");             /* NFC-A P2P device activated */
                
                gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_NFCDEP;
                break;
            }
            
            platformLog("NFC-F T3T device activated \r\n");                           /* NFC-F T3T device activated */
            gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_RF;
            break;
            
        /*******************************************************************************/
        /* NFC-V Activation                                                            */
        /*******************************************************************************/
        case EXAMPLE_RFAL_POLLER_TYPE_NFCV:
            
            rfalNfcvPollerInitialize();
            
            /* No specific activation needed for a T5T */
            platformLog("NFC-V T5T device activated \r\n");                           /* NFC-V T5T device activated */
            
            gDevList[devIt].rfInterface = EXAMPLE_RFAL_POLLER_INTERFACE_RF;
            break;
        
        /*******************************************************************************/
        default:
            return false;
    }
    
    gActiveDev = &gDevList[devIt];                                                    /* Assign active device to be used further on */
    return true;
}


/*!
 ******************************************************************************
 * \brief Poller NFC DEP Activate
 * 
 * This method performs NFC-DEP Activation 
 *  
 * \param[in]  devIt : device to be activated 
 * 
 * \return true         : Activation successful 
 * \return false        : Activation failed
 * 
 ******************************************************************************
 */
static bool exampleRfalPollerNfcDepActivate( exampleRfalPollerDevice *device )
{
    rfalNfcDepAtrParam   param;
                
    /*******************************************************************************/
    /* If Passive F use the NFCID2 retrieved from SENSF                            */
    if( device->type == EXAMPLE_RFAL_POLLER_TYPE_NFCF )
    {
        param.nfcid    = device->dev.nfcf.sensfRes.NFCID2;
        param.nfcidLen = RFAL_NFCF_NFCID2_LEN;
    }
    else
    {
        param.nfcid    = gNfcid3;
        param.nfcidLen = RFAL_NFCDEP_NFCID3_LEN;
    }    
    
    param.BS    = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    param.BR    = RFAL_NFCDEP_Bx_NO_HIGH_BR;
    param.DID   = RFAL_NFCDEP_DID_NO;
    param.NAD   = RFAL_NFCDEP_NAD_NO;
    param.LR    = RFAL_NFCDEP_LR_254;
    param.GB    = gGenBytes;
    param.GBLen = sizeof(gGenBytes);
    param.commMode  = RFAL_NFCDEP_COMM_PASSIVE;
    param.operParam = (RFAL_NFCDEP_OPER_FULL_MI_EN | RFAL_NFCDEP_OPER_EMPTY_DEP_DIS | RFAL_NFCDEP_OPER_ATN_EN | RFAL_NFCDEP_OPER_RTOX_REQ_EN);
    
    /* Perform NFC-DEP (P2P) activation: ATR and PSL if supported */
    return (rfalNfcDepInitiatorHandleActivation( &param, RFAL_BR_424, &device->proto.nfcDep ) == ERR_NONE);
}


/*!
 ******************************************************************************
 * \brief Data Exchange
 * 
 * This method performs Data Exchange by device's type and interface.
 *  
 * 
 * \return ERR_REQUEST     : Bad request
 * \return ERR_BUSY        : Data Exchange ongoing
 * \return ERR_NONE        : Data Exchange terminated successfully
 * 
 ******************************************************************************
 */
static ReturnCode exampleRfalPollerDataExchange( void )
{
    rfalTransceiveContext ctx;
    ReturnCode            err;
    rfalIsoDepTxRxParam   isoDepTxRx;
    rfalNfcDepTxRxParam   nfcDepTxRx;
    uint8_t               *txBuf;
    uint16_t              txBufLen;
    
    
    /*******************************************************************************/
    /* The Data Exchange is divided in two different moments, the trigger/Start of *
     *  the transfer followed by the check until its completion                    */
    if( gState == EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_START )                      /* Trigger/Start the data exchange */
    {
        switch( gActiveDev->rfInterface )                                             /* Check which RF interface shall be used/has been activated */
        {
            /*******************************************************************************/
            case EXAMPLE_RFAL_POLLER_INTERFACE_RF:
    
                switch( gActiveDev->type )                                            /* Over RF interface no specific protocol is selected, each device supports a different protocol */
                {
                    /*******************************************************************************/
                    case EXAMPLE_RFAL_POLLER_TYPE_NFCA:
                        switch( gActiveDev->dev.nfca.type )
                        {
                            /*******************************************************************************/
                            case RFAL_NFCA_T1T:
                                
                                /* To perform presence check, on this example a T1T Read command is used */
                                ST_MEMCPY( &t1tReadReq[3], gActiveDev->dev.nfca.nfcId1, RFAL_NFCA_CASCADE_1_UID_LEN );  /* Assign device's NFCID for read command */
                                                        
                                txBuf    = t1tReadReq;
                                txBufLen = sizeof(t1tReadReq);
                                break;
                                
                            /*******************************************************************************/
                            case RFAL_NFCA_T2T:
                                
                                /* To perform presence check, on this example a T2T Read command is used */
                                txBuf    = t2tReadReq;
                                txBufLen = sizeof(t2tReadReq);
                                break;
                            
                            /*******************************************************************************/
                            default:
                                return ERR_REQUEST;;
                        }
                        break;

                        
                    /*******************************************************************************/
                    case EXAMPLE_RFAL_POLLER_TYPE_NFCB:
                        
                        /* To perform presence check, no specific command is used */
                        txBuf    = nfcbReq;
                        txBufLen = sizeof(nfcbReq);
                        break;
                        
                        
                    /*******************************************************************************/
                    case EXAMPLE_RFAL_POLLER_TYPE_NFCF:
                        
                        /* To perform presence check, on this example a T3T Check/Read command is used */
                        ST_MEMCPY( &t3tCheckReq[1], gActiveDev->dev.nfcf.sensfRes.NFCID2, RFAL_NFCF_NFCID2_LEN );  /* Assign device's NFCID for Check command */
                        
                        txBuf    = t3tCheckReq;
                        txBufLen = sizeof(t3tCheckReq);
                        break;
                        
                        
                    /*******************************************************************************/
                    case EXAMPLE_RFAL_POLLER_TYPE_NFCV:
                        
                        /* To perform presence check, on this example a Get System Information command is used */
                        txBuf    = t5tSysInfoReq;
                        txBufLen = sizeof(t5tSysInfoReq);
                        break;
                        
                        
                    /*******************************************************************************/
                    default:
                        return ERR_REQUEST;
                }
                
                /*******************************************************************************/
                /* Trigger a RFAL Transceive using the previous defined frames                 */
                rfalCreateByteFlagsTxRxContext( ctx, txBuf, txBufLen, gRxBuf.rfRxBuf, sizeof(gRxBuf.rfRxBuf), &gRcvLen, RFAL_TXRX_FLAGS_DEFAULT, rfalConvMsTo1fc(20) );
                return (((err = rfalStartTransceive( &ctx )) == ERR_NONE) ? ERR_BUSY : err);     /* Signal ERR_BUSY as Data Exchange has been started and is ongoing */
                
            case EXAMPLE_RFAL_POLLER_INTERFACE_ISODEP:
                
                ST_MEMCPY( gTxBuf.isoDepTxBuf.inf, t4tSelectReq, sizeof(t4tSelectReq) );
                
                isoDepTxRx.DID          = RFAL_ISODEP_NO_DID;
                isoDepTxRx.ourFSx       = RFAL_ISODEP_FSX_KEEP;
                isoDepTxRx.FSx          = gActiveDev->proto.isoDep.info.FSx;
                isoDepTxRx.dFWT         = gActiveDev->proto.isoDep.info.dFWT;
                isoDepTxRx.FWT          = gActiveDev->proto.isoDep.info.FWT;
                isoDepTxRx.txBuf        = &gTxBuf.isoDepTxBuf;
                isoDepTxRx.txBufLen     = sizeof(t4tSelectReq);
                isoDepTxRx.isTxChaining = false;
                isoDepTxRx.rxBuf        = &gRxBuf.isoDepRxBuf;
                isoDepTxRx.rxLen        = &gRcvLen;
                isoDepTxRx.isRxChaining = &gRxChaining;
                
                /*******************************************************************************/
                /* Trigger a RFAL ISO-DEP Transceive                                           */
                return (((err = rfalIsoDepStartTransceive( isoDepTxRx )) == ERR_NONE) ? ERR_BUSY : err); /* Signal ERR_BUSY as Data Exchange has been started and is ongoing */
                
                
            case EXAMPLE_RFAL_POLLER_INTERFACE_NFCDEP:
                
                ST_MEMCPY( gTxBuf.nfcDepTxBuf.inf, llcpSymm, sizeof(llcpSymm) );
                
                nfcDepTxRx.DID          = RFAL_NFCDEP_DID_KEEP;
                nfcDepTxRx.FSx          = rfalNfcDepLR2FS( rfalNfcDepPP2LR( gActiveDev->proto.nfcDep.activation.Target.ATR_RES.PPt ) );
                nfcDepTxRx.dFWT         = gActiveDev->proto.nfcDep.info.dFWT;
                nfcDepTxRx.FWT          = gActiveDev->proto.nfcDep.info.FWT;
                nfcDepTxRx.txBuf        = &gTxBuf.nfcDepTxBuf;
                nfcDepTxRx.txBufLen     = sizeof(llcpSymm);
                nfcDepTxRx.isTxChaining = false;
                nfcDepTxRx.rxBuf        = &gRxBuf.nfcDepRxBuf;
                nfcDepTxRx.rxLen        = &gRcvLen;
                nfcDepTxRx.isRxChaining = &gRxChaining;
                
                /*******************************************************************************/
                /* Trigger a RFAL NFC-DEP Transceive                                           */
                return (((err = rfalNfcDepStartTransceive( &nfcDepTxRx )) == ERR_NONE) ? ERR_BUSY : err);  /* Signal ERR_BUSY as Data Exchange has been started and is ongoing */
                
            default:
                break;
        }
    }
    /*******************************************************************************/
    /* The Data Exchange has been started, wait until completed                    */
    else if( gState == EXAMPLE_RFAL_POLLER_STATE_DATAEXCHANGE_CHECK )
    {
        switch( gActiveDev->rfInterface )
        {
            /*******************************************************************************/
            case EXAMPLE_RFAL_POLLER_INTERFACE_RF:
                return rfalGetTransceiveStatus();
                
            /*******************************************************************************/
            case EXAMPLE_RFAL_POLLER_INTERFACE_ISODEP:
                return rfalIsoDepGetTransceiveStatus();
                
            /*******************************************************************************/
            case EXAMPLE_RFAL_POLLER_INTERFACE_NFCDEP:
                return rfalNfcDepGetTransceiveStatus();
                
            /*******************************************************************************/
            default:
                return ERR_PARAM;
        }
    }
    return ERR_REQUEST;
}


/*!
 ******************************************************************************
 * \brief Poller NFC DEP Deactivate
 * 
 * This method Deactivates the device if a deactivation procedure exists 
 * 
 * \return true         : Deactivation successful 
 * \return false        : Deactivation failed
 * 
 ******************************************************************************
 */
static bool exampleRfalPollerDeactivate( void )
{
    if( gActiveDev != NULL )                                                          /* Check if a device has been activated */
    {
        switch( gActiveDev->rfInterface )
        {
            /*******************************************************************************/
            case EXAMPLE_RFAL_POLLER_INTERFACE_RF:
                break;                                                                /* No specific deactivation to be performed */
                
            /*******************************************************************************/
            case EXAMPLE_RFAL_POLLER_INTERFACE_ISODEP:
                rfalIsoDepDeselect();                                                 /* Send a Deselect to device */
                break;
                
            /*******************************************************************************/
            case EXAMPLE_RFAL_POLLER_INTERFACE_NFCDEP:
                rfalNfcDepRLS();                                                      /* Send a Release to device */
                break;
                
            default:
                return false;
        }
        platformLog("Device deactivated \r\n");
    }
    
    return true;
}


/*!
 *****************************************************************************
 * \brief log to buffer
 *
 * Append log information to actual logging buffer
 * 
 *  \param format   : sprintf compatible string formating
 *  \return size    : size of current log buffer
 * 
 *****************************************************************************
 */
int platformLog(const char* format, ...)
{
	#define TMP_BUFFER_SIZE 256
	char tmpBuffer[TMP_BUFFER_SIZE];

	va_list argptr;
	va_start(argptr, format);
	int cnt = vsnprintf(tmpBuffer, TMP_BUFFER_SIZE, format, argptr);    
	va_end(argptr);  
	  
	int pos = strlen(logBuffer);
	if((pos + cnt) < LOG_BUFFER_SIZE){
	  	strcat(logBuffer, tmpBuffer);
	}

	return pos;
}



void platformLogCreateHeader(char* buf)
{
	strcpy(buf, LOG_HEADER); 
	logCnt++;

	for(uint32_t i = 0; i < logCnt % 22; i++)
		strcat(buf, "."); 

	strcat(buf, "\n\n");	
	platformDelay(40);
}

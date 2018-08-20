
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
/*! \file
 *
 *  \author 
 *
 *  \brief Platform header file. Defining platform independent functionality.
 *
 */

/*
 *      PROJECT:   
 *      $Revision: $
 *      LANGUAGE:  ISO C99
 */

/*! \file platform.h
 *
 *  \author Gustavo Patricio
 *
 *  \brief Platform specific definition layer  
 *  
 *  This should contain all platform and hardware specifics such as 
 *  GPIO assignment, system resources, locks, IRQs, etc
 *  
 *  Each distinct platform/system/board must provide this definitions 
 *  for all SW layers to use
 *
 *  Modified for Linux platform.
 *  
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "st_errno.h"
#include "pltf_timer.h"
#include "pltf_spi.h"
#include "pltf_gpio.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/*
#define LED_NFCA_PIN				13
#define LED_NFCB_PIN				6
#define LED_NFCF_PIN				5
#define LED_NFCV_PIN				22
#define LED_AP2P_PIN				27 
#define LED_NFCA_PORT				0
#define LED_NFCB_PORT				0
#define LED_NFCF_PORT				0
#define LED_NFCV_PORT				0  */       /* These have been removed as they are not required as LEDs are not being used*/
#define LED_FIELD_Pin				24
#define LED_FIELD_GPIO_Port			0

#define ST25R391X_SS_PIN                                                /*!< GPIO pin used for ST25R3911 SPI SS */ 
#define ST25R391X_SS_PORT                                               /*!< GPIO port used for ST25R3911 SPI SS port */ 

#define ST25R391X_INT_PIN                     PLTF_GPIO_INTR_PIN        /*!< GPIO pin used for ST25R3911 External Interrupt */
#define ST25R391X_INT_PORT                    0                         /*!< GPIO port used for ST25R3911 External Interrupt */

#ifdef LED_FIELD_Pin
  #define PLATFORM_LED_FIELD_PIN              LED_FIELD_Pin             /*!< GPIO pin used as field LED */
#endif

#ifdef LED_FIELD_GPIO_Port
  #define PLATFORM_LED_FIELD_PORT             LED_FIELD_GPIO_Port       /*!< GPIO port used as field LED */
#endif


/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define ST25R391X_COM_SINGLETXRX                                        /*!< Enable single SPI frame transmission */

#define platformProtectST25R391xComm()        pltf_protect_com()
#define platformUnprotectST25R391xComm()      pltf_unprotect_com()   

#define platformSpiSelect()                                             /*!< SPI SS\CS: Chip|Slave Select */
#define platformSpiDeselect()                                           /*!< SPI SS\CS: Chip|Slave Deselect */

#define platformProtectST25R391xIrqStatus()   pltf_protect_interrupt_status()   /*!< Acquire the lock for safe access of RFAL interrupt status variable */  
#define platformUnprotectST25R391xIrqStatus() pltf_unprotect_interrupt_status() /*!< Release the lock aquired for safe accessing of RFAL interrupt status variable */ 

#define platformIrqST25R3911SetCallback(cb)          
#define platformIrqST25R3911PinInitialize()                

#define platformGpioSet(port, pin)            gpio_set(port, pin)       /*!< Turns the given GPIO High */
#define platformGpioClear(port, pin)          gpio_clear(port, pin)     /*!< Turns the given GPIO Low  */
#define platformGpioToggle(port, pin)         (platformGpioIsHigh(port, pin) ? platformGpioClear( port, pin) : platformGpioSet(port, pin))  /*!< Toogles the given GPIO    */
#define platformGpioIsHigh(port, pin)         (gpio_readpin(port, pin) == GPIO_PIN_SET)                                                     /*!< Checks if the given GPIO is High */
#define platformGpioIsLow(port, pin)          (!platformGpioIsHigh(port, pin))                                                              /*!< Checks if the given GPIO is Low  */

#define platformLedsInitialize()                                                /*!< Initializes the pins used as LEDs to outputs*/
#define platformLedOff(port, pin)             platformGpioClear(port, pin)      /*!< Turns the given LED Off */ /* Support for old board version MB1325-A */
#define platformLedOn(port, pin)              platformGpioSet(port, pin)        /*!< Turns the given LED On  */ /* Support for old board version MB1325-A */

#define platformTimerCreate(t)                timerCalculateTimer(t)    /*!< Create a timer with the given time (ms)     */
#define platformTimerIsExpired(timer)         timerIsExpired(timer)     /*!< Checks if the given timer is expired        */
#define platformDelay(t)                      timerDelay(t)             /*!< Performs a delay for the given time (ms)    */
#define platformGetSysTick()                  platformGetSysTick_linux()/*!< Get System Tick ( 1 tick = 1 ms)            */

#define platformSpiTxRx(txBuf, rxBuf, len)    spiTxRx(txBuf, rxBuf, len)/*!< SPI transceive */
                                              
#define platformI2CTx(txBuf, len)                                       /*!< I2C Transmit  */
#define platformI2CRx(txBuf, len)                                       /*!< I2C Receive   */
#define platformI2CStart()                                              /*!< I2C Start condition */
#define platformI2CStop()                                               /*!< I2C Stop condition  */
#define platformI2CRepeatStart()                                        /*!< I2C Repeat Start    */
#define platformI2CSlaveAddrWR(add)                                     /*!< I2C Slave address for Write operation       */
#define platformI2CSlaveAddrRD(add)                                     /*!< I2C Slave address for Read operation        */

/*
******************************************************************************
* RFAL FEATURES CONFIGURATION
******************************************************************************
*/

#define RFAL_FEATURE_NFCA                       true                    /*!< Enable/Disable RFAL support for NFC-A (ISO14443A)                         */
#define RFAL_FEATURE_NFCB                       true                    /*!< Enable/Disable RFAL support for NFC-B (ISO14443B)                         */
#define RFAL_FEATURE_NFCF                       true                    /*!< Enable/Disable RFAL support for NFC-F (FeliCa)                            */
#define RFAL_FEATURE_NFCV                       true                    /*!< Enable/Disable RFAL support for NFC-V (ISO15693)                          */
#define RFAL_FEATURE_T1T                        true                    /*!< Enable/Disable RFAL support for T1T (Topaz)                               */
#define RFAL_FEATURE_ST25TB                     true                    /*!< Enable/Disable RFAL support for ST25TB                                    */
#define RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG      true                    /*!< Enable/Disable Analog Configs to be dynamically updated (RAM)             */
#define RFAL_FEATURE_DYNAMIC_POWER              false                   /*!< Enable/Disable RFAL dynamic power support                                 */
#define RFAL_FEATURE_ISO_DEP                    true                    /*!< Enable/Disable RFAL support for ISO-DEP (ISO14443-4)                      */
#define RFAL_FEATURE_NFC_DEP                    true                    /*!< Enable/Disable RFAL support for NFC-DEP (NFCIP1/P2P)                      */


#define RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN     256                     /*!< ISO-DEP I-Block max length. Please use values as defined by rfalIsoDepFSx */
#define RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN       1024                    /*!< ISO-DEP APDU max length. Please use multiples of I-Block max length       */

#endif /* PLATFORM_H */



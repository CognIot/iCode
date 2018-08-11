
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

/*! \file pltf_spi.h
 *
 *  \author Shikha Singh
 *
 *  \brief Function declarations to initilalize platform SPI interface and to communicate
 *	with ST25R3911XX using SPI interface.
 *  
 */

#ifndef PLATFORMSPI_H
#define PLATFORMSPI_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <stdint.h>
#include "st_errno.h"

/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */
/* Enumeration for SPI communication status */
typedef enum
{
	HAL_OK		= 0x00,
	HAL_ERROR	= 0x01,
	HAL_BUSY	= 0x02,
	HAL_TIMEOUT	= 0x03
} HAL_statusTypeDef;

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */

/*! 
 *****************************************************************************
 * \brief  Initialize SPI Interface
 *  
 * This methods initialize SPI interface so that Linux host can use SPI interface
 *	to communicate with ST25R3911XX.
 *
 * \return ERR_IO	: SPI interface not initialized successfuly 
 * \return ERR_NONE	: No error
 *****************************************************************************
 */
ReturnCode spi_init(void);

/*! 
 *****************************************************************************
 * \brief  SPI Interface
 *  
 * This methods initialize SPI interface so that Linux host can use SPI interface
 *	to communicate with ST25R3911XX.
 *
 * \return ERR_IO	: SPI interface not initialized successfuly 
 * \return ERR_NONE	: No error
 *****************************************************************************
 */
/* function for full duplex SPI communication */
HAL_statusTypeDef spiTxRx(const uint8_t *txData, uint8_t *rxData, uint8_t length);

/*! 
 *****************************************************************************
 * \brief  To protect SPI communication
 *  
 * This method acquire a mutex and shall be used before communication takes 
 * place.
 * 
 *****************************************************************************
 */
void pltf_protect_com(void);

/*! 
 *****************************************************************************
 * \brief  To unprotect SPI communication
 *  
 * This method release the mutex that was acquired with pltf_protect_com.
 * 
 *****************************************************************************
 */
void pltf_unprotect_com(void); 

#endif /* PLATFORM_SPI */

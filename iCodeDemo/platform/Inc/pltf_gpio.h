
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

/*! \file pltf_gpio.h
 *
 *  \author Shikha Singh
 *
 *  \brief Function declarations that can be used by RFAL and application to
 *  initialize interrupt mechanism in user space and to set/clear the gpio lines.
 *  
 *  It provides the functionality to use a GPIO line to receive interrupts from ST25R3911X
 *  and to set/clear the gpio to glow and off the LEDs. 
 *
 */

#ifndef PLATFORMGPIO_H
#define PLATFORMGPIO_H

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "st_errno.h"

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */
/* GPIO pin no. 22 is used as interrupt line to receive interrupts from ST25R3911X.
 */ 

#define PLTF_GPIO_INTR_PIN	22

/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */
/* GPIO pin set and reset enumeration */
typedef enum {
	GPIO_PIN_RESET = 0,
	GPIO_PIN_SET
}GPIO_PinState;

/*
 ******************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 ******************************************************************************
 */

/*! 
 *****************************************************************************
 * \brief  Initialize free GPIO as interrupt pin
 *  
 * This methods initialize GPIO SYSFS interface for interrupt gpio line
 * to control GPIO pin from user space.
 * This method configures the GPIO line as interrupt pin with rising edge 
 * to receive interrupts from ST25R3911X.
 *
 * \return ERR_IO	: GPIO is not successfuly configured as interrupt pin 
 * \return ERR_NONE	: No error
 *****************************************************************************
 */
ReturnCode gpio_init(void);

/*! 
 *****************************************************************************
 * \brief  To read GPIO pin state
 *  
 * This methods reads the state of given GPIO pin.
 * \param[in]	: GPIO port of the given GPIO pin
 * \param[in]	: GPIO pin number
 * 
 * \return GPIO_PIN_RESET	: State of GPIO pin is low 
 * \return GPIO_PIN_SET		: State of GPIo pin is high
 *****************************************************************************
 */
GPIO_PinState gpio_readpin(int port, int pin_no); 

/*! 
 *****************************************************************************
 * \brief  To initialize the interrupt mechanism in user space
 *  
 * This method starts a pthread that polls the interrupt line and call the ISR 
 * when there is an event (interrupt) on gpio line.
 * It provides the functionality to use a GPIO line to receive interrupts from 
 * ST25R3911XX in user space.
 *
 * \return ERR_IO	: Error in initializing interrupt mechanism 
 * \return ERR_NONE	: No error
 *****************************************************************************
 */
ReturnCode interrupt_init(void);

/*! 
 *****************************************************************************
 * \brief  To set GPIO pin 
 *  
 * This method sets the state of GPIO pin high.
 * \param[in]	: GPIO port of the given GPIO pin
 * \param[in]	: GPIO pin number
 * 
 *****************************************************************************
 */
void gpio_set(int port, int pin_no); 

/*! 
 *****************************************************************************
 * \brief  To clear GPIO pin 
 *  
 * This method sets the state of GPIO pin Low.
 * \param[in]	: GPIO port of the given GPIO pin
 * \param[in]	: GPIO pin number
 * 
 *****************************************************************************
 */
void gpio_clear(int port, int pin_no);

/*! 
 *****************************************************************************
 * \brief  To protect interrupt status variable  of RFAL 
 *  
 * This method acquire a mutex before assigning new veluw to interrupt status
 * variable.
 * 
 *****************************************************************************
 */
void pltf_protect_interrupt_status(void);

/*! 
 *****************************************************************************
 * \brief  To protect interrupt status variable  of RFAL 
 *  
 * This method release the mutex after assigning new velue to interrupt status
 * variable.
 * 
 *****************************************************************************
 */
void pltf_unprotect_interrupt_status(void); 

#endif /* PLATFORMGPIO_H */



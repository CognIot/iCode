
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

/*! \file pltf_gpio.c
 *
 *  \author Shikha Singh
 *
 *  \brief Implementation of GPIO specific functions and its helper functions.
 *  
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <stdio.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "pltf_gpio.h"
#include "st25r3911_interrupt.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
/* Max size of file path to access */ 
#define SIZE	60

/*
 ******************************************************************************
 * STATIC VARIABLES
 ******************************************************************************
 */
static int isGPIOInit	= 0;
static int fd_readGPIO	= 0;
static pthread_mutex_t lock;

/*
 ******************************************************************************
 * GLOBAL AND HELPER FUNCTIONS
 ******************************************************************************
 */
ReturnCode gpio_init(void)
{
	int fd_exportGPIO = 0;
	int fd_dirGPIO = 0;
	int fd_edgeGPIO = 0;
	int fd_unexport	= 0;
	int ret = 0;
	char buf[SIZE];
	char buf_tmp[SIZE];
	struct stat sts;	

	/* Export the control of interrupt GPIO to user space */

	/* if already exported, first unexport it */
	sprintf(buf, "/sys/class/gpio/gpio%d", PLTF_GPIO_INTR_PIN);
	if (!stat (buf, &sts)) {
		fd_unexport = open("/sys/class/gpio/unexport", O_WRONLY);
		if (fd_unexport < 0) {
			printf("Error: opening gpio export file in gpio_init\n");
			goto error;
		}
		sprintf(buf_tmp, "%d", PLTF_GPIO_INTR_PIN);
        	ret = write(fd_unexport, buf_tmp, strlen(buf_tmp));
		if (ret <= 0) {
			printf("Error: writing to gpio export file in gpio_init\n");
			goto error;
		}	
	}
	
	fd_exportGPIO = open("/sys/class/gpio/export", O_WRONLY);
	if (fd_exportGPIO < 0)  {
		printf ("Error: opening gpio export file for interrupt pin\n");
		goto error;
	}

	sprintf(buf, "%d", PLTF_GPIO_INTR_PIN);
	ret = write(fd_exportGPIO, buf, strlen(buf)); 
	if(ret < 0) {
		printf("Error: writing interrupt pin to gpio export file\n");
		goto error;
	}
	 
	/* set the direction of interrupt pin */
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", PLTF_GPIO_INTR_PIN);
	fd_dirGPIO = open(buf, O_WRONLY);
	if (fd_dirGPIO < 0) {
		printf("Error: opening gpio direction file for interrupt pin\n");
		goto error;
	}
	
	ret = write(fd_dirGPIO, "in", 3);
	if (ret <= 0) {
		printf("Error: writing gpio direction for interrupt pin\n");
		goto error;
	}

	sprintf(buf, "/sys/class/gpio/gpio%d/value", PLTF_GPIO_INTR_PIN);
	fd_readGPIO = open(buf, O_RDWR);
	if (fd_readGPIO < 1) {
		printf("Error: opening gpio value file for interrupt pin\n");
		goto error;
	}

	sprintf(buf, "/sys/class/gpio/gpio%d/edge", PLTF_GPIO_INTR_PIN);
	fd_edgeGPIO = open(buf, O_WRONLY);
	if (fd_edgeGPIO < 1) {
		printf("Error: opening edge file to configure interrupt pin\n");
		goto error;
	}
	ret = write(fd_edgeGPIO, "rising", 7);
	if (ret <= 0) {
		printf("Error: writing gpio edge setting for interrupt pin\n");
		goto error;
	}	
	ret = pthread_mutex_init(&lock, NULL);
	if (ret != 0)
	{
		printf("Error: mutex init to protect interrupt status is failed\n");
		goto error;
	}

	isGPIOInit = 1;

error: 
	if (fd_exportGPIO > 0)
		close(fd_exportGPIO);
	if (fd_dirGPIO > 0)
		close(fd_dirGPIO);
	if (fd_edgeGPIO > 0)
		close(fd_edgeGPIO);
	if (fd_unexport > 0)
		close(fd_unexport);
	if(isGPIOInit)
		return ERR_NONE;
	else
		return ERR_IO;

}


GPIO_PinState gpio_readpin(int port, int pin_no) 
{
	char value;
	GPIO_PinState state;
	int ret;

	/* First check if GPIOInit is done or not */
	if (!isGPIOInit) {
		printf("GPIO is not initialized\n");
		return ERR_WRONG_STATE;
	}

	lseek(fd_readGPIO, 0, SEEK_SET);
	ret = read(fd_readGPIO, &value, 1);
	if (ret < 0) {
		printf("Error: while reading GPIO pin state\n");
		return ERR_IO;
	}

	if (value == '0') {
		return GPIO_PIN_RESET;
	} else {
		return GPIO_PIN_SET;
	}
}

void* pthread_func()
{
	int ret = 0;
	int c = 0; 
	uint16_t event;
	struct pollfd poll_fd;
	poll_fd.fd = fd_readGPIO; 
	poll_fd.events = POLLPRI;

	/* First check if GPIOInit is done or not */
	if (!isGPIOInit) {
		printf("GPIO is not initialized\n");
		return NULL;
	}

	/* poll interrupt line forever */
	while(true) 
	{ 	
		ret = poll(&poll_fd, 1, -1);
		if (ret < 1) {
			printf("Error: in polling for interrupt line\n");
			return NULL; 
		}
		if (poll_fd.revents & (POLLPRI|POLLERR))
		{
			lseek(poll_fd.fd, 0, SEEK_SET);
			read(poll_fd.fd, &c, 1);
			/* Call RFAL Isr */
			st25r3911Isr();
		}	
	}

	return NULL;
}

ReturnCode interrupt_init(void)
{
	pthread_t intr_thread;
	struct sched_param params;
	int ret;

	/* create a pthread to poll for interrupt */
	ret = pthread_create(&intr_thread, NULL, pthread_func, NULL);
	if (ret) {
		printf("Error: poll thread creation %d\n", ret);
		return ERR_IO;
	}
	
	/* Assign highest priority to polling thread */
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	ret = pthread_setschedparam(intr_thread, SCHED_FIFO, &params);
	if (ret) {
		printf("Error: assigning high priority to polling thread\n");
		return ERR_IO;
	}	

	return ERR_NONE;
}

void gpio_set(int port, int pin_no) 
{
	char buf[SIZE];
	char buf_tmp[SIZE];
	char direction;
	int fd_export = 0;
        int fd_value = 0;
	int fd_dir = 0;
	int ret = 0;
	struct stat sts;

	/* check if gpio pin is exported in user space */ 
	sprintf(buf, "/sys/class/gpio/gpio%d", pin_no);
	if (stat (buf, &sts)) {
		/* export the pin in user space first */
		fd_export = open("/sys/class/gpio/export", O_WRONLY);
		if (fd_export < 0) {
			printf("Error: gpio_set() opening gpio export file\n");
			goto error;
		}
		sprintf(buf_tmp, "%d", pin_no);
        	ret = write(fd_export, buf_tmp, strlen(buf_tmp));
		if(ret <= 0) {
			printf("Error: gpio_set() writing to export file\n");
			goto error; 
		}	
	}

	/* set the direction of gpio pin as out if not already */
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin_no);
	fd_dir = open(buf, O_RDWR);
	lseek(fd_dir, 0, SEEK_SET);
	ret = read(fd_dir, &direction, 1);
	/* By reading first byte direction can be determined */
	if (direction == 'i') 
	{
		ret = write(fd_dir, "out", 4);	
		if (ret <= 0){
			printf("Error: gpio_set() writing to dir file\n"); 
			goto error;
		}
	}  


	/* set the value as high */
	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin_no);
	fd_value = open(buf, O_WRONLY);
	if (fd_value < 0) {
		printf("Error: gpio_set() opening GPIO value file \n");
		goto error;
	}
	
	ret = write(fd_value, "1", 1);
	if (ret < 0) {
		printf("Error: gpio_set() writiing gpio value\n");
		goto error;
	}
	
	if (fd_export)
		close(fd_export);
	if (fd_dir)
		close(fd_dir);
	close(fd_value);

error:
	if (fd_export)
		close(fd_export);
	if (fd_dir)
		close(fd_dir);
	if (fd_value)
		close(fd_value);
}

void gpio_clear(int port, int pin_no) 
{
	char buf[SIZE];
	char buf_tmp[SIZE];
	int fd_export = 0;
        int fd_value = 0;
	int fd_dir = 0;
	char direction;
	int ret = 0;
	struct stat sts;

	/* check if gpio pin is exported in user space */ 
	sprintf(buf, "/sys/class/gpio/gpio%d", pin_no);
	if (stat (buf, &sts)) {
		/* export the pin in user space first */
		fd_export = open("/sys/class/gpio/export", O_WRONLY);
		if (fd_export < 0) {
			printf("Error: gpio_clear() opening gpio export file\n");
			goto error;
		}
		sprintf(buf_tmp, "%d", pin_no);
        	ret = write(fd_export, buf_tmp, strlen(buf_tmp));
		if(ret <= 0) {
			printf("Error: gpio_clear() writing to export file\n");
			goto error; 
		}	
	}

	/* set the direction of gpio pin as out if not already*/
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin_no);
	fd_dir = open(buf, O_RDWR);
	lseek(fd_dir, 0, SEEK_SET);
	ret = read(fd_dir, &direction, 1);
	if (direction == 'i') 
	{
		ret = write(fd_dir, "out", 4);	
		if (ret <= 0){
			printf("Error: gpio_set() writing to dir file\n"); 
			goto error;
		}
	}  

	

	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin_no);
	fd_value = open(buf, O_WRONLY);
	if (fd_value < 0) {
		printf("Error: gpio_clear() opening GPIO value file \n");
		goto error;
	}
	
	ret = write(fd_value, "0", 1);
	if (ret < 0) {
		printf("Error: gpio_clear() writiing gpio value\n");
		goto error;
	}

	if (fd_export)
		close(fd_export);
	if (fd_dir)
		close(fd_dir);
	close(fd_value);

error:
	if (fd_export)
		close(fd_export);
	if (fd_dir)
		close(fd_dir);
	if (fd_value)
		close(fd_value);
}

void pltf_protect_interrupt_status(void)
{
	pthread_mutex_lock(&lock);
}

void pltf_unprotect_interrupt_status(void)
{
	pthread_mutex_unlock(&lock);
}

/*
* Provided by Bostin Technology
*
* Based on the Demo software provided by 
*     COPYRIGHT 2018 STMicroelectronics
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
*/

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <stdio.h>
#include "st_errno.h"
#include "platform.h"
#include "rfal_analogConfig.h"
#include "rfal_nfca.h"
#include "example_poller.h"
/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define logUsart	printf

int splashscreen(void)
{
    printf("***********************************************");
    printf("*             Bostin Technology               *");
    printf("*                                             *");
    printf("*                NFC2 Reader                  *");
    printf("*                                             *");
    printf("*           Based on the ST25R3911B           *");
    printf("*             demo provided by ST             *");
    printf("*                                             *");
    printf("*        for more info www.cognIoT.eu         *");
    printf("***********************************************\n");
    return (0);
}
/*
 ******************************************************************************
 * MAIN FUNCTION
 ******************************************************************************
 */
int main(void)
{
	//logUsart("Welcome to the ST25R3911B NFC Poller Demo on Linux..\n");
	setlinebuf(stdout);
	int ret = 0;

    splashscreen();

	/* Initialize the platform */
	/* Initialize GPIO */
  	ret = gpio_init();
	if(ret != ERR_NONE)
		goto error;
	/* Initialize SPI */
	ret = spi_init();
	if(ret != ERR_NONE)
		goto error;
	/* Initialize interrupt mechanism */
	ret = interrupt_init();
	if (ret != ERR_NONE)
		goto error;

	/* Initialize rfal and run example code for NFCA */
	exampleRfalPollerRun();

error:
	return ret;	 
}



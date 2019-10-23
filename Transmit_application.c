/*
 * Application.c
 *
 *  Created on: Oct 20, 2019
 *      Author: Mariam Taha
 */

#include "BCM.h"
#include "lcd.h"

#define buffer_size 10

uint8 Data_buffer[buffer_size]={7,8,4,5,0,9,2,3,1,6};

BCM_ConfigType BCN_cofg = {9600};

void BCM_send_status(EnmBCMError_t status)
{
	if(status == success)
	{
		LCD_displayString("Done");
	}
}

int main(void)
{
	EnmBCMError_t BCM_Error;

	LCD_init();
	_delay_ms(1000);
	BCM_Init(&BCN_cofg);

	BCM_Error= BCM_Send(Data_buffer,buffer_size);

	if(BCM_Error == fail)
	{
		LCD_displayString("Buffer is Locked");
	}
	else if (BCM_Error == success)
	{
		LCD_displayString("Send ");
	}

	BCM_Send_SetCallback(BCM_send_status);


	while(1)
	{
		BCM_TxDispatch();
	}
}

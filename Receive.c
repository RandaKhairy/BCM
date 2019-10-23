/*
 * Application.c
 *
 *  Created on: Oct 20, 2019
 *      Author: Mariam Taha
 */

#include "BCM.h"
#include "lcd.h"

#define buffer_size 10

uint8 Data_buffer[buffer_size]={0};

BCM_ConfigType BCN_cofg = {9600};

uint8 complete_flag = 0;

void BCM_receive_status(EnmBCMError_t status)
{
	if(status == success)
	{
		LCD_clearScreen();
		LCD_displayString("Done");
		complete_flag = 1;

	}
}

int main(void)
{
	uint8 loop_index;

	LCD_init();
	BCM_Init(&BCN_cofg);

	Set_Rx_buffer(Data_buffer);
	BCM_Receive_SetCallback(BCM_receive_status);
	BCM_Receive();

	while(1)
	{
		BCM_RxDispatch();

		if(complete_flag == 1)
		{
			for(loop_index=0;loop_index<buffer_size;loop_index++)
			{
				LCD_intgerToString(Data_buffer[loop_index]);
			}
			BCM_Rx_buffer_unlock();
			complete_flag = 0;
		}

	}
}

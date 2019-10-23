/*
 * BCM.c
 *
 *  Created on: Oct 20, 2019
 *      Author: Mariam Taha
 */

#include "BCM.h"

//Send data buffer
static Str_BCM_buffer Tx_buffer;
//Receive data buffer
static Str_BCM_buffer Rx_buffer;
//Initial state of send
static uint8 g_state = IDLE;
//Initial state of receive
static uint8 g_Receive_state = IDLE;
//indicate sending is finish
static volatile uint8 g_Sending_complete_flag=0;
//frame bytes data counter
static uint16 Frame_data_counter=0;
static uint16 Tx_Frame_data_counter=0;
// data bytes counter
static uint16 data_counter=0;
// ID flag
uint8 g_Rx_ID_flag=0;
uint8 g_Tx_ID_flag=0;
//receive data variable
uint8 g_Rx_Data=0;

static ptrBcmFunctionCallback g_BCM_Send_callBack = NULL;
static ptrBcmFunctionCallback g_BCM_Receive_callBack = NULL;

EnmBCMError_t BCM_Init (const BCM_ConfigType * ConfigPtr)
{
	EnmBCMError_t BCM_init_status;
	UART.baud_rate = ConfigPtr->baudrate;
	BCM_init_status = UART_init(&UART);

	return BCM_init_status;
}


EnmBCMError_t BCM_Send(uint8* data_BufferAddress, uint32 Data_size)
{
	EnmBCMError_t BCM_send_status;

	UART_TX_set_callback(Send_ISR_CallBack);

	if(Tx_buffer.Lock_flag == 0)
	{
		g_state = SENDING_BYTE;
		Tx_buffer.ID = BCM_ID;
		Tx_buffer.data_size = Data_size;

		//pointer is NULL error
		Tx_buffer.data_BufferAddress = data_BufferAddress;
		Tx_buffer.CheckSum = 0;
		Tx_buffer.Lock_flag = 1;

		BCM_send_status = Sending_frame_success;
		return BCM_send_status;
	}
	else
	{
		g_state = IDLE;
		BCM_send_status = Sending_frame_fail;
		return BCM_send_status;
	}
}

void Set_Rx_buffer(uint8* RX_data_buffer)
{
	Rx_buffer.data_BufferAddress = RX_data_buffer;
}

void BCM_Receive()
{
	Rx_buffer.ID = BCM_ID;
	Rx_buffer.data_size = NA;
	Rx_buffer.CheckSum = 0;
	UART_RX_set_callback(BCM_Rx_ISR_saveData);
	g_Receive_state = RECEIVE_BYTE;
}

void BCM_Rx_ISR_saveData(uint8 data_received)
{
	g_Rx_Data = data_received;

	if(Rx_buffer.ID == g_Rx_Data)
	{
		g_Rx_ID_flag=1;
		Frame_data_counter++;
		g_Receive_state = RECEIVE_BYTE_COMPLETE;
	}
	else if((g_Rx_ID_flag==1)&&(Rx_buffer.Lock_flag==0))
	{
		if(Frame_data_counter==1)
		{
			Rx_buffer.data_size = g_Rx_Data;
			Frame_data_counter++;
			g_Receive_state = RECEIVE_BYTE_COMPLETE;
		}
		else if(Frame_data_counter==2)
		{
			Rx_buffer.data_size |= (g_Rx_Data<<8);
			Frame_data_counter=0;
			g_Receive_state = RECEIVE_BYTE_COMPLETE;
		}
		else
		{
			*(Rx_buffer.data_BufferAddress) = g_Rx_Data;
			Rx_buffer.CheckSum += g_Rx_Data;
			data_counter++;
			g_Receive_state = RECEIVE_BYTE_COMPLETE;
		}
	}
}

void BCM_Rx_buffer_unlock(void)
{
	Rx_buffer.Lock_flag = 0;
}

void BCM_TxDispatch(void)
{
	static uint16 data_counter=0;
	static uint8 column = 0;
	EnmBCMError_t dispatcherStatus = Sending_frame_fail;
	switch (g_state)
	{
	case IDLE:
		//PORTB |=(1<<PB0);
		data_counter = 0;
		Tx_Frame_data_counter = 0;
		break;
	case SENDING_BYTE:
	{
		//PORTB |=(1<<PB1);
		//PORTB &=~(1<<PB2);
		g_Sending_complete_flag=0;
		Tx_Frame_data_counter++;
       // LCD_intgerToString(Tx_Frame_data_counter);
		if(Tx_Frame_data_counter==1)
		{
			UART_sendByte(Tx_buffer.ID);
			g_Tx_ID_flag=1;
			LCD_goToRowColumn(0,0);
			LCD_intgerToString(Tx_buffer.ID);
			g_state = SENDING_BYTE_COMPLETE;
		}
		else if(Tx_Frame_data_counter==2)
		{
			UART_sendByte((uint8)(Tx_buffer.data_size));
			LCD_goToRowColumn(0,4);
			LCD_intgerToString((uint8)(Tx_buffer.data_size));
			g_state = SENDING_BYTE_COMPLETE;
		}
		else if(Tx_Frame_data_counter==3)
		{
			UART_sendByte((uint8)(Tx_buffer.data_size >> 8));
			LCD_goToRowColumn(0,7);
			LCD_intgerToString((uint8)(Tx_buffer.data_size >> 8));
			g_state = SENDING_BYTE_COMPLETE;
		}
		else if(g_Tx_ID_flag==1)
		{
			Tx_buffer.CheckSum += *(Tx_buffer.data_BufferAddress);
			UART_sendByte(*(Tx_buffer.data_BufferAddress));
			LCD_goToRowColumn(1,column);
			LCD_intgerToString(*(Tx_buffer.data_BufferAddress));
			(Tx_buffer.data_BufferAddress)++;
			data_counter++;
			g_state = SENDING_BYTE_COMPLETE;
			column++;
		}
	}
	break;
	case SENDING_BYTE_COMPLETE:
	{
		g_Sending_complete_flag=0;
		PORTB |=(1<<PB2);
		PORTB &=~(1<<PB1);
		if(data_counter == Tx_buffer.data_size)
		{
			//send checksum
			UART_sendByte(Tx_buffer.CheckSum);
			LCD_goToRowColumn(0,9);
			LCD_intgerToString(Tx_buffer.CheckSum);
			g_state = SENDING_FRAME_COMPLETE;
		}
		else
		{
			g_state = SENDING_BYTE;
		}
	}
	break;
	case SENDING_FRAME_COMPLETE:
	{
		PORTB |=(1<<PB3);

		//data send call back function
		dispatcherStatus = Sending_frame_success;
		if(g_BCM_Send_callBack != NULL)
		{
			//call back function to inform the application if sending is success or fail
			g_BCM_Send_callBack(dispatcherStatus);
		}
		Tx_buffer.Lock_flag = 0;
		g_state = IDLE;
	}
	break;
	}
}

void BCM_RxDispatch(void)
{
	switch(g_Receive_state)
	{
	case IDLE:
		data_counter = 0;
		break;
	case RECEIVE_BYTE:
		break;
	case RECEIVE_BYTE_COMPLETE:
	{
		if(g_Rx_ID_flag==1)
		{
			if(data_counter==Rx_buffer.data_size)
			{
				if(Rx_buffer.CheckSum == g_Rx_Data)
				{
					//status sucess
				}
				g_Rx_ID_flag = 0;
				g_Receive_state = RECEIVE_FRAME_COMPLETE;
			}
			else
			{
				g_Receive_state = RECEIVE_BYTE;
			}
		}
	}
	break;
	case RECEIVE_FRAME_COMPLETE:

		Rx_buffer.Lock_flag = 1;
		// callback app
		g_Receive_state = IDLE;
		break;
	}
}
void Send_ISR_CallBack(void)
{
	g_Sending_complete_flag=1;
	g_state = SENDING_BYTE_COMPLETE;
}

void BCM_Send_SetCallback(ptrBcmFunctionCallback returnStatus)
{
	g_BCM_Send_callBack = returnStatus;
}

void BCM_Receive_SetCallback(ptrBcmFunctionCallback returnStatus)
{
	g_BCM_Receive_callBack = returnStatus;
}


/*EnmBCMError_t BCM_DeInit (void)
{

}*/

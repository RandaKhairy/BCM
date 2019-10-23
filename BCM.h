/*
 * BCM.h
 *
 *  Created on: Oct 20, 2019
 *      Author: Mariam Taha
 */

#ifndef BCM_H_
#define BCM_H_

#include "BCM_cnfg.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "std_types.h"
#include "UART.h"
#include "UART_Pbcfg.h"
#include "LCD.h"


#define BCM_ID  0xA1

#define IDLE 0
#define SENDING_BYTE 1
#define RECEIVE_BYTE 1
#define SENDING_BYTE_COMPLETE 2
#define RECEIVE_BYTE_COMPLETE 2

#define SENDING_FRAME_COMPLETE 3
#define RECEIVE_FRAME_COMPLETE 3

#define NA 0xff

typedef void(*ptrBcmFunctionCallback)(uint8);

typedef struct{
	uint8 ID;
	uint16 data_size;
    uint8* data_BufferAddress;
    uint8 CheckSum;
    uint8 Lock_flag;
}Str_BCM_buffer;

typedef enum {Sending_frame_fail,Sending_frame_success} EnmBCMError_t;

EnmBCMError_t BCM_Init (const BCM_ConfigType* ConfigPtr);
EnmBCMError_t BCM_Send(uint8* data_BufferAddress, uint32 Data_size);
void BCM_TxDispatch(void);
EnmBCMError_t BCM_DeInit (void);
void BCM_Rx_ISR_saveData(uint8 data_received);
void BCM_Send_SetCallback(ptrBcmFunctionCallback returnStatus);
void Send_ISR_CallBack(void);

#endif /* BCM_H_ */

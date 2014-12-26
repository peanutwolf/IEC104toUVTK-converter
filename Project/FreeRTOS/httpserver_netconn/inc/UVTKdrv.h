#ifndef UVTK_DRV_H
#define UVTK_DRV_H

#include "sys_arch.h"
#include "stm32f4xx_spi.h"
#include <stdio.h>
#include "STM324x7i_eval.h"
#include "timers.h"

#define UVTK_TASK_PRIO    				( tskIDLE_PRIORITY + 2 )
#define UVTK_INROGEN_MSG_SIZE			5
#define UVTK_INROGEN_QUEUE_SIZE		5 //5
#define UVTK_TS_GR_SIZE   				8 //8
#define UVTK_TS_GR_NUM	   				2 //2
#define UVTK_TI_GR_SIZE   				16
#define UVTK_TI_GR_NUM	   				2 //2
#define UVTK_POLL_DELAY   				20000 		//20000
#define KP_ADR										0x19
#define UVTK_INV_CODE							0x03
#define NUM_TIMERS 							  4
#define IV_DELAY 							  	19000 			//40000
#define INROGEN_TIMER_ID 					NUM_TIMERS + 1
#define INROGEN_TIMER_DELAY				2000


typedef struct{
	xTimerHandle xUVTKTimer;
	uint8_t timer_counter:5;
	uint8_t timer_started:1;
	uint8_t data_type:5;
	uint8_t is_iv:1;
}__attribute__((__packed__))UVTKState;

void UVTK_init_task(void);
void UVTK_set_inv(uint8_t inv);
void stopUVTKTimers(void);

void UVTK_poll(void * pvParameters);
void UVTK_TS_poll(void * pvParameters);
void vUVTKTimerCallback(xTimerHandle pxTimer);

#endif

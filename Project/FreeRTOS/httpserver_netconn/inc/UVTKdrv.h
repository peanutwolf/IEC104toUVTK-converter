#ifndef UVTK_DRV_H
#define UVTK_DRV_H

#include "sys_arch.h"
#include "stm32f4xx_spi.h"
#include <stdio.h>
#include "STM324x7i_eval.h"

#define UVTK_TASK_PRIO    				( tskIDLE_PRIORITY + 2 )
#define UVTK_INROGEN_MSG_SIZE			5
#define UVTK_INROGEN_QUEUE_SIZE		5
#define UVTK_TS_GR_SIZE   				8
#define UVTK_TI_GR_SIZE   				16
#define KP_ADR										0x19

void UVTK_init_task(void);

void UVTK_poll(void * pvParameters);
void UVTK_TS_poll(void * pvParameters);

#endif

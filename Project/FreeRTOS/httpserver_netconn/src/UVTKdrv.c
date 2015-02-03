/**
  * @file    UVTKdrv.c
  * @author  Vigurskiy.ES
  * @version V0.0.1
  * @date    05.03.2014
  * @brief   
  */

/******************************************/
/***************TODO***********************/
/* 1. make correct inversion set to UVTK - OK */
/******************************************/

#include "UVTKdrv.h"

xSemaphoreHandle xSPI_UVTK_Mutex = NULL;
xSemaphoreHandle xSPI_UVTK_Semaphore = NULL;
xTimerHandle xUVTKTimer = NULL;
volatile uint8_t SPI_data = 0x00;
UVTKState UVTK_timer[NUM_TIMERS] = {{NULL, 0x00, 0x00, 0x0C, 0x01}, {NULL, 0x00, 0x00, 0x1C, 0x01},
																		{NULL, 0x00, 0x00, 0x05, 0x01}, {NULL, 0x00, 0x00, 0x15, 0x01}};
uint8_t UVTK_inrogenTS 	[UVTK_INROGEN_MSG_SIZE] = {0x02, 0xFF, 0x24, 0x0C, 0x00};
uint8_t UVTK_inrogenTI1 [UVTK_INROGEN_MSG_SIZE] = {0x02, 0xFF, 0x24, 0x05, 0x00};
uint8_t UVTK_inrogenTI2 [UVTK_INROGEN_MSG_SIZE] = {0x02, 0xFF, 0x24, 0x15, 0x00};
uint8_t** UVTK_inrogenData = NULL;
uint8_t* UVTK_ts_grp_data = NULL;
uint16_t* UVTK_ti_grp_data = NULL;


void UVTK_init_task(){
	uint8_t i =0;
	
	xSPI_UVTK_Mutex = xSemaphoreCreateMutex();
	vSemaphoreCreateBinary(xSPI_UVTK_Semaphore);
  xSemaphoreTake( xSPI_UVTK_Semaphore, portMAX_DELAY );	  // Take once just created semaphore
	
	UVTK_inrogenTS[1] = UVTK_KP_ADR;
	UVTK_inrogenTI1[1] = UVTK_KP_ADR;
	UVTK_inrogenTI2[1] = UVTK_KP_ADR;
	
	form_UVTK_inrogen_data_mas();
	form_UVTK_ts_data_mas();
	form_UVTK_ti_data_mas();
	
	if(xSPI_UVTK_Mutex != NULL && xSPI_UVTK_Semaphore != NULL){
		UVTK_set_inv(UVTK_INV_CODE);
		for(i = 0; i < NUM_TIMERS; i++){
			UVTK_timer[i].xUVTKTimer = xTimerCreate((const signed char *)"UVTKTimer", (IV_DELAY), pdTRUE, (void*)i, vUVTKTimerCallback);
		}
		xTaskCreate(UVTK_TS_poll, (int8_t *) "UVTK_TS_poll", configMINIMAL_STACK_SIZE, NULL, UVTK_TASK_PRIO, NULL);
		xTaskCreate(UVTK_poll, (int8_t *) "UVTK_poll", configMINIMAL_STACK_SIZE, NULL, UVTK_TASK_PRIO, NULL);		
	}
	else{
		STM_EVAL_LEDOn(LED2);
		for(;;){}
			//Smth went wrong cant create mutex or semaphore
	}
}
	
	


/**
  * @brief  UVTK Poll Task
	* Uses SPI3 to receive UVTK message from UVTK_module,
	* than puts TS and TI data accordingly to TS and TI arrays.
	* Moreover stops and zeroes IV timers if any valuable data received.
  * @param  pvParameters not used
  * @retval None
	* @TODO 	If we receive message ... 0xFF 0x7E, than we have not fixed bug,
  */
void UVTK_poll(void * pvParameters)
{
	uint8_t UVTK_msg_cnt = 0;
	uint8_t UVTK_msg_type = 0;
	uint8_t UVTK_msg_data_size = 0;
	uint8_t i = 0;
  for( ;; )
  {
	  xSemaphoreTake( xSPI_UVTK_Mutex, portMAX_DELAY );
    while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
			SPI_I2S_SendData(SPI3, 0x00);
		xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
		UVTK_msg_cnt = SPI_data;
		for(i = 0; i < UVTK_msg_cnt; i++){
			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
				vTaskDelay(1);
				SPI_I2S_SendData(SPI3, 0x00);		
			xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
			printf("in for 0x%08x\n",SPI_data);
			if(i == 0x03){
				UVTK_msg_type = SPI_data;		
				switch(UVTK_msg_type){
				case 0x0C:
					UVTK_timer[0].is_iv = 0x00;
					UVTK_timer[0].timer_started = 0x00;
					xTimerStop(UVTK_timer[0].xUVTKTimer, 0);
					UVTK_msg_data_size = UVTK_TS_GR_SIZE + 0x04;
				  break;
				case 0x1C:
					UVTK_timer[1].is_iv = 0x00;
					UVTK_timer[1].timer_started = 0x00;
					xTimerStop(UVTK_timer[1].xUVTKTimer, 0);
					UVTK_msg_data_size = UVTK_TS_GR_SIZE + 0x04;
				  break;
				case 0x05:
					UVTK_timer[2].is_iv = 0x00;
					UVTK_timer[2].timer_started = 0x00;
					xTimerStop(UVTK_timer[2].xUVTKTimer, 0);
					UVTK_msg_data_size = UVTK_TI_GR_SIZE + 0x04;
					break;
				case 0x15:
					UVTK_timer[3].is_iv = 0x00;
					UVTK_timer[3].timer_started = 0x00;
					xTimerStop(UVTK_timer[3].xUVTKTimer, 0);
					UVTK_msg_data_size = UVTK_TI_GR_SIZE + 0x04;
					break;
				default:
					break;
				}
			}
			else if((i > 0x03) && (i < UVTK_msg_data_size)){
				switch(UVTK_msg_type){
					case 0x0C:
						UVTK_ts_grp_data[i-0x04] = SPI_data;
						break;
					case 0x1C:
						UVTK_ts_grp_data[i-0x04 + UVTK_TS_GR_SIZE] = SPI_data;
						break;
					case 0x05:
						UVTK_ti_grp_data[i-0x04] = (uint16_t)SPI_data;
						break;
					case 0x15:
						UVTK_ti_grp_data[i-0x04 + UVTK_TI_GR_SIZE] = (uint16_t)SPI_data;
						break;
					default:				
						break;
				}
			}
			
		}
	  xSemaphoreGive(xSPI_UVTK_Mutex);
    vTaskDelay(100);
  }
}

/**
  * @brief  UVTK TS cmd Poll Task
  * @param  pvParameters not used
  * @retval None
  */
void UVTK_TS_poll(void * pvParameters)
{
	uint8_t i = 0;
	uint8_t n = 0;
	uint8_t grp_num = (UVTK_TS_GR_NUM > 0 ? 1 : 0) + UVTK_TI_GR_NUM;
  for( ;; )
  {
//		printf("(UVTK_TS_GR_NUM > 0 ? 1 : 0) + UVTK_TI_GR_NUM %d\n", (UVTK_TS_GR_NUM > 0 ? 1 : 0) + UVTK_TI_GR_NUM);
		for(i = 0; i < grp_num; i++){
			xSemaphoreTake( xSPI_UVTK_Mutex, portMAX_DELAY );
			UVTK_set_inv(UVTK_INV_CODE);
			vTaskDelay(SPI_UVTK_DELAY);
			SPI_SendDataArray(SPI3, (const uint8_t*)UVTK_inrogenData[i], UVTK_INROGEN_MSG_SIZE);
			for(n = 0; (UVTK_inrogenData[i][3] != UVTK_timer[n].data_type) && (n < NUM_TIMERS); n++){}
			if((n == 0) && (UVTK_TS_GR_NUM > 1)){
				if(UVTK_timer[n+1].timer_started != 0x01){
					xTimerReset(UVTK_timer[n+1].xUVTKTimer, 0);
					xTimerStart(UVTK_timer[n+1].xUVTKTimer, 0);
					UVTK_timer[n+1].timer_started = 0x01;
				}
			}
			if(UVTK_timer[n].timer_started != 0x01){
				xTimerReset(UVTK_timer[n].xUVTKTimer, 0);
				xTimerStart(UVTK_timer[n].xUVTKTimer, 0);
				UVTK_timer[n].timer_started = 0x01;
			}
			STM_EVAL_LEDToggle(LED4);
			xSemaphoreGive(xSPI_UVTK_Mutex);
			vTaskDelay(UVTK_POLL_DELAY * 1000); 
		}
  }
}

void UVTK_set_inv(uint8_t inv){
	 while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
					vTaskDelay(1);
					SPI_I2S_SendData(SPI3, 0x01);
		xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
	 while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
					vTaskDelay(1);
					SPI_I2S_SendData(SPI3, inv);
		xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
		while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
					SPI_I2S_SendData(SPI3, 0x00);
					vTaskDelay(1);
		xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
}

void vUVTKTimerCallback( xTimerHandle pxTimer ){
	uint8_t timerIndex;
	
	configASSERT(pxTimer);
	timerIndex = (uint8_t) pvTimerGetTimerID( pxTimer );
	if(timerIndex == INROGEN_TIMER_ID){
		xTimerStop(pxTimer, 0);
	}
	else{
		UVTK_timer[timerIndex].is_iv = 0x01;
		UVTK_timer[timerIndex].timer_started = 0x00;
		xTimerStop(pxTimer, 0);
		printf("IV Timer! %d\n", UVTK_timer[timerIndex].data_type);
	}
}

void stopUVTKTimers(){
	uint8_t i = 0;
	for(i = 0; i < NUM_TIMERS; i++){
			UVTK_timer[i].is_iv = 0x00;
			UVTK_timer[i].timer_started = 0x00;
			xTimerStop(UVTK_timer[i].xUVTKTimer, 0);
	}
}

void form_UVTK_inrogen_data_mas(void){
		
		uint8_t len = (UVTK_TS_GR_NUM > 0 ? 1 : 0) + UVTK_TI_GR_NUM;
//		printf("len %d \n", len);
//		printf("UVTK_TS_GR_NUM %d \n", UVTK_TS_GR_NUM);
//		printf("UVTK_TI_GR_NUM %d \n", UVTK_TI_GR_NUM);
//		printf("len*sizeof(uint8_t*) %d \n", len*sizeof(uint8_t*));
		uint8_t i = 0;
		UVTK_inrogenData = (uint8_t**)pvPortMalloc(len*sizeof(uint8_t*));
		if(UVTK_TS_GR_NUM > 0){
			UVTK_inrogenData[i] = UVTK_inrogenTS;
			i++;
		}
		if(UVTK_TI_GR_NUM > 0){
			UVTK_inrogenData[i] = UVTK_inrogenTI1;
			i++;
		}
		if(UVTK_TI_GR_NUM > 1){
			UVTK_inrogenData[i] = UVTK_inrogenTI2;
			i++;
		}
}

void form_UVTK_ts_data_mas(void){
		uint8_t len = UVTK_TS_GR_NUM * UVTK_TS_GR_SIZE;
		UVTK_ts_grp_data = (uint8_t*)pvPortMalloc(len); 
}

void form_UVTK_ti_data_mas(void){
		uint8_t len = 2*UVTK_TI_GR_NUM * UVTK_TI_GR_SIZE;
		UVTK_ti_grp_data = (uint16_t*)pvPortMalloc(len);
		
}


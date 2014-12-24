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
const uint8_t UVTK_inrogenTS [UVTK_INROGEN_MSG_SIZE] = {0x02, KP_ADR, 0x24, 0x0C, 0x00};
const uint8_t UVTK_inrogenTI1 [UVTK_INROGEN_MSG_SIZE] = {0x02, KP_ADR, 0x24, 0x05, 0x00};
const uint8_t UVTK_inrogenTI2 [UVTK_INROGEN_MSG_SIZE] = {0x02, KP_ADR, 0x24, 0x15, 0x00};
const uint8_t* const UVTK_inrogenData[UVTK_INROGEN_QUEUE_SIZE] = {UVTK_inrogenTI1, UVTK_inrogenTI2, UVTK_inrogenTS, UVTK_inrogenTI1, UVTK_inrogenTI2};
//const uint8_t* const UVTK_inrogenData[UVTK_INROGEN_QUEUE_SIZE] = {UVTK_inrogenTS};
uint8_t UVTK_ts_grp_data[UVTK_TS_GR_SIZE*UVTK_TS_GR_NUM];
uint16_t UVTK_ti_grp_data[UVTK_TI_GR_SIZE*UVTK_TI_GR_NUM];


void UVTK_init_task(){
	uint8_t i =0;
	xSPI_UVTK_Mutex = xSemaphoreCreateMutex();
	vSemaphoreCreateBinary(xSPI_UVTK_Semaphore);
  xSemaphoreTake( xSPI_UVTK_Semaphore, portMAX_DELAY );	  // Take once just created semaphore
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
			//Smth went wrong cant create mutex or semaphore
	}
}
	
	


/**
  * @brief  UVTK Poll Task
  * @param  pvParameters not used
  * @retval None
	* @TODO 	If we have message 0xFF 0x7E than we have not fixed bug,
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
//			if(((i == 0x00) && (SPI_data != 0x7E)) || ((i == 0x01) && (SPI_data != KP_ADR)) || ((i == 0x02) && (SPI_data == 0x00))){
//					break;
//			}
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
  for( ;; )
  {
		
		for(i = 0; i < UVTK_INROGEN_QUEUE_SIZE; i++){
			xSemaphoreTake( xSPI_UVTK_Mutex, portMAX_DELAY );
			UVTK_set_inv(UVTK_INV_CODE);
			vTaskDelay(SPI_UVTK_DELAY);
			SPI_SendDataArray(SPI3, UVTK_inrogenData[i], UVTK_INROGEN_MSG_SIZE);
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
			vTaskDelay(UVTK_POLL_DELAY);
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
		printf("Timer! %d\n", UVTK_timer[timerIndex].data_type);
	}
}


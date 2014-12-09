#include "UVTKdrv.h"

xSemaphoreHandle xSPI_UVTK_Mutex = NULL;
xSemaphoreHandle xSPI_UVTK_Semaphore = NULL;
uint8_t SPI_data = 0x00;
const uint8_t UVTK_inrogenTS [UVTK_INROGEN_MSG_SIZE] = {0x02, KP_ADR, 0x24, 0x0C, 0x00};
const uint8_t UVTK_inrogenTI1 [UVTK_INROGEN_MSG_SIZE] = {0x02, KP_ADR, 0x24, 0x05, 0x00};
const uint8_t UVTK_inrogenTI2 [UVTK_INROGEN_MSG_SIZE] = {0x02, KP_ADR, 0x24, 0x15, 0x00};
const uint8_t* const UVTK_inrogenData[UVTK_INROGEN_QUEUE_SIZE] = {UVTK_inrogenTI1, UVTK_inrogenTI2, UVTK_inrogenTS, UVTK_inrogenTI1, UVTK_inrogenTI2};
uint8_t UVTK_1ts_grp_data[UVTK_TS_GR_SIZE];
uint8_t UVTK_2ts_grp_data[UVTK_TS_GR_SIZE];
uint8_t UVTK_1ti_grp_data[UVTK_TI_GR_SIZE];
uint8_t UVTK_2ti_grp_data[UVTK_TI_GR_SIZE];


void UVTK_init_task(){
	xSPI_UVTK_Mutex = xSemaphoreCreateMutex();
	vSemaphoreCreateBinary(xSPI_UVTK_Semaphore);
  xSemaphoreTake( xSPI_UVTK_Semaphore, portMAX_DELAY );	  // Take once just created semaphore
	if(xSPI_UVTK_Mutex != NULL && xSPI_UVTK_Semaphore != NULL){
		xTaskCreate(UVTK_poll, (int8_t *) "UVTK_poll", configMINIMAL_STACK_SIZE, NULL, UVTK_TASK_PRIO, NULL);
		xTaskCreate(UVTK_TS_poll, (int8_t *) "UVTK_TS_poll", configMINIMAL_STACK_SIZE, NULL, UVTK_TASK_PRIO, NULL);
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
    if(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == RESET){
					SPI_I2S_SendData(SPI3, 0x00);
		}
		xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
		//printf("0x%08x\n",SPI_data);
		UVTK_msg_cnt = SPI_data;
		for(i = 0; i < UVTK_msg_cnt; i++){
			if(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == RESET){
				vTaskDelay(1);
				SPI_I2S_SendData(SPI3, 0x00);		
			}
			xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
			printf("in for 0x%08x\n",SPI_data);
			if(i == 0x03){
				UVTK_msg_type = SPI_data;		
				switch(UVTK_msg_type & 0x0F){
				case 0x0C:
					UVTK_msg_data_size = UVTK_TS_GR_SIZE;
				  //printf("switch msg type 0x%08x\n",UVTK_msg_type);
				  break;
				case 0x05:
					UVTK_msg_data_size = UVTK_TI_GR_SIZE;
				  //printf("switch msg type 0x%08x\n",UVTK_msg_type);
					break;
				default:
					break;
				}
			}
			else if((i > 0x03) && (i < UVTK_msg_data_size)){
				switch(UVTK_msg_type){
					case 0x0C:
						UVTK_1ts_grp_data[i-0x04] = SPI_data;
						break;
					case 0x1C:
						UVTK_2ts_grp_data[i-0x04] = SPI_data;
						break;
					case 0x05:
						UVTK_1ti_grp_data[i-0x04] = SPI_data;
						break;
					case 0x15:
						UVTK_2ti_grp_data[i-0x04] = SPI_data;
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
  for( ;; )
  {
		for(i = 0; i < UVTK_INROGEN_QUEUE_SIZE; i++){
			xSemaphoreTake( xSPI_UVTK_Mutex, portMAX_DELAY );
			SPI_SendDataArray(SPI3, UVTK_inrogenData[i], UVTK_INROGEN_MSG_SIZE);
			xSemaphoreTake(xSPI_UVTK_Semaphore, portMAX_DELAY);
			STM_EVAL_LEDToggle(LED4);
			xSemaphoreGive(xSPI_UVTK_Mutex);
			vTaskDelay(30000);
		}
  }
}


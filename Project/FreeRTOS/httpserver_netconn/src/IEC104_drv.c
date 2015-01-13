/**
  * @file    IEC104_drv.c
  * @author  Vigurskiy.ES
  * @version V0.0.1
  * @date    05.03.2014
  * @brief   IEC104 gateway between UVTK data array
	 ******************************************************************************
  */
/******************************************/
/***************TODO***********************/
/* 1. make correct .iv flag set -OK!          */
/* 2. Check for mv functionality is correct - OK!						        */
/* 3. Make spon TI or TS to be chained in a single iframe -OK!						*/
/* 3. Make IWDT more robust check every task/ currently reloaded in LED_Blink			*/
/******************************************/

#include "IEC104_drv.h"

extern uint8_t UVTK_ts_grp_data[UVTK_TS_GR_SIZE*UVTK_TS_GR_NUM];
extern uint16_t UVTK_ti_grp_data[UVTK_TI_GR_SIZE*UVTK_TI_GR_NUM];


extern fifo_t* iec_fifo_buf;

extern UVTKState UVTK_timer[NUM_TIMERS];

struct iec_type1_data  ts_mas[IEC104_TS_SIZE] = {[0 ... IEC104_TS_SIZE-1] = {{0,0,0,0,0,1},0}};
struct iec_type9_data  ti_mas[IEC104_TI_SIZE] = {[0 ... IEC104_TI_SIZE-1] = {{0,0,0,0,0,0,1},0}};

xSemaphoreHandle xIEC104_Poll_Mutex = NULL;


void IEC104_init_task(){
	xIEC104_Poll_Mutex = xSemaphoreCreateMutex();
	xTaskCreate(IEC104_UVTK_TS_poll, (int8_t *) "IEC104_UVTK_TS_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_TS_poll, (int8_t *) "IEC104_TS_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_UVTK_TI_poll, (int8_t *) "IEC104_UVTK_TI_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_TI_poll, (int8_t *) "IEC104_TI_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_UVTK_IV_poll, (int8_t *) "IEC104_UVTK_IV_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
}

void IEC104_UVTK_TS_poll(void * pvParameters){
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t n = 0;
	uint8_t UVTK_ts_grp_data_tmp = 0;
	uint8_t iec104_gr_num = 0;
	uint8_t uvtk_gr_ts_tmp = 0;

	for(;;){
		xSemaphoreTake( xIEC104_Poll_Mutex, portMAX_DELAY );
		for(n = 0; n < UVTK_TS_GR_NUM; n++){
			for(i = 0; i < UVTK_TS_GR_SIZE; i++){
				UVTK_ts_grp_data_tmp = UVTK_ts_grp_data[i + UVTK_TS_GR_SIZE * n];
				for(j = 0; j < 8; j++){
					iec104_gr_num = (i*8) + j + UVTK_TS_GR_SIZE * 8 * n;	
					uvtk_gr_ts_tmp = (UVTK_ts_grp_data_tmp>>j) & 0x01;
					if(ts_mas[iec104_gr_num].sp.sp != uvtk_gr_ts_tmp){
						ts_mas[iec104_gr_num].sp.sp = uvtk_gr_ts_tmp;
						ts_mas[iec104_gr_num].spon = 0x01;
					}
				}
			}
		}
		xSemaphoreGive(xIEC104_Poll_Mutex);
		vTaskDelay(10);
	}
}

void IEC104_TS_poll(void * pvParameters){
	static uint16_t start = 0;
	static uint16_t end = 0;
	uint8_t i = 0;
	for(;;){
		xSemaphoreTake( xIEC104_Poll_Mutex, portMAX_DELAY );
		for(i = 0; i < IEC104_TS_SIZE; i++){
			if(ts_mas[i].spon != 0x00){		
				//printf("spon ts\n");				
				ts_mas[i].spon = 0x00;
				if(start == 0x00 ){
					start = i+1;
				}
				if(end < 0x0F){
					end++;
				}
				else{
					fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_SP_TB_1, start-1, end+1));
					start = 0x00;
					end = 0x00;
				}
				if(((i == IEC104_TS_SIZE-1) || (ts_mas[i+1].spon == 0x00)) && (start != 0x00)){
					fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_SP_TB_1, start-1, end));
					start = 0x00;
					end = 0x00;
				}
				stopUVTKTimers();
			}
			
		}
		xSemaphoreGive(xIEC104_Poll_Mutex);
		vTaskDelay(10);
	}
}


void IEC104_UVTK_TI_poll(void * pvParameters){
	uint8_t i = 0;
	uint8_t n = 0;
	uint16_t uvtk_gr_ti_tmp = 0;
	
	for(;;){
	 for(n = 0; n < UVTK_TI_GR_NUM; n++){
		for(i = 0; i < UVTK_TI_GR_SIZE; i++){
			uvtk_gr_ti_tmp = UVTK_ti_grp_data[i + UVTK_TI_GR_SIZE * n];
			if(ti_mas[i + UVTK_TI_GR_SIZE * n].mv.mv != uvtk_gr_ti_tmp){
				ti_mas[i + UVTK_TI_GR_SIZE * n].mv.mv = uvtk_gr_ti_tmp;
				ti_mas[i + UVTK_TI_GR_SIZE * n].spon = 0x01;
			}
		}
	 }
		vTaskDelay(10);
	}

}

void IEC104_TI_poll(void * pvParameters){
	static uint8_t start = 0;
	static uint8_t end = 0;
	uint8_t i = 0;
	for(;;){
		for(i = 0; i < IEC104_TI_SIZE; i++){
			if(ti_mas[i].spon != 0x00){
				ti_mas[i].spon = 0x00;
				//printf("spon ti\n");
				if(start == 0x00 ){
					start = i+1;
				}
				if(end < 0x0F){
					end++;
				}
				else{
					fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_ME_TD_1, start-1, end+1));
					start = 0x00;
					end = 0x00;
				}
				if(((i == IEC104_TI_SIZE-1) || (ti_mas[i+1].spon == 0x00)) && (start != 0x00)){
					fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_ME_TD_1, start-1, end));
					start = 0x00;
					end = 0x00;
				}				
			}
		}
		vTaskDelay(10);
	}
}

void IEC104_UVTK_IV_poll(void * pvParameters){
	uint8_t i = 0;
	for(;;){
		xSemaphoreTake( xIEC104_Poll_Mutex, portMAX_DELAY );
		if((UVTK_timer[0].is_iv != ts_mas[0].sp.iv) && (UVTK_TS_GR_NUM >= 1)){
			for(i = 0; i < UVTK_TS_GR_SIZE * 8; i++){
				ts_mas[i].sp.iv = UVTK_timer[0].is_iv;
			}
			for(i = 0; i < UVTK_TS_GR_SIZE * 8; i += 0x10){
				fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_SP_TB_1, i, 0x10));	
			}
		}
		if((UVTK_timer[1].is_iv != ts_mas[UVTK_TS_GR_SIZE * 8].sp.iv) && (UVTK_TS_GR_NUM >= 2)){
			//printf("spon iv ts2\n");	
			for(i = UVTK_TS_GR_SIZE * 8; i < UVTK_TS_GR_SIZE * 16; i++){
				ts_mas[i].sp.iv = UVTK_timer[1].is_iv;
			}
			for(i = UVTK_TS_GR_SIZE * 8; i < UVTK_TS_GR_SIZE * 16; i += 0x10){
				fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_SP_TB_1, i, 0x10));	
			}
		}
		if((UVTK_timer[2].is_iv != ti_mas[0].mv.iv) && (UVTK_TI_GR_NUM >= 1)){
			//printf("spon iv ti1\n");	
			for(i = 0; i < UVTK_TI_GR_SIZE; i++){
				ti_mas[i].mv.iv = UVTK_timer[2].is_iv;
			}
			fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_ME_TD_1, 0x00, 0x10));
		}
		if((UVTK_timer[3].is_iv != ti_mas[UVTK_TI_GR_SIZE].mv.iv) && (UVTK_TI_GR_NUM >= 2)){
				//printf("spon iv ti2\n");	
			for(i = UVTK_TI_GR_SIZE; i < UVTK_TI_GR_SIZE * 2; i++){
				ti_mas[i].mv.iv = UVTK_timer[3].is_iv;
			}
			fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_ME_TD_1, 0x10, 0x10));	
		}
		xSemaphoreGive(xIEC104_Poll_Mutex);
		vTaskDelay(100);
  }
}

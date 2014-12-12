/******************************************/
/***************TODO***********************/
/* 1. make correct .iv flag set           */
/* 2. Check for mv functionality is correct - OK!						        */
/* 3. Make spon TI or TS to be chained in a single iframe						*/
/******************************************/

#include "IEC104_drv.h"

extern uint8_t UVTK_1ts_grp_data[UVTK_TS_GR_SIZE];
extern uint8_t UVTK_2ts_grp_data[UVTK_TS_GR_SIZE];
extern uint16_t UVTK_1ti_grp_data[UVTK_TI_GR_SIZE];
extern uint16_t UVTK_2ti_grp_data[UVTK_TI_GR_SIZE];

extern fifo_t* iec_fifo_buf;

struct iec_type1_data  ts_mas[IEC104_TS_SIZE];
struct iec_type9_data  ti_mas[IEC104_TI_SIZE];


void IEC104_init_task(){
	xTaskCreate(IEC104_UVTK_TS_poll, (int8_t *) "IEC104_UVTK_TS_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_TS_poll, (int8_t *) "IEC104_TS_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_UVTK_TI_poll, (int8_t *) "IEC104_UVTK_TI_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_TI_poll, (int8_t *) "IEC104_TI_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
}


void IEC104_UVTK_TS_poll(void * pvParameters){
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t UVTK_1ts_grp_data_tmp = 0;
	uint8_t UVTK_2ts_grp_data_tmp = 0;
	uint8_t iec104_1gr_num = 0;
	uint8_t iec104_2gr_num = 0;
	uint8_t uvtk_1gr_ts_tmp = 0;
	uint8_t uvtk_2gr_ts_tmp = 0;
	for(;;){
		for(i = 0; i < UVTK_TS_GR_SIZE; i++){
			uint8_t UVTK_1ts_grp_data_tmp = UVTK_1ts_grp_data[i];
			uint8_t UVTK_2ts_grp_data_tmp = UVTK_2ts_grp_data[i];
			for(j = 0; j < 8; j++){
				iec104_1gr_num = (i*8) + j;
	      iec104_2gr_num = (i*8) + IEC104_TS_SIZE/2 + j;
				uvtk_1gr_ts_tmp = (UVTK_1ts_grp_data_tmp>>j) & 0x01;
				uvtk_2gr_ts_tmp = (UVTK_2ts_grp_data_tmp>>j) & 0x01;
				if(ts_mas[iec104_1gr_num].sp.sp != uvtk_1gr_ts_tmp){
					ts_mas[iec104_1gr_num].sp.sp = uvtk_1gr_ts_tmp;
					ts_mas[iec104_1gr_num].spon = 0x01;
				}
				if(ts_mas[iec104_2gr_num].sp.sp != uvtk_2gr_ts_tmp){
					ts_mas[iec104_2gr_num].sp.sp = uvtk_2gr_ts_tmp;
					ts_mas[iec104_2gr_num].spon = 0x01;
				}
			}
		}
		vTaskDelay(10);
	}
}

void IEC104_TS_poll(void * pvParameters){
	uint8_t i = 0;
	for(;;){
		for(i = 0; i < IEC104_TS_SIZE; i++){
			if(ts_mas[i].spon != 0x00){
				//printf("spon %d, i = %d\n", ts_mas[i].spon, i);
				fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_SP_TB_1, i, 0x01));
				ts_mas[i].spon = 0x00;
			}
		}
		vTaskDelay(10);
	}
}


void IEC104_UVTK_TI_poll(void * pvParameters){
	uint8_t i = 0;
	uint8_t uvtk_1gr_ti_tmp = 0;
	uint8_t uvtk_2gr_ti_tmp = 0;
	
	for(;;){
		for(i = 0; i < UVTK_TI_GR_SIZE; i++){
			uint8_t uvtk_1gr_ti_tmp = UVTK_1ti_grp_data[i];
			uint8_t uvtk_2gr_ti_tmp = UVTK_2ti_grp_data[i];
			if(ti_mas[i].mv.mv != uvtk_1gr_ti_tmp){
				ti_mas[i].mv.mv = uvtk_1gr_ti_tmp;
				ti_mas[i].spon = 0x01;
			}
			if(ti_mas[i + UVTK_TI_GR_SIZE].mv.mv != uvtk_2gr_ti_tmp){
				ti_mas[i + UVTK_TI_GR_SIZE].mv.mv = uvtk_2gr_ti_tmp;
				ti_mas[i + UVTK_TI_GR_SIZE].spon = 0x01;
			}
		}
		vTaskDelay(10);
	}

}

void IEC104_TI_poll(void * pvParameters){
	uint8_t i = 0;
	for(;;){
		for(i = 0; i < IEC104_TI_SIZE; i++){
			if(ti_mas[i].spon != 0x00){
				fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_ME_NA_1, i, 0x01));
				//printf("spon %d, i = %d\n", ti_mas[i].spon, i);
				ti_mas[i].spon = 0x00;
			}
		}
		vTaskDelay(10);
	}
}

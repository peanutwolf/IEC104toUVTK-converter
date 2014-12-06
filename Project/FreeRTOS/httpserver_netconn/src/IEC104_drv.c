#include "IEC104_drv.h"

extern uint8_t UVTK_1ts_grp_data[UVTK_TS_GR_SIZE];
extern uint8_t UVTK_2ts_grp_data[UVTK_TS_GR_SIZE];
extern uint8_t UVTK_1ti_grp_data[UVTK_TI_GR_SIZE];
extern uint8_t UVTK_2ti_grp_data[UVTK_TI_GR_SIZE];

extern fifo_t* iec_fifo_buf;

struct iec_type1_data  ts_mas[IEC104_TS_SIZE];
struct iec_type13_data ti_mas[IEC104_TI_SIZE];

void IEC104_init_task(){
	xTaskCreate(IEC104_UVTK_TS_poll, (int8_t *) "IEC104_UVTK_TS_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
	xTaskCreate(IEC104_TS_poll, (int8_t *) "IEC104_TS_poll", configMINIMAL_STACK_SIZE, NULL, IEC104_TASK_PRIO, NULL);
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
				iec104_1gr_num = (i*8)+j;
	      iec104_2gr_num = (i*8)+UVTK_TS_GR_SIZE+j;
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
				fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_SP_TB_1, i, 0x01));
				ts_mas[i].spon = 0x00;
			}
		}
		vTaskDelay(10);
	}
}

void IEC104_UVTK_TI_poll(void * pvParameters){
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t UVTK_1ti_grp_data_tmp = 0;
	uint8_t UVTK_2ti_grp_data_tmp = 0;
	uint8_t iec104_1gr_num = 0;
	uint8_t iec104_2gr_num = 0;
	uint8_t uvtk_1gr_ti_tmp = 0;
	uint8_t uvtk_2gr_ti_tmp = 0;
	for(;;){
		for(i = 0; i < UVTK_TI_GR_SIZE; i++){
			uint8_t UVTK_1ti_grp_data_tmp = UVTK_1ti_grp_data[i];
			uint8_t UVTK_2ti_grp_data_tmp = UVTK_2ti_grp_data[i];
			for(j = 0; j < 8; j++){
				iec104_1gr_num = (i*8)+j;
	      iec104_2gr_num = (i*8)+UVTK_TI_GR_SIZE+j;
				uvtk_1gr_ti_tmp = (UVTK_1ti_grp_data_tmp>>j) & 0x01;
				uvtk_2gr_ti_tmp = (UVTK_2ti_grp_data_tmp>>j) & 0x01;
				if(ti_mas[iec104_1gr_num].mv.mv != uvtk_1gr_ti_tmp){
					ti_mas[iec104_1gr_num].mv.mv = uvtk_1gr_ti_tmp;
					ti_mas[iec104_1gr_num].spon = 0x01;
				}
				if(ti_mas[iec104_2gr_num].mv.mv != uvtk_2gr_ti_tmp){
					ti_mas[iec104_2gr_num].mv.mv = uvtk_2gr_ti_tmp;
					ti_mas[iec104_2gr_num].spon = 0x01;
				}
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
				fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_ME_NC_1, i, 0x01));
				ti_mas[i].spon = 0x00;
			}
		}
		vTaskDelay(10);
	}
}
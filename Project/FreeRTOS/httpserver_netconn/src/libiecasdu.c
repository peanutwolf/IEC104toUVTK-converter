/***********************************************************************/
/***************TODO****************************************************/
/* 1. Make timers functionality for INROGEN -OK!      								 */
/* 2. Make defines for INROGEN         												 			   */
/***********************************************************************/


#include "libiecasdu.h"

volatile u8_t NS = 0, NR = 0;
cp56time2a TM_cp56_time;
fifo_t* iec_fifo_buf;
struct iec_type30 iec_type30_tmpl = {0,0,0,0,0,0,0};
struct iec_type34 iec_type34_tmpl = {0,0,0,0,0,0,0};

extern struct iec_type1_data ts_mas[IEC104_TS_SIZE];
extern struct iec_type9_data ti_mas[IEC104_TI_SIZE];
extern xTimerHandle INROGEN_timer;


void parse_iframe(fifo_t* fifo_buf, struct iec_buf* buf){
	volatile struct iec_unit_id* asdu_head;	
	volatile u8_t type_id;
	
	  asdu_head = (struct iec_unit_id*)buf->data;
	   switch(asdu_head->type){
			 case C_CS_NA_1:
				 if(asdu_head->cause == ACT_COT){
					  memcpy(&TM_cp56_time, &buf->data[9], sizeof(cp56time2a));
					  set_iec_time(&TM_cp56_time);
					  vPortFree(buf);
					  fifoPushElem(fifo_buf, prepare_data_iframe(ACTCON_COT, C_CS_NA_1, 0x00, 0x01));
				 }
			   break;
			 case C_IC_NA_1:
			   if(asdu_head->cause == ACT_COT){
  				  vPortFree(buf);
					  fifoPushElem(fifo_buf, prepare_data_iframe(ACTCON_COT,  C_IC_NA_1, 0x00, 0x01));	
						if(xTimerIsTimerActive(INROGEN_timer) == pdFALSE){
							fifoPushElem(fifo_buf, prepare_data_iframe(INROGEN_COT, M_SP_NA_1, 0x00, 0x20));	
							fifoPushElem(fifo_buf, prepare_data_iframe(INROGEN_COT, M_SP_NA_1, 0x20, 0x20));
							fifoPushElem(fifo_buf, prepare_data_iframe(INROGEN_COT, M_SP_NA_1, 0x40, 0x20));	
							fifoPushElem(fifo_buf, prepare_data_iframe(INROGEN_COT, M_SP_NA_1, 0x60, 0x20));
							fifoPushElem(fifo_buf, prepare_data_iframe(INROGEN_COT, M_ME_NA_1, 0x00, 0x10));
							fifoPushElem(fifo_buf, prepare_data_iframe(INROGEN_COT, M_ME_NA_1, 0x10, 0x10));	
							xTimerReset(INROGEN_timer, 0);
							xTimerStart(INROGEN_timer, 0);
						}
				 }
			   break;
			 default:
				 break;
		 }
}


struct iec_buf* prepare_data_iframe(u8_t COT, u8_t type, u16_t inner_adr, u8_t num){
	u8_t frame_size = 0, sizeof_obj = 0, i = 0;
	u16_t ioa = 0;
	u8_t INROGEN_QOI = 0x14;
	volatile u8_t* objs_pnt = 0;
	struct iec_buf* buf = NULL;
	struct iec_unit_id* asdu_head = NULL;
	
	if(type == M_SP_NA_1){
	   sizeof_obj = sizeof(struct iec_type1) + 3;
		 ioa = inner_adr + SP_IOA_OFFSET;
	}
	else if(type == M_ME_NA_1){
	   sizeof_obj = sizeof(struct iec_type9) + 3;
		 ioa = inner_adr + MV_IOA_OFFSET;
	}
	else if(type == M_SP_TB_1){
	   sizeof_obj = sizeof(struct iec_type30) + 2;
		 ioa = inner_adr + SP_IOA_OFFSET;
		 get_iec_time(&TM_cp56_time);
	}
	else if(type == M_ME_TD_1){
	   sizeof_obj = sizeof(struct iec_type34) + 2;
		 ioa = inner_adr + MV_IOA_OFFSET;
		 get_iec_time(&TM_cp56_time);
	}
	else if(type == C_IC_NA_1 || type == C_CS_NA_1){
	   sizeof_obj = 4;
		 ioa = inner_adr;
	}
	frame_size = sizeof(struct iechdr) + sizeof(struct iec_unit_id) + sizeof_obj * num;
	buf = pvPortMalloc(frame_size);	
	printf("Pointer:0x%p\n",buf);
	
	buf->data_len = frame_size;
	asdu_head = ((struct iec_unit_id*)buf->data);
	buf->h.start = 0x68;
	buf->h.ic.ft = 0x00;
	buf->h.ic.res = 0x00;
	buf->h.ic.nr = NR;
	buf->h.ic.ns = NS++;
	buf->h.length = frame_size - 2;
	asdu_head->type = type;
	asdu_head->num = num;
	asdu_head->sq = 0;
	asdu_head->cause = COT;
	asdu_head->t = 0;
	asdu_head->pn = 0;
	asdu_head->oa = 0;
	asdu_head->ca = 0x15;
	
	objs_pnt = (u8_t*)(++asdu_head);  //!!!be careful asdu_head doesnt point to asdu_head anymore!!!

	for(i = 0; i < num; i++){
		*objs_pnt = ioa;
		*(++objs_pnt) = ioa>>8;	
		ioa++;
		*(++objs_pnt) = 0x00;		
		if(type == M_SP_NA_1){
			memcpy((void*)++objs_pnt, &(ts_mas[inner_adr+i].sp), sizeof(struct iec_type1));
		}
		else if(type == M_ME_NA_1){
			memcpy((void*)++objs_pnt, &(ti_mas[inner_adr+i].mv), sizeof(struct iec_type9));
		}
		else if(type == M_ME_TD_1){
			memcpy(&iec_type34_tmpl, &(ti_mas[inner_adr+i].mv), sizeof(struct iec_type9));
			iec_type34_tmpl.time = TM_cp56_time;
			memcpy((void*)++objs_pnt, &iec_type34_tmpl, sizeof(struct iec_type34));
		}
		else if(type == M_SP_TB_1){
			memcpy(&iec_type30_tmpl, &(ts_mas[inner_adr+i].sp), sizeof(struct iec_type1));
			iec_type30_tmpl.time = TM_cp56_time;
			memcpy((void*)++objs_pnt, &iec_type30_tmpl, sizeof(struct iec_type30));
		}
		else if(type == C_IC_NA_1 || type == C_CS_NA_1){
			memcpy((void*)++objs_pnt, &INROGEN_QOI, 0x01);
		}
		objs_pnt += (sizeof_obj-3);
	}
	return buf;
}

struct iec_buf* prepare_sframe(void){
	
struct iec_buf* buf = NULL;
	
	buf = pvPortMalloc(sizeof(struct iec_buf));
	buf->data_len = 0x06;
  buf->h.start = 0x68;
	buf->h.length = 0x04;
	buf->h.sc.res1 = 0x00;
	buf->h.sc.res2 = 0x00;
	buf->h.sc.ft = 0x01;
	buf->h.sc.nr = NR;
	
	return buf;
}


void set_iec_time(cp56time2a* TM_cp56_time){
  RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	
	RTC_TimeStruct.RTC_Hours = TM_cp56_time->hour;
	RTC_TimeStruct.RTC_Minutes = TM_cp56_time->min;
	RTC_TimeStruct.RTC_Seconds = TM_cp56_time->msec/0x03E8;
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
	
	RTC_DateStruct.RTC_Date = TM_cp56_time->mday;
	RTC_DateStruct.RTC_Month = TM_cp56_time->month;
	RTC_DateStruct.RTC_WeekDay = TM_cp56_time->wday;
	RTC_DateStruct.RTC_Year = TM_cp56_time->year;
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
}

void get_iec_time(cp56time2a* TM_cp56_time){
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	
	TM_cp56_time->hour = RTC_TimeStruct.RTC_Hours;
	TM_cp56_time->min = RTC_TimeStruct.RTC_Minutes;
	TM_cp56_time->msec = RTC_TimeStruct.RTC_Seconds*0x03E8 + RTC_GetSubSecond()/8;
	
	TM_cp56_time->mday = RTC_DateStruct.RTC_Date;
	TM_cp56_time->month = RTC_DateStruct.RTC_Month;
	TM_cp56_time->wday = RTC_DateStruct.RTC_WeekDay;
	TM_cp56_time->year = RTC_DateStruct.RTC_Year;
}



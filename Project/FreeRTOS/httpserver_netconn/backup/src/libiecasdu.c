
#include "libiecasdu.h"

volatile u8_t NS = 0, NR = 0;
struct iec_type1 TS1;
cp56time2a TM_cp56_time;
fifo_t* iec_fifo_buf;
struct iec_buf* g_buf;

struct iec_type1 SP_mas[SP_TOTAL] = {{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, 
															 {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};

struct iec_mv MV_mas[MV_TOTAL] = {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}};


void parse_iframe(fifo_t* fifo_buf, struct iec_buf* buf){
	volatile struct iec_unit_id* asdu_head;	
	struct iec_buf* inner_buf;
	volatile u8_t type_id;
	
	  asdu_head = (struct iec_unit_id*)buf->data;
	   switch(asdu_head->type){
			 case C_CS_NA_1:
				 if(asdu_head->cause == ACT_COT){
					  memcpy(&TM_cp56_time, &buf->data[9], sizeof(cp56time2a));
				    //asdu_head->cause = ACTCON_COT;
						//buf->h.ic.ns = NS++; 
					  //buf->h.ic.nr = NR;
					  fifoPushElem(fifo_buf, prepare_data_iframe(ACTCON_COT, C_CS_NA_1, 0x00, 0x01));
				 }
			   break;
			 case C_IC_NA_1:
			   if(asdu_head->cause == ACT_COT){
						//asdu_head->cause = ACTCON_COT;
						//buf->h.ic.ns = NS++; 
					  //buf->h.ic.nr = NR;
					  inner_buf = prepare_data_iframe(ACTCON_COT, C_IC_NA_1, 0x00, 0x01);
					  fifoPushElem(fifo_buf, inner_buf);
					  inner_buf =prepare_data_iframe(INROGEN_COT, M_SP_NA_1, 0x00, 0x01);
					  fifoPushElem(fifo_buf, inner_buf);
					  inner_buf = prepare_data_iframe(INROGEN_COT, M_SP_NA_1, 0x02, 0x01);
					  fifoPushElem(fifo_buf, inner_buf);
				 }
			   break;
			 default:
				 break;
		 }
}


struct iec_buf* prepare_data_iframe(u8_t COT, u8_t type, u16_t inner_adr, u8_t num){
	u8_t frame_size = 0, sizeof_obj = 0, i = 0;
	u16_t ioa = 0;
	volatile u8_t* objs_pnt = 0;
	struct iec_buf* buf = NULL;
	struct iec_unit_id* asdu_head = NULL;
	if(type == M_SP_NA_1){
	   sizeof_obj = sizeof(struct iec_type1) + 3;
		 ioa = inner_adr + SP_IOA_OFFSET;
		 //num = SP_TOTAL;
	}
	else if(type == C_IC_NA_1 || type == C_CS_NA_1){
	   sizeof_obj = 4;
		 ioa = inner_adr;
	}
	frame_size = sizeof(struct iechdr) + sizeof(struct iec_unit_id) + sizeof_obj * num;
	
	buf = malloc(frame_size);	
	buf->data_len = frame_size;
	asdu_head = ((struct iec_unit_id*)buf->data);
	buf->h.start = 0x68;
	buf->h.ic.ft = 0x00;
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
	asdu_head->ca = 0x01;
	objs_pnt = (u8_t*)(++asdu_head);  //!!!be careful asdu_head doesnt point to asdu_head anymore!!!
	for(i = 0; i <= num; i++){
		*objs_pnt = ioa;
		*(++objs_pnt) = ioa>>8;		
		ioa++;
		objs_pnt += 2;
		if(type == M_SP_NA_1){
			memcpy((void*)objs_pnt, &SP_mas[inner_adr+i], sizeof(struct iec_type1));
			objs_pnt++;
		}
		else if(type == C_IC_NA_1 || type == C_CS_NA_1){
			*objs_pnt = 0x14;
			objs_pnt++;
		}
	}
	return buf;
}






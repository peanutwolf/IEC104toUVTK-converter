/******************************************/
/***************TODO***********************/
/* 1. Correct parse i_frame.  - OK!         */
/* 2. Linked List to store iec_bufs - OK! */
/* 3. Func to form correct pbufs - OK!    */
/* 4. Make RTC functionality - OK!        															 */
/* 5. Bug in prepare_tcp_iec_buf() - pbuf_chain() makes hard fault  -OK!      */
/* 5. Bug if two INROGEN at the same time. Need to insert timers -OK!     */
/******************************************/

#include "IEC104_server.h"
#include "libiecasdu.h"

struct pbuf* IEC104_send_buf;
extern fifo_t* iec_fifo_buf;
extern volatile u8_t NS, NR;
xTimerHandle INROGEN_timer = NULL;

void init_IEC104_server(void){
	iec_fifo_buf = createFifoToHeap(sizeof(struct iec_buf*), DATA_BUF_SIZE);
	INROGEN_timer = xTimerCreate((const signed char *)"IEC104Timer", (INROGEN_TIMER_DELAY), pdTRUE, (void*)INROGEN_TIMER_ID, vUVTKTimerCallback);
}

/**************************************************
* Generates chain of pbufs to be sent to Eth interface
* Set CHAIN_NUM_PBUFS (number of pbufs in chain);
**************************************************/
struct pbuf* prepare_tcp_iec_buf(fifo_t* fifo_buf){
  struct pbuf *p = NULL, *p_tmp = NULL;
	struct iec_buf* buf;
	err_t	err;
	u8_t i = 0;
	if(fifoIsEmpty(fifo_buf)){
	    return NULL;
	}
	while(!fifoIsEmpty(fifo_buf) && i <= CHAIN_NUM_PBUFS){
	  buf = fifoPopElem(fifo_buf);
		p_tmp = pbuf_alloc(PBUF_TRANSPORT, buf->data_len , PBUF_RAM);
		if(!p_tmp){
			  printf("Cannot allocate p_tmp\n");
				fifoPushElem(fifo_buf, buf);
				return NULL;
		}
		err = pbuf_take(p_tmp, &buf->h, buf->data_len);
		if(err == ERR_OK){
			if(i){
				pbuf_chain(p, p_tmp);
			}
			else{
				p = p_tmp;
			}
			i++;
			printf("Free pointer:0x%p\n",buf);
			vPortFree(buf);
			buf = NULL;
			printf("Heap after free:%d\n",xPortGetFreeHeapSize());
		}  
		else{
			printf("ERR_OK is not OK");
			fifoPushElem(fifo_buf, buf);
			pbuf_free(p_tmp);
			 while(p!=NULL){
				  p_tmp = p;
				  p = p_tmp->next;
					pbuf_free(p_tmp);
			 }	
			p = NULL;
			p_tmp = NULL;
			return NULL;
		}
	}
	return p;
}

struct iec_buf* prepare_uframe_answ(struct iec_buf* buf, enum mode_type mode, enum uframe_func func){
	if(mode == IEC_SLAVE){
	switch(uframe_func(&buf->h)){
	  case STARTACT:
			buf->h.uc.start_act = 0;
		  buf->h.uc.start_con = 1;
		  /*TODO func to start TM*/
		  break;
		case STOPACT:
			buf->h.uc.stop_act = 0;
		  buf->h.uc.stop_con = 1;
		  /*TODO func to close socket*/
		  break;
		case TESTACT:
			buf->h.uc.test_act = 0;
		  buf->h.uc.test_con = 1;
		  break;
		default:
			break;
	}	
 }
	else {
	switch(func){
	  case STARTACT:
			buf->h.uc.start_act = 1;
		  buf->h.uc.start_con = 0;
		  break;
		case STOPACT:
			buf->h.uc.stop_act = 1;
		  buf->h.uc.stop_con = 0;
		  break;
		case TESTACT:
			buf->h.uc.test_act = 1;
		  buf->h.uc.test_con = 0;
		  break;
		default:
			switch(uframe_func(&buf->h)){
	       case STARTCON:
		       /*TODO func to read TM*/
		        break;
		     case STOPCON:
		       /*TODO func to close socket*/
		        break;
		     case TESTCON:
		        break;
		     default:
			      break;
	    }	
			break;
	}
	}
	return buf;
}

struct pbuf* generatIECansw(struct pbuf* p){

  //err_t	err;
	struct pbuf* ptr;
	struct iec_buf* buf;
	
//	/* Check IEC header size*/
//	if(p -> len < IEC104_HEADER_SIZE){
//			return p;
//	}
	printf("Processing IEC frame\n");
	/*Allocate memory for iec104 telegramm frame*/
	printf("Heap1:%d\n",xPortGetFreeHeapSize());
  buf = (struct iec_buf*)pvPortMalloc(p -> len);
	buf -> data_len = p -> len;
	memcpy(&buf->h, p -> payload, p -> len);		
		if(frame_type(&buf->h) == FRAME_TYPE_U){
			 buf = prepare_uframe_answ(buf, IEC_SLAVE, 0);
			 fifoPushElem(iec_fifo_buf, buf); 
		}	
		else if(frame_type(&buf->h) == FRAME_TYPE_I){
			NR++;			
		  parse_iframe(iec_fifo_buf, buf);		
		} 
		else if(frame_type(&buf->h) == FRAME_TYPE_S){
			buf->h.sc.nr = NR;
			fifoPushElem(iec_fifo_buf, buf);
		}
		else{
			free(buf);
			buf = NULL;
		}
		IEC104_send_buf = prepare_tcp_iec_buf(iec_fifo_buf);
		while(p != NULL){
			ptr = p;
			p = ptr->next;
			pbuf_free(ptr);	
		}		     
		p = IEC104_send_buf;
 return p;
}




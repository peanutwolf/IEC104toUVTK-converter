#ifndef __IEC104_SERVER_H
#define __IEC104_SERVER_H

#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "FIFO_buffer.h"
//#include "libiecasdu.h"
#include "FreeRTOS.h"
#include "portable.h"
#include <stdlib.h>
#include <string.h>

#define IEC104_HEADER_SIZE 		6
#define CHAIN_NUM_PBUFS 			2


enum mode_type{
	IEC_SLAVE,
	IEC_MASTER
};

enum frame_type {
	FRAME_TYPE_I,
	FRAME_TYPE_S,
	FRAME_TYPE_U
};

enum uframe_func {
	STARTACT,
	STARTCON,
	STOPACT,
	STOPCON,
	TESTACT,
	TESTCON
};

struct  iec_buf {
	u8_t	data_len;	/* actual ASDU length */
	
	 struct iechdr {
		u8_t	start;
		u8_t	length;
		u8_t raw[0];
		 union {
			 struct iec_i {
				u8_t	ft:1;
				u16_t	ns:15;
				u8_t	res:1;
				u16_t	nr:15;
			} ic;
			 struct iec_s {
				u8_t	ft:1;
				u16_t	res1:15;
				u8_t	res2:1;
				u16_t	nr:15;
			} sc;
			 struct iec_u {
				u8_t	ft:2;
				u8_t	start_act:1;
				u8_t	start_con:1;
				u8_t	stop_act:1;
				u8_t	stop_con:1;
				u8_t	test_act:1;
				u8_t	test_con:1;
				u8_t	res1;
				u16_t	res2;
			} uc;
		};
	} h;	
	u8_t	data[0];
}__attribute__ ((__packed__));

static inline enum frame_type frame_type(struct iechdr *h)
{
	if (!(h->raw[0] & 0x1))
		return FRAME_TYPE_I;
	else if (!(h->raw[0] & 0x2))
		return FRAME_TYPE_S;
	else
		return FRAME_TYPE_U; 
}

static inline enum uframe_func uframe_func(struct iechdr *h)
{
	if (h->raw[0] & 0x4)
		return STARTACT;
	else if (h->raw[0] & 0x8)
		return STARTCON;
	else if (h->raw[0] & 0x10)
		return STOPACT;
	else if (h->raw[0] & 0x20)
		return STOPCON;
	else if (h->raw[0] & 0x40)
		return TESTACT;
	else
		return TESTCON;
}

struct iec_buf* prepare_uframe_answ(struct iec_buf* buf, enum mode_type, enum uframe_func);
struct pbuf* prepare_tcp_iec_buf(fifo_t* fifo_buf);
void init_IEC104_server(void);
struct pbuf* generatIECansw(struct pbuf* p);


#endif

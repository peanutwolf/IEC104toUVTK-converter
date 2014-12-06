#ifndef __LIBIECASDU_H
#define __LIBIECASDU_H

#define DATA_BUF_SIZE 20
#define SP_IOA_OFFSET 0x65
#define SP_TOTAL 10
#define MV_TOTAL 10

#include "IEC104_server.h"
#include "iec104_types.h"
#include "FIFO_buffer.h"


void parse_iframe(fifo_t* fifo_buf, struct iec_buf *buf);
struct iec_buf* prepare_data_iframe(u8_t COT, u8_t type, u16_t adr, u8_t num);

#endif

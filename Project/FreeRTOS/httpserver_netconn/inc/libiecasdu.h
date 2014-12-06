#ifndef __LIBIECASDU_H
#define __LIBIECASDU_H

#define DATA_BUF_SIZE 20
#define SP_IOA_OFFSET 0x65
#define MV_IOA_OFFSET 0x3E9
#define SP_TOTAL 10
#define MV_TOTAL 10

#include "IEC104_server.h"
#include "iec104_types.h"
#include "FIFO_buffer.h"
#include "IEC104_drv.h"


void parse_iframe(fifo_t* fifo_buf, struct iec_buf *buf);
struct iec_buf* prepare_data_iframe(u8_t COT, u8_t type, u16_t adr, u8_t num);
struct iec_buf* prepare_sframe(void);
void set_iec_time(cp56time2a* TM_cp56_time);
void get_iec_time(cp56time2a* TM_cp56_time);

#endif

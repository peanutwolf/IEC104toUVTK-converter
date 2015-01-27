#ifndef __LIBIECASDU_H
#define __LIBIECASDU_H

#define DATA_BUF_SIZE 20
#define IEC_SP_IOA_OFFSET 					*(uint16_t*)(SETTINGS_ADDRESS + IEC_SP_IOA_OFFSET_ADDR)
#define IEC_MV_IOA_OFFSET 					*(uint16_t*)(SETTINGS_ADDRESS + IEC_MV_IOA_OFFSET_ADDR)
#define IEC_ASDU_ADDR								*(uint16_t*)(SETTINGS_ADDRESS + IEC_ASDU_ADDR_ADDR)


#include "IEC104_server.h"
#include "iec104_types.h"
#include "FIFO_buffer.h"
#include "IEC104_drv.h"
#include "main.h"


void parse_iframe(fifo_t* fifo_buf, struct iec_buf *buf);
struct iec_buf* prepare_data_iframe(u8_t COT, u8_t type, u16_t adr, u8_t num);
struct iec_buf* prepare_sframe(void);
void set_iec_time(cp56time2a* TM_cp56_time);
void get_iec_time(cp56time2a* TM_cp56_time);

#endif

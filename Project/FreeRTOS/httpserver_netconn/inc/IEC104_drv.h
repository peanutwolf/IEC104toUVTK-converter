#ifndef IEC104_DRV_H
#define IEC104_DRV_H

#include "sys_arch.h"
#include <stdio.h>
#include "libiecasdu.h"
#include "UVTKdrv.h"

#define IEC104_TASK_PRIO    		( tskIDLE_PRIORITY + 2 )
#define IEC104_TS_SIZE					UVTK_TS_GR_SIZE * UVTK_TS_GR_NUM * 8 //128
//#define IEC104_TS_SIZE					UVTK_TS_GR_SIZE * UVTK_TS_GR_NUM * 8  //128
#define IEC104_TI_SIZE					UVTK_TI_GR_SIZE * UVTK_TI_GR_NUM
//#define IEC104_TI_SIZE					UVTK_TI_GR_SIZE * UVTK_TI_GR_NUM

void IEC104_init_task(void);
void form_IEC_ts_data_mas(void);
void form_IEC_ti_data_mas(void);

void IEC104_UVTK_TS_poll(void * pvParameters);
void IEC104_TS_poll(void * pvParameters);
void IEC104_UVTK_TI_poll(void * pvParameters);
void IEC104_TI_poll(void * pvParameters);
void IEC104_UVTK_IV_poll(void * pvParameters);

#endif

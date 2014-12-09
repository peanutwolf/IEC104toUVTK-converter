#ifndef IEC104_DRV_H
#define IEC104_DRV_H

#include "sys_arch.h"
#include <stdio.h>
#include "libiecasdu.h"
#include "UVTKdrv.h"

#define IEC104_TASK_PRIO    		( tskIDLE_PRIORITY + 2 )
#define IEC104_TS_SIZE					64
#define IEC104_TI_SIZE					32

void IEC104_init_task(void);

void IEC104_UVTK_TS_poll(void * pvParameters);
void IEC104_TS_poll(void * pvParameters);
void IEC104_UVTK_TI_poll(void * pvParameters);
void IEC104_TI_poll(void * pvParameters);

#endif

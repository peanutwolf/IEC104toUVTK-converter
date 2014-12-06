#ifndef __FIFO_BUFFER_H
#define __FIFO_BUFFER_H

#include "arch/cc.h"
#include "FreeRTOS.h"
#include "portable.h"
#include <stdlib.h>

#define CHECK_FIFO_NULL(fifo) if (fifo == NULL) return 0

typedef struct {
     void * buffer;
	   u8_t elemSize;
     u8_t head;
     u8_t tail;
     u8_t size;
} fifo_t;


fifo_t* createFifoToHeap(u8_t elemSize, u8_t size);

u8_t fifoPushElem(fifo_t* fifo, void* data);

u8_t fifoPushBytes(fifo_t* fifo, u8_t* bytes, u8_t count);

void* fifoPopElem(fifo_t* fifo);

u8_t fifoPopBytes(fifo_t* fifo, u8_t* bytes, u8_t count);

u8_t fifoIsFull(fifo_t* fifo);

u8_t fifoIsEmpty(fifo_t* fifo);

u8_t fifoBytesFilled(fifo_t* fifo);

#endif


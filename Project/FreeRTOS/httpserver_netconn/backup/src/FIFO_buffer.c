
#include "FIFO_buffer.h"

fifo_t* createFifoToHeap(u8_t elemSize, u8_t size)
{
    void* buffer = malloc(elemSize * size);
	
    if (buffer == NULL)
        return NULL;

    fifo_t* fifo = (fifo_t*)malloc(sizeof(fifo_t));

    if (fifo == NULL)
    {
       free(buffer);
       return NULL;
    }

    fifo->buffer = buffer;
		fifo->elemSize = elemSize;
    fifo->head = 0;
    fifo->tail = 0;
    fifo->size = size;

    return fifo;
}

u8_t fifoPushElem(fifo_t* fifo, void* data)
{
	void* destAddr;
    CHECK_FIFO_NULL(fifo);
    if (fifoIsFull(fifo) == 0x01)
       return 0;
     
    destAddr = (u8_t*)fifo->buffer + fifo->head * fifo->elemSize;
    *((u32_t*)destAddr) = (u32_t)data;
    fifo->head++;
    if (fifo->head == fifo->size)
       fifo->head = 0;

    return 1;
}

/*
u8_t fifoPushBytes(fifo_t* fifo, void* bytes, u8_t count)
{
    CHECK_FIFO_NULL(fifo);

    for (uint32_t i = 0; i < count; i++)
    {
        if (fifoPushByte(fifo, bytes[i]) == 0)
            return i;
    }

    return count;
}
*/

void* fifoPopElem(fifo_t* fifo)
{
	void* sourceAddr;
	void* byte;
    CHECK_FIFO_NULL(fifo);

    if (fifoIsEmpty(fifo) == 0x01)
        return 0;

		sourceAddr = (u8_t*)fifo->buffer + fifo->tail * fifo->elemSize;
    byte = (u32_t*)(*(u32_t*)sourceAddr);

    fifo->tail++;
    if (fifo->tail == fifo->size)
        fifo->tail = 0;

    return byte;
}
/*
u8_t fifoPopBytes(fifo_t* fifo, u8_t* bytes, u8_t count)
{
    CHECK_FIFO_NULL(fifo);

    for (uint32_t i = 0; i < count; i++)
    {
        if (fifoPopByte(fifo, bytes + i) == 0)
            return i;
    }

    return count;
}
*/
u8_t fifoIsFull(fifo_t* fifo)
{
    if ((fifo->head == (fifo->size - 1) && fifo->tail == 0) || (fifo->head == (fifo->tail - 1)))
        return 1;
    else
        return 0;
}

u8_t fifoIsEmpty(fifo_t* fifo)
{
    if (fifo->head == fifo->tail)
        return 0x01;
    else
        return 0x00;
}

u8_t fifoBytesFilled(fifo_t* fifo)
{
    if (fifo->head == fifo->tail)
        return 0;
    else if ((fifo->head == (fifo->size - 1) && fifo->tail == 0) || (fifo->head == (fifo->tail - 1)))
        return fifo->size;
    else if (fifo->head < fifo->tail)
        return (fifo->head) + (fifo->size - fifo->tail);
    else
        return fifo->head - fifo->tail; 
}

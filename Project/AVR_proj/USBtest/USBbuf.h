#ifndef USBBUF_H_INCLUDED
#define USBBUF_H_INCLUDED

#include <avr/io.h>

typedef struct struct_USBBuffer
{
    uint8_t *dataptr;			///< the physical memory address where the buffer is stored
	uint8_t size;			///< the allocated size of the buffer
	uint8_t dataindex;		///< the index into the buffer where the data starts
}MyUSBBuffer;

// function prototypes

//! initialize a buffer to start at a given address and have given size
void			MyUSBbufferInit(MyUSBBuffer* buffer, uint8_t *start, uint8_t size);

//! add a byte to the end of the buffer
uint8_t	USBbufferAddToEnd(MyUSBBuffer* buffer, uint8_t data);

uint8_t* USBbufferSendAll(MyUSBBuffer* buffer);

uint8_t USBbufferGetSize(MyUSBBuffer* buffer);

#endif // USBBUF_H_INCLUDED

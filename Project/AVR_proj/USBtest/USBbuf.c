

// global AVRLIB defines
#include "avrlibdefs.h"
#include "USBbuf.h"

#ifndef CRITICAL_SECTION_START
#define CRITICAL_SECTION_START	unsigned char _sreg = SREG; cli();
#define CRITICAL_SECTION_END	SREG = _sreg;
#endif

// initialization

void MyUSBbufferInit(MyUSBBuffer* buffer, uint8_t *start, uint8_t size)
{
	// begin critical section
    CRITICAL_SECTION_START
	// set start pointer of the buffer
	buffer->dataptr = start;
	buffer->size = size;
	// initialize index and length
	buffer->dataindex = 0;
	// end critical section
    CRITICAL_SECTION_END
}

uint8_t	USBbufferAddToEnd(MyUSBBuffer* buffer, uint8_t data)
{
    // begin critical section
	CRITICAL_SECTION_START;
	// make sure the buffer has room
	if(buffer->dataindex < buffer->size)
	{
		// save data byte at end of buffer
		buffer->dataptr[buffer->dataindex] = data;
		// increment the length
		buffer->dataindex++;
		// end critical section
		CRITICAL_SECTION_END;
		// return success
		return -1;
	}
	// end critical section
	CRITICAL_SECTION_END;
	// return failure
	return 0;
}

uint8_t* USBbufferSendAll(MyUSBBuffer* buffer)
{
    return buffer->dataptr;
}

uint8_t USBbufferGetSize(MyUSBBuffer* buffer)
{
    uint8_t dataindex_tmp;
    dataindex_tmp = buffer->dataindex;
    buffer->dataindex = 0x00;
    return dataindex_tmp;
}


#ifndef STM32DRV_H_INCLUDED
#define STM32DRV_H_INCLUDED
#include <stdint.h>
#include "avr_fifo.h"

#ifndef CRITICAL_SECTION_START
#define CRITICAL_SECTION_START	unsigned char _sreg = SREG; cli();
#define CRITICAL_SECTION_END	SREG = _sreg;
#endif

#define SPI_BUFFER_SIZE     64
#define SPI_MSG_SIZE        16
#define UVTK_MAX_MSG_SIZE   23

void sendUVTKData(uint8_t* data, uint8_t fang);
uint8_t getUVTKData();
void getServData(uint8_t data);

#endif // STM32DRV_H_INCLUDED

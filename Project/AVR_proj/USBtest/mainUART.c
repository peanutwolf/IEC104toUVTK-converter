/*
 */

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

#include "uart_lib/uart.h"

#define UART_BAUD_RATE    38400
#define CHAR_BIT 8

volatile uint8_t input, counter1, counter2, KZP;
volatile uint8_t *input_p;

void reverseByte (volatile uint8_t* v );


ISR (TIMER1_COMPA_vect){


    *input_p <<= 1;
    if (bit_is_set(PIND,3)){
      input = input|0x01;
    }

    if(KZP){
        if (input == 0xF8){
        KZP = 0x00;
        counter1 = 0x07;
        }
    }
        counter1++;


     /*                           //Func to generate test meander
    if (bit_is_set(PORTD,3)){
        PORTD &= ~_BV(PD3);
    }
    else{
        PORTD |= _BV(PD3);
    }
    */
}

void reverseByte( volatile uint8_t* v){
//unsigned int v;     // input bits to be reversed
uint8_t r = *v; // r will be reversed bits of v; first get LSB of v
int s = sizeof(*v) * CHAR_BIT - 1; // extra shift needed at end

for (*v >>= 1; *v; *v >>= 1)
{
  r <<= 1;
  r |= *v & 1;
  s--;
}
r <<= s; // shift when v's highest bits are zero
*v = r;
}

int main(void){

    input_p = &input;
    *input_p = 0x00;
    counter1 = 0x00;
    KZP = 0x01;

    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

    OCR1AH = 0x4E;
    OCR1AL = 0x20;           // Compare register value
    //TCNT1H = 0x27;           // (H)
    //TCNT1L = 0x10;           // Intintial Timer value (L)
    TIMSK |= _BV(OCIE1A);
    TIFR |= _BV(OCF1A);
    TCCR1B |= _BV(WGM12)|_BV(CS11);

    DDRD &= ~_BV(PD3);    // Set PORTD3 for input
    PORTD |= _BV(PD3);
    sei();

    while(1){
     asm volatile(
        "nop\n\t"
        "nop\n\t");
      if (counter1 == 0x08){
        reverseByte(input_p);
        uart_putc(*input_p);
        counter1 = 0;
          if(!input && counter2 <= 0x1E){
            counter2++;
            if (counter2 >= 0x07)
               KZP = 0x01;
          }
          else if(input)
            counter2 = 0;
        //GICR |= _BV(INT1);
       }

     }

    return 0;
}

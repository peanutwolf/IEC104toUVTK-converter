/* Name: main.c
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.
We assume that an LED is connected to port B bit 0. If you connect it to a
different port or bit, change the macros below:
*/
#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define LED_BIT             0
#define USB_BUFFER_SIZE     8
#define CHAR_BIT            8

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv/usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#include "USBbuf.h"         // ring buffer util

volatile uint8_t *data_p, *input_p;
volatile MyUSBBuffer* MyUSBbuf;
volatile uint8_t input, counter1, counter2, KZP, data;

void reverseByte (volatile uint8_t* v );

ISR (TIMER1_COMPA_vect){

  *input_p <<= 1;
        if (bit_is_set(PIND,3)){
            input = input|0x01;
        }

        if(KZP){
            if (input == 0x7F){
                KZP = 0x00;
                counter1 = 0x07;
            }
        }
        counter1++;
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

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;
static uchar    dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */

    if(rq->bRequest == CUSTOM_RQ_ECHO){ /* echo -- used for reliability tests */
        dataBuffer[0] = rq->wValue.bytes[0];
        dataBuffer[1] = rq->wValue.bytes[1];
        dataBuffer[2] = rq->wIndex.bytes[0];
        dataBuffer[3] = rq->wIndex.bytes[1];
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 4;
    }else if(rq->bRequest == CUSTOM_RQ_SET_STATUS){
        if(rq->wValue.bytes[0] & 1){    /* set LED */
            LED_PORT_OUTPUT |= _BV(LED_BIT);
        }else{                          /* clear LED */
            LED_PORT_OUTPUT &= ~_BV(LED_BIT);
        }
    }else if(rq->bRequest == CUSTOM_RQ_GET_STATUS){
        dataBuffer[0] = ((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0);
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}
/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{
    uchar   i;
    MyUSBBuffer MyUSBbuf_r;
    uint8_t size_of_arr;
    uint8_t USB_buf_array[USB_BUFFER_SIZE];

    input_p = &input;
    *input_p = 0x00;
    counter1 = 0x00;
    KZP = 0x01;

    data_p = &data;
    *data_p = 0x00;        //data initial value
    MyUSBbuf = &MyUSBbuf_r;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
     // Set compare timer value
    OCR1AH = 0x4E;
    OCR1AL = 0x20;           // Compare register value
    TIMSK |= _BV(OCIE1A);
    TIFR |= _BV(OCF1A);
    TCCR1B |= _BV(WGM12)|_BV(CS11);

    DDRD &= ~_BV(PD3);    // Set PORTD3 for input
    PORTD |= _BV(PD3);

    MyUSBbufferInit((MyUSBBuffer*)MyUSBbuf, USB_buf_array, 0x08);

    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();

    LED_PORT_DDR |= _BV(LED_BIT);   /* make the LED bit an output */
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */


    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
         if (counter1 == 0x08){
             *data_p = *input_p;
             //reverseByte(data_p);
            if(usbInterruptIsReady()){               // only if previous data was sent
                USBbufferAddToEnd((MyUSBBuffer*)MyUSBbuf, *data_p);
                size_of_arr = USBbufferGetSize((MyUSBBuffer*)MyUSBbuf);
                usbSetInterrupt(USBbufferSendAll((MyUSBBuffer*)MyUSBbuf), size_of_arr);
            }
            else{
                USBbufferAddToEnd((MyUSBBuffer*)MyUSBbuf, *data_p);
            }
          counter1 = 0;
           if(!input && counter2 <= 0x1E){              // increment counter2 until we have less then 30 zero bytes
             counter2++;
                if (counter2 >= 0x08)
                    KZP = 0x00;
           }
           else if(input)
             counter2 = 0;
         }
    }
}

/* ------------------------------------------------------------------------- */

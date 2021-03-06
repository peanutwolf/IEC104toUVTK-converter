/* Name: main.c
 * Project: UVTKreader
 * Author: Vigurskiy.ES
 * Creation Date: 12.11.13
 */

 /****************************************************/
 // TODO:
 // delete func set_output_request()
 // clean usbFunction()
 /****************************************************/


#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define LED_BIT             0
#define MISO_PORT_DDR       DDRB
#define USB_BUFFER_SIZE     8
#define CHAR_BIT            8
#define KP_BILL_CODE        5
#define PU_BILL_CODE        3

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv/usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#include "USBbuf.h"         // ring buffer util
#include "CRC16/lib_crc.h"
#include "STM32drv.h"

typedef struct HDLC_flags{
  uint8_t HDLC_bit;
  uint8_t fang;
  uint8_t KZP;
  uint8_t input_bit;
  uint8_t new_input_bit;
  uint8_t high_bit_counter;
  uint8_t bill_counter_flag;
} HDLC_flags;

typedef struct output_struct{
  uint8_t bit_counter;
  uint8_t byte_counter;
  uint8_t output_bit;
  uint8_t* transmit_arr_p;
  uint8_t transmit_arr_len;
} output_struct;

volatile uint8_t *data_p, *input_p, *high_bit_counter_p, *USB_msg_data_p, *data_byte_counter_p, *bill_is_set_p, *inv_p;
volatile MyUSBBuffer* MyUSBbuf;
volatile uint8_t input, counter1, data;
volatile HDLC_flags *flags_p, *output_flags_p;
volatile output_struct *out_struct_p, *out_struct_KZP_p, *out_struct_opros_p, *out_struct_bill_p;
unsigned short *crc_kermit_p;

uint8_t transmit_arr_KZP [] = {
0b00000000,
0b00000000,
0b01111111,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
};

uint8_t transmit_arr_bill [] = {
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b01111110,
0b00001001,
0b00000011,
0b01000011,
0b00101011,
0b01111110,
0b00000000,
0b00000000,
0b00000000,
};

uint8_t transmit_arr []  = {
0b01111110,
0b00001001,
0b00100100,
0b00001100,
0b11100011,
0b11010100,
0b01111110,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
};

void reverse_byte (volatile uint8_t* v );
void polish_input_bit(uint8_t input_bit);
void set_output_bit(volatile output_struct *out_struct_p);
void set_output_request();
uint8_t set_bill_answ(uint8_t* data_byte_counter_p, uint8_t* data_p);
void spi_slave_init();

ISR (TIMER1_COMPA_vect){

        if (bit_is_set(PIND,3)){
            if((*inv_p & 0x01)){
                flags_p->input_bit = 0x00;
            }
            else flags_p->input_bit = 0x01;
        }

        else{
            if((*inv_p & 0x01)){
                flags_p->input_bit = 0x01;
            }
            else flags_p->input_bit = 0x00;
        }
        if (out_struct_p->output_bit){
            if(((*inv_p & 0x02)>>1)){
                PORTD &= ~_BV(PD5);
            }
            else PORTD |= _BV(PD5);
        }
        else{
            if(((*inv_p & 0x02)>>1)){
                PORTD |= _BV(PD5);
            }
            else PORTD &= ~_BV(PD5);
        }
     flags_p->new_input_bit = 0x01;
}

ISR(SPI_STC_vect){
    uint8_t spi_data;
    spi_data = SPDR;
    if(spi_data == 0x00){
        SPDR = getUVTKData();
         //SPDR = 0xBB;
    }
    else{
        getServData(spi_data);
        SPDR = 0xAA;
    }
}

void polish_input_bit (uint8_t input_bit){

    *input_p <<= 1;                 //shift input byte
        if (input_bit){
            input = input|0x01;
            (*high_bit_counter_p)++;
            if (*(high_bit_counter_p) == 0x01 && (flags_p->fang != 0x01)){
                counter1 = 0x01;
            }
            else if(*high_bit_counter_p == 0x05){
                flags_p->HDLC_bit = 0x01;
            }
            else if(*high_bit_counter_p == 0x06){
                flags_p->HDLC_bit = 0x00;
                flags_p->fang ^= 0x01;
            }
            else if(*high_bit_counter_p == 0x07){
                flags_p->HDLC_bit = 0x00;
                flags_p->fang = 0x00;
                flags_p->KZP = 0x01;
                *high_bit_counter_p = 0x00;
            }
        }

        else{
            if(flags_p->HDLC_bit == 0x01){
                *input_p >>= 1;
                counter1--;
                flags_p->HDLC_bit = 0x00;
            }
            else if(flags_p->KZP == 0x01){
                flags_p->fang = 0x00;
                flags_p->KZP = 0x00;
            }
            else if (flags_p->fang == 0x01){
                if(flags_p->bill_counter_flag){
                   *data_byte_counter_p = 0x00;
                   flags_p->bill_counter_flag = 0x00;
                   *bill_is_set_p = 0x00;
                }
            }
            if (flags_p->fang == 0x00){
                flags_p->bill_counter_flag = 0x01;
            }
            *high_bit_counter_p = 0x00;
        }

        counter1++;
}

void set_output_bit(volatile output_struct *out_struct_pnt){
   uint8_t *send_byte = (&(out_struct_pnt->transmit_arr_p[out_struct_p->byte_counter]));
   if(output_flags_p->HDLC_bit){
        out_struct_pnt->output_bit = 0x00;
        output_flags_p->HDLC_bit = 0x00;
        (out_struct_pnt->bit_counter)--;
    }
    else if(bit_is_set(*send_byte, out_struct_pnt->bit_counter)){
        out_struct_pnt->output_bit = 0x01;
        //TODO: If transmit arr doesnt start with non zero byte, this line makes a problem :: probably fixed
        //if((out_struct_pnt->byte_counter != 0x00) & (out_struct_pnt->byte_counter != 0x06)){
        if(((*send_byte) != 0x7E) & ((*send_byte) != 0x7F)){
            (output_flags_p->high_bit_counter)++;
            if((output_flags_p->high_bit_counter) == 0x05){
                output_flags_p->high_bit_counter = 0x00;
                output_flags_p->HDLC_bit = 0x01;
            }
        }
    }
    else{
        output_flags_p->high_bit_counter = 0x00;
        out_struct_pnt->output_bit = 0x00;
    }

    (out_struct_pnt->bit_counter)++;

    if(out_struct_pnt->bit_counter == 0x08){
        (out_struct_pnt->byte_counter)++;
        if (out_struct_pnt->byte_counter == out_struct_pnt->transmit_arr_len){
            out_struct_pnt->byte_counter = 0x00;

            if(*USB_msg_data_p == 0x01){
                out_struct_p = out_struct_opros_p;
                *USB_msg_data_p = 0x00;
            }
            else if((flags_p->fang == 0x00) & (output_flags_p->fang == 0x01)){
                out_struct_p = out_struct_bill_p;
                output_flags_p->fang = 0x00;
            }
            else if(*USB_msg_data_p == 0x00){
                out_struct_p = out_struct_KZP_p;
            }

        }
        out_struct_pnt->bit_counter = 0x00;
    }
}

void set_output_request(){
    if(*USB_msg_data_p == 0x00){
        out_struct_p = out_struct_KZP_p;
    }
    else if(*USB_msg_data_p == 0x01){
        out_struct_p = out_struct_opros_p;
        //*USB_msg_data_p = 0x00;
    }
}

uint8_t set_bill_answ(uint8_t* data_byte_counter_p, uint8_t* data_p){
    if((flags_p->fang == 0x01)){
        (*data_byte_counter_p)++;
    if((*data_byte_counter_p) == 0x03){
        if(((*data_p) != 0x03) & ((*data_p) != 0x05)){   //if data is not receipt
            *bill_is_set_p = 0x01;
            return 1;
        }
        else{
            *bill_is_set_p = 0x01;
            return 0;
        }
    }
    }
  return 0;
}

void reverse_byte( volatile uint8_t* v){
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

void spi_slave_init(){
    MISO_PORT_DDR |= _BV(PB4);
    SPCR |= _BV(SPE)|_BV(SPIE)|_BV(CPHA);
    SPCR &= ~_BV(CPOL)|~_BV(CPHA);
    SPSR &= ~_BV(SPIF);
    SPDR = 0x00;
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;
static uchar dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */

    if(rq->bRequest == CUSTOM_RQ_SET_INV){
        *inv_p |= _BV(7);           //set inv flag for usbFunctionWrite.
        return USB_NO_MSG;
    }else if(rq->bRequest == CUSTOM_RQ_SET_STATUS){
        if(rq->wValue.bytes[0]){
            return USB_NO_MSG;        // tell driver to use usbFunctionWrite()
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

uchar usbFunctionWrite(uchar *data, uchar len)
{
    if(!((*inv_p & 0x80)>>7)){
        transmit_arr[1] = data[0];
        transmit_arr[2] = data[1];
        transmit_arr[3] = data[2];
        transmit_arr_bill[15] = data[0];
        *USB_msg_data_p = 0xFF;
    }
    else{
        *inv_p = data[0];
    }
    return 1;
}
/* ------------------------------------------------------------------------- */


int __attribute__((noreturn)) main(void)
{
    uchar   i;
    MyUSBBuffer MyUSBbuf_r;
    HDLC_flags flags = {0, 0, 0, 0, 0, 0, 1};
    HDLC_flags output_flags = {0, 0, 0, 0, 0, 0, 0};
    output_struct output_struct_KZP = {0, 0, 0};
    output_struct output_struct_opros = {0, 0, 0};
    output_struct output_struct_bill = {0, 0, 0};
    uint8_t size_of_arr, high_bit_counter, USB_msg_data, KP_adr_tmp, data_byte_counter, bill_is_set, inv;
    uint8_t USB_buf_array[USB_BUFFER_SIZE];
    unsigned short crc_kermit;

    crc_kermit_p = &crc_kermit;
    *crc_kermit_p = 0x00;

    input_p = &input;
    *input_p = 0x00;
    high_bit_counter_p = &high_bit_counter;
    *high_bit_counter_p = 0x00;
    data_byte_counter_p = &data_byte_counter;
    *data_byte_counter_p = 0x00;
    bill_is_set_p = &bill_is_set;
    *bill_is_set_p = 0x00;
    inv_p = &inv;
    *inv_p = 0x03;
    flags_p = &flags;
    output_flags_p = &output_flags;

    output_struct_KZP.transmit_arr_p = transmit_arr_KZP;
    output_struct_KZP.transmit_arr_len = 0x0C;

    output_struct_opros.transmit_arr_p = transmit_arr;
    output_struct_opros.transmit_arr_len = 0x1C;

    output_struct_bill.transmit_arr_p = transmit_arr_bill;
    output_struct_bill.transmit_arr_len = 0x17;

    out_struct_KZP_p = &output_struct_KZP;
    out_struct_opros_p = &output_struct_opros;
    out_struct_bill_p = &output_struct_bill;
    out_struct_p = out_struct_KZP_p;

    counter1 = 0x00;
    data_p = &data;
    *data_p = 0x00;        //data initial value
    USB_msg_data_p = &USB_msg_data;
    *USB_msg_data_p = 0x00;
    MyUSBbuf = &MyUSBbuf_r;

    KP_adr_tmp = transmit_arr_bill[15];

    wdt_enable(WDTO_1S);


     // Set compare timer value
    OCR1AH = 0x4E;
    OCR1AL = 0x20;           // Compare register value
    TIMSK |= _BV(OCIE1A);
    TIFR |= _BV(OCF1A);
    TCCR1B |= _BV(WGM12)|_BV(CS11);

    DDRD &= ~_BV(PD3);    // Set PORTD3 for input
    PORTD |= _BV(PD3);
    DDRD |= _BV(PD5);     // Set PORTD5 for output

    MyUSBbufferInit((MyUSBBuffer*)MyUSBbuf, USB_buf_array, 0x08);
    spi_slave_init();

    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();

    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
        if(flags_p->new_input_bit){
            polish_input_bit(flags_p->input_bit);
            set_output_bit(out_struct_p);
            flags_p->new_input_bit = 0x00;
        }
        if(*USB_msg_data_p == 0xFF){
            crc_kermit = 0x00;
            for(i = 1 ; i < 4; i++){
            crc_kermit = update_crc_kermit(crc_kermit, transmit_arr[i]);
            }
            transmit_arr[4] = (uint8_t)(crc_kermit & 0x00ff);
            transmit_arr[5] = (uint8_t)((crc_kermit & 0xff00) >> 8);
            *USB_msg_data_p = 0x01;
            if(KP_adr_tmp != transmit_arr_bill[15]){
                crc_kermit = 0x00;
                for(i = 15 ; i < 17; i++){
                    crc_kermit = update_crc_kermit(crc_kermit, transmit_arr_bill[i]);
                }
                transmit_arr_bill[17] = (uint8_t)(crc_kermit & 0x00ff);
                transmit_arr_bill[18] = (uint8_t)((crc_kermit & 0xff00) >> 8);
                KP_adr_tmp = transmit_arr_bill[15];
            }
        }
         if (counter1 == 0x08){
             *data_p = *input_p;
             reverse_byte(data_p);
             if(!bill_is_set){
                output_flags_p->fang = set_bill_answ((uint8_t*)data_byte_counter_p, (uint8_t*)data_p);
             }
            if(usbInterruptIsReady()){               // only if previous data was sent
                USBbufferAddToEnd((MyUSBBuffer*)MyUSBbuf, *data_p);
                size_of_arr = USBbufferGetSize((MyUSBBuffer*)MyUSBbuf);
                usbSetInterrupt(USBbufferSendAll((MyUSBBuffer*)MyUSBbuf), size_of_arr);
            }
            else{
                USBbufferAddToEnd((MyUSBBuffer*)MyUSBbuf, *data_p);
            }
          counter1 = 0;
          CRITICAL_SECTION_START
          sendUVTKData((uint8_t*)data_p, flags_p->fang);
          CRITICAL_SECTION_END
         }
    }
}

/* ------------------------------------------------------------------------- */

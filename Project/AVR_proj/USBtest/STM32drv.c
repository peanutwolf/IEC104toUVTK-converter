#include "STM32drv.h"

extern volatile uint8_t *inv_p, *USB_msg_data_p;
extern uint8_t transmit_arr [];
extern uint8_t transmit_arr_bill [];

MK_FIFO(SPI_BUFFER_SIZE);
DEFINE_FIFO(spi_fifo, SPI_BUFFER_SIZE);

MK_FIFO(SPI_MSG_SIZE);
DEFINE_FIFO(spi_msg_size_fifo, SPI_MSG_SIZE);

void sendUVTKData(uint8_t* data, uint8_t fang){
    static uint8_t in_fang_flag = 0x00;
    static uint8_t msg_len = 0x00;
    static uint8_t buf_err = 0x00;
    uint8_t* data_tmp = 0x00;

    if(fang == 0x01 && buf_err == 0x00){
        if(in_fang_flag == 0x00){
            if(FifoFreeDataLength(spi_fifo) < UVTK_MAX_MSG_SIZE){
                buf_err = 0x01;
                return;
            }
            in_fang_flag = 0x01;
            msg_len = 0x00;
        }
        if(!FifoWrite(spi_fifo, *data)){
            msg_len++;
        }
    }
    else{
        if(fang == 0x00 && in_fang_flag == 0x01){
            in_fang_flag = 0x00;
            if(!FifoWrite(spi_fifo, *data)){
                msg_len++;
            }
            FifoWrite(spi_msg_size_fifo, msg_len);
        }
        else if(fang == 0x00 && buf_err == 0x01){
            buf_err = 0x00;
            FifoPeek(spi_msg_size_fifo, data_tmp);
            if(*data_tmp != 0xFF){
                FifoWrite(spi_msg_size_fifo, 0xFF);
            }
        }
    }
}

uint8_t getUVTKData(){
    static uint8_t msg_snd_flag = 0x00;
    static uint8_t msg_len = 0x00;
    uint8_t* data = 0x00;

    if(!msg_snd_flag){
        if(FifoRead(spi_msg_size_fifo, data)){
            return 0x00;
        }
        else if(*data != 0xFF){
            msg_snd_flag = 0x01;
            msg_len = *data;
        }
        return *data;
    }
    else{
        if(msg_len == 0x01){
            msg_snd_flag = 0x00;
        }
        FifoRead(spi_fifo, data);
        msg_len--;
        return *data;
    }
}

void getServData(uint8_t data){
    static uint8_t msg_len = 0x00;
    static uint8_t serv_rcv_code = 0x00;

    if(serv_rcv_code == 0x00 && data != 0x00){
        switch(data){
            case 0x01:
                msg_len = 0x01;
                serv_rcv_code = 0x01;
                break;
            case 0x02:
                msg_len = 0x03;
                serv_rcv_code = 0x02;
                break;
            default:
                for(;;);
        }
    }
    else{
        switch(serv_rcv_code){
            case 0x01:
                *inv_p = data;
                msg_len--;
                serv_rcv_code = 0x00;
                break;
            case 0x02:
                if(msg_len == 0x01){
                   serv_rcv_code = 0x00;
                   transmit_arr_bill[15] = transmit_arr[1];
                   *USB_msg_data_p = 0xFF;
                }
                transmit_arr[0x04 - msg_len] = data;
                msg_len--;
                break;
            default:
                for(;;);
        }
    }
}








#ifndef AVR_FIFO_H_INCLUDED
#define AVR_FIFO_H_INCLUDED

struct fifo {
    uint8_t size;           /* size of buffer in bytes */
    uint8_t read;           /* read pointer */
    uint8_t write;          /* write pointer */
    unsigned char buffer[]; /* fifo ring buffer */
};

/* define a FIFO type for 'size' bytes */
#define MK_FIFO(size) \
    struct fifo_ ## size {                  \
        struct fifo f;                      \
        unsigned char buffer_bytes[size];   \
    }

/* define a fifo (type: struct fifo *) with name 'name' for 's' bytes */
#define DEFINE_FIFO(name, s)                                \
    struct fifo_##s _raw_##name = { .f = { .size = s } };   \
    struct fifo *name = &_raw_##name.f


uint8_t FifoDataLength (struct fifo *fifo);


uint8_t FifoWrite (struct fifo *fifo, unsigned char data);

uint8_t FifoRead (struct fifo *fifo, unsigned char *data);

uint8_t FifoFreeDataLength (struct fifo *fifo);

uint8_t FifoPeek (struct fifo *fifo, unsigned char *data);


#endif // AVR_FIFO_H_INCLUDED

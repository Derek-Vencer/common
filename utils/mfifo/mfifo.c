#include <stdio.h>
#include <string.h>
#include "mfifo.h"
#include "hal_trace.h"

void mfifo_init(struct mfifo *fifo, unsigned char *buffer, unsigned int len)
{
    fifo->size = len;
    fifo->buffer = buffer;
    fifo->in = fifo->out = 0;
}
unsigned int mfifo_put(struct mfifo *fifo, unsigned char *buffer, unsigned int len)
{
    unsigned int l;
    l = fifo->size - fifo->in + fifo->out;
    len = MIN(len, l);
    __sync_synchronize();

    l = MIN(len, fifo->size - (fifo->in % fifo->size));
    memcpy(fifo->buffer + (fifo->in % fifo->size), buffer, l);
    memcpy(fifo->buffer, buffer + l, len - l);

    __sync_synchronize();
    fifo->in += len;

    return len;
}
unsigned int mfifo_get(struct mfifo *fifo, unsigned char *buffer, unsigned int len)
{
    unsigned int l;
    l = fifo->in - fifo->out;
    len = MIN(len, l);
    __sync_synchronize();

    l = MIN(len, fifo->size - (fifo->out % fifo->size));
    memcpy(buffer, fifo->buffer + (fifo->out % fifo->size), l);
    memcpy(buffer + l, fifo->buffer, len - l);

    __sync_synchronize();
    fifo->out += len;

    return len;
}
unsigned int mfifo_peek(struct mfifo *fifo, unsigned int len_want, unsigned char **buff1, unsigned char **buff2, unsigned int *len1, unsigned int *len2)
{
    unsigned int l,len,off;

    *buff1 = *buff2 = NULL;
    *len1 = *len2 = 0;
    l = fifo->in - fifo->out;
    len = MIN(len_want, l);
    if (len < len_want) {
        return 0;
    }

    __sync_synchronize();
    off = fifo->out % fifo->size;
    l = MIN(len, fifo->size - off);
    *buff1 = fifo->buffer + off;
    *len1 = l;
    if (l < len) {
        *buff2 = fifo->buffer;
        *len2 = len-l;
    }

    return len_want;
}

unsigned int mfifo_peek_to_buf(struct mfifo *fifo, unsigned char *buff, unsigned int len)
{
    unsigned char *buf1 = NULL, *buf2 = NULL;
    unsigned int len1 = 0, len2 = 0;

    mfifo_peek(fifo, len, &buf1, &buf2, &len1, &len2);

    if (len == (len1 + len2)) {
        memcpy(buff, buf1, len1);
        memcpy(buff + len1, buf2, len2);
    } else {
        return 0;
    }

    return len;
}

unsigned int mfifo_len(struct mfifo *fifo)
{
    return (fifo->in - fifo->out);
}

unsigned int mfifo_get_free_space(struct mfifo *fifo)
{
    return (fifo->size - fifo->in + fifo->out);
}

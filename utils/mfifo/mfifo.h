#ifndef MFIFO_H
#define MFIFO_H 1

#include "plat_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct mfifo {
    unsigned char *buffer;     /* the buffer holding the data */
    unsigned int size;         /* the size of the allocated buffer */
    unsigned int in;           /* data is added at offset (in % size) */
    unsigned int out;          /* data is extracted from off. (out % size) */
};

void mfifo_init(struct mfifo *k, unsigned char *buff, unsigned int len);
unsigned int mfifo_put(struct mfifo *k, unsigned char *buff, unsigned int len);
unsigned int mfifo_get(struct mfifo *k, unsigned char *buff, unsigned int len);
unsigned int mfifo_peek(struct mfifo *k, unsigned int len_want, unsigned char **buff1, unsigned char **buff2, unsigned int *len1, unsigned int *len2);
unsigned int mfifo_peek_to_buf(struct mfifo *fifo, unsigned char *buff, unsigned int len);
unsigned int mfifo_len(struct mfifo *fifo);
unsigned int mfifo_get_free_space(struct mfifo *fifo);

#if defined(__cplusplus)
}
#endif

#endif /* MFIFO_H */

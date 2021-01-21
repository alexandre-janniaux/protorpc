#include <stdlib.h>
#include <string.h>
#include "cprotorpc/serializer.h"

#define SIDL_SERIALIZER_CAPACITY (128)
#define SIDL_SERIALIZER_FD_CAPACITY (16)

int sidl_serializer_init(sidl_serializer_t* s)
{
    s->data_size = 0;
    s->data_capacity = SIDL_SERIALIZER_CAPACITY;
    s->data = malloc(s->data_capacity);

    if (!s->data)
        return -1;

    s->fd_count = 0;
    s->fd_capacity = SIDL_SERIALIZER_FD_CAPACITY;
    s->fds = malloc(sizeof(int) * s->fd_capacity);

    if (!s->fds)
    {
        free(s->data);
        return -1;
    }

    return 0;
}

void sidl_serializer_destroy(sidl_serializer_t* s)
{
    free(s->data);
    free(s->fds);
}

int sidl_serializer_write_raw(sidl_serializer_t* s, const void* data, size_t size)
{
    if (s->data_size + size > s->data_capacity)
    {
        size_t new_capacity = s->data_capacity;

        while (s->data_size + size > new_capacity)
            new_capacity *= 2;

        void* new_data = realloc(s->data, new_capacity);

        if (!new_data)
            return -1;

        s->data_capacity = new_capacity;
        s->data = new_data;
    }

    memcpy((char*)s->data + s->data_size, data, size);
    s->data_size += size;

    return 0;
}

int sidl_serializer_write_fd(sidl_serializer_t* s, int fd)
{
    if (s->fd_count == s->fd_capacity)
    {
        size_t new_capacity = s->fd_capacity * 2;
        int* new_fds = realloc(s->fds, sizeof(int) * new_capacity);

        if (!new_fds)
            return -1;

        s->fd_capacity = new_capacity;
        s->fds = new_fds;
    }

    s->fds[s->fd_count++] = fd;

    return 0;
}

#define DEFINE_SERIALIZER(SIDL_NAME, C_TYPE) \
int sidl_serializer_write_##SIDL_NAME(sidl_serializer_t* s, C_TYPE value) {\
    if (s->data_size + sizeof(C_TYPE) > s->data_capacity) \
    { \
        size_t new_capacity = s->data_capacity * 2; \
        void* new_data = realloc(s->data, new_capacity); \
        \
        if (!new_data) \
            return -1; \
        \
        s->data_capacity = new_capacity; \
        s->data = new_data; \
    } \
    \
    *(C_TYPE*)((char*)s->data + s->data_size) = value; \
    s->data_size += sizeof(C_TYPE); \
    \
    return 0; \
}

DEFINE_SERIALIZER(u8, uint8_t)
DEFINE_SERIALIZER(u16, uint16_t)
DEFINE_SERIALIZER(u32, uint32_t)
DEFINE_SERIALIZER(u64, uint64_t)
DEFINE_SERIALIZER(i8, int8_t)
DEFINE_SERIALIZER(i16, int16_t)
DEFINE_SERIALIZER(i32, int32_t)
DEFINE_SERIALIZER(i64, int64_t)
DEFINE_SERIALIZER(usize, size_t)

#undef DEFINE_SERIALIZER

int sidl_serializer_write_string(sidl_serializer_t* s, const char* str)
{
    size_t length = strlen(str);

    if (sidl_serializer_write_usize(s, length) < 0)
        return -1;

    if (sidl_serializer_write_raw(s, str, length) < 0)
        return -1;

    return 0;
}

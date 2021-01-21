#include <stdlib.h>
#include <string.h>
#include "cprotorpc/unserializer.h"

void sidl_unserializer_init(sidl_unserializer_t* u, void* data, size_t data_size, int* fds, size_t fd_count)
{
    u->data = data;
    u->data_size = data_size;
    u->data_offset = 0;

    u->fds = fds;
    u->fd_count = fd_count;
    u->fd_index = 0;
}

int sidl_unserializer_read_raw(sidl_unserializer_t* u, void* data, size_t size)
{
    if (u->data_offset + size > u->data_size)
        return -1;

    memcpy(data, (char*)u->data + u->data_offset, size);
    u->data_offset += size;

    return 0;
}

int sidl_unserializer_read_fd(sidl_unserializer_t* u, int* fd)
{
    if (u->fd_index == u->fd_count)
        return -1;

    *fd = u->fds[u->fd_index++];

    return 0;
}

#define DEFINE_UNSERIALIZER(SIDL_NAME, C_TYPE) \
int sidl_unserializer_read_##SIDL_NAME(sidl_unserializer_t* u, C_TYPE* value) { \
    if (u->data_offset + sizeof(C_TYPE) > u->data_size) \
        return -1; \
    \
    *value = *(C_TYPE*)((char*)u->data + u->data_offset); \
    u->data_offset += sizeof(C_TYPE); \
    \
    return 0; \
}

DEFINE_UNSERIALIZER(u8, uint8_t)
DEFINE_UNSERIALIZER(u16, uint16_t)
DEFINE_UNSERIALIZER(u32, uint32_t)
DEFINE_UNSERIALIZER(u64, uint64_t)
DEFINE_UNSERIALIZER(i8, int8_t)
DEFINE_UNSERIALIZER(i16, int16_t)
DEFINE_UNSERIALIZER(i32, int32_t)
DEFINE_UNSERIALIZER(i64, int64_t)
DEFINE_UNSERIALIZER(usize, size_t)

#undef DEFINE_UNSERIALIZER

int sidl_unserializer_read_string(sidl_unserializer_t* u, const char** str)
{
    size_t string_len = 0;

    if (sidl_unserializer_read_usize(u, &string_len) < 0)
        return -1;

    char* string = malloc(string_len + 1);

    if (sidl_unserializer_read_raw(u, string, string_len) < 0)
    {
        free(string);
        return -1;
    }

    string[string_len] = 0;
    *str = string;

    return 0;
}

#include <stdlib.h>
#include <string.h>
#include "cprotorpc/serializer.h"

#define SIDL_SERIALIZER_CAPACITY (128)

int sidl_serializer_init(sidl_serializer_t* s)
{
    s->size = 0;
    s->capacity = SIDL_SERIALIZER_CAPACITY;
    s->data = malloc(s->capacity);

    if (!s->data)
        return -1;

    return 0;
}

int sidl_serializer_write_raw(sidl_serializer_t* s, void* data, size_t length)
{
    if (s->size + length > s->capacity)
    {
        size_t new_capacity = s->capacity;

        while (s->size + length > new_capacity)
            new_capacity *= 2;

        void* new_data = realloc(s->data, new_capacity);

        if (!new_data)
            return -1;

        s->capacity = new_capacity;
        s->data = new_data;
    }

    memcpy((char*)s->data + s->size, data, length);
    s->size += length;

    return 0;
}

#define DEFINE_SERIALIZER(SIDL_NAME, C_TYPE) \
int sidl_serializer_write_##SIDL_NAME(sidl_serializer_t* s, C_TYPE value) {\
    if (s->size + sizeof(C_TYPE) > s->capacity) \
    { \
        size_t new_capacity = s->capacity * 2; \
        void* new_data = realloc(s->data, new_capacity); \
        \
        if (!new_data) \
            return -1; \
        \
        s->capacity = new_capacity; \
        s->data = new_data; \
    } \
    \
    *(C_TYPE*)(((char*)s->data) + s->size) = value; \
    s->size += sizeof(C_TYPE); \
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

    if (sidl_serializer_write_raw(s, s, length) < 0)
        return -1;

    return 0;
}

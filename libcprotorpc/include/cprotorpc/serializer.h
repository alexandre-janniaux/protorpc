#ifndef CPROTORPC_SERIALIZER
#define CPROTORPC_SERIALIZER

#include <stdint.h>
#include <stddef.h>

/*
 * Buffer used for serialization
 */
typedef struct sidl_serializer_t
{
    size_t size;
    size_t capacity;
    void* data;
} sidl_serializer_t;

int sidl_serializer_init(sidl_serializer_t* s);
int sidl_serializer_write_raw(sidl_serializer_t* s, void* data, size_t length);

// Standard data types
int sidl_serializer_write_u8(sidl_serializer_t* s, uint8_t value);
int sidl_serializer_write_u16(sidl_serializer_t* s, uint16_t value);
int sidl_serializer_write_u32(sidl_serializer_t* s, uint32_t value);
int sidl_serializer_write_u64(sidl_serializer_t* s, uint64_t value);
int sidl_serializer_write_i8(sidl_serializer_t* s, int8_t value);
int sidl_serializer_write_i16(sidl_serializer_t* s, int16_t value);
int sidl_serializer_write_i32(sidl_serializer_t* s, int32_t value);
int sidl_serializer_write_i64(sidl_serializer_t* s, int64_t value);
int sidl_serializer_write_usize(sidl_serializer_t* s, size_t value);
int sidl_serializer_write_string(sidl_serializer_t* s, const char* str);

#endif

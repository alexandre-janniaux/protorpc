#ifndef CPROTORPC_UNSERIALIZER
#define CPROTORPC_UNSERIALIZER

#include <stdint.h>
#include <stddef.h>

typedef struct sidl_unserializer_t
{
    // Input data
    size_t data_size;
    size_t data_offset;
    void* data;

    // Input file descriptors
    size_t fd_count;
    size_t fd_index;
    int* fds;
} sidl_unserializer_t;

void sidl_unserializer_init(sidl_unserializer_t* u, void* data, size_t data_size, int* fds, size_t fd_count);
int sidl_unserializer_read_raw(sidl_unserializer_t* u, void* data, size_t size);
int sidl_unserializer_read_fd(sidl_unserializer_t* u, int* fd);

// Standard data types
int sidl_unserializer_read_u8(sidl_unserializer_t* u, uint8_t* value);
int sidl_unserializer_read_u16(sidl_unserializer_t* u, uint16_t* value);
int sidl_unserializer_read_u32(sidl_unserializer_t* u, uint32_t* value);
int sidl_unserializer_read_u64(sidl_unserializer_t* u, uint64_t* value);
int sidl_unserializer_read_i8(sidl_unserializer_t* u, int8_t* value);
int sidl_unserializer_read_i16(sidl_unserializer_t* u, int16_t* value);
int sidl_unserializer_read_i32(sidl_unserializer_t* u, int32_t* value);
int sidl_unserializer_read_i64(sidl_unserializer_t* u, int64_t* value);
int sidl_unserializer_read_usize(sidl_unserializer_t* u, size_t* value);
int sidl_unserializer_read_string(sidl_unserializer_t* u, const char** str);

#endif

#ifndef CPROTORPC_SERIALIZER
#define CPROTORPC_SERIALIZER

#include <stdint.h>
#include <stddef.h>
#include "cprotorpc/sidl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Buffer used for serialization
 */
typedef struct sidl_serializer_t
{
    // Raw serialized data
    size_t data_size;
    size_t data_capacity;
    void* data;

    // File descriptors
    size_t fd_count;
    size_t fd_capacity;
    int* fds;
} sidl_serializer_t;

int sidl_serializer_init(sidl_serializer_t* s);
void sidl_serializer_destroy(sidl_serializer_t* s);
int sidl_serializer_write_raw(sidl_serializer_t* s, const void* data, size_t size);
int sidl_serializer_write_fd(sidl_serializer_t* s, int fd);

// Standard data types
#define SIDL_SERIALIZE_FUNCTION(SIDL_TYPE, C_TYPE) \
    int sidl_serializer_write_##SIDL_TYPE(sidl_serializer_t* s, C_TYPE value);

XM_SIDL_TYPES(SIDL_SERIALIZE_FUNCTION)

#undef SIDL_SERIALIZE_FUNCTION

int sidl_serializer_write_string(sidl_serializer_t* s, const char* str);



#ifdef __cplusplus
}
#endif

#endif

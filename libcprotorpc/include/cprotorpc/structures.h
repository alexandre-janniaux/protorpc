#ifndef CPROTORPC_STRUCTURES
#define CPROTORPC_STRUCTURES

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "cprotorpc/sidl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Vector declaration
#define SIDL_VSNAME(SIDL_TYPE) sidl_##SIDL_TYPE##_vector

#define SIDL_VEC_STRUCT(SIDL_TYPE, C_TYPE) \
typedef struct SIDL_VSNAME(SIDL_TYPE) \
{ \
    size_t size; \
    size_t capacity; \
    C_TYPE* elements; \
} SIDL_VSNAME(SIDL_TYPE);

#define SIDL_VEC_DECL(SIDL_TYPE, C_TYPE) \
int sidl_##SIDL_TYPE##_vector_init(SIDL_VSNAME(SIDL_TYPE)* v); \
int sidl_##SIDL_TYPE##_vector__from(SIDL_VSNAME(SIDL_TYPE)* v, C_TYPE* data, size_t count); \
int sidl_##SIDL_TYPE##_vector_append(SIDL_VSNAME(SIDL_TYPE)* v, C_TYPE value); \
void sidl_##SIDL_TYPE##_vector_destroy(SIDL_VSNAME(SIDL_TYPE)* v); \
\
static inline C_TYPE sidl_##SIDL_TYPE##_vector_get(SIDL_VSNAME(SIDL_TYPE)* v, size_t index) { \
    assert(index < v->size && "Vector access out of bounds"); \
    return v->elements[index]; \
}

#define SIDL_DECLARE_VECTOR_PROTO(SIDL_TYPE, C_TYPE) \
    SIDL_VEC_STRUCT(SIDL_TYPE, C_TYPE) \
    SIDL_VEC_DECL(SIDL_TYPE, C_TYPE)

XM_SIDL_TYPES(SIDL_DECLARE_VECTOR_PROTO)

// Special case for the string type
SIDL_DECLARE_VECTOR_PROTO(string, char*)

// Vector implementation
#define SIDL_VECTOR_CAPACITY (10)

#define SIDL_VEC_INIT_IMPL(SIDL_TYPE, C_TYPE) \
int sidl_##SIDL_TYPE##_vector_init(SIDL_VSNAME(SIDL_TYPE)* v) { \
    v->size = 0; \
    v->capacity = SIDL_VECTOR_CAPACITY; \
    v->elements = malloc(sizeof(C_TYPE) * v->capacity); \
    \
    if (!v->elements) \
        return -1; \
    \
    return 0; \
}

#define SIDL_VEC_INIT_FROM_IMPL(SIDL_TYPE, C_TYPE) \
int sidl_##SIDL_TYPE##_vector_from(SIDL_VSNAME(SIDL_TYPE)* v, C_TYPE* data, size_t count) { \
    v->size = count; \
    v->capacity = count; \
    v->elements = malloc(sizeof(C_TYPE) * v->capacity); \
    \
    if (!v->elements) \
        return -1; \
    \
    memcpy(v->elements, data, count * sizeof(C_TYPE)); \
    \
    return 0; \
}

#define SIDL_VEC_APPEND_IMPL(SIDL_TYPE, C_TYPE) \
int sidl_##SIDL_TYPE##_vector_append(SIDL_VSNAME(SIDL_TYPE)* v, C_TYPE value) { \
    if (v->size >= v->capacity) \
    { \
        size_t new_capacity = v->capacity * 2; \
        C_TYPE* new_array = realloc(v->elements, new_capacity * sizeof(C_TYPE)); \
        \
        if (!new_array) \
            return -1; \
        \
        v->capacity = new_capacity; \
        v->elements = new_array; \
    } \
    \
    v->elements[v->size++] = value; \
    \
    return 0; \
}

#define SIDL_VEC_DESTROY_IMPL(SIDL_TYPE, C_TYPE) \
void sidl_##SIDL_TYPE##_vector_destroy(SIDL_VSNAME(SIDL_TYPE)* v) { \
    free(v->elements); \
} \

#define SIDL_DECLARE_VECTOR_IMPL(SIDL_TYPE, C_TYPE) \
        SIDL_VEC_INIT_IMPL(SIDL_TYPE, C_TYPE) \
        SIDL_VEC_INIT_FROM_IMPL(SIDL_TYPE, C_TYPE) \
        SIDL_VEC_APPEND_IMPL(SIDL_TYPE, C_TYPE)

/*
 * Optional type. Object is present if element ptr is non null
 */

#define SIDL_OSNAME(SIDL_TYPE) sidl_##SIDL_TYPE##_optional
#define SIDL_OPT_STRUCT(SIDL_TYPE, C_TYPE) \
typedef struct sidl_##SIDL_TYPE##_optional \
{ \
    uint8_t present; \
    C_TYPE element; \
} sidl_##SIDL_TYPE##_optional; \

#define SIDL_OPT_DECL(SIDL_TYPE, C_TYPE) \
static inline int sidl_##SIDL_TYPE##_optional_empty(SIDL_OSNAME(SIDL_TYPE)* opt) \
{ \
    return !opt->present; \
} \
\
inline static C_TYPE* sidl_##SIDL_TYPE##_optional_data(SIDL_OSNAME(SIDL_TYPE)* opt) \
{ \
    assert(!sidl_##SIDL_TYPE##_optional_empty(opt) && "Tried to get element from empty optional"); \
    return &opt->element; \
} \
\
void sidl_##SIDL_TYPE##_optional_destroy(SIDL_OSNAME(SIDL_TYPE)* opt); \

#define SIDL_DECLARE_OPTIONAL_PROTO(SIDL_TYPE, C_TYPE) \
        SIDL_OPT_STRUCT(SIDL_TYPE, C_TYPE) \
        SIDL_OPT_DECL(SIDL_TYPE, C_TYPE)

XM_SIDL_TYPES(SIDL_DECLARE_OPTIONAL_PROTO)
SIDL_DECLARE_OPTIONAL_PROTO(string, char*)

#define SIDL_OPT_DESTROY_IMPL(SIDL_TYPE, C_TYPE) \
void sidl_##SIDL_TYPE##_optional_destroy(SIDL_OSNAME(SIDL_TYPE)* opt) \
{}

#define SIDL_DECLARE_OPTIONAL_IMPL(SIDL_TYPE, C_TYPE) \
        SIDL_OPT_DESTROY_IMPL(SIDL_TYPE, C_TYPE)

#ifdef __cplusplus
}
#endif

#endif

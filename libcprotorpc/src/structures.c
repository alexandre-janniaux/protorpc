#include <stdlib.h>
#include <string.h>
#include "cprotorpc/structures.h"

#define SIDL_VECTOR_CAPACITY (10)

int sidl_generic_generic_vector_new(sidl_generic_vector_t* vec)
{
    vec->size = 0;
    vec->capacity = SIDL_VECTOR_CAPACITY;
    vec->elements = calloc(vec->capacity, sizeof(void*));

    if (!vec->elements)
        return -1;

    return 0;
}

void sidl_generic_vector_destroy(sidl_generic_vector_t* vec)
{
    free(vec->elements);
}

int sidl_generic_vector_append(sidl_generic_vector_t* vec, void* element)
{
    if (vec->size >= vec->capacity)
    {
        size_t new_capacity = vec->capacity * 2;
        void** new_array = realloc(vec->elements, new_capacity * sizeof(void*));

        if (!new_array)
            return -1;

        vec->capacity = new_capacity;
        vec->elements = new_array;
    }

    vec->elements[vec->size++] = element;

    return 0;
}

// SIDL specific vectors
#define SIDL_VSNAME(SIDL_TYPE) sidl_##SIDL_TYPE##_vector_t

#define SIDL_VEC_INIT(SIDL_TYPE, C_TYPE) \
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

#define SIDL_VEC_INIT_FROM(SIDL_TYPE, C_TYPE) \
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

#define SIDL_VEC_APPEND(SIDL_TYPE, C_TYPE) \
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

#define SIDL_VEC_DESTROY(SIDL_TYPE, C_TYPE) \
void sidl_##SIDL_TYPE##_vector_destroy(SIDL_VSNAME(SIDL_TYPE)* v) { \
    free(v->elements); \
} \

XM_SIDL_TYPES(SIDL_VEC_INIT)
XM_SIDL_TYPES(SIDL_VEC_INIT_FROM)
XM_SIDL_TYPES(SIDL_VEC_APPEND)
XM_SIDL_TYPES(SIDL_VEC_DESTROY)

#undef SIDL_VSNAME
#undef SIDL_VEC_DESTROY
#undef SIDL_VEC_APPEND
#undef SIDL_VEC_INIT_FROM
#undef SIDL_VEC_INIT

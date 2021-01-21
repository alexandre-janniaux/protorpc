#ifndef CPROTORPC_STRUCTURES
#define CPROTORPC_STRUCTURES

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "cprotorpc/sidl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Generic vector of elements (used for user defined datatypes)
 */
typedef struct sidl_generic_vector_t
{
    size_t size;
    size_t capacity;
    void** elements;
} sidl_generic_vector_t;

int sidl_generic_vector_init(sidl_generic_vector_t* vec);
void sidl_generic_vector_destroy(sidl_generic_vector_t* vec);

/*
 * Frees inner content of the passed vector.
 */
void sidl_generic_vector_destroy(sidl_generic_vector_t* vec);
int sidl_generic_vector_append(sidl_generic_vector_t* vec, void* element);

inline static void* sidl_generic_vector_get(sidl_generic_vector_t* vec, size_t index)
{
    assert(index < vec->size && "Vector access out of bounds");
    return vec->elements[index];
}

/*
 * Using generic vectors for small copiable datatypes is a waste.
 */

#define SIDL_VSNAME(SIDL_TYPE) sidl_##SIDL_TYPE##_vector_t

#define SIDL_POD_VEC_STRUCT(SIDL_TYPE, C_TYPE) \
typedef struct SIDL_VSNAME(SIDL_TYPE) \
{ \
    size_t size; \
    size_t capacity; \
    C_TYPE* elements; \
} SIDL_VSNAME(SIDL_TYPE);

#define SIDL_POD_VEC_DECL(SIDL_TYPE, C_TYPE) \
int sidl_##SIDL_TYPE##_vector_init(SIDL_VSNAME(SIDL_TYPE)* v); \
int sidl_##SIDL_TYPE##_vector_from(SIDL_VSNAME(SIDL_TYPE)* v, C_TYPE* data, size_t count); \
int sidl_##SIDL_TYPE##_vector_append(SIDL_VSNAME(SIDL_TYPE)* v, C_TYPE value); \
void sidl_##SIDL_TYPE##_vector_destroy(SIDL_VSNAME(SIDL_TYPE)* v); \
\
static inline C_TYPE sidl_##SIDL_TYPE##_vector_get(SIDL_VSNAME(SIDL_TYPE)* v, size_t index) { \
    assert(index < v->size && "Vector access out of bounds"); \
    return v->elements[index]; \
}

XM_SIDL_TYPES(SIDL_POD_VEC_STRUCT)
XM_SIDL_TYPES(SIDL_POD_VEC_DECL)

#undef SIDL_VSNAME
#undef SIDL_POD_VEC_STRUCT
#undef SIDL_POD_VEC_DECL

/*
 * Optional type. Object is present if element ptr is non null
 */
typedef struct sidl_optional_t
{
    void* element;
} sidl_optional_t;

inline static int sidl_optional_empty(sidl_optional_t* opt)
{
    return opt->element != NULL;
}

inline static void* sidl_optional_data(sidl_optional_t* opt)
{
    assert(!sidl_optional_empty(opt) && "Tried to get element from empty optional");
    return opt->element;
}

#ifdef __cplusplus
}
#endif

#endif

#ifndef CPROTORPC_STRUCTURES
#define CPROTORPC_STRUCTURES

#include <stddef.h>
#include <assert.h>

/*
 * Generic vector of elements
 */
typedef struct sidl_vector_t
{
    size_t size;
    size_t capacity;
    void** elements;
} sidl_vector_t;

int sidl_vector_init(sidl_vector_t* vec);

/*
 * Frees inner content of the passed vector.
 */
void sidl_vector_destroy(sidl_vector_t* vec);
int sidl_vector_append(sidl_vector_t* vec, void* element);

inline static void* sidl_vector_get(sidl_vector_t* vec, size_t index)
{
    assert(index < vec->size && "Vector access out of bounds");
    return vec->elements[index];
}

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

#endif

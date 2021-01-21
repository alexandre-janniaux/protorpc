#include <stdlib.h>
#include "cprotorpc/structures.h"

#define SIDL_VECTOR_CAPACITY (10)

int sidl_vector_new(sidl_vector_t* vec)
{
    vec->size = 0;
    vec->capacity = SIDL_VECTOR_CAPACITY;
    vec->elements = calloc(vec->capacity, sizeof(void*));

    if (!vec->elements)
        return -1;

    return 0;
}

void sidl_vector_destroy(sidl_vector_t* vec)
{
    free(vec->elements);
}

int sidl_vector_append(sidl_vector_t* vec, void* element)
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

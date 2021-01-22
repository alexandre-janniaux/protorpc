#include <stdlib.h>
#include <string.h>
#include "cprotorpc/structures.h"

// Vectors
XM_SIDL_TYPES(SIDL_DECLARE_VECTOR_IMPL)
XM_SIDL_TYPES(SIDL_VEC_DESTROY_IMPL)

// Special case for the string type
SIDL_DECLARE_VECTOR_IMPL(string, char*)

void sidl_string_vector_destroy(SIDL_VSNAME(string)* v)
{
    for (size_t i = 0; i < v->size; i++)
        free(v->elements[i]);

    free(v->elements);
}

// Optionals
XM_SIDL_TYPES(SIDL_DECLARE_OPTIONAL_IMPL)

void sidl_string_optional_destroy(SIDL_OSNAME(string)* v)
{
    free(v->element);
}

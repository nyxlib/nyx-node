/*--------------------------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_number_t *nyx_number_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_number_t *object = nyx_memory_alloc(sizeof(nyx_number_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = NYX_OBJECT(NYX_TYPE_NUMBER);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->value = 0.0;

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_number_free(nyx_number_t *object)
{
    object->value = 0.0;

    nyx_memory_free(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

double nyx_number_get(const nyx_number_t *object)
{
    return object->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_number_set2(nyx_number_t *object, double value, bool propagate)
{
    if(isnan(value))
    {
        fprintf(stderr, "NaN number not allowed in `nyx_number_set`\n");
        fflush(stderr);
        return false;
    }

    bool modified = \
    object->value != value;
    object->value = value;

    nyx_object_notify(&object->base, modified, propagate);

    return modified;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_number_to_string(const nyx_number_t *object)
{
    return nyx_double_dup(object->value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

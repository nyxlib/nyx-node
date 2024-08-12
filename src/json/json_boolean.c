/*--------------------------------------------------------------------------------------------------------------------*/

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_boolean_t *nyx_boolean_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_boolean_t *object = nyx_memory_alloc(sizeof(nyx_boolean_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = NYX_OBJECT(NYX_TYPE_BOOLEAN);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->value = false;

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_boolean_free(nyx_boolean_t *object)
{
    object->value = false;

    nyx_memory_free(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_boolean_get(const nyx_boolean_t *object)
{
    return object->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_boolean_set(nyx_boolean_t *object, bool value)
{
    if(object->value != value)
    {
        object->value = value;

        nyx_object_notify(&object->base);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_boolean_to_string(const nyx_boolean_t *object)
{
    return nyx_boolean_dup(object->value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

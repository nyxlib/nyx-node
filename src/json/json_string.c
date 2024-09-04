/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_string_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_string_t *object = nyx_memory_alloc(sizeof(nyx_string_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = NYX_OBJECT(NYX_TYPE_STRING);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->value = (str_t) "";

    object->dyn = false;

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_string_free(nyx_string_t *object)
{
    if(object->dyn)
    {
        nyx_memory_free(object->value);
    }

    nyx_memory_free(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_string_get(const nyx_string_t *object)
{
    return object->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_string_dynamic_set2(nyx_string_t *object, STR_t value, bool notify)
{
    if(value == NULL)
    {
        fprintf(stderr, "Null string not allowed in `nyx_string_set`\n");
        fflush(stderr);
        return false;
    }

    if(strcmp(object->value, value) != 0 || object->dyn == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->dyn)
        {
            nyx_memory_free(object->value);
        }

        object->dyn = true;

        /*------------------------------------------------------------------------------------------------------------*/

        bool modified = strcmp(object->value, value) != 0;

        object->value = nyx_string_dup(value);

        /*------------------------------------------------------------------------------------------------------------*/

        if(notify)
        {
            nyx_object_notify(&object->base, modified);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        return modified;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_string_static_set2(nyx_string_t *object, STR_t value, bool notify)
{
    if(value == NULL)
    {
        fprintf(stderr, "Null string not allowed in `nyx_string_set`\n");
        fflush(stderr);
        return false;
    }

    if(strcmp(object->value, value) != 0 || object->dyn == true)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->dyn)
        {
            nyx_memory_free(object->value);
        }

        object->dyn = false;

        /*------------------------------------------------------------------------------------------------------------*/

        bool modified = strcmp(object->value, value) != 0;

        object->value = (/**/str_t/**/) value;

        /*------------------------------------------------------------------------------------------------------------*/

        if(notify)
        {
            nyx_object_notify(&object->base, modified);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        return modified;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_string_to_string(const nyx_string_t *object)
{
    nyx_string_builder_t *sb = nyx_string_builder_from(object->value);

    str_t result = nyx_string_builder_to_string(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_string_to_cstring(const nyx_string_t *object)
{
    nyx_string_builder_t *sb = nyx_string_builder_from(object->value);

    str_t result = nyx_string_builder_to_cstring(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

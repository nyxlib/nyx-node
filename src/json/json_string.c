/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_t *indi_string_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_string_t *object = indi_memory_alloc(sizeof(indi_string_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = INDI_OBJECT(INDI_TYPE_STRING);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->value = (str_t) "";

    object->dyn = false;

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_free(indi_string_t *object)
{
    if(object->dyn)
    {
        indi_memory_free(object->value);
    }

    indi_memory_free(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_string_get(const indi_string_t *object)
{
    return object->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_dynamic_set(indi_string_t *object, STR_t value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Null string not allowed in `indi_string_set`\n");
        fflush(stderr);
        return;
    }

    if(strcmp(object->value, value) != 0 || object->dyn == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->dyn)
        {
            indi_memory_free(object->value);
        }

        object->dyn = true;

        /*------------------------------------------------------------------------------------------------------------*/

        object->value = indi_string_dup(value);

        /*------------------------------------------------------------------------------------------------------------*/

        indi_object_notify(&object->base);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_static_set(indi_string_t *object, STR_t value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Null string not allowed in `indi_string_set`\n");
        fflush(stderr);
        return;
    }

    if(strcmp(object->value, value) != 0 || object->dyn == true)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->dyn)
        {
            indi_memory_free(object->value);
        }

        object->dyn = false;

        /*------------------------------------------------------------------------------------------------------------*/

        object->value = (/**/str_t/**/) value;

        /*------------------------------------------------------------------------------------------------------------*/

        indi_object_notify(&object->base);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_string(const indi_string_t *object)
{
    indi_string_builder_t *sb = indi_string_builder_from(object->value);

    str_t result = indi_string_builder_to_string(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_cstring(const indi_string_t *object)
{
    indi_string_builder_t *sb = indi_string_builder_from(object->value);

    str_t result = indi_string_builder_to_cstring(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

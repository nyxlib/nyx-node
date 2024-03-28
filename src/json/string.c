/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_t *indi_string_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_string_t *obj = indi_memory_alloc(sizeof(indi_string_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->base = INDI_OBJECT(INDI_TYPE_STRING);

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->value = indi_string_dup("");

    /*----------------------------------------------------------------------------------------------------------------*/

    return obj;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_free(indi_string_t *obj)
{
    indi_memory_free(obj->value);

    indi_memory_free(obj);
}

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_string_get(const indi_string_t *obj)
{
    return obj->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_set(indi_string_t *obj, STR_t value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Null string not allowed in `indi_string_set`\n");
        fflush(stderr);
        return;
    }

    if(strcmp(obj->value, value) != 0)
    {
        indi_memory_free(obj->value);

        obj->value = indi_string_dup(value);

        indi_object_notify(&obj->base);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_string(const indi_string_t *obj)
{
    indi_string_builder_t *sb = indi_string_builder_from(obj->value);

    str_t result = indi_string_builder_to_string(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_cstring(const indi_string_t *obj)
{
    indi_string_builder_t *sb = indi_string_builder_from(obj->value);

    str_t result = indi_string_builder_to_cstring(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

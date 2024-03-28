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

    obj->base.magic = INDI_OBJECT_MAGIC;
    obj->base.type = INDI_TYPE_STRING;

    obj->base.parent = NULL;
    obj->base.out_callback = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->data = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return obj;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_free(indi_string_t *obj)
{
    indi_memory_free(obj->data);

    indi_memory_free(obj);
}

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_string_get(indi_string_t *obj)
{
    return obj->data;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_set(indi_string_t *obj, STR_t data)
{
    if(data == NULL)
    {
        fprintf(stderr, "Null string not allowed in `indi_string_set`\n");
        fflush(stderr);
        return;
    }

    if(obj->data == NULL || strcmp(obj->data, data) != 0)
    {
        indi_memory_free(obj->data);

        obj->data = indi_string_dup(data);

        indi_object_notify(&obj->base);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_string(indi_string_t *obj)
{
    indi_string_builder_t *sb = indi_string_builder_from(obj->data);

    str_t result = indi_string_builder_to_string(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_cstring(indi_string_t *obj)
{
    indi_string_builder_t *sb = indi_string_builder_from(obj->data);

    str_t result = indi_string_builder_to_cstring(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

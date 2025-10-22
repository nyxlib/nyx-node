/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_string_new(void)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_string_t *object = nyx_memory_alloc(sizeof(nyx_string_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = NYX_OBJECT(NYX_TYPE_STRING);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->managed = false;
    object->length = 0x00;
    object->value = "";


    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_string_free(nyx_string_t *object)
{
    if(object->managed)
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

void nyx_string_get_buff(const nyx_string_t *object, __NYX_NULLABLE__ size_t *result_size, __NYX_NULLABLE__ buff_t *result_buff)
{
    if(result_size != NULL) {
        *result_size = object->length;
    }

    if(result_buff != NULL) {
        *result_buff = object->value;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_string_set_alt(nyx_string_t *object, STR_t value, bool manage, bool notify)
{
    if(value == NULL)
    {
        NYX_LOG_ERROR("Null string not allowed");

        return false;
    }

    bool modified = strcmp(object->value, value) != 0;

    if(modified || object->managed == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->managed)
        {
            nyx_memory_free(object->value);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        object->managed = /*--*/ manage;
        object->length = strlen(value);
        object->value = (str_t) value;

        /*------------------------------------------------------------------------------------------------------------*/

        if(notify && modified)
        {
            nyx_object_notify(&object->base);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        return modified;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_string_set_buff_alt(nyx_string_t *object, size_t size, BUFF_t buff, bool managed, bool notify)
{
    if(size == 0x00 || buff == NULL)
    {
        NYX_LOG_ERROR("Null string not allowed");

        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(object->managed)
    {
        nyx_memory_free(object->value);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    object->managed = /*---*/ managed;
    object->length = /*---*/ size;
    object->value = (str_t) buff;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(notify)
    {
        nyx_object_notify(&object->base);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_string_length(const nyx_string_t *object)
{
    return object->length;
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

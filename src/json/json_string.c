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
    object->length = 0x000000;
    object->value = (str_t) "";


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

bool nyx_string_set(nyx_string_t *object, STR_t value, bool managed)
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

        object->managed = /*--*/ managed;
        object->length = strlen(value);
        object->value = (str_t) value;

        /*------------------------------------------------------------------------------------------------------------*/

        return modified;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_string_set_buff(nyx_string_t *object, size_t size, BUFF_t buff, bool managed)
{
    if(buff == NULL)
    {
        NYX_LOG_ERROR("Null buffer not allowed");

        return false;
    }

    bool modified = object->length != size || memcmp(object->value, buff, size) != 0;

    if(modified || object->managed == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->managed)
        {
            nyx_memory_free(object->value);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        object->managed = /*---*/ managed;
        object->length = /*---*/ size;
        object->value = (str_t) buff;

        /*------------------------------------------------------------------------------------------------------------*/

        return modified;
    }

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
    nyx_string_builder_t *sb = nyx_string_builder_new();

    nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "\"");
    nyx_string_builder_append(sb, NYX_SB_ESCAPE_JSON, object->value);
    nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "\"");

    str_t result = nyx_string_builder_to_string(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_string_to_cstring(const nyx_string_t *object)
{
    nyx_string_builder_t *sb = nyx_string_builder_new();

    ///_string_builder_append(sb, NYX_SB_NO_ESCAPE, "\"");
    nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, object->value);
    ///_string_builder_append(sb, NYX_SB_NO_ESCAPE, "\"");

    str_t result = nyx_string_builder_to_string(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

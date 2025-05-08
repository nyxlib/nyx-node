/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

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

    object->length = 0x00000000;
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

void nyx_string_get_buff(const nyx_string_t *object, __NULLABLE__ size_t *result_size, __NULLABLE__ buff_t *result_buff, bool base64_decode)
{
    if(base64_decode)
    {
        buff_t buff = nyx_base64_decode(result_size, object->length, object->value);

        if(result_buff == NULL) {
            nyx_memory_free(buff);
        }
        else {
            *result_buff = buff;
        }
    }
    else
    {
        if(result_size) {
            *result_size = object->length;
        }

        if(result_buff) {
            *result_buff = object->value;
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_string_set_dup_alt(nyx_string_t *object, STR_t value, bool notify)
{
    if(value == NULL)
    {
        NYX_LOG_ERROR("Null string not allowed");

        return false;
    }

    bool modified = strcmp(object->value, value) != 0;

    if(modified || object->dyn == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->dyn)
        {
            nyx_memory_free(object->value);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        object->dyn = true;
        object->length = strlen(value);
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

bool nyx_string_set_ref_alt(nyx_string_t *object, STR_t value, bool notify)
{
    if(value == NULL)
    {
        NYX_LOG_ERROR("Null string not allowed");

        return false;
    }

    bool modified = strcmp(object->value, value) != 0;

    if(modified || object->dyn == true)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(object->dyn)
        {
            nyx_memory_free(object->value);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        object->dyn = false;
        object->length = strlen(value);
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

bool nyx_string_set_buff_alt(nyx_string_t *object, size_t size, BUFF_t buff, bool base64_encode, bool notify)
{
    if(size == 0x00 || buff == NULL)
    {
        NYX_LOG_ERROR("Null string not allowed");

        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(object->dyn)
    {
        nyx_memory_free(object->value);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(base64_encode)
    {
        object->dyn = true;

        object->value = nyx_base64_encode(&object->length, size, buff);
    }
    else
    {
        object->dyn = false;
        object->length = size;
        object->value = (str_t) buff;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(notify)
    {
        nyx_object_notify(&object->base, true);
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

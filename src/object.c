/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef HAVE_MALLOC_SIZE
size_t malloc_size(buff_t);
#endif

#ifdef HAVE_MALLOC_USABLE_SIZE
size_t malloc_usable_size(buff_t);
#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#if defined(HAVE_MALLOC_SIZE) || defined(HAVE_MALLOC_USABLE_SIZE)
static size_t used_mem = 0;
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_memory_initialize()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    #if defined(HAVE_MALLOC_SIZE) || defined(HAVE_MALLOC_USABLE_SIZE)
    used_mem = 0;
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_memory_finalize()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    #if defined(HAVE_MALLOC_SIZE) || defined(HAVE_MALLOC_USABLE_SIZE)
    if(__atomic_load_n(&used_mem, __ATOMIC_SEQ_CST) > 0)
    {
        NYX_LOG_ERROR("Memory leak: %ld bytes", used_mem);

        return false;
    }
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_memory_free(__NULLABLE__ buff_t buff)
{
    if(buff == NULL)
    {
        return 0x00;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_SIZE
    size_t result = malloc_size(buff);

    __atomic_fetch_sub(&used_mem, result, __ATOMIC_SEQ_CST);
    #endif

    #ifdef HAVE_MALLOC_USABLE_SIZE
    size_t result = malloc_usable_size(buff);

    __atomic_fetch_sub(&used_mem, result, __ATOMIC_SEQ_CST);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    free(buff);

    /*----------------------------------------------------------------------------------------------------------------*/

    #if defined(HAVE_MALLOC_SIZE) || defined(HAVE_MALLOC_USABLE_SIZE)
    return result;
    #else
    return 0x0000;
    #endif
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_memory_alloc(__ZEROABLE__ size_t size)
{
    if(size == 0x00)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    buff_t result = malloc(size);

    if(result == NULL)
    {
        NYX_LOG_FATAL("Out of memory");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_SIZE
    __atomic_fetch_add(&used_mem, malloc_size(result), __ATOMIC_SEQ_CST);
    #endif

    #ifdef HAVE_MALLOC_USABLE_SIZE
    __atomic_fetch_add(&used_mem, malloc_usable_size(result), __ATOMIC_SEQ_CST);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_memory_realloc(__NULLABLE__ buff_t buff, __ZEROABLE__ size_t size)
{
    if(buff == NULL) {
        return nyx_memory_alloc(size);
    }

    if(size == 0x00) {
        nyx_memory_free(buff); return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_SIZE
    __atomic_fetch_sub(&used_mem, malloc_size(buff), __ATOMIC_SEQ_CST);
    #endif

    #ifdef HAVE_MALLOC_USABLE_SIZE
    __atomic_fetch_sub(&used_mem, malloc_usable_size(buff), __ATOMIC_SEQ_CST);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    buff_t result = realloc(buff, size);

    if(result == NULL)
    {
        NYX_LOG_FATAL("Out of memory");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_SIZE
    __atomic_fetch_add(&used_mem, malloc_size(result), __ATOMIC_SEQ_CST);
    #endif

    #ifdef HAVE_MALLOC_USABLE_SIZE
    __atomic_fetch_add(&used_mem, malloc_usable_size(result), __ATOMIC_SEQ_CST);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* UTILITIES                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_boolean_dup(bool b)
{
    str_t str;

    if(b) {
        str = strcpy(nyx_memory_alloc(4 + 1), "true");
    }
    else {
        str = strcpy(nyx_memory_alloc(5 + 1), "false");
    }

    return str;
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__ str_t nyx_double_dup(double d)
{
    if(!isnan(d))
    {
        str_t str = nyx_memory_alloc(32 + 1);

        snprintf(str, 32, "%lf", d);

        return str;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__ str_t nyx_string_dup(__NULLABLE__ STR_t s)
{
    if(s != NULL)
    {
        str_t str = nyx_memory_alloc(strlen(s) + 1);

        strcpy(str, s);

        return str;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_str_t nyx_str_s(__NULLABLE__ STR_t s)
{
    nyx_str_t str = {(char *) s, s == NULL ? 0x0000000 : strlen(s)};

    return str;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_object_free(__NULLABLE__ nyx_object_t *object)
{
    if(object == NULL)
    {
        return;
    }

    if(object->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    switch(object->type)
    {
        case NYX_TYPE_NULL:
            nyx_null_free((nyx_null_t *) object);
            break;

        case NYX_TYPE_NUMBER:
            nyx_number_free((nyx_number_t *) object);
            break;

        case NYX_TYPE_BOOLEAN:
            nyx_boolean_free((nyx_boolean_t *) object);
            break;

        case NYX_TYPE_STRING:
            nyx_string_free((nyx_string_t *) object);
            break;

        case NYX_TYPE_LIST:
            nyx_list_free((nyx_list_t *) object);
            break;

        case NYX_TYPE_DICT:
            nyx_dict_free((nyx_dict_t *) object);
            break;

        default:
            NYX_LOG_FATAL("Internal error");
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_object_equal(__NULLABLE__ const nyx_object_t *object1, __NULLABLE__ const nyx_object_t *object2)
{
    if(object1 == NULL || object2 == NULL)
    {
        return false;
    }

    if(object1->magic != NYX_OBJECT_MAGIC || object2->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    if(object1->type != object2->type)
    {
        return false;
    }

    switch(object1->type)
    {
        case NYX_TYPE_NULL:
            return true;

        case NYX_TYPE_NUMBER:
            return ((nyx_number_t *) object1)->value == ((nyx_number_t *) object2)->value;

        case NYX_TYPE_BOOLEAN:
            return ((nyx_boolean_t *) object1)->value == ((nyx_boolean_t *) object2)->value;

        case NYX_TYPE_STRING:
            return strcmp(((nyx_string_t *) object1)->value, ((nyx_string_t *) object2)->value) == 0;

        case NYX_TYPE_LIST:
        case NYX_TYPE_DICT:
            return object1 == object2;

        default:
            NYX_LOG_FATAL("Internal error");
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_object_to_string(__NULLABLE__ const nyx_object_t *object)
{
    if(object == NULL)
    {
        return NULL;
    }

    if(object->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    switch(object->type)
    {
        case NYX_TYPE_NULL:
            return nyx_null_to_string((nyx_null_t *) object);

        case NYX_TYPE_NUMBER:
            return nyx_number_to_string((nyx_number_t *) object);

        case NYX_TYPE_BOOLEAN:
            return nyx_boolean_to_string((nyx_boolean_t *) object);

        case NYX_TYPE_STRING:
            return nyx_string_to_string((nyx_string_t *) object);

        case NYX_TYPE_LIST:
            return nyx_list_to_string((nyx_list_t *) object);

        case NYX_TYPE_DICT:
            return nyx_dict_to_string((nyx_dict_t *) object);

        default:
            NYX_LOG_FATAL("Internal error");
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_object_to_cstring(__NULLABLE__ const nyx_object_t *object)
{
    if(object == NULL)
    {
        return NULL;
    }

    if(object->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    switch(object->type)
    {
        case NYX_TYPE_NULL:
            return nyx_null_to_string((nyx_null_t *) object);

        case NYX_TYPE_NUMBER:
            return nyx_number_to_string((nyx_number_t *) object);

        case NYX_TYPE_BOOLEAN:
            return nyx_boolean_to_string((nyx_boolean_t *) object);

        case NYX_TYPE_STRING:
            return nyx_string_to_cstring((nyx_string_t *) object);

        case NYX_TYPE_LIST:
            return nyx_list_to_string((nyx_list_t *) object);

        case NYX_TYPE_DICT:
            return nyx_dict_to_string((nyx_dict_t *) object);

        default:
            NYX_LOG_FATAL("Internal error");
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_object_notify(__NULLABLE__ nyx_object_t *object, bool modified)
{
    for(; object != NULL; object = object->parent)
    {
        if(object->out_callback != NULL)
        {
            object->out_callback(
                object,
                modified
            );
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

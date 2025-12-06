/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_MALLOC_SIZE
#  include <stdatomic.h>

size_t malloc_size(void *);
#endif

#ifdef HAVE_MALLOC_USABLE_SIZE
#  include <stdatomic.h>

size_t malloc_usable_size(void *);
#endif

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#if defined(HAVE_MALLOC_SIZE) || defined(HAVE_MALLOC_USABLE_SIZE)
static unsigned long used_mem = 0UL;
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_memory_initialize(void)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    #if defined(HAVE_MALLOC_SIZE) || defined(HAVE_MALLOC_USABLE_SIZE)
    atomic_store_explicit(&used_mem, 0UL, memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_memory_finalize(void)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    #if defined(HAVE_MALLOC_SIZE) || defined(HAVE_MALLOC_USABLE_SIZE)
    unsigned long leaks = atomic_exchange_explicit(&used_mem, 0UL, memory_order_relaxed);

    if(leaks > 0UL)
    {
        NYX_LOG_ERROR("Memory leak: %lu bytes", leaks);

        return false;
    }
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_memory_free(buff_t buff)
{
    if(buff == NULL)
    {
        return 0x00;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_SIZE
    size_t result = malloc_size(buff);

    atomic_fetch_sub_explicit(&used_mem, result, memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_USABLE_SIZE
    size_t result = malloc_usable_size(buff);

    atomic_fetch_sub_explicit(&used_mem, result, memory_order_relaxed);
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

buff_t nyx_memory_alloc(size_t size)
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
    atomic_fetch_add_explicit(&used_mem, malloc_size(result), memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_USABLE_SIZE
    atomic_fetch_add_explicit(&used_mem, malloc_usable_size(result), memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_memory_realloc(buff_t buff, size_t size)
{
    if(buff == NULL) {
        return nyx_memory_alloc(size);
    }

    if(size == 0x00) {
        nyx_memory_free(buff); return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_SIZE
    atomic_fetch_sub_explicit(&used_mem, malloc_size(buff), memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_USABLE_SIZE
    atomic_fetch_sub_explicit(&used_mem, malloc_usable_size(buff), memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    buff_t result = realloc(buff, size);

    if(result == NULL)
    {
        NYX_LOG_FATAL("Out of memory");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_SIZE
    atomic_fetch_add_explicit(&used_mem, malloc_size(result), memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef HAVE_MALLOC_USABLE_SIZE
    atomic_fetch_add_explicit(&used_mem, malloc_usable_size(result), memory_order_relaxed);
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* UTILITIES                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_bool_dup(bool b)
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

str_t nyx_double_dup(double d)
{
    if(!isnan(d))
    {
        str_t str = nyx_memory_alloc(32 + 1);

        snprintf(str, 32 + 1, "%f", d);

        return str;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_string_dup(STR_t s)
{
    if(s != NULL)
    {
        size_t len = strlen(s);

        str_t str = nyx_memory_alloc(len + 1);

        memcpy(str, s, len);

        str[len] = '\0';

        return str;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_string_ndup(STR_t s, size_t n)
{
    if(s != NULL)
    {
        size_t len = strnlen(s, n);

        str_t str = nyx_memory_alloc(len + 1);

        memcpy(str, s, len);

        str[len] = '\0';

        return str;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_str_t nyx_str_s(STR_t s)
{
    nyx_str_t str = {(str_t) /* NOSONAR */ s, s == NULL ? 0x0000000 : strlen(s)};

    return str;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_object_free(nyx_object_t *object)
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

bool nyx_object_equal(const nyx_object_t *object1, const nyx_object_t *object2)
{
    if(object1 == NULL || object2 == NULL)
    {
        return false;
    }

    if(object1->magic != NYX_OBJECT_MAGIC || object2->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(object1 == object2)
    {
        return true;
    }

    if(object1->type != object2->type)
    {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    switch(object1->type)
    {
        case NYX_TYPE_NULL:
            return true;

        case NYX_TYPE_NUMBER:
            return ((const nyx_number_t *) object1)->value == ((const nyx_number_t *) object2)->value;

        case NYX_TYPE_BOOLEAN:
            return ((const nyx_boolean_t *) object1)->value == ((const nyx_boolean_t *) object2)->value;

        case NYX_TYPE_STRING:
            return strcmp(((const nyx_string_t *) object1)->value, ((const nyx_string_t *) object2)->value) == 0;

        case NYX_TYPE_LIST:
        case NYX_TYPE_DICT:
            {
                str_t s1 = nyx_object_to_string(object1);
                str_t s2 = nyx_object_to_string(object2);

                bool equal = strcmp(s1, s2) == 0;

                nyx_memory_free(s1);
                nyx_memory_free(s2);

                return equal;
            }

        default:
            NYX_LOG_FATAL("Internal error");
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_object_to_string(const nyx_object_t *object)
{
    if(object == NULL)
    {
        return NULL;
    }

    if(object->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    switch(object->type)
    {
        case NYX_TYPE_NULL:
            return nyx_null_to_string((const nyx_null_t *) object);

        case NYX_TYPE_NUMBER:
            return nyx_number_to_string((const nyx_number_t *) object);

        case NYX_TYPE_BOOLEAN:
            return nyx_boolean_to_string((const nyx_boolean_t *) object);

        case NYX_TYPE_STRING:
            return nyx_string_to_string((const nyx_string_t *) object);

        case NYX_TYPE_LIST:
            return nyx_list_to_string((const nyx_list_t *) object);

        case NYX_TYPE_DICT:
            return nyx_dict_to_string((const nyx_dict_t *) object);

        default:
            NYX_LOG_FATAL("Internal error");
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_object_to_cstring(const nyx_object_t *object)
{
    if(object == NULL)
    {
        return NULL;
    }

    if(object->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    switch(object->type)
    {
        case NYX_TYPE_NULL:
            return nyx_null_to_string((const nyx_null_t *) object);

        case NYX_TYPE_NUMBER:
            return nyx_number_to_string((const nyx_number_t *) object);

        case NYX_TYPE_BOOLEAN:
            return nyx_boolean_to_string((const nyx_boolean_t *) object);

        case NYX_TYPE_STRING:
            return nyx_string_to_cstring((const nyx_string_t *) object);

        case NYX_TYPE_LIST:
            return nyx_list_to_string((const nyx_list_t *) object);

        case NYX_TYPE_DICT:
            return nyx_dict_to_string((const nyx_dict_t *) object);

        default:
            NYX_LOG_FATAL("Internal error");
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

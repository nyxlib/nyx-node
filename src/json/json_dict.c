/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_dict_node_s
{
    STR_t key;

    nyx_object_t *value;

    struct nyx_dict_node_s *next;

} node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

static void internal_dict_clear(
    nyx_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_dict_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *object = nyx_memory_alloc(sizeof(nyx_dict_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = NYX_OBJECT(NYX_TYPE_DICT);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->head = NULL;
    object->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_dict_free(nyx_dict_t *object)
{
    internal_dict_clear(object);

    nyx_memory_free(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void internal_dict_clear(nyx_dict_t *object)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = object->head; node != NULL;)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        node_t *temp = node;

        node = node->next;

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_object_free(temp->value);

        nyx_memory_free(temp);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    object->head = NULL;
    object->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_dict_clear(nyx_dict_t *object)
{
    internal_dict_clear(object);

    nyx_object_notify(&object->base, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_dict_del(nyx_dict_t *object, STR_t key)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *prev_node = NULL, *curr_node = object->head; curr_node != NULL; prev_node = curr_node, curr_node = curr_node->next)
    {
        if(strcmp(curr_node->key, key) == 0)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            if(prev_node == NULL)
            {
                object->head = curr_node->next;
            }
            else
            {
                prev_node->next = curr_node->next;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_object_free(curr_node->value);

            nyx_memory_free(curr_node);

            /*--------------------------------------------------------------------------------------------------------*/

            break;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_dict_iterate(nyx_dict_iter_t *iter, STR_t *key, nyx_object_t **object)
{
    if(iter->type == NYX_TYPE_DICT && iter->head != NULL)
    {
        if(key != NULL) {
            *key = iter->head->key;
        }

        if(object != NULL) {
            *object = iter->head->value;
        }

        iter->idx += 0x0000000000001;
        iter->head = iter->head->next;

        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_object_t *nyx_dict_get(const nyx_dict_t *object, STR_t key)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    {
        if(strcmp(curr_node->key, key) == 0)
        {
            return curr_node->value;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_dict_set_alt(nyx_dict_t *object, STR_t key, buff_t value, bool notify)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(((nyx_object_t *) value)->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    ((nyx_object_t *) value)->parent = (nyx_object_t *) object;

    /*----------------------------------------------------------------------------------------------------------------*/

    bool modified = true;

    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    {
        if(strcmp(curr_node->key, key) == 0)
        {
            modified = !nyx_object_equal(curr_node->value, value);

            nyx_object_free(curr_node->value);

            curr_node->value = value;

            goto _ok;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node_t *node = nyx_memory_alloc(sizeof(node_t) + strlen(key) + 1);

    node->key = strcpy((str_t) (node + 1), key);

    node->value = value;
    node->next = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(object->head == NULL)
    {
        object->head = node;
        object->tail = node;
    }
    else
    {
        object->tail->next = node;
        object->tail /*-*/ = node;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_ok:
    if(notify)
    {
        nyx_object_notify(&object->base, modified);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return modified;
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_dict_size(const nyx_dict_t *object)
{
    size_t result = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = object->head; node != NULL; node = node->next, result++);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_dict_to_string(const nyx_dict_t *object)
{
    nyx_string_builder_t *sb = nyx_string_builder_new();

    /**/    nyx_string_builder_append(sb, "{");
    /**/
    /**/    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    /**/    {
    /**/        str_t curr_node_val = nyx_object_to_string(curr_node->value);
    /**/
    /**/        /**/    nyx_string_builder_append(sb, "\"", curr_node->key, "\"", ":", curr_node_val);
    /**/
    /**/        nyx_memory_free(curr_node_val);
    /**/
    /**/        if(curr_node->next != NULL)
    /**/        {
    /**/            nyx_string_builder_append(sb, ",");
    /**/        }
    /**/    }
    /**/
    /**/    nyx_string_builder_append(sb, "}");

    str_t result = nyx_string_builder_to_cstring(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

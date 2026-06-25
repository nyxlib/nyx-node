/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_list_node_s
{
    nyx_object_t *value;

    struct nyx_list_node_s *next;

} node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

static void internal_list_clear(
    nyx_list_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_list_t *nyx_list_new(void)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_list_t *object = nyx_memory_alloc(sizeof(nyx_list_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = NYX_OBJECT(NYX_TYPE_LIST);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->head = NULL;
    object->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_list_free(nyx_list_t *object)
{
    internal_list_clear(object);

    nyx_memory_free(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void internal_list_clear(nyx_list_t *object)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = object->head; node != NULL;)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        node_t *temp = node;

        node = node->next;

        /*------------------------------------------------------------------------------------------------------------*/

        temp->value->parent = NULL;

        nyx_object_unref(temp->value);

        nyx_memory_free(temp);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    object->head = NULL;
    object->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_list_clear(nyx_list_t *object)
{
    internal_list_clear(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_list_del(nyx_list_t *object, size_t idx)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *prev_node = NULL, *curr_node = object->head; curr_node != NULL; prev_node = curr_node, curr_node = curr_node->next, idx--)
    {
        if(idx == 0)
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

            if(curr_node == object->tail)
            {
                object->tail = prev_node;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            curr_node->value->parent = NULL;

            nyx_object_unref(curr_node->value);

            nyx_memory_free(curr_node);

            /*--------------------------------------------------------------------------------------------------------*/

            break;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_list_iterate(nyx_list_iter_t *iter, size_t *idx, nyx_object_t **object)
{
    if(iter->head != NULL)
    {
        if(idx != NULL) {
            *idx = iter->idx;
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

nyx_object_t *nyx_list_get(const nyx_list_t *object, size_t idx)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next, idx--)
    {
        if(idx == 0)
        {
            return curr_node->value;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_list_set(nyx_list_t *object, size_t idx, void *value)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(((nyx_object_t *) value)->magic != NYX_OBJECT_MAGIC)
    {
        NYX_LOG_FATAL("Invalid object");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(((nyx_object_t *) value)->parent != NULL)
    {
        NYX_LOG_ERROR("Object already has a parent");

        return false;
    }

    for(nyx_object_t *parent = (nyx_object_t *) object; parent != NULL; parent = parent->parent)
    {
        if(parent == (nyx_object_t *) value)
        {
            NYX_LOG_ERROR("An object cannot be its own parent");

            return false;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool modified = true;

    for(node_t *curr_node = /* NOSONAR */ object->head; curr_node != NULL; curr_node = curr_node->next, idx--)
    {
        if(idx == 0)
        {
            modified = !nyx_object_equal(curr_node->value, value);

            curr_node->value->parent = NULL;

            nyx_object_ref(/*-*/ value /*-*/);
            nyx_object_unref(curr_node->value);

            curr_node->value = value;

            goto _ok;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node_t *node = nyx_memory_alloc(sizeof(node_t));

    nyx_object_ref(value);

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
    ((nyx_object_t *) value)->parent = (nyx_object_t *) object;

    return modified;
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_list_size(const nyx_list_t *object)
{
    size_t result = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = object->head; node != NULL; node = node->next, result++) { /* NOSONAR */ };

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_list_to_string(const nyx_list_t *object)
{
    nyx_string_builder_t *sb = nyx_string_builder_new();

    /**/    nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "[");
    /**/
    /**/    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    /**/    {
    /**/        /*----------------------------------------------------------------------------------------------------*/
    /**/
    /**/        str_t curr_node_val = nyx_object_to_string(curr_node->value);
    /**/
    /**/        nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, curr_node_val);
    /**/
    /**/        nyx_memory_free(curr_node_val);
    /**/
    /**/        /*----------------------------------------------------------------------------------------------------*/
    /**/
    /**/        if(curr_node->next != NULL)
    /**/        {
    /**/            nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, ",");
    /**/        }
    /**/        /*----------------------------------------------------------------------------------------------------*/
    /**/    }
    /**/
    /**/    nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "]");

    str_t result = nyx_string_builder_to_string(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

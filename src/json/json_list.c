/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

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

nyx_list_t *nyx_list_new()
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

void nyx_list_clear(nyx_list_t *object)
{
    internal_list_clear(object);

    nyx_object_notify(&object->base, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_list_del(nyx_list_t *object, int idx)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(idx >= 0) for(node_t *prev_node = NULL, *curr_node = object->head; curr_node != NULL; prev_node = curr_node, curr_node = curr_node->next, idx--)
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

            nyx_object_free(curr_node->value);

            nyx_memory_free(curr_node);

            /*--------------------------------------------------------------------------------------------------------*/

            break;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_list_iterate(nyx_list_iter_t *iter, int *idx, nyx_object_t **object)
{
    if(iter->type == NYX_TYPE_LIST && iter->head != NULL)
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

nyx_object_t *nyx_list_get(const nyx_list_t *object, int idx)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(idx >= 0) for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next, idx--)
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

nyx_list_t *nyx_list_set2(nyx_list_t *object, size_t idx, buff_t value, bool notify)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(((nyx_object_t *) value)->magic != NYX_OBJECT_MAGIC)
    {
        fprintf(stderr, "Invalid object in `nyx_list_set`\n");
        fflush(stderr);
        exit(1);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    ((nyx_object_t *) value)->parent = (nyx_object_t *) object;

    /*----------------------------------------------------------------------------------------------------------------*/

    bool modified = true;

    if(idx >= 0) for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next, idx--)
    {
        if(idx == 0)
        {
            modified = nyx_object_compare(curr_node->value, value);

            nyx_object_free(curr_node->value);

            curr_node->value = value;

            goto _ok;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node_t *node = nyx_memory_alloc(sizeof(node_t));

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

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_list_size(const nyx_list_t *object)
{
    size_t result = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = object->head; node != NULL; node = node->next, result++);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_list_to_string(const nyx_list_t *object)
{
    nyx_string_builder_t *sb = nyx_string_builder_new();

    /**/    nyx_string_builder_append(sb, "[");
    /**/
    /**/    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    /**/    {
    /**/        str_t curr_node_val = nyx_object_to_string(curr_node->value);
    /**/
    /**/        /**/    nyx_string_builder_append(sb, curr_node_val);
    /**/
    /**/        nyx_memory_free(curr_node_val);
    /**/
    /**/        if(curr_node->next != NULL)
    /**/        {
    /**/            nyx_string_builder_append(sb, ",");
    /**/        }
    /**/    }
    /**/
    /**/    nyx_string_builder_append(sb, "]");

    str_t result = nyx_string_builder_to_cstring(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

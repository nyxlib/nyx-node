/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct indi_dict_node_s
{
    STR_t key;

    indi_object_t *val;

    struct indi_dict_node_s *next;

} node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

static void internal_dict_clear(
    indi_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_dict_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *object = indi_memory_alloc(sizeof(indi_dict_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    object->base = INDI_OBJECT(INDI_TYPE_DICT);

    /*----------------------------------------------------------------------------------------------------------------*/

    object->head = NULL;
    object->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_dict_free(indi_dict_t *object)
{
    internal_dict_clear(object);

    indi_memory_free(object);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void internal_dict_clear(indi_dict_t *object)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = object->head; node != NULL;)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        node_t *temp = node;

        node = node->next;

        /*------------------------------------------------------------------------------------------------------------*/

        indi_object_free(temp->val);

        indi_memory_free(temp);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    object->head = NULL;
    object->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_dict_clear(indi_dict_t *object)
{
    internal_dict_clear(object);

    indi_object_notify(&object->base);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_dict_del(indi_dict_t *object, STR_t key)
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

            indi_object_free(curr_node->val);

            indi_memory_free(curr_node);

            /*--------------------------------------------------------------------------------------------------------*/

            break;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool indi_dict_iterate(indi_dict_iter_t *iter, STR_t *key, indi_object_t **object)
{
    if(iter->type == INDI_TYPE_DICT && iter->head != NULL)
    {
        if(key != NULL) {
            *key = iter->head->key;
        }

        if(object != NULL) {
            *object = iter->head->val;
        }

        iter->idx += 0x0000000000001;
        iter->head = iter->head->next;

        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_dict_get(const indi_dict_t *object, STR_t key)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    {
        if(strcmp(curr_node->key, key) == 0)
        {
            return curr_node->val;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_dict_set(indi_dict_t *object, STR_t key, buff_t value)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(((indi_object_t *) value)->magic != INDI_OBJECT_MAGIC)
    {
        fprintf(stderr, "Invalid object in `indi_dict_set`\n");
        fflush(stderr);
        exit(1);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    ((indi_object_t *) value)->parent = (indi_object_t *) object;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    {
        if(strcmp(curr_node->key, key) == 0)
        {
            indi_object_free(curr_node->val);

            curr_node->val = value;

            goto _ok;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node_t *node = indi_memory_alloc(sizeof(node_t) + strlen(key) + 1);

    node->key = strcpy((str_t) (node + 1), key);

    node->val = value;
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
    indi_object_notify((indi_object_t *) value);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t indi_dict_size(const indi_dict_t *object)
{
    size_t result = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = object->head; node != NULL; node = node->next, result++);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_dict_to_string(const indi_dict_t *object)
{
    indi_string_builder_t *sb = indi_string_builder_new();

    /**/    indi_string_builder_append(sb, "{");
    /**/
    /**/    for(node_t *curr_node = object->head; curr_node != NULL; curr_node = curr_node->next)
    /**/    {
    /**/        str_t curr_node_val = indi_object_to_string(curr_node->val);
    /**/
    /**/        /**/    indi_string_builder_append(sb, "\"", curr_node->key, "\"", ":", curr_node_val);
    /**/
    /**/        indi_memory_free(curr_node_val);
    /**/
    /**/        if(curr_node->next != NULL)
    /**/        {
    /**/            indi_string_builder_append(sb, ",");
    /**/        }
    /**/    }
    /**/
    /**/    indi_string_builder_append(sb, "}");

    str_t result = indi_string_builder_to_cstring(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

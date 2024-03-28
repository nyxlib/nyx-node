/*--------------------------------------------------------------------------------------------------------------------*/

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct indi_list_node_s
{
    indi_object_t *val;

    struct indi_list_node_s *next;

} node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

static void indi_list_clear_internal(indi_list_t *obj);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_list_t *indi_list_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_list_t *obj = indi_memory_alloc(sizeof(indi_list_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->base.magic = INDI_OBJECT_MAGIC;
    obj->base.type = INDI_TYPE_LIST;

    obj->base.parent = NULL;
    obj->base.callback = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->head = NULL;
    obj->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return obj;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_list_free(indi_list_t *obj)
{
    indi_list_clear_internal(obj);

    indi_memory_free(obj);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void indi_list_clear_internal(indi_list_t *obj)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = obj->head; node != NULL;)
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

    obj->head = NULL;
    obj->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_list_clear(indi_list_t *obj)
{
    indi_list_clear_internal(obj);

    indi_object_notify(&obj->base);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_list_del(indi_list_t *obj, int idx)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(idx >= 0) for(node_t *prev_node = NULL, *curr_node = obj->head; curr_node != NULL; prev_node = curr_node, curr_node = curr_node->next, idx--)
    {
        if(idx == 0)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            if(prev_node == NULL)
            {
                obj->head = curr_node->next;
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

bool indi_list_iterate(indi_list_iter_t *iter, int *idx, indi_object_t **obj)
{
    if(iter->type == INDI_TYPE_LIST && iter->head != NULL)
    {
        if(idx != NULL) {
            *idx = iter->idx;
        }

        if(obj != NULL) {
            *obj = iter->head->val;
        }

        iter->idx += 0x0000000000001;
        iter->head = iter->head->next;

        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_list_get(indi_list_t *obj, int idx)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(idx >= 0) for(node_t *curr_node = obj->head; curr_node != NULL; curr_node = curr_node->next, idx--)
    {
        if(idx == 0)
        {
            return curr_node->val;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_list_t *indi_list_set(indi_list_t *obj, size_t idx, buff_t val)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    ((indi_object_t *) val)->parent = (indi_object_t *) obj;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(idx >= 0) for(node_t *curr_node = obj->head; curr_node != NULL; curr_node = curr_node->next, idx--)
    {
        if(idx == 0)
        {
            indi_object_free(curr_node->val);

            curr_node->val = val;

            goto _ok;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node_t *node = indi_memory_alloc(sizeof(node_t));

    node->val = val;
    node->next = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(obj->head == NULL)
    {
        obj->head = node;
        obj->tail = node;
    }
    else
    {
        obj->tail->next = node;
        obj->tail /*-*/ = node;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_ok:
    indi_object_notify((indi_object_t *) val);

    /*----------------------------------------------------------------------------------------------------------------*/

    return obj;
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t indi_list_size(indi_list_t *obj)
{
    size_t result = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = obj->head; node != NULL; node = node->next, result++);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_list_to_string(indi_list_t *obj)
{
    indi_string_builder_t *sb = indi_string_builder_new();

    /**/    indi_string_builder_append(sb, "[");
    /**/
    /**/    for(node_t *curr_node = obj->head; curr_node != NULL; curr_node = curr_node->next)
    /**/    {
    /**/        str_t curr_node_val = indi_object_to_string(curr_node->val);
    /**/
    /**/        /**/    indi_string_builder_append(sb, curr_node_val);
    /**/
    /**/        indi_memory_free(curr_node_val);
    /**/
    /**/        if(curr_node->next != NULL)
    /**/        {
    /**/            indi_string_builder_append(sb, ",");
    /**/        }
    /**/    }
    /**/
    /**/    indi_string_builder_append(sb, "]");

    str_t result = indi_string_builder_to_cstring(sb);

    indi_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

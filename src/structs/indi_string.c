/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct indi_string_node_s
{
    struct indi_string_node_s *next;

} node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_t *indi_string_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_string_t *obj = indi_memory_alloc(sizeof(indi_string_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->base.magic = INDI_OBJECT_MAGIC;
    obj->base.type = INDI_TYPE_STRING;

    obj->base.parent = NULL;
    obj->base.callback = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->head = NULL;
    obj->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return obj;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_free(indi_string_t *obj)
{
    indi_string_clear(obj);

    indi_memory_free(obj);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_clear(indi_string_t *obj)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = obj->head; node != NULL;)
    {
        node_t *temp = node;

        node = node->next;

        /**/

        indi_memory_free(temp);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->head = NULL;
    obj->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_object_notify(&obj->base);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_append(indi_string_t *obj, STR_t data)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    node_t *node;

    if(data == NULL)
    {
        node = indi_memory_alloc(sizeof(node_t) + 0x00000000000006 + 1);

        strcpy((str_t) (node + 1), "(null)");
    }
    else
    {
        node = indi_memory_alloc(sizeof(node_t) + strlen(data) + 1);

        strcpy((str_t) (node + 1), data);
    }

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
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_notify(indi_string_t *obj)
{
    indi_object_notify(&obj->base);
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t indi_string_length(indi_string_t *obj)
{
    size_t length = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = obj->head; node != NULL; node = node->next)
    {
        length += strlen((str_t) (node + 1));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return length;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static str_t to_string(indi_string_t *obj, bool json_string)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t length = indi_string_length(obj);

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t result = indi_memory_alloc(length + 3);

    str_t p = result;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(json_string) *p++ = '"';

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = obj->head; node != NULL; node = node->next)
    {
        size_t size = strlen((str_t) (node + 1));

        memcpy(p, (str_t) (node + 1), size);

        p += size;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(json_string) *p++ = '"';

    /*----------------------------------------------------------------------------------------------------------------*/

    *p = '\0';

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_string(indi_string_t *obj)
{
    return to_string(obj, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_to_cstring(indi_string_t *obj)
{
    return to_string(obj, false);
}

/*--------------------------------------------------------------------------------------------------------------------*/

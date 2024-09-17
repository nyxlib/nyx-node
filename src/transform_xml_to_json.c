/*--------------------------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_object_t *transform(const nyx_xmldoc_t *curr_node) // NOLINT(misc-no-recursion)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from((str_t) curr_node->name));

    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_xmldoc_t *new_node = curr_node->children; new_node != NULL; new_node = new_node->next)
    {
        if(new_node->type == NYX_XML_TEXT)
        {
            STR_t content_s = (STR_t) new_node->data - 0x0000;
            size_t length = strlen((str_t) content_s);
            str_t content_e = (str_t) new_node->data + length;

            while((isspace(*(content_s + 0)) || *(content_s + 0) == '"') && length > 0) {
                content_s++;
                length--;
            }

            while((isspace(*(content_e - 1)) || *(content_e - 1) == '"') && length > 0) {
                content_e--;
                length--;
            }

            if(length > 0)
            {
                *content_e = '\0';

                nyx_dict_set(result, "$", nyx_string_from((str_t) content_s));
            }

            break;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_xmldoc_t *attribute = curr_node->attributes; attribute != NULL; attribute = attribute->next)
    {
        nyx_string_builder_t *sb = nyx_string_builder_from("@", (str_t) attribute->name);

        /**/    str_t attribute_name = nyx_string_builder_to_cstring(sb);
        /**/
        /**/    /**/
        /**/    /**/    nyx_dict_set(result, (str_t) attribute_name, nyx_string_from(attribute->data));
        /**/    /**/
        /**/
        /**/    nyx_memory_free(attribute_name);

        nyx_string_builder_free(sb);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(curr_node->children)
    {
        nyx_list_t *list = NULL;

        for(nyx_xmldoc_t *new_node = curr_node->children; new_node != NULL; new_node = new_node->next)
        {
            if(new_node->type == NYX_XML_ELEM)
            {
                if(list == NULL)
                {
                    nyx_dict_set(result, "children", list = nyx_list_new());
                }

                nyx_list_push(list, transform(new_node));
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return (nyx_object_t *) result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_object_t *nyx_xmldoc_to_object(__NULLABLE__ const nyx_xmldoc_t *xmldoc, bool validate)
{
    if(xmldoc == NULL)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(validate && nyx_validation_check(xmldoc) == false)
    {
        return NULL;
    }

    nyx_object_t *object = transform(xmldoc);

    /*----------------------------------------------------------------------------------------------------------------*/

    return object;
}

/*--------------------------------------------------------------------------------------------------------------------*/

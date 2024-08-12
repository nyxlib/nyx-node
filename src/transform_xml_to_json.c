/*--------------------------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>

#include <libxml/tree.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_object_t *transform(const xmlNode *curr_node) // NOLINT(misc-no-recursion)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from((str_t) curr_node->name));

    /*----------------------------------------------------------------------------------------------------------------*/

    for(xmlNode *new_node = curr_node->children; new_node != NULL; new_node = new_node->next)
    {
        if(new_node->type == XML_TEXT_NODE)
        {
            xmlChar *content_s = new_node->content - 0x0000;
            size_t length = strlen((str_t) content_s);
            xmlChar *content_e = new_node->content + length;

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

    for(xmlAttr *attribute = curr_node->properties; attribute != NULL; attribute = attribute->next)
    {
        nyx_string_builder_t *sb = nyx_string_builder_from("@", (str_t) attribute->name);

        /**/    str_t attribute_name = nyx_string_builder_to_cstring(sb);
        /**/
        /**/    /**/    xmlChar *attribute_val = xmlNodeListGetString(curr_node->doc, attribute->children, 1);
        /**/    /**/
        /**/    /**/    /**/    nyx_dict_set(result, (str_t) attribute_name, nyx_string_from((str_t) attribute_val));
        /**/    /**/
        /**/    /**/    xmlFree(attribute_val);
        /**/
        /**/    nyx_memory_free(attribute_name);

        nyx_string_builder_free(sb);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(curr_node->children)
    {
        nyx_list_t *list = NULL;

        for(xmlNode *new_node = curr_node->children; new_node != NULL; new_node = new_node->next)
        {
            if(new_node->type == XML_ELEMENT_NODE)
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

    xmlNode *root = xmlDocGetRootElement(xmldoc);

    if(root == NULL)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(validate == false || nyx_validation_check(xmldoc) == true)
    {
        return transform(root);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/
#if !defined(ARDUINO)
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

    nyx_dict_set(result, "<>", nyx_string_from_dup(curr_node->name));

    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_xmldoc_t *new_node = curr_node->children; new_node != NULL; new_node = new_node->next)
    {
        if(new_node->type == NYX_XML_TEXT)
        {
            STR_t content_s = (STR_t) /* NOSONAR */ new_node->data - 0x0000;
            size_t length = strlen(content_s);
            str_t content_e = (str_t) /* NOSONAR */ new_node->data + length;

            while((isspace((unsigned char) *(content_s + 0)) || *(content_s + 0) == '"') && length > 0) {
                content_s++;
                length--;
            }

            while((isspace((unsigned char) *(content_e - 1)) || *(content_e - 1) == '"') && length > 0) {
                content_e--;
                length--;
            }

            if(length > 0)
            {
                *content_e = '\0';

                nyx_dict_set(result, "$", nyx_string_from_dup(content_s));
            }

            break;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_xmldoc_t *attribute = curr_node->attributes; attribute != NULL; attribute = attribute->next)
    {
        nyx_string_builder_t *sb = nyx_string_builder_from(NYX_SB_ESCAPE_JSON, "@", attribute->name);

        /**/    str_t attribute_name = nyx_string_builder_to_string(sb);
        /**/
        /**/    /**/
        /**/    /**/    nyx_dict_set(result, attribute_name, nyx_string_from_dup(attribute->data));
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

nyx_object_t *nyx_xmldoc_to_object(const nyx_xmldoc_t *xmldoc)
{
    return xmldoc != NULL ? transform(xmldoc) : NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/

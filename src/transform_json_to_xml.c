/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/
#if !defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *transform(const nyx_object_t *dict) // NOLINT(misc-no-recursion)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *node = nyx_xmldoc_new(NYX_XML_ELEM);

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t key;

    nyx_object_t *obj1;

    for(nyx_dict_iter_t iter1 = NYX_DICT_ITER(dict); nyx_dict_iterate(&iter1, &key, &obj1);)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        /**/ if(strcmp(key, "<>") == 0)
        {
            str_t value = nyx_object_to_cstring(obj1);

            nyx_xmldoc_set_name(node, value);

            nyx_memory_free(value);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        else if(strcmp(key, "$") == 0)
        {
            str_t value = nyx_object_to_cstring(obj1);

            nyx_xmldoc_set_content(node, value);

            nyx_memory_free(value);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        else if(key[0] == '@')
        {
            str_t value = nyx_object_to_cstring(obj1);

            nyx_xmldoc_add_attribute(node, key + 1, value);

            nyx_memory_free(value);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        else if(strcmp(key, "children") == 0)
        {
            size_t idx;

            nyx_object_t *obj2;

            for(nyx_list_iter_t iter2 = NYX_LIST_ITER(obj1); nyx_list_iterate(&iter2, &idx, &obj2);)
            {
                nyx_xmldoc_add_child(node, transform(obj2));
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return node;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_xmldoc_t *nyx_object_to_xmldoc(__NYX_NULLABLE__ const nyx_object_t *object)
{
    return object != NULL ? transform(object) : NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/

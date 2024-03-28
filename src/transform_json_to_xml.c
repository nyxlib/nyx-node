/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include <libxml/tree.h>

#include "indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static xmlNode *transform(const indi_object_t *dict) // NOLINT(misc-no-recursion)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    xmlNode *node = xmlNewNode(NULL, BAD_CAST "");

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t key;

    indi_object_t *obj1;

    for(indi_dict_iter_t iter1 = INDI_DICT_ITER(dict); indi_dict_iterate(&iter1, &key, &obj1);)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        /**/ if(strcmp(key, "<>") == 0)
        {
            str_t val = indi_object_to_cstring(obj1);

            xmlNodeSetName(node, /*--------*/ BAD_CAST val);

            indi_memory_free(val);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        else if(strcmp(key, "$") == 0)
        {
            str_t val = indi_object_to_cstring(obj1);

            xmlNodeSetContent(node, /*--------*/ BAD_CAST val);

            indi_memory_free(val);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        else if(key[0] == '@')
        {
            str_t val = indi_object_to_cstring(obj1);

            xmlNewProp(node, BAD_CAST (key + 1), BAD_CAST val);

            indi_memory_free(val);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        else if(strcmp(key, "children") == 0)
        {
            int idx;

            indi_object_t *obj2;

            for(indi_list_iter_t iter2 = INDI_LIST_ITER(obj1); indi_list_iterate(&iter2, &idx, &obj2);)
            {
                xmlAddChild(node, transform(obj2));
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return node;
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_xmldoc_t *indi_object_to_xmldoc(const indi_object_t *obj, bool validate)
{
    if(obj == NULL)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    xmlNode *root = transform(obj);

    if(root == NULL)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_xmldoc_t *xml = xmlNewDoc(BAD_CAST "1.0");

    xmlDocSetRootElement(xml, root);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(validate == true && indi_validation_check(xml) == false)
    {
        xmlFreeDoc(xml);

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return xml;
}

/*--------------------------------------------------------------------------------------------------------------------*/

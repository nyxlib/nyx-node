/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_xmldoc_t *nyx_xmldoc_new(nyx_xml_type_t type)
{
    nyx_xmldoc_t *result = (nyx_xmldoc_t *) nyx_memory_alloc(sizeof(nyx_xmldoc_t));

    memset(result, 0x00, sizeof(nyx_xmldoc_t));

    result->type = type;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void nyx_xmldoc_delete_all(__NULLABLE__ nyx_xmldoc_t *xmldoc, bool itself, bool children, bool attributes) // NOLINT(*-no-recursion)
{
    if(xmldoc == NULL)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* REMOVE ATTRIBUTES                                                                                              */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(attributes)
    {
        for(nyx_xmldoc_t *curr_child = xmldoc->attributes, *next_child; curr_child; curr_child = next_child)
        {
            next_child = curr_child->next;

            nyx_xmldoc_delete_all(curr_child, true, true, true);
        }

        xmldoc->attributes = NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* REMOVE CHILDREN                                                                                                */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(children)
    {
        for(nyx_xmldoc_t *curr_child = xmldoc->children, *next_child; curr_child; curr_child = next_child)
        {
            next_child = curr_child->next;

            nyx_xmldoc_delete_all(curr_child, true, true, true);
        }

        xmldoc->children = NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* REMOVE ITSELF                                                                                                  */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(itself)
    {
        nyx_memory_free(xmldoc->name);

        nyx_memory_free(xmldoc->data);

        nyx_memory_free(xmldoc);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_free(__NULLABLE__ nyx_xmldoc_t *xmldoc) // NOLINT(*-no-recursion)
{
    nyx_xmldoc_delete_all(xmldoc, true, true, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_xmldoc_get_name(const nyx_xmldoc_t *xmldoc)
{
    return xmldoc->name;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_set_name(nyx_xmldoc_t *xmldoc, STR_t name)
{
    if(xmldoc->name != NULL)
    {
        nyx_memory_free(xmldoc->name);
    }

    xmldoc->name = nyx_string_dup(name);
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_xmldoc_get_content(const nyx_xmldoc_t *xmldoc)
{
    for(nyx_xmldoc_t *curr_child = xmldoc->children, *next_child; curr_child; curr_child = next_child)
    {
        next_child = curr_child->next;

        if(curr_child->type == NYX_XML_TEXT_NODE
           ||
           curr_child->type == NYX_XML_CDATA_NODE
        ) {
            return curr_child->data;
        }
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_set_content(nyx_xmldoc_t *xmldoc, STR_t data)
{
    nyx_xmldoc_delete_all(xmldoc, false, true, false);

    nyx_xmldoc_t *node = nyx_xmldoc_new(NYX_XML_TEXT_NODE);

    node->name = nyx_string_dup("@@");
    node->data = nyx_string_dup(data);

    nyx_xmldoc_add_child(xmldoc, node);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_add_child(nyx_xmldoc_t *xmldoc, __NULLABLE__ nyx_xmldoc_t *child)
{
    if(child == NULL)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(child->type == NYX_XML_ATTR_NODE)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* ADD ATTRIBUTE                                                                                              */
        /*------------------------------------------------------------------------------------------------------------*/

        nyx_xmldoc_t *last_child = xmldoc->attributes;

        if(last_child != NULL)
        {
            while(last_child->next != NULL)
            {
                last_child = last_child->next;
            }

            last_child->next = child;
        }
        else
        {
            xmldoc->attributes = child;
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* ADD ELEMENT / CDATA / TEXT                                                                                 */
        /*------------------------------------------------------------------------------------------------------------*/

        nyx_xmldoc_t *last_child = xmldoc->children;

        if(last_child != NULL)
        {
            while(last_child->next != NULL)
            {
                last_child = last_child->next;
            }

            last_child->next = child;
        }
        else
        {
            xmldoc->children = child;
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    child->parent = xmldoc;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_add_attr(nyx_xmldoc_t *xmldoc, STR_t name, STR_t data)
{
    nyx_xmldoc_t *node = nyx_xmldoc_new(NYX_XML_ATTR_NODE);

    node->name = nyx_string_dup(name);
    node->data = nyx_string_dup(data);

    nyx_xmldoc_add_child(xmldoc, node);
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_xmldoc_to_string(__NULLABLE__ const nyx_xmldoc_t *xmldoc) // NOLINT(*-no-recursion)
{
    nyx_string_builder_t *sb = nyx_string_builder_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_string_builder_append(sb, "<", xmldoc->name);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_xmldoc_t *curr_child = xmldoc->attributes, *next_child; curr_child; curr_child = next_child)
    {
        next_child = curr_child->next;

        nyx_string_builder_append(sb, " ", curr_child->name, "=\"");
        nyx_string_builder_append_xml(sb, curr_child->data);
        nyx_string_builder_append(sb, "\"");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(xmldoc->self_closing == false)
    {
        nyx_string_builder_append(sb, ">");

        for(nyx_xmldoc_t *curr_child = xmldoc->children, *next_child; curr_child; curr_child = next_child)
        {
            next_child = curr_child->next;

            /*--------------------------------------------------------------------------------------------------------*/

            /**/ if(curr_child->type == NYX_XML_ELEM_NODE)
            {
                str_t node = nyx_xmldoc_to_string(curr_child);

                nyx_string_builder_append(sb, node);

                nyx_memory_free(node);
            }

            /*--------------------------------------------------------------------------------------------------------*/

            else if(curr_child->type == NYX_XML_CDATA_NODE)
            {
                nyx_string_builder_append(sb, "<![CDATA[", curr_child->data, "]]>");
            }

            /*--------------------------------------------------------------------------------------------------------*/

            else if(curr_child->type == NYX_XML_TEXT_NODE)
            {
                nyx_string_builder_append_xml(sb, curr_child->data);
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        nyx_string_builder_append(sb, "</", xmldoc->name, ">");
    }
    else
    {
        nyx_string_builder_append(sb, "/>");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t result = nyx_string_builder_to_cstring(sb);

    nyx_string_builder_free(sb);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

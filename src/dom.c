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

nyx_xmldoc_t *nyx_xmldoc_new(nyx_xml_type_t type)
{
    nyx_xmldoc_t *result = nyx_memory_alloc(sizeof(nyx_xmldoc_t));

    memset(result, 0x00, sizeof(nyx_xmldoc_t));

    result->type = type;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void nyx_xmldoc_delete_all(__NYX_NULLABLE__ nyx_xmldoc_t *xmldoc, bool itself, bool children, bool attributes) // NOLINT(*-no-recursion)
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

void nyx_xmldoc_free(__NYX_NULLABLE__ nyx_xmldoc_t *xmldoc) // NOLINT(*-no-recursion)
{
    nyx_xmldoc_delete_all(xmldoc, true, true, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NYX_NULLABLE__ str_t nyx_xmldoc_get_name(const nyx_xmldoc_t *xmldoc)
{
    return xmldoc->name;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_set_name(nyx_xmldoc_t *xmldoc, __NYX_NULLABLE__ STR_t name)
{
    if(xmldoc->name != NULL)
    {
        nyx_memory_free(xmldoc->name);
    }

    xmldoc->name = nyx_string_dup(name);
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NYX_NULLABLE__ str_t nyx_xmldoc_get_content(const nyx_xmldoc_t *xmldoc)
{
    for(nyx_xmldoc_t *curr_child = xmldoc->children, *next_child; curr_child; curr_child = next_child)
    {
        next_child = curr_child->next;

        if(curr_child->type == NYX_XML_TEXT
           ||
           curr_child->type == NYX_XML_CDATA
        ) {
            return curr_child->data;
        }
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_set_content(nyx_xmldoc_t *xmldoc, __NYX_NULLABLE__ STR_t data)
{
    nyx_xmldoc_delete_all(xmldoc, false, true, false);

    if(data != NULL)
    {
        nyx_xmldoc_t *node = nyx_xmldoc_new(NYX_XML_TEXT);

        node->name = nyx_string_dup("@@");
        node->data = nyx_string_dup(data);

        nyx_xmldoc_add_child(xmldoc, node);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_add_child(nyx_xmldoc_t *xmldoc, __NYX_NULLABLE__ nyx_xmldoc_t *child)
{
    if(child == NULL)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(child->type == NYX_XML_ATTR)
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

void nyx_xmldoc_add_attribute(nyx_xmldoc_t *xmldoc, __NYX_NULLABLE__ STR_t name, __NYX_NULLABLE__ STR_t data)
{
    if(name != NULL
       &&
       data != NULL
    ) {
        nyx_xmldoc_t *node = nyx_xmldoc_new(NYX_XML_ATTR);

        node->name = nyx_string_dup(name);
        node->data = nyx_string_dup(data);

        nyx_xmldoc_add_child(xmldoc, node);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NYX_INLINE__ void to_string_append_attribute(nyx_string_builder_t *sb, const nyx_xmldoc_t *xmldoc)
{
    for(nyx_xmldoc_t *curr_child = xmldoc->attributes, *next_child; curr_child; curr_child = next_child)
    {
        next_child = curr_child->next;

        nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, " ", curr_child->name, "=\"");
        nyx_string_builder_append(sb, NYX_SB_ESCAPE_XML, curr_child->data);
        nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "\"");
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NYX_INLINE__ void to_string_append_content(nyx_string_builder_t *sb, const nyx_xmldoc_t *xmldoc) // NOLINT(*-no-recursion)
{
    for(nyx_xmldoc_t *curr_child = xmldoc->children, *next_child; curr_child; curr_child = next_child)
    {
        next_child = curr_child->next;

        /*------------------------------------------------------------------------------------------------------------*/

        /**/ if(curr_child->type == NYX_XML_ELEM)
        {
            str_t node = nyx_xmldoc_to_string(curr_child);

            nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, node);

            nyx_memory_free(node);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        else if(curr_child->type == NYX_XML_COMMENT)
        {
            nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "<!--", curr_child->data, "-->");
        }
        else if(curr_child->type == NYX_XML_CDATA)
        {
            nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "<![CDATA[", curr_child->data, "]]>");
        }
        else if(curr_child->type == NYX_XML_TEXT)
        {
            nyx_string_builder_append(sb, NYX_SB_ESCAPE_XML, curr_child->data);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_xmldoc_to_string(const nyx_xmldoc_t *xmldoc) // NOLINT(*-no-recursion)
{
    nyx_string_builder_t *sb = nyx_string_builder_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(xmldoc->self_closing)
    {
        nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "<", xmldoc->name); to_string_append_attribute(sb, xmldoc); nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, " />");
    }
    else
    {
        nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "<", xmldoc->name); to_string_append_attribute(sb, xmldoc); nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, ">");

        to_string_append_content(sb, xmldoc);

        nyx_string_builder_append(sb, NYX_SB_NO_ESCAPE, "</", xmldoc->name, ">");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t result = nyx_string_builder_to_cstring(sb);

    nyx_string_builder_free(sb);

    return result;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include <libxml/tree.h>

#include "indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

indi_xmldoc_t *indi_xmldoc_parse_buff(__NULLABLE__ BUFF_t buff, size_t size)
{
    return (buff != NULL && size > 0x00) ? xmlReadMemory(buff, (int) size, "message.xml", "iso-8859-1", 0) : NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_xmldoc_t *indi_xmldoc_parse(__NULLABLE__ STR_t text)
{
    return (text != NULL) ? xmlReadMemory(text, (int) strlen(text), "message.xml", "iso-8859-1", 0) : NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_xmldoc_free(__NULLABLE__ indi_xmldoc_t *xmldoc)
{
    if(xmldoc != NULL)
    {
        xmlFreeDoc(xmldoc);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_xmldoc_to_string(__NULLABLE__ const indi_xmldoc_t *xmldoc)
{
    if(xmldoc == NULL)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    int length;

    str_t result;

    xmlDocDumpMemoryEnc((xmlDoc *) xmldoc, (xmlChar **) &result, &length, "iso-8859-1");

    memmove(result, result + 44, length - 44 + 1);

    return result;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../src/nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "Syntax: %s <xml string>\n", argv[0]);

        return 1;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_initialize();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *xmldoc = nyx_xmldoc_parse("<xml foo=\"bar\">\"Hello World!\"<qux><![CDATA[The World was round]]></qux>Bye!</xml>");

    if(xmldoc == NULL)
    {
        goto _err;
    }

    str_t string = nyx_xmldoc_to_string(xmldoc);

    printf("%s\n", string);

    nyx_memory_free(string);
    nyx_xmldoc_free(xmldoc);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_finalize();

    printf("[SUCCESS]\n\n");

    return 0;

_err:
    nyx_memory_finalize();

    printf("[ERROR]\n\n");

    return 1;
}

/*--------------------------------------------------------------------------------------------------------------------*/

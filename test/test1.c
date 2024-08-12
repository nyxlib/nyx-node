/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../src/nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

int main()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_initialize();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(nyx_validation_initialize() == false)
    {
        printf("Error initializing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *def1 = nyx_switch_def_new("turn_on", "Turn ON", NYX_ONOFF_ON);
    nyx_dict_t *def2 = nyx_switch_def_new("turn_off", "Turn OFF", NYX_ONOFF_OFF);

    nyx_dict_t *defs[] = {def1, def2, NULL};

    nyx_dict_t *switch_vector = nyx_switch_def_vector_new(
        "my_device",
        "my_device_onoff",
        NYX_STATE_OK,
        NYX_PERM_RW,
        NYX_RULE_AT_MOST_ONE,
        defs,
        NULL
    );

    nyx_xmldoc_t *doc = nyx_object_to_xmldoc(&switch_vector->base, true);

    str_t xml = nyx_xmldoc_to_string(doc);
    printf("%s\n", xml);
    nyx_memory_free(xml);

    nyx_xmldoc_free(doc);

    nyx_switch_def_set(def1, NYX_ONOFF_OFF);
    nyx_switch_def_set(def1, NYX_ONOFF_ON);

    printf("def value: %d\n", nyx_switch_def_get(def1));

    nyx_dict_free(switch_vector);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(nyx_validation_finalize() == false)
    {
        printf("Error finalizing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_finalize();

    printf("[SUCCESS]\n");

    return 0;

_err:
    nyx_memory_finalize();

    printf("[ERROR]\n");

    return 1;
}

/*--------------------------------------------------------------------------------------------------------------------*/

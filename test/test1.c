/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../src/indi_base.h"

/*--------------------------------------------------------------------------------------------------------------------*/

int main()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_memory_initialize();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(indi_validation_initialize() == false)
    {
        printf("Error initializing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *def1 = indi_switch_def_new("turn_on", "Turn ON", INDI_ONOFF_ON);
    indi_dict_t *def2 = indi_switch_def_new("turn_off", "Turn OFF", INDI_ONOFF_OFF);

    indi_dict_t *defs[] = {def1, def2, NULL};

    indi_dict_t *switch_vector = indi_switch_vector_new(
        "my_device",
        "my_device_onoff",
        INDI_STATE_OK,
        INDI_PERM_RW,
        INDI_RULE_AT_MOST_ONE,
        defs,
        NULL
    );

    indi_xmldoc_t *doc = indi_object_to_xmldoc(&switch_vector->base, true);

    str_t xml = indi_xmldoc_to_string(doc);
    printf("%s\n", xml);
    indi_memory_free(xml);

    indi_xmldoc_free(doc);

    indi_switch_set(def1, INDI_ONOFF_OFF);
    indi_switch_set(def1, INDI_ONOFF_ON);

    indi_dict_free(switch_vector);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(indi_validation_finalize() == false)
    {
        printf("Error finalizing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_memory_finalize();

    printf("[SUCCESS]\n");

    return 0;

_err:
    indi_memory_finalize();

    printf("[ERROR]\n");

    return 1;
}

/*--------------------------------------------------------------------------------------------------------------------*/

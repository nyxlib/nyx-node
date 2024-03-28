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

    indi_list_t *driver_list = (indi_list_t *) indi_object_parse("[\"foo\", \"bar\", \"qux\"]");

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *def1 = indi_switch_def_new("turn_on", "Turn ON", INDI_ONOFF_ON);
    indi_dict_t *def2 = indi_switch_def_new("turn_off", "Turn OFF", INDI_ONOFF_OFF);

    indi_dict_t *defs[] = {def1, def2, NULL};

    indi_dict_t *switch_vector = indi_switch_def_vector_new(
        "my_device",
        "my_device_onoff",
        INDI_STATE_OK,
        INDI_PERM_RW,
        INDI_RULE_AT_MOST_ONE,
        defs,
        NULL
    );

    indi_dict_t *vector_list[] = {switch_vector, NULL};

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_run("mqtt://localhost:1883", NULL, NULL, "TOTO", driver_list, vector_list);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(indi_dict_t **vector_ptr = vector_list; *vector_ptr != NULL; vector_ptr++)
    {
        indi_dict_free(*vector_ptr);
    }

    indi_list_free(driver_list);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(indi_validation_finalize() == false)
    {
        printf("Error finalizing validation\n");

        goto _err;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_err:
    indi_memory_finalize();

    /*----------------------------------------------------------------------------------------------------------------*/

    printf("Bye.\n");

    /*----------------------------------------------------------------------------------------------------------------*/

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_del_property_new(STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message)
{
    indi_dict_t *result = indi_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    char timestamp[32];

    internal_get_timestamp(timestamp, sizeof(timestamp));

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from("delProperty"));

    indi_dict_set(result, "@client", indi_string_from("unknown"));
    indi_dict_set(result, "@device", indi_string_from(device));
    indi_dict_set(result, "@timestamp", indi_string_from(timestamp));

    /*----------------------------------------------------------------------------------------------------------------*/

    if(name != NULL) {
        indi_dict_set(result, "@name", indi_string_from(name));
    }

    if(message != NULL) {
        indi_dict_set(result, "@message", indi_string_from(message));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

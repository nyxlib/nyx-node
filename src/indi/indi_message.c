/*--------------------------------------------------------------------------------------------------------------------*/

#include "../indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_message_new(STR_t device, STR_t message)
{
    indi_dict_t *result = indi_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    char timestamp[32];

    internal_get_timestamp(timestamp, sizeof(timestamp));

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from("message"));

    indi_dict_set(result, "@client", indi_string_from("unknown"));
    indi_dict_set(result, "@device", indi_string_from(device));
    indi_dict_set(result, "@message", indi_string_from(message));
    indi_dict_set(result, "@timestamp", indi_string_from(timestamp));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_log_level_t nyx_log_level = NYX_LOGGER_INFO;

nyx_log_func_t nyx_log_func = NULL;

void *nyx_log_args = NULL;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_log_level_to_str(nyx_log_level_t level)
{
    switch(level)
    {
        case NYX_LOGGER_NONE:
            return "NONE";
        case NYX_LOGGER_ERROR:
            return "ERROR";
        case NYX_LOGGER_INFO:
            return "INFO";
        case NYX_LOGGER_DEBUG:
            return "DEBUG";
        case NYX_LOGGER_VERBOSE:
            return "VERBOSE";
    }

    return "???";
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_log_level_t nyx_log_level = NYX_LOG_LEVEL_INFO;

nyx_log_func_t nyx_log_func = NULL;

void *nyx_log_args = NULL;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_log_level_to_str(nyx_log_level_t level)
{
    switch(level)
    {
        case NYX_LOG_LEVEL_NONE:
            return "NONE";
        case NYX_LOG_LEVEL_FATAL:
            return "FATAL";
        case NYX_LOG_LEVEL_ERROR:
            return "ERROR";
        case NYX_LOG_LEVEL_INFO:
            return "INFO";
        case NYX_LOG_LEVEL_DEBUG:
            return "DEBUG";
        case NYX_LOG_LEVEL_VERBOSE:
            return "VERBOSE";
    }

    return "???";
}

/*--------------------------------------------------------------------------------------------------------------------*/

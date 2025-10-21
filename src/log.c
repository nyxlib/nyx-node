/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_log_level_t nyx_log_level = NYX_LOG_LEVEL_INFO;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_log_level_to_str(nyx_log_level_t level)
{
    switch(level)
    {
        case NYX_LOG_LEVEL_NONE:
            return "NONE";
        case NYX_LOG_LEVEL_FATAL:
            return "\033[91;1mFATAL\033[0m";
        case NYX_LOG_LEVEL_ERROR:
            return "\033[91;1mERROR\033[0m";
        case NYX_LOG_LEVEL_WARN:
            return "\033[93;1mWARN\033[0m";
        case NYX_LOG_LEVEL_INFO:
            return "\033[32mINFO\033[0m";
        case NYX_LOG_LEVEL_DEBUG:
            return "\033[34mDEBUG\033[0m";
        case NYX_LOG_LEVEL_TRACE:
            return "\033[36mTRACE\033[0m";
    }

    return "???";
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_set_log_level(nyx_log_level_t level)
{
    nyx_log_level = level;
}

/*--------------------------------------------------------------------------------------------------------------------*/

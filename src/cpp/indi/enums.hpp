/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_ENUMS_HPP
#define NYX_INDI_ENUMS_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../common.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup ENUMS_CPP Enumerations used by NyxNode (C++)
 * @brief Enumerations used by the NyxNode C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup ENUMS_CPP
 * @brief Vector state hint.
 */
using State = nyx_state_t;
/**
 * @ingroup ENUMS_CPP
 * @brief Vector permission hint.
 */
using Perm = nyx_perm_t;
/**
 * @ingroup ENUMS_CPP
 * @brief Switch vector rule hint.
 */
using Rule = nyx_rule_t;
/**
 * @ingroup ENUMS_CPP
 * @brief Switch state.
 */
using OnOff = nyx_onoff_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Converts a State value as an integer.
 *
 * @param value The value to convert.
 * @return The State value as an integer.
 */
inline int to_int(State value)
{
    return static_cast<int>(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

inline int to_int(Perm value)
{
    return static_cast<int>(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

inline int to_int(Rule value)
{
    return static_cast<int>(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

inline int to_int(OnOff value)
{
    return static_cast<int>(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Converts a State value as a string.
 *
 * @param value The value to convert.
 * @return The State value as a string.
 */
inline STR_t to_str(State value)
{
    return nyx_state_to_str(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

inline STR_t to_str(Perm value)
{
    return nyx_perm_to_str(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

inline STR_t to_str(Rule value)
{
    return nyx_rule_to_str(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

inline STR_t to_str(OnOff value)
{
    return nyx_onoff_to_str(value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_ENUMS_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

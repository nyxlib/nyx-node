/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_COMMON_HPP
#define NYX_COMMON_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <initializer_list>

/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief JSON object type.
 */
using ObjectType = nyx_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Exception thrown by the Nyx API.
 */
class Error : public std::runtime_error
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    using std::runtime_error::runtime_error;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

inline std::string take_string(str_t value)
{
    if(value == nullptr)
    {
        return {};
    }

    std::string result{value};

    nyx_memory_free(value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

template<typename T> void check_ptr(const T *ptr, STR_t what)
{
    if(ptr == nullptr)
    {
        throw Error(what);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

inline nyx_object_t *object_ptr(nyx_null_t *ptr)
{
    return ptr != nullptr ? &ptr->base : nullptr;
}

inline nyx_object_t *object_ptr(nyx_number_t *ptr)
{
    return ptr != nullptr ? &ptr->base : nullptr;
}

inline nyx_object_t *object_ptr(nyx_boolean_t *ptr)
{
    return ptr != nullptr ? &ptr->base : nullptr;
}

inline nyx_object_t *object_ptr(nyx_string_t *ptr)
{
    return ptr != nullptr ? &ptr->base : nullptr;
}

inline nyx_object_t *object_ptr(nyx_dict_t *ptr)
{
    return ptr != nullptr ? &ptr->base : nullptr;
}

inline nyx_object_t *object_ptr(nyx_list_t *ptr)
{
    return ptr != nullptr ? &ptr->base : nullptr;
}

/*--------------------------------------------------------------------------------------------------------------------*/

template<typename T> T *as(nyx_object_t *ptr)
{
    return static_cast<T *>(static_cast<void *>(ptr));
}

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_COMMON_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

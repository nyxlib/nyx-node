/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_JSON_BOOLEAN_HPP
#define NYX_JSON_BOOLEAN_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../obj.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup BOOLEAN_OBJECT_CPP JSON Boolean Object (C++)
 * @ingroup OBJECT_CPP
 * @brief JSON boolean object C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup BOOLEAN_OBJECT_CPP
 * @brief JSON boolean object.
 */
class Boolean : public Object
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new JSON boolean object or wraps one.
     *
     * @param value Optional default value.
     */
    explicit Boolean(bool value = false) : Object(object_ptr(nyx_boolean_from(value)))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit Boolean(nyx_boolean_t *ptr, bool borrowed = false) : Object(object_ptr(ptr), borrowed)
    {
        check_type(Object::ptr(), NYX_TYPE_BOOLEAN, "Invalid nyx_boolean_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_boolean_t *boolean_ptr() const
    {
        return as<nyx_boolean_t>(ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the value of this JSON boolean object.
     *
     * @return The current value.
     */
    bool value() const
    {
        return nyx_boolean_get(boolean_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the value of this JSON boolean object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(bool value) const
    {
        return nyx_boolean_set(boolean_ptr(), value);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_JSON_BOOLEAN_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

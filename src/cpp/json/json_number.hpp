/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_JSON_NUMBER_HPP
#define NYX_JSON_NUMBER_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../obj.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup NUMBER_OBJECT_CPP JSON Number Object (C++)
 * @ingroup OBJECT_CPP
 * @brief JSON number object C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NUMBER_OBJECT_CPP
 * @brief JSON number object.
 */
class Number : public Object
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new JSON number object or wraps one.
     *
     * @param value Optional default value.
     */
    explicit Number(double value = 0.0) : Object(object_ptr(nyx_number_from(value)))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit Number(nyx_number_t *ptr, bool borrowed = false) : Object(object_ptr(ptr), borrowed)
    {
        check_type(Object::ptr(), NYX_TYPE_NUMBER, "Invalid nyx_number_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_number_t *number_ptr() const
    {
        return as<nyx_number_t>(ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the value of this JSON number object.
     *
     * @return The current value.
     */
    double value() const
    {
        return nyx_number_get(number_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the value of this JSON number object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(double value) const
    {
        return nyx_number_set(number_ptr(), value);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_JSON_NUMBER_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

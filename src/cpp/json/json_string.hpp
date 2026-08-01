/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_JSON_STRING_HPP
#define NYX_JSON_STRING_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../obj.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup STRING_OBJECT_CPP JSON String Object
 * @ingroup OBJECT_CPP
 * @brief JSON string object API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup STRING_OBJECT_CPP
 * @brief JSON string object.
 */
class String : public Object
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new JSON string object or wraps one.
     *
     * @param value Optional default value.
     */
    explicit String(STR_t value = "") : Object(object_ptr(nyx_string_new()))
    {
        if(value != nullptr && value[0] != '\0')
        {
            if(!nyx_string_set(string_ptr(), nyx_string_dup(value), true))
            {
                throw Error("Unable to set Nyx string value");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new JSON string object from a byte buffer.
     *
     * @param size Number of content bytes.
     * @param buff Optional default buffer.
     * @param managed If true, ownership of the provided buffer is transferred to the object.
     */
    explicit String(size_t size, BUFF_t buff, bool managed = false) : Object(object_ptr(nyx_string_new()))
    {
        if(size > 0 && buff != nullptr)
        {
            if(!nyx_string_set_buff(string_ptr(), size, buff, managed))
            {
                throw Error("Unable to set Nyx string buffer");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit String(nyx_string_t *ptr, bool borrowed = false) : Object(object_ptr(ptr), borrowed)
    {
        check_type(Object::ptr(), NYX_TYPE_STRING, "Invalid nyx_string_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_string_t *string_ptr() const
    {
        return as<nyx_string_t>(ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the text value of this JSON string object.
     *
     * @return The current text value.
     * @note The returned text value remains valid until the object is modified or released.
     */
    STR_t value() const
    {
        return nyx_string_get(string_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the content of this JSON string object as a byte buffer.
     *
     * @param result_size Optional pointer receiving the content byte count.
     * @param result_buff Optional pointer receiving the content buffer.
     * @note The returned buffer remains valid until the object is modified or released.
     */
    void value(size_t *result_size, buff_t *result_buff) const
    {
        nyx_string_get_buff(string_ptr(), result_size, result_buff);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the text value of this JSON string object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(STR_t value) const
    {
        value = value != nullptr ? value : "";

        if(std::strcmp(nyx_string_get(string_ptr()), value) == 0)
        {
            return false;
        }

        return nyx_string_set(string_ptr(), nyx_string_dup(value), true);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the content of this JSON string object as a byte buffer.
     *
     * @param size Number of bytes.
     * @param buff Buffer pointer.
     * @param managed If true, ownership of the provided buffer is transferred to the object.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(size_t size, BUFF_t buff, bool managed = false) const
    {
        if(size == 0 || buff == nullptr)
        {
            size = 0;
            buff = "";
        }

        return nyx_string_set_buff(string_ptr(), size, buff, managed);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t size() const
    {
        return nyx_string_length(string_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_JSON_STRING_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

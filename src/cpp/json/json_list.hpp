/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_JSON_LIST_HPP
#define NYX_JSON_LIST_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "json_boolean.hpp"
#include "json_number.hpp"
#include "json_string.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup LIST_OBJECT_CPP JSON List Object (C++)
 * @ingroup OBJECT_CPP
 * @brief JSON list object C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup LIST_OBJECT_CPP
 * @brief JSON list object.
 */
class List : public Object
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new JSON list object.
     */
    List() : Object(object_ptr(nyx_list_new()))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit List(nyx_list_t *ptr, bool borrowed = false) : Object(object_ptr(ptr), borrowed)
    {
        check_type(Object::ptr(), NYX_TYPE_LIST, "Invalid nyx_list_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_list_t *list_ptr() const
    {
        return as<nyx_list_t>(ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Clears the content of this JSON list object.
     */
    void clear() const
    {
        nyx_list_clear(list_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Deletes the entry at the provided index.
     *
     * @param idx Index.
     */
    void del(size_t idx) const
    {
        nyx_list_del(list_ptr(), idx);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the JSON object at the provided index.
     *
     * @param idx Index.
     * @return The JSON object at the provided index.
     */
    Object get(size_t idx) const
    {
        return borrowed(nyx_list_get(list_ptr(), idx));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the JSON object at the provided index.
     *
     * @param idx Index.
     * @return The JSON object at the provided index.
     */
    Object operator[](size_t idx) const
    {
        return get(idx);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets a JSON object at the provided index.
     *
     * @param idx Index.
     * @param value JSON object to be added.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set(size_t idx, const Object &value) const
    {
        return nyx_list_set(list_ptr(), idx, value.ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool set(size_t idx, bool value) const
    {
        Boolean object(value);

        return set(idx, object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool set(size_t idx, double value) const
    {
        Number object(value);

        return set(idx, object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool set(size_t idx, STR_t value) const
    {
        String object(value);

        return set(idx, object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Appends a JSON object in this JSON list object.
     *
     * @param value JSON object to be added.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool append(const Object &value) const
    {
        return nyx_list_push(list_ptr(), value.ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool append(bool value) const
    {
        Boolean object(value);

        return append(object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool append(double value) const
    {
        Number object(value);

        return append(object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool append(STR_t value) const
    {
        String object(value);

        return append(object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool get_boolean(size_t idx) const
    {
        return nyx_list_get_boolean(list_ptr(), idx);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    double get_number(size_t idx) const
    {
        return nyx_list_get_number(list_ptr(), idx);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t get_string(size_t idx) const
    {
        return nyx_list_get_string(list_ptr(), idx);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    void get_buff(size_t idx, size_t *result_size, buff_t *result_buff) const
    {
        nyx_list_get_buff(list_ptr(), idx, result_size, result_buff);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the number of items in this JSON list object.
     *
     * @return The number of items.
     */
    size_t size() const
    {
        return nyx_list_size(list_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_list_iter_t iter() const
    {
        return NYX_LIST_ITER(list_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_JSON_LIST_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

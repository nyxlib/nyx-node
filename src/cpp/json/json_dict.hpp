/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_JSON_DICT_HPP
#define NYX_JSON_DICT_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "json_boolean.hpp"
#include "json_number.hpp"
#include "json_string.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup DICT_OBJECT_CPP JSON Dict Object
 * @ingroup OBJECT_CPP
 * @brief JSON dict object API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup DICT_OBJECT_CPP
 * @brief JSON dict object.
 */
class Dict : public Object
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new JSON dict object.
     */
    Dict() : Object(object_ptr(nyx_dict_new()))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit Dict(nyx_dict_t *ptr, bool borrowed = false) : Object(object_ptr(ptr), borrowed)
    {
        check_type(Object::ptr(), NYX_TYPE_DICT, "Invalid nyx_dict_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *dict_ptr() const
    {
        return as<nyx_dict_t>(ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Clears the content of this JSON dict object.
     */
    void clear() const
    {
        nyx_dict_clear(dict_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Deletes the entry of the provided key.
     *
     * @param key Key.
     */
    void del(STR_t key) const
    {
        nyx_dict_del(dict_ptr(), key);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the JSON object of the provided key.
     *
     * @param key Key.
     * @return The JSON object.
     */
    Object get(STR_t key) const
    {
        return borrowed(nyx_dict_get(dict_ptr(), key));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the JSON object of the provided key.
     *
     * @param key Key.
     * @return The JSON object.
     */
    Object operator[](STR_t key) const
    {
        return get(key);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets a JSON object in this JSON dict object.
     *
     * @param key Key.
     * @param value JSON object to be added.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set(STR_t key, const Object &value) const
    {
        return nyx_dict_set(dict_ptr(), key, value.ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool set(STR_t key, bool value) const
    {
        Boolean object(value);

        return set(key, object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool set(STR_t key, double value) const
    {
        Number object(value);

        return set(key, object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool set(STR_t key, STR_t value) const
    {
        String object(value);

        return set(key, object);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool get_boolean(STR_t key) const
    {
        return nyx_dict_get_boolean(dict_ptr(), key);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    double get_number(STR_t key) const
    {
        return nyx_dict_get_number(dict_ptr(), key);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t get_string(STR_t key) const
    {
        return nyx_dict_get_string(dict_ptr(), key);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    void get_buff(STR_t key, size_t *result_size, buff_t *result_buff) const
    {
        nyx_dict_get_buff(dict_ptr(), key, result_size, result_buff);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the number of items in this JSON dict object.
     *
     * @return The number of items.
     */
    size_t size() const
    {
        return nyx_dict_size(dict_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_iter_t iter() const
    {
        return NYX_DICT_ITER(dict_ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

protected:
    /*----------------------------------------------------------------------------------------------------------------*/

    bool set_string_attr(STR_t key, STR_t value) const
    {
        return set(key, value != nullptr ? value : "");
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_JSON_DICT_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

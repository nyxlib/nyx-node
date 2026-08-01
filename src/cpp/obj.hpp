/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_OBJ_HPP
#define NYX_OBJ_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "common.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup OBJECT_CPP JSON serialization / deserialization
 * @brief JSON serialization / deserialization API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

class XMLDoc;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup OBJECT_CPP
 * @brief Base class for JSON Nyx objects.
 */
class Object
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Creates an empty wrapper.
     */
    Object() = default;

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Wraps a C JSON object pointer.
     *
     * @param ptr JSON object pointer.
     * @param borrowed If true, increments the C object reference count.
     */
    explicit Object(nyx_object_t *ptr, bool borrowed = false) : m_ptr(ptr)
    {
        check_ptr(m_ptr, "Invalid nyx_object_t pointer");

        if(borrowed)
        {
            nyx_object_ref(m_ptr);
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    Object(const Object &other) : m_ptr(nyx_object_ref(other.m_ptr))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    Object(Object &&other) noexcept : m_ptr(other.m_ptr)
    {
        other.m_ptr = nullptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    virtual ~Object()
    {
        nyx_object_unref(m_ptr);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    Object &operator=(const Object &other)
    {
        if(this != &other)
        {
            nyx_object_t *ptr = nyx_object_ref(other.m_ptr);

            nyx_object_unref(m_ptr);

            m_ptr = ptr;
        }

        return *this;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    Object &operator=(Object &&other) noexcept
    {
        if(this != &other)
        {
            nyx_object_unref(m_ptr);

            m_ptr = other.m_ptr;

            other.m_ptr = nullptr;
        }

        return *this;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @private
     * @brief C pointer to the JSON object.
     *
     * @return The JSON object pointer.
     */
    nyx_object_t *ptr() const
    {
        check_ptr(m_ptr, "Nyx object has been closed");

        return m_ptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool valid() const
    {
        return m_ptr != nullptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    ObjectType type() const
    {
        return nyx_object_get_type(ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Notifies this Nyx / INDI object to the clients.
     *
     * @return @c true if the object was notified, @c false otherwise.
     */
    bool notify() const
    {
        return nyx_object_notify(ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool equal(const Object &other) const
    {
        return nyx_object_equal(ptr(), other.ptr());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Returns a string representing this JSON object.
     *
     * @param json_string If @c true, the resulting string is escaped.
     * @return A string that represents this JSON document.
     */
    std::string to_string(bool json_string = true) const
    {
        return json_string ? take_string(nyx_object_to_string(ptr()))
                           : take_string(nyx_object_to_cstring(ptr()))
        ;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    std::string to_cstring() const
    {
        return take_string(nyx_object_to_cstring(ptr()));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_object_t *release()
    {
        nyx_object_t *ptr = m_ptr;

        m_ptr = nullptr;

        return ptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Parses a JSON object from a string.
     *
     * @param string JSON string.
     * @return The new JSON object.
     */
    static Object parse(STR_t string)
    {
        return Object{nyx_object_parse(string)};
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Parses a JSON object from a string.
     *
     * @param string JSON string.
     * @return The new JSON object.
     */
    static Object from_string(STR_t string)
    {
        return parse(string);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    static Object parse(size_t size, BUFF_t buff)
    {
        return Object{nyx_object_parse_buff(size, buff)};
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    static Object borrowed(nyx_object_t *ptr)
    {
        return Object{ptr, true};
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    friend bool operator==(const Object &lhs, const Object &rhs)
    {
        return lhs.equal(rhs);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    friend bool operator!=(const Object &lhs, const Object &rhs)
    {
        return !lhs.equal(rhs);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

#if !defined(ARDUINO)
    /**
     * @brief Converts this JSON Nyx / INDI command to the XML one.
     *
     * @return The corresponding XML Nyx / INDI command.
     */
    XMLDoc to_xmldoc() const;
#endif

    /*----------------------------------------------------------------------------------------------------------------*/

protected:
    /*----------------------------------------------------------------------------------------------------------------*/

    static void check_type(const nyx_object_t *ptr, ObjectType type, STR_t what)
    {
        check_ptr(ptr, what);

        if(nyx_object_get_type(ptr) != type)
        {
            throw Error(what);
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_object_t *m_ptr = nullptr;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_OBJ_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

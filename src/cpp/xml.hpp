/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_XML_HPP
#define NYX_XML_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "obj.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

#if !defined(ARDUINO)

/*--------------------------------------------------------------------------------------------------------------------*/
/* XML                                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup XMLDOC_CPP XML serialization / deserialization (C++)
 * @brief XML serialization / deserialization C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup XMLDOC_CPP
 * @brief XML document.
 */
class XMLDoc
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Wraps a C XML document pointer.
     *
     * @param ptr XML document pointer.
     */
    explicit XMLDoc(nyx_xmldoc_t *ptr) : m_ptr(ptr)
    {
        check_ptr(m_ptr, "Invalid nyx_xmldoc_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    XMLDoc(const XMLDoc &) = delete;
    XMLDoc &operator=(const XMLDoc &) = delete;

    /*----------------------------------------------------------------------------------------------------------------*/

    XMLDoc(XMLDoc &&other) noexcept : m_ptr(other.m_ptr)
    {
        other.m_ptr = nullptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    ~XMLDoc()
    {
        nyx_xmldoc_free(m_ptr);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    XMLDoc &operator=(XMLDoc &&other) noexcept
    {
        if(this != &other)
        {
            nyx_xmldoc_free(m_ptr);

            m_ptr = other.m_ptr;

            other.m_ptr = nullptr;
        }

        return *this;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @private
     * @brief C pointer to the XML document object.
     */
    nyx_xmldoc_t *ptr() const
    {
        check_ptr(m_ptr, "Nyx XML document has been closed");

        return m_ptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Returns a string representing this XML document.
     *
     * @return A string that represents this XML document.
     */
    std::string to_string() const
    {
        return take_string(nyx_xmldoc_to_string(ptr()));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Converts this XML Nyx / INDI command to the JSON one.
     *
     * @return The corresponding JSON Nyx / INDI command.
     */
    Object to_object() const
    {
        return Object(nyx_xmldoc_to_object(ptr()));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Converts this XML Nyx / INDI command to the JSON one.
     *
     * @return The corresponding JSON Nyx / INDI command.
     */
    Object to_json() const
    {
        return to_object();
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Parses an XML document from a string.
     *
     * @param string XML string.
     * @return The new XML document.
     */
    static XMLDoc parse(STR_t string)
    {
        return XMLDoc(nyx_xmldoc_parse(string));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Parses an XML document from a string.
     *
     * @param string XML string.
     * @return The new XML document.
     */
    static XMLDoc from_string(STR_t string)
    {
        return parse(string);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *m_ptr = nullptr;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

inline XMLDoc Object::to_xmldoc() const
{
    return XMLDoc(nyx_object_to_xmldoc(ptr()));
}

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* ARDUINO */

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_XML_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

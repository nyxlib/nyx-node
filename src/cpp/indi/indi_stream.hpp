/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_STREAM_HPP
#define NYX_INDI_STREAM_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "vector.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup STREAM_MESSAGE_CPP Nyx Stream Message (C++)
 * @brief Nyx Stream Message C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup STREAM_MESSAGE_CPP
 * @brief Nyx Stream property.
 */
class StreamProp : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new Nyx Stream property.
     *
     * @param name Property name.
     * @param label Property label.
     * @note If the property name ends with @c .b, the payload is automatically Base64-encoded.
     * @note If the property name ends with @c .z, the payload is automatically ZLib-compressed.
     */
    explicit StreamProp(STR_t name, STR_t label = nullptr) : Dict(nyx_stream_prop_new(name, label))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the @c name property of this object.
     *
     * @return The current value.
     */
    STR_t name() const { return get_string("@name"); }

    /**
     * @brief Gets the @c label property of this object.
     *
     * @return The current value.
     */
    STR_t label() const { return get_string("@label"); }

    /**
     * @brief Gets the @c hash property of this object.
     *
     * @return The current value.
     */
    STR_t hash() const { return get_string("@hash"); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the @c name property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_name(STR_t value) const { return set_string_attr("@name", value); }

    /**
     * @brief Sets the @c label property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_label(STR_t value) const { return set_string_attr("@label", value); }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/**
 * @ingroup STREAM_MESSAGE_CPP
 * @brief Nyx Stream vector.
 */
class StreamVector : public Vector
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new Nyx Stream vector.
     *
     * @param device Device name.
     * @param name Vector name.
     * @param state Vector state.
     * @param props Properties.
     * @param opts Options (group, label, hints, timeout, message).
     */
    StreamVector(
        STR_t device,
        STR_t name,
        State state,
        const std::vector<StreamProp> &props,
        const Opts &opts = Opts()
    ) : Vector(nyx_stream_vector_new(device, name, state, nullptr, &opts))
    {
        for(const auto &prop: props)
        {
            if(!add(prop))
            {
                throw Error("Unable to add Nyx stream property");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    StreamVector(
        STR_t device,
        STR_t name,
        State state,
        std::initializer_list<StreamProp> props,
        const Opts &opts = Opts()
    ) : StreamVector(device, name, state, std::vector<StreamProp>(props), opts)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Appends a property to this vector.
     *
     * @param prop Property to be added.
     * @return @c true if the vector was modified, @c false otherwise.
     */
    bool add(const StreamProp &prop) const { return children().append(prop); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief If Nyx Stream is enabled, publishes an entry to a stream.
     *
     * @param n_fields Number of fields.
     * @param field_sizes Field payload sizes, one per field.
     * @param field_buffs Field payload buffers, one per field.
     * @return @c true if the provided fields match the vector content, @c false otherwise.
     * @note Field payloads may contain arbitrary binary data.
     */
    bool stream_pub(size_t n_fields, const size_t field_sizes[], const buff_t field_buffs[]) const
    {
        return nyx_stream_pub(dict_ptr(), n_fields, field_sizes, field_buffs);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_STREAM_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_BLOB_HPP
#define NYX_INDI_BLOB_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "utils.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup BLOB_MESSAGE_CPP Nyx BLOB Message
 * @brief Nyx / INDI BLOB Message API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup BLOB_MESSAGE_CPP
 * @brief INDI / Nyx BLOB property.
 */
class BLOBProp : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx BLOB property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param format Payload format.
     * @param size Number of initial payload bytes.
     * @param buff Initial payload buffer.
     * @param managed If true, ownership of the provided buffer is transferred to the object.
     * @note If a format ends with @c .z, the payload is automatically ZLib+Base64-compressed, otherwise, the payload is automatically Base64-encoded.
     */
    BLOBProp(STR_t name, STR_t label = nullptr, STR_t format = nullptr, size_t size = 0, BUFF_t buff = nullptr, bool managed = false) : Dict(nyx_blob_prop_new(name, label, format, size, buff, managed))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit BLOBProp(nyx_dict_t *ptr, bool borrowed = false) : Dict(ptr, borrowed)
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
     * @brief Gets the @c format property of this object.
     *
     * @return The current value.
     */
    STR_t format() const { return get_string("@format"); }

    /**
     * @brief Gets the @c value property of this object as a byte buffer.
     *
     * @param result_size Optional pointer receiving the number of payload bytes.
     * @param result_buff Optional pointer receiving the payload buffer.
     * @note The returned buffer remains valid until the property object is modified or released.
     */
    void get_value(size_t *result_size, buff_t *result_buff) const
    {
        nyx_blob_prop_get(dict_ptr(), result_size, result_buff);
    }

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

    /**
     * @brief Sets the @c format property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_format(STR_t value) const { return set_string_attr("@format", value); }

    /**
     * @brief Sets the @c value property of this object from a byte buffer.
     *
     * @param size Number of payload bytes.
     * @param buff Payload buffer.
     * @param managed If true, ownership of the provided buffer is transferred to the object.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(size_t size, BUFF_t buff, bool managed = false) const { return nyx_blob_prop_set(dict_ptr(), size, buff, managed); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const BLOBProp &, size_t, BUFF_t)>
    void on() const
    {
        dict_ptr()->base.callback._buffer = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const BLOBProp &, size_t, BUFF_t)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, size_t size, BUFF_t buff)
    {
        Vector vector_obj(vector, true);
        BLOBProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, size, buff);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/**
 * @ingroup BLOB_MESSAGE_CPP
 * @brief INDI / Nyx BLOB vector.
 */
class BLOBVector : public Vector
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx BLOB vector.
     *
     * @param device Device name.
     * @param name Vector name.
     * @param state Vector state.
     * @param perm Vector permissions.
     * @param props Properties.
     * @param opts Options (group, label, hints, timeout, message).
     */
    BLOBVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        const std::vector<BLOBProp> &props,
        const Opts &opts = Opts()
    ) : Vector(nyx_blob_vector_new(device, name, state, perm, nullptr, &opts))
    {
        for(const auto &prop: props)
        {
            if(!add(prop))
            {
                throw Error("Unable to add Nyx BLOB property");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    BLOBVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        std::initializer_list<BLOBProp> props,
        const Opts &opts = Opts()
    ) : BLOBVector(device, name, state, perm, std::vector<BLOBProp>(props), opts)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the @c perm property of this object.
     *
     * @return The current value.
     */
    Perm perm() const { return nyx_str_to_perm(get_string("@perm")); }

    /**
     * @brief Sets the @c perm property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_perm(Perm value) const { return set_string_attr("@perm", nyx_perm_to_str(value)); }

    /**
     * @brief Appends a property to this vector.
     *
     * @param prop Property to be added.
     * @return @c true if the vector was modified, @c false otherwise.
     */
    bool add(const BLOBProp &prop) const { return children().append(prop); }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_BLOB_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

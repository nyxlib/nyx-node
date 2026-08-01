/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_TEXT_HPP
#define NYX_INDI_TEXT_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "vector.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup TEXT_MESSAGE_CPP Nyx Text Message
 * @brief Nyx / INDI Text Message API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup TEXT_MESSAGE_CPP
 * @brief INDI / Nyx text property.
 */
class TextProp : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx text property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param value Initial text value.
     */
    TextProp(STR_t name, STR_t label = nullptr, STR_t value = nullptr) : Dict(nyx_text_prop_new(name, label, nyx_string_dup(value), true))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit TextProp(nyx_dict_t *ptr, bool borrowed = false) : Dict(ptr, borrowed)
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
     * @brief Gets the @c value property of this object.
     *
     * @return The current text value.
     */
    STR_t value() const { return nyx_text_prop_get(dict_ptr()); }

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
     * @brief Sets the @c value property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(STR_t value) const { return nyx_text_prop_set(dict_ptr(), value != nullptr ? value : "", true); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const TextProp &, STR_t, STR_t)>
    void on() const
    {
        dict_ptr()->base.callback._str = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const TextProp &, STR_t, STR_t)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, STR_t new_value, STR_t old_value)
    {
        Vector vector_obj(vector, true);
        TextProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, new_value, old_value);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/**
 * @ingroup TEXT_MESSAGE_CPP
 * @brief INDI / Nyx text vector.
 */
class TextVector : public Vector
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx text vector.
     *
     * @param device Device name.
     * @param name Vector name.
     * @param state Vector state.
     * @param perm Vector permissions.
     * @param props Properties.
     * @param opts Options (group, label, hints, timeout, message).
     */
    TextVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        const std::vector<TextProp> &props,
        const Opts &opts = Opts()
    ) : Vector(nyx_text_vector_new(device, name, state, perm, nullptr, &opts))
    {
        for(const auto &prop: props)
        {
            if(!add(prop))
            {
                throw Error("Unable to add Nyx text property");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    TextVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        std::initializer_list<TextProp> props,
        const Opts &opts = Opts()
    ) : TextVector(device, name, state, perm, std::vector<TextProp>(props), opts)
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
    bool add(const TextProp &prop) const { return children().append(prop); }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_TEXT_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

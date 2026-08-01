/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_LIGHT_HPP
#define NYX_INDI_LIGHT_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "vector.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup LIGHT_MESSAGE_CPP Nyx Light Message
 * @brief Nyx / INDI Light Message API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup LIGHT_MESSAGE_CPP
 * @brief INDI / Nyx light property.
 */
class LightProp : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx light property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param value Initial value.
     */
    LightProp(STR_t name, STR_t label = nullptr, State value = NYX_STATE_IDLE) : Dict(nyx_light_prop_new(name, label, value))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit LightProp(nyx_dict_t *ptr, bool borrowed = false) : Dict(ptr, borrowed)
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
     * @return The current value.
     */
    State value() const { return nyx_light_prop_get(dict_ptr()); }

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
    bool set_value(State value) const { return nyx_light_prop_set(dict_ptr(), value); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const LightProp &, State, State)>
    void on() const
    {
        dict_ptr()->base.callback._int = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const LightProp &, State, State)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, int new_value, int old_value)
    {
        Vector vector_obj(vector, true);
        LightProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, static_cast<State>(new_value), static_cast<State>(old_value));
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/**
 * @ingroup LIGHT_MESSAGE_CPP
 * @brief INDI / Nyx light vector.
 */
class LightVector : public Vector
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx light vector.
     *
     * @param device Device name.
     * @param name Vector name.
     * @param state Vector state.
     * @param props Properties.
     * @param opts Options (group, label, hints, timeout, message).
     */
    LightVector(
        STR_t device,
        STR_t name,
        State state,
        const std::vector<LightProp> &props,
        const Opts &opts = Opts()
    ) : Vector(nyx_light_vector_new(device, name, state, nullptr, &opts))
    {
        for(const auto &prop: props)
        {
            if(!add(prop))
            {
                throw Error("Unable to add Nyx light property");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    LightVector(
        STR_t device,
        STR_t name,
        State state,
        std::initializer_list<LightProp> props,
        const Opts &opts = Opts()
    ) : LightVector(device, name, state, std::vector<LightProp>(props), opts)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Appends a property to this vector.
     *
     * @param prop Property to be added.
     * @return @c true if the vector was modified, @c false otherwise.
     */
    bool add(const LightProp &prop) const { return children().append(prop); }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_LIGHT_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

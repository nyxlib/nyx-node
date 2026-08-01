/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_SWITCH_HPP
#define NYX_INDI_SWITCH_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "vector.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup SWITCH_MESSAGE_CPP Nyx Switch Message (C++)
 * @brief Nyx / INDI Switch Message C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup SWITCH_MESSAGE_CPP
 * @brief INDI / Nyx switch property.
 */
class SwitchProp : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx switch property.
     *
     * @param name Property name.
     * @param label Property label.
     * @param value Initial value.
     */
    SwitchProp(STR_t name, STR_t label = nullptr, OnOff value = NYX_ONOFF_OFF) : Dict(nyx_switch_prop_new(name, label, value))
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    SwitchProp(STR_t name, STR_t label, bool value) : SwitchProp(name, label, value ? NYX_ONOFF_ON : NYX_ONOFF_OFF)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    explicit SwitchProp(nyx_dict_t *ptr, bool borrowed = false) : Dict(ptr, borrowed)
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
    OnOff value() const { return nyx_switch_prop_get(dict_ptr()); }

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
    bool set_value(OnOff value) const { return nyx_switch_prop_set(dict_ptr(), value); }

    /**
     * @brief Sets the @c value property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_value(bool value) const { return set_value(value ? NYX_ONOFF_ON : NYX_ONOFF_OFF); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this property.
     */
    template<bool (*Callback)(const Vector &, const SwitchProp &, OnOff, OnOff)>
    void on() const
    {
        dict_ptr()->base.callback._int = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<bool (*Callback)(const Vector &, const SwitchProp &, OnOff, OnOff)>
    static bool _callback(nyx_dict_t *vector, nyx_dict_t *prop, int new_value, int old_value)
    {
        Vector vector_obj(vector, true);
        SwitchProp prop_obj(prop, true);

        return Callback(vector_obj, prop_obj, static_cast<OnOff>(new_value), static_cast<OnOff>(old_value));
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/**
 * @ingroup SWITCH_MESSAGE_CPP
 * @brief INDI / Nyx switch vector.
 */
class SwitchVector : public Vector
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx switch vector.
     *
     * @param device Device name.
     * @param name Vector name.
     * @param state Vector state.
     * @param perm Vector permissions.
     * @param rule Vector rules.
     * @param props Properties.
     * @param opts Options (group, label, hints, timeout, message).
     */
    SwitchVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        Rule rule,
        const std::vector<SwitchProp> &props,
        const Opts &opts = Opts()
    ) : Vector(nyx_switch_vector_new(device, name, state, perm, rule, nullptr, &opts))
    {
        for(const auto &prop: props)
        {
            if(!add(prop))
            {
                throw Error("Unable to add Nyx switch property");
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    SwitchVector(
        STR_t device,
        STR_t name,
        State state,
        Perm perm,
        Rule rule,
        std::initializer_list<SwitchProp> props,
        const Opts &opts = Opts()
    ) : SwitchVector(device, name, state, perm, rule, std::vector<SwitchProp>(props), opts)
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
     * @brief Gets the @c rule property of this object.
     *
     * @return The current value.
     */
    Rule rule() const { return nyx_str_to_rule(get_string("@rule")); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the @c perm property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_perm(Perm value) const { return set_string_attr("@perm", nyx_perm_to_str(value)); }

    /**
     * @brief Sets the @c rule property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_rule(Rule value) const { return set_string_attr("@rule", nyx_rule_to_str(value)); }

    /**
     * @brief Appends a property to this vector.
     *
     * @param prop Property to be added.
     * @return @c true if the vector was modified, @c false otherwise.
     */
    bool add(const SwitchProp &prop) const { return children().append(prop); }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_SWITCH_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

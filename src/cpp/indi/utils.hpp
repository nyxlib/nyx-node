/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_UTILS_HPP
#define NYX_INDI_UTILS_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "enums.hpp"
#include "../json/json_dict.hpp"
#include "../json/json_list.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/
/* NYX VECTOR OPTIONS                                                                                                 */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief INDI / Nyx vector options.
 *
 * Options are group, label, hints, message and timeout.
 */
struct Opts : nyx_opts_t
{
    /**
     * @brief Builds INDI / Nyx vector options.
     *
     * @param opt_group Optional group name.
     * @param opt_label Optional vector label.
     * @param opt_hints Optional display hints.
     * @param opt_message Optional human-oriented message.
     * @param opt_timeout Optional timeout.
     */
    explicit Opts(
        STR_t opt_group = nullptr,
        STR_t opt_label = nullptr,
        STR_t opt_hints = nullptr,
        STR_t opt_message = nullptr,
        double opt_timeout = 0.0
    ) : nyx_opts_t{opt_group, opt_label, opt_hints, opt_message, opt_timeout}
    {
    }
};

/*--------------------------------------------------------------------------------------------------------------------*/
/* NYX VECTOR BASE                                                                                                    */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Base class for INDI / Nyx vectors.
 */
class Vector : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    explicit Vector(nyx_dict_t *ptr, bool borrowed = false) : Dict(ptr, borrowed)
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the @c device property of this object.
     *
     * @return The current value.
     */
    STR_t device() const { return get_string("@device"); }

    /**
     * @brief Gets the @c name property of this object.
     *
     * @return The current value.
     */
    STR_t name() const { return get_string("@name"); }

    /**
     * @brief Gets the @c state property of this object.
     *
     * @return The current value.
     */
    State state() const { return nyx_str_to_state(get_string("@state")); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sets the @c device property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_device(STR_t value) const { return set_string_attr("@device", value); }

    /**
     * @brief Sets the @c name property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_name(STR_t value) const { return set_string_attr("@name", value); }

    /**
     * @brief Sets the @c state property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_state(State value) const { return set_string_attr("@state", nyx_state_to_str(value)); }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Gets the properties attached to this vector.
     *
     * @return The vector children list.
     */
    List children() const
    {
        return List(as<nyx_list_t>(nyx_dict_get(dict_ptr(), "children")), true);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a callback triggered when clients modify this vector.
     */
    template<void (*Callback)(const Vector &, bool)>
    void on() const
    {
        dict_ptr()->base.callback._vector = _callback<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    template<void (*Callback)(const Vector &, bool)>
    static void _callback(nyx_dict_t *vector, bool modified)
    {
        Vector vector_obj(vector, true);

        Callback(vector_obj, modified);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_UTILS_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

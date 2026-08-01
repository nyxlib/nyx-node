/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_DEL_PROPERTY_HPP
#define NYX_INDI_DEL_PROPERTY_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../json/json_dict.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NYX_MESSAGE_CPP
 * @brief INDI / Nyx @c delete-property message object.
 */
class DelProperty : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx @c delete-property message object.
     *
     * @param device Device name.
     * @param name Optional vector name, where nullptr means the whole device.
     * @param message Optional human-oriented message.
     * @note Prefer using Node::send_del_property().
     */
    explicit DelProperty(STR_t device, STR_t name = nullptr, STR_t message = nullptr) : Dict(nyx_del_property_new(device, name, message))
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
     * @brief Gets the @c message property of this object.
     *
     * @return The current value.
     */
    STR_t message() const { return get_string("@message"); }

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
     * @brief Sets the @c message property of this object.
     *
     * @param value New value.
     * @return @c true if the value was modified, @c false otherwise.
     */
    bool set_message(STR_t value) const { return set_string_attr("@message", value); }

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_INDI_DEL_PROPERTY_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_INDI_MESSAGE_HPP
#define NYX_INDI_MESSAGE_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../json/json_dict.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/* NYX MESSAGES                                                                                                       */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup NYX_MESSAGE_CPP Other Nyx Messages
 * @brief Other Nyx Message API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NYX_MESSAGE_CPP
 * @brief INDI / Nyx human-oriented message object.
 */
class Message : public Dict
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates a new INDI / Nyx human-oriented message object.
     *
     * @param device Device name.
     * @param message Human-oriented message.
     * @note Prefer using Node::send_message().
     */
    explicit Message(STR_t device, STR_t message = nullptr) : Dict(nyx_message_new(device, message))
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

#endif /* NYX_INDI_MESSAGE_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

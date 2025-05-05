/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_sub(nyx_node_t *node, STR_t topic)
{
    nyx_str_t _topic = nyx_str_s(topic);

    internal_mqtt_sub(
        node,
        _topic
    );
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_pub(nyx_node_t *node, STR_t topic, __NULLABLE__ BUFF_t message_buff, __ZEROABLE__ size_t message_size)
{
    nyx_str_t _topic = nyx_str_s(topic);

    nyx_str_t _message = NYX_STR_S(
        message_buff,
        message_size
    );

    internal_mqtt_pub(
        node,
        _topic,
        _message
    );
}

/*--------------------------------------------------------------------------------------------------------------------*/

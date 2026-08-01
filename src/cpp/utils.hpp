/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_UTILS_HPP
#define NYX_UTILS_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "common.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup UTILS_CPP Nyx utilities
 * @brief Nyx utilities API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup UTILS_CPP
 * @brief Hashes a string using the MurmurHash2 algorithm.
 *
 * @param value String to hash.
 * @param seed Initial seed value.
 * @return The computed 32-bit hash.
 */
inline uint32_t hash(const std::string &value, uint32_t seed)
{
    return ::nyx_hash(value.size(), value.data(), seed);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup UTILS_CPP
 * @brief Hashes a string using the MurmurHash2 algorithm.
 *
 * @param value String to hash.
 * @param seed Initial seed value.
 * @return The computed 32-bit hash.
 */
inline uint32_t hash(STR_t value, uint32_t seed)
{
    return value != nullptr && value[0] != '\0' ? ::nyx_hash(std::strlen(value), value, seed)
                                                : seed
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup UTILS_CPP
 * @brief Hashes a buffer using the MurmurHash2 algorithm.
 *
 * @param size Number of bytes to hash.
 * @param buff Buffer to hash.
 * @param seed Initial seed value.
 * @return The computed 32-bit hash.
 */
inline uint32_t hash(size_t size, BUFF_t buff, uint32_t seed)
{
    return ::nyx_hash(size, buff, seed);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup UTILS_CPP
 * @brief Generates a MAC address based on a node identifier.
 *
 * @param result Output array receiving the generated MAC address.
 * @param mac0 First fixed byte of the MAC address.
 * @param mac1 Second fixed byte of the MAC address.
 * @param node_id Unique node identifier used to hash the remaining bytes.
 */
inline void generate_mac_addr(uint8_t result[6], uint8_t mac0, uint8_t mac1, STR_t node_id)
{
    ::nyx_generate_mac_addr(result, mac0, mac1, node_id);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup UTILS_CPP
 * @brief Encodes a buffer using the Base64 algorithm.
 *
 * @param value_size Number of data bytes.
 * @param value_buff Data to encode.
 * @return The encoded string.
 */
inline std::string base64_encode(size_t value_size, BUFF_t value_buff)
{
    size_t result_size = 0;

    str_t string = ::nyx_base64_encode(&result_size, value_size, value_buff);

    return take_string(string);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup UTILS_CPP
 * @brief Decodes a string using the Base64 algorithm.
 *
 * @param result_size Optional pointer receiving the decoded buffer size.
 * @param value Base64 data to decode.
 * @return The decoded buffer, or nullptr when no data is provided.
 */
inline buff_t base64_decode(size_t *result_size, const std::string &value)
{
    return ::nyx_base64_decode(result_size, value.size(), value.data());
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup UTILS_CPP
 * @brief Decodes a string using the Base64 algorithm.
 *
 * @param result_size Optional pointer receiving the decoded buffer size.
 * @param value_size Number of Base64 bytes.
 * @param value Base64 data to decode.
 * @return The decoded buffer, or nullptr when no data is provided.
 */
inline buff_t base64_decode(size_t *result_size, size_t value_size, STR_t value)
{
    return ::nyx_base64_decode(result_size, value_size, value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_UTILS_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/

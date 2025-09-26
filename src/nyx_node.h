/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_NODE_H
#define NYX_NODE_H

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef ARDUINO
#  pragma clang diagnostic push
#  pragma ide diagnostic ignored "OCUnusedMacroInspection"
#  pragma ide diagnostic ignored "UnreachableCallsOfFunction"
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

double nan(const char *tag);

/*--------------------------------------------------------------------------------------------------------------------*/

#define __NOTNULL__ \
            /* do nothing */

#define __NULLABLE__ \
            /* do nothing */

#define __ZEROABLE__ \
            /* do nothing */

#define __UNUSED__ \
            __attribute__((unused))

#define __INLINE__ \
            __attribute__((always_inline)) static inline

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @defgroup MEMORY Memory
  * Memory primitives with leak detection.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef /*-*/ void *buff_t;                                                                     //!< Alias for `void *`.
typedef const void *BUFF_t;                                                                     //!< Alias for `const void *`.

typedef /*-*/ char *str_t;                                                                      //!< Alias for `char *`.
typedef const char *STR_t;                                                                      //!< Alias for `const char *`.

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Initialize the memory subsystem.
 */

void nyx_memory_initialize();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Finalize the memory subsystem.
 */

bool nyx_memory_finalize();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Similar to libc free except that it returns the amount of memory freed.
 */

__ZEROABLE__ size_t nyx_memory_free(
    __NULLABLE__ buff_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Similar to libc malloc except that a memory overflow causes the node to stop.
 */

__NULLABLE__ buff_t nyx_memory_alloc(
    __ZEROABLE__ size_t size
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Similar to libc realloc except that a memory overflow causes the node to stop.
 */

__NULLABLE__ buff_t nyx_memory_realloc(
    __NULLABLE__ buff_t buff,
    __ZEROABLE__ size_t size
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* LOGGER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup LOGGER Logger
  * Logger.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Nyx log levels.
 */

typedef enum nyx_log_level_e
{
    NYX_LOG_LEVEL_NONE    = 100,
    NYX_LOG_LEVEL_FATAL   = 101,
    NYX_LOG_LEVEL_ERROR   = 102,
    NYX_LOG_LEVEL_INFO    = 103,
    NYX_LOG_LEVEL_DEBUG   = 104,
    NYX_LOG_LEVEL_VERBOSE = 105,

} nyx_log_level_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sets the log level threshold.
 *
 * @param level The log level threshold.
 */

void nyx_set_log_level(
    nyx_log_level_t level
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

void nyx_log(
    nyx_log_level_t level,
    STR_t file,
    STR_t func,
    int line,
    STR_t fmt,
    ...
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Logs a fatal message.
 *
 * @param fmt Printf-style formatting string.
 * @param ... Format arguments.
 */

#define NYX_LOG_FATAL(fmt, ...) \
            do { nyx_log(NYX_LOG_LEVEL_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__); } while(1)

/**
 * \brief Logs an error message.
 *
 * @param fmt Printf-style formatting string.
 * @param ... Format arguments.
*/

#define NYX_LOG_ERROR(fmt, ...) \
            do { nyx_log(NYX_LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

/**
 * \brief Logs an info message.
 *
 * @param fmt Printf-style formatting string.
 * @param ... Format arguments.
 */

#define NYX_LOG_INFO(fmt, ...) \
            do { nyx_log(NYX_LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

/**
 * \brief Logs a debug message.
 *
 * @param fmt Printf-style formatting string.
 * @param ... Format arguments.
 */

#define NYX_LOG_DEBUG(fmt, ...) \
            do { nyx_log(NYX_LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

/**
 * \brief Logs a verbose message.
 *
 * @param fmt Printf-style formatting string.
 * @param ... Format arguments.
 */

#define NYX_LOG_VERBOSE(fmt, ...) \
            do { nyx_log(NYX_LOG_LEVEL_VERBOSE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

/*--------------------------------------------------------------------------------------------------------------------*/
/* UTILITIES                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup Utilities
  * Utilities.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Hashes a buffer using the MurmurHash2 algorithm.
 *
 * \param size Size of the buffer to hash.
 * \param buff Pointer to the buffer to hash.
 * \param seed Initial seed value.
 * \return The computed 32-bit hash.
 */

uint32_t nyx_hash32(
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff,
    uint32_t seed
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Generates a MAC address based on a node identifier.
 *
 * @param result_mac Output array to store the generated MAC address.
 * @param mac0 First fixed byte of the MAC address.
 * @param mac1 Second fixed byte of the MAC address.
 * @param node_id Unique node identifier used to hash the remaining bytes.
 */

void nyx_generate_mac_addr(
    uint8_t result_mac[6],
    uint8_t mac0,
    uint8_t mac1,
    STR_t node_id
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Encodes a buffer using the base64 algorithm.
 *
 * \param result_len Optional pointer to store the length of the encoded string.
 * \param size Size of the buffer to encode.
 * \param buff Pointer to the buffer to encode.
 * \return The encoded string.
 */

__NULLABLE__ str_t nyx_base64_encode(
    __NULLABLE__ size_t *result_len,
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Decodes a string using the base64 algorithm.
 *
 * \param result_size Optional pointer to store the size of the decoded buffer.
 * \param len Length of the string to decode.
 * \param str Pointer to the string to decode.
 * \return The decoded buffer.
 */

__NULLABLE__ buff_t nyx_base64_decode(
    __NULLABLE__ size_t *result_size,
    __ZEROABLE__ size_t len,
    __NULLABLE__ STR_t str
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Compresses a buffer using the ZLib algorithm.
 *
 * \param result_buff Optional pointer to store the size of the compressed buffer.
 * \param size Size of the buffer to compressed.
 * \param buff Pointer to the buffer to compressed.
 * \return The compressed buffer.
 */

__NULLABLE__ buff_t nyx_zlib_deflate(
    __NULLABLE__ size_t *result_buff,
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Uncompresses a buffer using the ZLib algorithm.
 *
 * \param result_size Mandatory pointer to provide and store the size of the uncompressed buffer.
 * \param size Size of the buffer to uncompress.
 * \param buff Pointer to the buffer to uncompress.
 * \return The uncompressed buffer.
 */

__NULLABLE__ buff_t nyx_zlib_inflate(
    __NOTNULL__ size_t *result_size,
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Compresses a buffer using the ZLib+base64 algorithm.
 *
 * \param result_len Optional pointer to store the length of the compressed string.
 * \param size Size of the buffer to compress.
 * \param buff Pointer to the buffer to compress.
 * \return The compressed string.
 */

__NULLABLE__ str_t nyx_zlib_base64_deflate(
    __NULLABLE__ size_t *result_len,
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Uncompresses a string using the ZLib+base64 algorithm.
 *
 * \param result_size Mandatory pointer to provide and store the size of the uncompressed buffer.
 * \param len Length of the string to uncompress.
 * \param str Pointer to the string to uncompress.
 * \return The uncompressed buffer.
 */

__NULLABLE__ buff_t nyx_zlib_base64_inflate(
    __NOTNULL__ size_t *result_size,
    __ZEROABLE__ size_t len,
    __NULLABLE__ STR_t str
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup OBJECT JSON serialization / deserialization
  * JSON serialization / deserialization.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_OBJECT_MAGIC 0x65656565U                                                            //!< Magic number for identifying JSON objects.

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_FLAGS_DISABLED    ((uint64_t) 0x0000000000000001U)                                  //!< Flag indicating that the object is disabled.
/* 0b0000000000000000000000000000000_0000000000000000000000000000000_01 */

#define NYX_FLAGS_BLOB_MASK   ((uint64_t) 0x00000001FFFFFFFCU)                                  //!< Mask indicating the Nyx blob emission per client.
/* 0b0000000000000000000000000000000_1111111111111111111111111111111_00 */

#define NYX_FLAGS_STREAM_MASK ((uint64_t) 0xFFFFFFFE00000000U)                                  //!< Mask indicating the Nyx stream emission per client.
/* 0b1111111111111111111111111111111_0000000000000000000000000000000_00 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief JSON object types.
 */

typedef enum
{
    NYX_TYPE_NULL = 200,                                                                        //!< Null object.
    NYX_TYPE_BOOLEAN = 201,                                                                     //!< Boolean object.
    NYX_TYPE_NUMBER = 202,                                                                      //!< Number object.
    NYX_TYPE_STRING = 203,                                                                      //!< String object.
    NYX_TYPE_DICT = 204,                                                                        //!< Dict object.
    NYX_TYPE_LIST = 205,                                                                        //!< List object.

} nyx_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

struct nyx_dict_s;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @struct nyx_object_t
 * \brief Struct describing a JSON object.
 */

typedef struct nyx_object_s
{
    uint32_t magic;                                                                             //!< Magic number, must always be @ref NYX_OBJECT_MAGIC.
    uint64_t flags;                                                                             //!< Mask of flags, see NYX_FLAGS_XXX definitions.

    nyx_type_t type;                                                                            //!< Type of object, see @ref nyx_type_t.

    __NULLABLE__ struct nyx_node_s *node;                                                       //!< Pointer to the associated Nyx node.

    __NULLABLE__ struct nyx_object_s *parent;                                                   //!< Pointer to the parent object.

    union {

        __NULLABLE__ void *_ptr;                                                                //!< The untyped pointer.

        __NULLABLE__ bool (* _int)(
            struct nyx_dict_s *vector,                                                          //!< The parent vector object.
            struct nyx_dict_s *def,                                                             //!< This object.
            int new_value,                                                                      //!< The new value.
            int old_value                                                                       //!< The old value.
        );

        __NULLABLE__ bool (* _uint)(
            struct nyx_dict_s *vector,                                                          //!< The parent vector object.
            struct nyx_dict_s *def,                                                             //!< This object.
            unsigned int new_value,                                                             //!< The new value.
            unsigned int old_value                                                              //!< The old value.
        );

        __NULLABLE__ bool (* _long)(
            struct nyx_dict_s *vector,                                                          //!< The parent vector object.
            struct nyx_dict_s *def,                                                             //!< The def object.
            long new_value,                                                                     //!< The new value.
            long old_value                                                                      //!< The old value.
        );

        __NULLABLE__ bool (* _ulong)(
            struct nyx_dict_s *vector,                                                          //!< The parent vector object.
            struct nyx_dict_s *def,                                                             //!< The def object.
            unsigned long new_value,                                                            //!< The new value.
            unsigned long old_value                                                             //!< The old value.
        );

        __NULLABLE__ bool (* _double)(
            struct nyx_dict_s *vector,                                                          //!< The parent vector object.
            struct nyx_dict_s *def,                                                             //!< The def object.
            double new_value,                                                                   //!< The new value.
            double old_value                                                                    //!< The old value.
        );

        __NULLABLE__ bool (* _str)(
            struct nyx_dict_s *vector,                                                          //!< The parent vector object.
            struct nyx_dict_s *def,                                                             //!< The def object.
            STR_t new_value,                                                                    //!< The new value.
            STR_t old_value                                                                     //!< The old value.
        );

        __NULLABLE__ bool (* _buffer)(
            struct nyx_dict_s *vector,                                                          //!< The parent vector object.
            struct nyx_dict_s *def,                                                             //!< The def object.
            size_t size,                                                                        //!< The size of the new buffer.
            BUFF_t buff                                                                         //!< The pointer to the new buffer.
        );

        __NULLABLE__ void (* _vector)(
            struct nyx_dict_s *vector                                                           //!< The vector object.
        );

    } in_callback;                                                                              //!< Callback triggered when the client modifies this object.

    __NULLABLE__ void (* out_callback)(
        struct nyx_object_s *object                                                             //!< This object.
    );                                                                                          //!< Callback triggered when the server modifies this object.

    __NULLABLE__ void *ctx;                                                                     //!< User context pointer passed to callbacks.

} nyx_object_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_object_t
 * \brief Parses a JSON object from a string buffer.
 *
 * \param size The string size.
 * \param buff The string pointer.
 * \return The new JSON object.
 */

__NULLABLE__ nyx_object_t *nyx_object_parse_buff(
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/**
 * @memberof nyx_object_t
 * \brief Parses a JSON object from a string.
 *
 * \param text The string.
 * \return The new JSON object.
 */

__NULLABLE__ nyx_object_t *nyx_object_parse(
    __NULLABLE__ STR_t text
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_object_t
 * \brief Frees memory of the provided JSON object.
 *
 * @param object The provided JSON document.
 */

void nyx_object_free(
    __NULLABLE__ nyx_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_object_t
 * \brief Compares two JSON objects.
 *
 * @param object1 The first JSON object.
 * @param object2 The second JSON object.
 * @return `true` if the objects are equal, and `false` if not.
 */

bool nyx_object_equal(
    __NULLABLE__ const nyx_object_t *object1,
    __NULLABLE__ const nyx_object_t *object2
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_object_t
 * \brief Returns a string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
 */

str_t nyx_object_to_string(
    __NULLABLE__ const nyx_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_object_t
 * \brief Returns a C/C++ string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
 */

str_t nyx_object_to_cstring(
    __NULLABLE__ const nyx_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NULL                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NULL_OBJECT JSON Null Object
  * @ingroup OBJECT
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Struct describing a JSON null value.
 */

typedef struct
{
    nyx_object_t base;                                                                          //!< Common object header for JSON values.

} nyx_null_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_null_t
 * \brief Allocates a new JSON null object.
 *
 * @return The new JSON null.
 */

nyx_null_t *nyx_null_new();

/**
 * @memberof nyx_null_t
 * \brief Frees memory of the provided JSON null object.
 *
 * @param object The provided JSON null object.
 */

void nyx_null_free(
    /*-*/ nyx_null_t *object
);

/**
 * @memberof nyx_null_t
 * \brief Returns a string representing the provided JSON null object.
 *
 * @param object The provided object.
 * @return The string representing the provided JSON null object.
 */

str_t nyx_null_to_string(
    const nyx_null_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NUMBER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NUMBER_OBJECT JSON Number Object
  * @ingroup OBJECT
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Struct describing a JSON number.
 */

typedef struct
{
    nyx_object_t base;                                                                          //!< Common object header for JSON values.

    double value;                                                                               //!< Number payload.

} nyx_number_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_number_t
 * \brief Allocates a new JSON number object.
 *
 * @return The new JSON number.
 */

nyx_number_t *nyx_number_new();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_number_t
 * \brief Frees memory of the provided JSON number object.
 *
 * @param object The provided JSON number object.
 */

void nyx_number_free(
    /*-*/ nyx_number_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_number_t
 * \brief Get the value of the provided JSON number object.
 *
 * @param object The provided JSON number object.
 * @return The value of the provided JSON number object.
 */

double nyx_number_get(
    const nyx_number_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_number_set_alt(
    /*-*/ nyx_number_t *object,
    double value,
    bool notify
);

/**
 * @memberof nyx_number_t
 * \brief Set the value of the provided JSON number object.
 *
 * @param object The provided JSON number object.
 * @param value The value for the provided JSON number object.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_number_set(nyx_number_t *object, double value)
{
    return nyx_number_set_alt(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_number_t
 * \brief Returns a string representing the provided JSON number object.
 *
 * @param object The provided object.
 * @return The string representing the provided JSON number object.
 */

str_t nyx_number_to_string(
    const nyx_number_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_number_t
 * \brief Returns a JSON number object holding the value of the provided argument.
 *
 * @param value The value for the new JSON number object.
 * @return The new JSON number object.
 */

__INLINE__ nyx_number_t *nyx_number_from(double value)
{
    nyx_number_t *result = nyx_number_new();

    nyx_number_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* BOOLEAN                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup BOOLEAN_OBJECT JSON Boolean Object
  * @ingroup OBJECT
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Struct describing a JSON boolean.
 */

typedef struct
{
    nyx_object_t base;                                                                          //!< Common object header for JSON values.

    bool value;                                                                                 //!< Boolean payload.

} nyx_boolean_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_boolean_t
 * \brief Allocates a new JSON boolean object.
 *
 * @return The new JSON boolean.
 */

nyx_boolean_t *nyx_boolean_new();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_boolean_t
 * \brief Frees memory of the provided JSON boolean object.
 *
 * @param object The provided JSON boolean object.
 */

void nyx_boolean_free(
    /*-*/ nyx_boolean_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_boolean_t
 * \brief Get the value of the provided JSON boolean object.
 *
 * @param object The provided JSON boolean object.
 * @return The value of the provided JSON boolean object.
 */

bool nyx_boolean_get(
    const nyx_boolean_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_boolean_set_alt(
    /*-*/ nyx_boolean_t *object,
    bool value,
    bool notify
);

/**
 * @memberof nyx_boolean_t
 * \brief Set the value of the provided JSON boolean object.
 *
 * @param object The provided JSON boolean object.
 * @param value The value for the provided JSON boolean object.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_boolean_set(nyx_boolean_t *object, bool value)
{
    return nyx_boolean_set_alt(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_boolean_t
 * \brief Returns a string representing the provided JSON boolean object.
 *
 * @param object The provided object.
 * @return The string representing the provided JSON boolean object.
 */

str_t nyx_boolean_to_string(
    const nyx_boolean_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_boolean_t
 * \brief Returns a JSON boolean object holding the value of the provided argument.
 *
 * @param value The value for the new JSON boolean object.
 * @return The new JSON boolean object.
 */

__INLINE__ nyx_boolean_t *nyx_boolean_from(bool value)
{
    nyx_boolean_t *result = nyx_boolean_new();

    nyx_boolean_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STRING                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup STRING_OBJECT JSON String Object
  * @ingroup OBJECT
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Struct describing a JSON string.
 */

typedef struct
{
    nyx_object_t base;                                                                          //!< Common object header for JSON values.

    size_t raw_size;                                                                            //!< Raw payload size before encoding/compression.
    size_t length;                                                                              //!< String length in bytes (UTF-8), excluding `NULL`.
    str_t value;                                                                                //!< ???

    bool dyn;                                                                                   //!< ???

} nyx_string_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Allocates a new JSON string object.
 *
 * @return The new JSON string.
 */

nyx_string_t *nyx_string_new();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Frees memory of the provided JSON string object.
 *
 * @param object The provided JSON string object.
 */

void nyx_string_free(
    /*-*/ nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Get the value of the provided JSON string object.
 *
 * @param object The provided JSON string object.
 * @return The value of the provided JSON string object.
 */

STR_t nyx_string_get(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

void nyx_string_get_buff(
    const nyx_string_t *object,
    __NULLABLE__ size_t *result_size,
    __NULLABLE__ buff_t *result_buff,
    bool base64_decode,
    bool compress
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_string_set_dup_alt(
    /*-*/ nyx_string_t *object,
    STR_t value,
    bool notify
);

/**
 * @memberof nyx_string_t
 * \brief Set the value of the provided JSON string object (string duplication).
 *
 * @param object The provided JSON string object.
 * @param value The value for the provided JSON string object.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_string_set_dup(nyx_string_t *object, STR_t value)
{
    return nyx_string_set_dup_alt(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_string_set_ref_alt(
    /*-*/ nyx_string_t *object,
    STR_t value,
    bool notify
);

/**
 * @memberof nyx_string_t
 * \brief Set the value of the provided JSON string object (string reference).
 *
 * @param object The provided JSON string object.
 * @param value The value for the provided JSON string object.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_string_set_ref(nyx_string_t *object, STR_t value)
{
    return nyx_string_set_ref_alt(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_string_set_buff_alt(
    /*-*/ nyx_string_t *object,
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff,
    bool base64_encode,
    bool compress,
    bool notify
);

/**
 * @memberof nyx_string_t
 * \brief Set the value of the provided JSON string object (buffer reference or base64 encoding).
 *
 * @param object The provided JSON string object.
 * @param size The value size for the provided JSON string object.
 * @param buff The value buffer for the provided JSON string object.
 * @param base64_encode Indicates whether the buffer must be base64-encoded.
 * @param compress Indicates whether the buffer must be ZLib-compressed.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_string_set_buff(nyx_string_t *object, size_t size, BUFF_t buff, bool base64_encode, bool compress)
{
    return nyx_string_set_buff_alt(object, size, buff, base64_encode, compress, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns the raw size (before base64-encoding or compressing) of JSON string object.
 *
 * @param object The provided JSON string object.
 * @return
 */

size_t nyx_string_raw_size(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns the length of the provided JSON string object.
 *
 * @param object The provided JSON string object.
 * @return
 */

size_t nyx_string_length(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a string representing the provided JSON string object.
 *
 * @param object The provided object.
 * @return The string representing the provided JSON string object.
 */

str_t nyx_string_to_string(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a C/C++ string representing the provided JSON string object.
 *
 * @param object The provided object.
 * @return The string representing the provided JSON string object.
 */

str_t nyx_string_to_cstring(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a JSON string object holding the value of the provided argument (string duplication).
 *
 * @param value The value for the new JSON string object.
 * @return The new JSON string object.
 */

__INLINE__ nyx_string_t *nyx_string_from_dup(STR_t value)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_set_dup(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a JSON string object holding the value of the provided argument (string reference).
 *
 * @param value The value for the new JSON string object.
 * @return The new JSON string object.
 */

__INLINE__ nyx_string_t *nyx_string_from_ref(STR_t value)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_set_ref(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a JSON string object holding the value of the provided argument (buffer reference or base64 encoding).
 *
 * @param size The buffer size for the new JSON string object.
 * @param buff The buffer pointer for the new JSON string object.
 * @param base64_encode Indicates whether the buffer must be base64-encoded.
 * @param compress Indicates whether the buffer must be ZLib-compressed.
 * @return The new JSON string object.
 */

__INLINE__ nyx_string_t *nyx_string_from_buff(size_t size, BUFF_t buff, bool base64_encode, bool compress)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_set_buff(result, size, buff, base64_encode, compress);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Alias to @ref nyx_string_set_dup.
 */

#define nyx_string_set nyx_string_set_dup

/**
 * \brief Alias to @ref nyx_string_from_dup.
 */

#define nyx_string_from nyx_string_from_dup

/*--------------------------------------------------------------------------------------------------------------------*/
/* DICT                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup DICT_OBJECT JSON Dict Object
  * @ingroup OBJECT
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @struct nyx_dict_t
 * \brief Struct describing a JSON dict.
 */

typedef struct nyx_dict_s
{
    nyx_object_t base;                                                                          //!< Common object header for JSON values.

    struct nyx_dict_node_s *head;                                                               //!< Linked list of key/value entries.
    struct nyx_dict_node_s *tail;                                                               //!< Linked list of key/value entries.

} nyx_dict_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Struct describing a JSON dict iterator.
 */

typedef struct
{
    int idx;                                                                                    //!< ???

    nyx_type_t type;                                                                            //!< ???

    struct nyx_dict_node_s *head;                                                               //!< ???

} nyx_dict_iter_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_DICT_ITER(object) \
                ((nyx_dict_iter_t) {.idx = 0, .type = ((nyx_dict_t *) (object))->base.type, .head = ((nyx_dict_t *) (object))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief Allocates a new JSON dict object.
 *
 * @return The new JSON dict.
 */

nyx_dict_t *nyx_dict_new();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief Frees memory of the provided JSON dict object.
 *
 * @param object The provided JSON dict object.
 */

void nyx_dict_free(
    /*-*/ nyx_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief Clears the content of the provided JSON dict object.
 *
 * @param object The provided JSON dict object.
 */

void nyx_dict_clear(
    /*-*/ nyx_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief
 *
 * @param object
 * @param key
 */

void nyx_dict_del(
    /*-*/ nyx_dict_t *object,
    STR_t key
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief
 *
 * @param iter
 * @param key
 * @param object
 * @return
 */

bool nyx_dict_iterate(
    nyx_dict_iter_t *iter,
    STR_t *key,
    nyx_object_t **object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief
 *
 * @param object
 * @param key
 * @return
 */

nyx_object_t *nyx_dict_get(
    const nyx_dict_t *object,
    STR_t key
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_dict_set_alt(
    /*-*/ nyx_dict_t *object,
    STR_t key,
    buff_t value,
    bool notify
);

/**
 * @memberof nyx_dict_t
 * \brief
 *
 * @param object
 * @param key
 * @param value
 * @return
 */

__INLINE__ bool nyx_dict_set(nyx_dict_t *object, STR_t key, buff_t value)
{
    return nyx_dict_set_alt(object, key, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief Gets the number of items in the provided JSON dict object.
 *
 * @param object The provided JSON dict object.
 * @return The number of items in the provided JSON dict object.
 */

size_t nyx_dict_size(
    const nyx_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief Returns a string representing the provided JSON dict object.
 *
 * @param object The provided object.
 * @return The string representing the provided JSON dict object.
 */

str_t nyx_dict_to_string(
    const nyx_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief
 *
 * @param object
 * @param key
 * @return
 */

__INLINE__ double nyx_dict_get_number(const nyx_dict_t *object, STR_t key)
{
    nyx_object_t *number = nyx_dict_get(object, key);

    return (number != NULL && number->type == NYX_TYPE_NUMBER) ? nyx_number_get((nyx_number_t *) number)
                                                               : nan("1")
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_dict_t
 * \brief
 *
 * @param object
 * @param key
 * @return
 */

__INLINE__ STR_t nyx_dict_get_string(const nyx_dict_t *object, STR_t key)
{
    nyx_object_t *string = nyx_dict_get(object, key);

    return (string != NULL && string->type == NYX_TYPE_STRING) ? nyx_string_get((nyx_string_t *) string)
                                                               : NULL
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* LIST                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup LIST_OBJECT JSON List Object
  * @ingroup OBJECT
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @struct nyx_list_t
 * \brief Struct describing a JSON list.
 */

typedef struct nyx_list_s
{
    nyx_object_t base;                                                                          //!< Common object header for JSON values.

    struct nyx_list_node_s *head;                                                               //!< Linked list of key/value entries.
    struct nyx_list_node_s *tail;                                                               //!< Linked list of key/value entries.

} nyx_list_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Struct describing a JSON list iterator.
 */

typedef struct
{
    int idx;                                                                                    //!< ???

    nyx_type_t type;                                                                            //!< ???

    struct nyx_list_node_s *head;                                                               //!< ???

} nyx_list_iter_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_LIST_ITER(object) \
                ((nyx_list_iter_t) {.idx = 0, .type = ((nyx_list_t *) (object))->base.type, .head = ((nyx_list_t *) (object))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief Allocates a new JSON list.
 *
 * @return The new JSON list.
 */

nyx_list_t *nyx_list_new();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief Frees memory of the provided JSON list object.
 *
 * @param object The provided JSON list object.
 */

void nyx_list_free(
    /*-*/ nyx_list_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief Clears the content of the provided JSON list object.
 *
 * @param object The provided JSON dict object.
 */

void nyx_list_clear(
    /*-*/ nyx_list_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief
 *
 * @param object
 * @param idx
 */

void nyx_list_del(
    /*-*/ nyx_list_t *object,
    int idx
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief
 *
 * @param iter
 * @param idx
 * @param object
 * @return
 */

bool nyx_list_iterate(
    nyx_list_iter_t *iter,
    int *idx,
    nyx_object_t **object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief
 *
 * @param object
 * @param idx
 * @return
 */

nyx_object_t *nyx_list_get(
    const nyx_list_t *object,
    int idx
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_list_t *nyx_list_set_alt(
    /*-*/ nyx_list_t *object,
    size_t idx,
    buff_t value,
    bool notify
);

/**
 * @memberof nyx_list_t
 * \brief
 *
 * @param object
 * @param value
 * @return
 */

__INLINE__ bool nyx_list_push(nyx_list_t *object, buff_t value)
{
    return nyx_list_set_alt(object, -1, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief Gets the number of items in the provided JSON list object.
 *
 * @param object The provided JSON list object.
 * @return The number of items in the provided JSON list object.
 */

size_t nyx_list_size(
    const nyx_list_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief Returns a string representing the provided JSON list object.
 *
 * @param object The provided object.
 * @return The string representing the provided JSON list object.
 */

str_t nyx_list_to_string(
    const nyx_list_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief
 *
 * @param object
 * @param idx
 * @return
 */

__INLINE__ double nyx_list_get_number(const nyx_list_t *object, int idx)
{
    nyx_object_t *number = nyx_list_get(object, idx);

    return (number != NULL && number->type == NYX_TYPE_NUMBER) ? nyx_number_get((nyx_number_t *) number)
                                                               : nan("1")
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_list_t
 * \brief
 *
 * @param object
 * @param idx
 * @return
 */

__INLINE__ STR_t nyx_list_get_string(const nyx_list_t *object, int idx)
{
    nyx_object_t *string = nyx_list_get(object, idx);

    return (string != NULL && string->type == NYX_TYPE_STRING) ? nyx_string_get((nyx_string_t *) string)
                                                               : NULL
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* XMLDOC                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup XMLDOC XML serialization / deserialization
  * XML serialization / deserialization.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief XML node types.
 */

typedef enum
{
    NYX_XML_ELEM = 300,                                                                         //!< Element node.
    NYX_XML_ATTR = 301,                                                                         //!< Attribute node.
    NYX_XML_COMMENT = 302,                                                                      //!< Comment node.
    NYX_XML_CDATA = 303,                                                                        //!< CDATA content.
    NYX_XML_TEXT = 304,                                                                         //!< Text content.

} nyx_xml_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @struct nyx_xmldoc_t
 * \brief Struct describing an XML document.
 */

typedef struct nyx_xmldoc_s
{
    str_t name;
    nyx_xml_type_t type;
    str_t data;

    struct nyx_xmldoc_s *next;
    struct nyx_xmldoc_s *parent;

    struct nyx_xmldoc_s *children;
    struct nyx_xmldoc_s *attributes;

    bool self_closing;

} nyx_xmldoc_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_xmldoc_t
 * \brief Parses an XML document from a string buffer.
 *
 * \param size The string size.
 * \param buff The string pointer.
 * \return The new XML document.
 */

__NULLABLE__ nyx_xmldoc_t *nyx_xmldoc_parse_buff(
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/**
 * @memberof nyx_xmldoc_t
 * \brief Parses an XML document from a string.
 *
 * \param text The string.
 * \return The new XML document.
 */

__NULLABLE__ nyx_xmldoc_t *nyx_xmldoc_parse(
    __NULLABLE__ STR_t text
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_xmldoc_t
 * \brief Frees memory of the provided XML document.
 *
 * @param xmldoc The provided XML document.
 */

void nyx_xmldoc_free(
    __NULLABLE__ nyx_xmldoc_t *xmldoc
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_xmldoc_t
 * \brief Returns a string representing the provided XML document.
 *
 * @param xmldoc The provided XML document.
 * @return The string representing the provided XML document.
 */

str_t nyx_xmldoc_to_string(
    const nyx_xmldoc_t *xmldoc
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* TRANSFORM                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup TRANSFORM JSON <-> XML
  * JSON <-> XML Nyx / INDI commands.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Convert an XML Nyx / INDI command to the JSON one.
 *
 * @param xmldoc The provided XML Nyx / INDI command.
 * @return The generated JSON Nyx / INDI command.
 */

__NULLABLE__ nyx_object_t *nyx_xmldoc_to_object(
    __NULLABLE__ const nyx_xmldoc_t *xmldoc
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Convert an JSON Nyx / INDI command to the XML one.
 *
 * @param object The provided JSON Nyx / INDI command.
 * @return The generated XML Nyx / INDI command.
 */

__NULLABLE__ nyx_xmldoc_t *nyx_object_to_xmldoc(
    __NULLABLE__ const nyx_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NYX                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX Nyx protocol
  * Nyx protocol.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_INDI_VERSION "1.7"                                                                  //!< INDI backward compatibility version.

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Vector state hint.
 */

typedef enum
{
    NYX_STATE_IDLE = 400,                                                                       //!< State is idle.
    NYX_STATE_OK = 401,                                                                         //!< State is ok.
    NYX_STATE_BUSY = 402,                                                                       //!< State is busy.
    NYX_STATE_ALERT = 403,                                                                      //!< State is alert.

} nyx_state_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

STR_t nyx_state_to_str(
    nyx_state_t state
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_state_t nyx_str_to_state(
    STR_t state
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Vector permission hint.
 */

typedef enum
{
    NYX_PERM_RO = 500,                                                                          //!< Read only.
    NYX_PERM_WO = 501,                                                                          //!< Write only.
    NYX_PERM_RW = 502,                                                                          //!< Read & write.

} nyx_perm_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

STR_t nyx_perm_to_str(
    nyx_perm_t perm
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_perm_t nyx_str_to_perm(
    STR_t perm
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Switch vector rule hint.
 */

typedef enum
{
    NYX_RULE_ONE_OF_MANY = 600,                                                                 //!< Only one switch of many can be ON (e.g. radio buttons).
    NYX_RULE_AT_MOST_ONE = 601,                                                                 //!< At most one switch can be ON, but all switches can be off.
    NYX_RULE_ANY_OF_MANY = 602,                                                                 //!< Any number of switches can be ON (e.g. check boxes).

} nyx_rule_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

STR_t nyx_rule_to_str(
    nyx_rule_t rule
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_rule_t nyx_str_to_rule(
    STR_t rule
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Switch state.
 */

typedef enum
{
    NYX_ONOFF_ON = 700,                                                                         //!< Switch is ON.
    NYX_ONOFF_OFF = 701,                                                                        //!< Switch is OFF.

} nyx_onoff_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

STR_t nyx_onoff_to_str(
    nyx_onoff_t onoff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_onoff_t nyx_str_to_onoff(
    STR_t onoff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Struct describing the options for INDI / Nyx vectors.
 */

typedef struct
{
    __NULLABLE__ STR_t group;                                                                   //!< GUI group membership, "Main" by default.
    __NULLABLE__ STR_t label;                                                                   //!< GUI label, device name by default.
    __NULLABLE__ STR_t hints;                                                                   //!< GUI Markdown description.
    __NULLABLE__ STR_t message;                                                                 //!< Free comment.
    __ZEROABLE__ double timeout;                                                                //!< Worse-case time [sec] to affect, 0 by default, N/A for RO.

} nyx_opts_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

typedef struct
{
    /*----------------------------------------------------------------------------------------------------------------*/

    enum
    {
        NYX_VARIANT_TYPE_INT = 1000,
        NYX_VARIANT_TYPE_UINT = 1001,
        NYX_VARIANT_TYPE_LONG = 1002,
        NYX_VARIANT_TYPE_ULONG = 1003,
        NYX_VARIANT_TYPE_DOUBLE = 1004,

    } type;

    /*----------------------------------------------------------------------------------------------------------------*/

    union
    {
        int32_t _int;
        uint32_t _uint;
        int64_t _long;
        uint64_t _ulong;
        double _double;

    } value;

    /*----------------------------------------------------------------------------------------------------------------*/

} nyx_variant_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

__INLINE__ nyx_variant_t NYX_VARIANT_FROM_INT(int32_t value)
{
    return (nyx_variant_t) {NYX_VARIANT_TYPE_INT, {._int = value}};
}

/**
 * @private
 */

__INLINE__ nyx_variant_t NYX_VARIANT_FROM_UINT(uint32_t value)
{
    return (nyx_variant_t) {NYX_VARIANT_TYPE_UINT, {._uint = value}};
}

/**
 * @private
 */

__INLINE__ nyx_variant_t NYX_VARIANT_FROM_LONG(int64_t value)
{
    return (nyx_variant_t) {NYX_VARIANT_TYPE_LONG, {._long = value}};
}

/**
 * @private
 */

__INLINE__ nyx_variant_t NYX_VARIANT_FROM_ULONG(uint64_t value)
{
    return (nyx_variant_t) {NYX_VARIANT_TYPE_ULONG, {._ulong = value}};
}

/**
 * @private
 */

__INLINE__ nyx_variant_t NYX_VARIANT_FROM_DOUBLE(double value)
{
    return (nyx_variant_t) {NYX_VARIANT_TYPE_DOUBLE, {._double = value}};
}

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_NUMBER Nyx Number
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_dict_t *nyx_number_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t format,
    nyx_variant_t min,
    nyx_variant_t max,
    nyx_variant_t step,
    nyx_variant_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx `int32_t` number.
 * @param name Definition name.
 * @param label Definition label.
 * @param format C-like format of the value.
 * @param min Range min, ignored if min == max.
 * @param max Range max, ignored if min == max.
 * @param step Step size, ignored if step == 0.
 * @param value Initial `int32_t` value.
 * @return The new definition object.
 */

__INLINE__ nyx_dict_t *nyx_number_def_new_int(STR_t name,__NULLABLE__ STR_t label, STR_t format, int32_t min, int32_t max, int32_t step, int32_t value)
{
    return nyx_number_def_new(name, label, format, NYX_VARIANT_FROM_INT(min), NYX_VARIANT_FROM_INT(max), NYX_VARIANT_FROM_INT(step), NYX_VARIANT_FROM_INT(value));
}

/**
 * \brief Allocates a new INDI / Nyx `uint32_t` number.
 * @param name Definition name.
 * @param label Definition label.
 * @param format C-like format of the value.
 * @param min Range min, ignored if min == max.
 * @param max Range max, ignored if min == max.
 * @param step Step size, ignored if step == 0.
 * @param value Initial `uint32_t` value.
 * @return The new definition object.
 */

__INLINE__ nyx_dict_t *nyx_number_def_new_uint(STR_t name,__NULLABLE__ STR_t label, STR_t format, uint32_t min, uint32_t max, uint32_t step, uint32_t value)
{
    return nyx_number_def_new(name, label, format, NYX_VARIANT_FROM_UINT(min), NYX_VARIANT_FROM_UINT(max), NYX_VARIANT_FROM_UINT(step), NYX_VARIANT_FROM_UINT(value));
}

/**
 * \brief Allocates a new INDI / Nyx `int64_t` number.
 * @param name Definition name.
 * @param label Definition label.
 * @param format C-like format of the value.
 * @param min Range min, ignored if min == max.
 * @param max Range max, ignored if min == max.
 * @param step Step size, ignored if step == 0.
 * @param value Initial `int64_t` value.
 * @return The new definition object.
 */

__INLINE__ nyx_dict_t *nyx_number_def_new_long(STR_t name,__NULLABLE__ STR_t label, STR_t format, int64_t min, int64_t max, int64_t step, int64_t value)
{
    return nyx_number_def_new(name, label, format, NYX_VARIANT_FROM_LONG(min), NYX_VARIANT_FROM_LONG(max), NYX_VARIANT_FROM_LONG(step), NYX_VARIANT_FROM_LONG(value));
}

/**
 * \brief Allocates a new INDI / Nyx `uint64_t` number.
 * @param name Definition name.
 * @param label Definition label.
 * @param format C-like format of the value.
 * @param min Range min, ignored if min == max.
 * @param max Range max, ignored if min == max.
 * @param step Step size, ignored if step == 0.
 * @param value Initial `uint64_t` value.
 * @return The new definition object.
 */

__INLINE__ nyx_dict_t *nyx_number_def_new_ulong(STR_t name,__NULLABLE__ STR_t label, STR_t format, uint64_t min, uint64_t max, uint64_t step, uint64_t value)
{
    return nyx_number_def_new(name, label, format, NYX_VARIANT_FROM_ULONG(min), NYX_VARIANT_FROM_ULONG(max), NYX_VARIANT_FROM_ULONG(step), NYX_VARIANT_FROM_ULONG(value));
}

/**
 * \brief Allocates a new INDI / Nyx `double` number.
 * @param name Definition name.
 * @param label Definition label.
 * @param format C-like format of the value.
 * @param min Range min, ignored if min == max.
 * @param max Range max, ignored if min == max.
 * @param step Step size, ignored if step == 0.
 * @param value Initial `double` value.
 * @return The new definition object.
 */

__INLINE__ nyx_dict_t *nyx_number_def_new_double(STR_t name,__NULLABLE__ STR_t label, STR_t format, double min, double max, double step, double value)
{
    return nyx_number_def_new(name, label, format, NYX_VARIANT_FROM_DOUBLE(min), NYX_VARIANT_FROM_DOUBLE(max), NYX_VARIANT_FROM_DOUBLE(step), NYX_VARIANT_FROM_DOUBLE(value));
}

/*--------------------------------------------------------------------------------------------------------------------*/

/** @private
 */

bool nyx_number_def_set(
    nyx_dict_t *def,
    nyx_variant_t value
);

/** @private
 */

nyx_variant_t nyx_number_def_get(
    const nyx_dict_t *def
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `int32_t` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_number_def_set_int(nyx_dict_t *def, int32_t value)
{
    return nyx_number_def_set(def, NYX_VARIANT_FROM_INT(value));
}

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `int32_t` value.
 */

__INLINE__ int32_t nyx_number_def_get_int(const nyx_dict_t *def)
{
    return nyx_number_def_get(def).value._int;
}

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `uint32_t` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_number_def_set_uint(nyx_dict_t *def, uint32_t value)
{
    return nyx_number_def_set(def, NYX_VARIANT_FROM_UINT(value));
}

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `uint32_t` value.
 */

__INLINE__ uint32_t nyx_number_def_get_uint(const nyx_dict_t *def)
{
    return nyx_number_def_get(def).value._uint;
}

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `int64_t` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_number_def_set_long(nyx_dict_t *def, int64_t value)
{
    return nyx_number_def_set(def, NYX_VARIANT_FROM_LONG(value));
}

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `int64_t` value.
 */

__INLINE__ int64_t nyx_number_def_get_long(const nyx_dict_t *def)
{
    return nyx_number_def_get(def).value._long;
}

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `uint64_t` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_number_def_set_ulong(nyx_dict_t *def, uint64_t value)
{
    return nyx_number_def_set(def, NYX_VARIANT_FROM_ULONG(value));
}

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `uint64_t` value.
 */

__INLINE__ uint64_t nyx_number_def_get_ulong(const nyx_dict_t *def)
{
    return nyx_number_def_get(def).value._ulong;
}

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `double` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

__INLINE__ bool nyx_number_def_set_double(nyx_dict_t *def, double value)
{
    return nyx_number_def_set(def, NYX_VARIANT_FROM_DOUBLE(value));
}

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `double` value.
 */

__INLINE__ double nyx_number_def_get_double(const nyx_dict_t *def)
{
    return nyx_number_def_get(def).value._double;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx number vector.
 * @param device Device name.
 * @param name Vector name.
 * @param state Vector state.
 * @param perm Vector permissions.
 * @param defs Array of definitions with `NULL` sentinel.
 * @param opts Options (group, label, hints, timeout, message).
 * @return The new vector object.
 */

nyx_dict_t *nyx_number_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    __NULLABLE__ const nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_dict_t *nyx_number_set_vector_new(
    const nyx_dict_t *vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_TEXT Nyx Text
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx text.
 * @param name Definition name.
 * @param label Definition label.
 * @param value Initial value.
 * @return The new definition object.
 */

nyx_dict_t *nyx_text_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    __NULLABLE__ STR_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `STR_t` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

bool nyx_text_def_set(
    nyx_dict_t *def,
    __NULLABLE__ STR_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `STR_t` value.
 */

STR_t nyx_text_def_get(
    const nyx_dict_t *def
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx text vector.
 * @param device Device name.
 * @param name Vector name.
 * @param state Vector state.
 * @param perm Vector permissions.
 * @param defs Array of definitions with `NULL` sentinel.
 * @param opts Options (group, label, hints, timeout, message).
 * @return The new vector object.
 */

nyx_dict_t *nyx_text_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    __NULLABLE__ const nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_dict_t *nyx_text_set_vector_new(
    const nyx_dict_t *vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_LIGHT Nyx Light
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx light.
 * @param name Definition name.
 * @param label Definition label.
 * @param value Initial value.
 * @return The new definition object.
 */

nyx_dict_t *nyx_light_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    nyx_state_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `nyx_state_t` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

bool nyx_light_def_set(
    nyx_dict_t *def,
    nyx_state_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `nyx_state_t` value.
 */

nyx_state_t nyx_light_def_get(
    const nyx_dict_t *def
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx light vector.
 * @param device Device name.
 * @param name Vector name.
 * @param state Vector state.
 * @param defs Array of definitions with `NULL` sentinel.
 * @param opts Options (group, label, hints, timeout, message).
 * @return The new vector object.
 */

nyx_dict_t *nyx_light_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_dict_t *defs[],
    __NULLABLE__ const nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_dict_t *nyx_light_set_vector_new(
    const nyx_dict_t *vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_SWITCH Nyx Switch
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx switch.
 * @param name Definition name.
 * @param label Definition label.
 * @param value Initial value.
 * @return The new definition object.
 */

nyx_dict_t *nyx_switch_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    nyx_onoff_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param value The new `nyx_onoff_t` value.
 * @return \c true if the value was modified, \c false otherwise.
 */

bool nyx_switch_def_set(
    nyx_dict_t *def,
    nyx_onoff_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @return The current `nyx_onoff_t` value.
 */

nyx_onoff_t nyx_switch_def_get(
    const nyx_dict_t *def
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx switch vector.
 * @param device Device name.
 * @param name Vector name.
 * @param state Vector state.
 * @param perm Vector permissions.
 * @param rule Vector rules.
 * @param defs Array of definitions with `NULL` sentinel.
 * @param opts Options (group, label, hints, timeout, message).
 * @return The new vector object.
 */

nyx_dict_t *nyx_switch_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_rule_t rule,
    nyx_dict_t *defs[],
    __NULLABLE__ const  nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_dict_t *nyx_switch_set_vector_new(
    const nyx_dict_t *vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_BLOB Nyx BLOB
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx BLOB.
 * @param name Definition name.
 * @param label Definition label.
 * @param format Payload format.
 * @param size Size of the initial payload content.
 * @param buff Pointer to the initial payload content.
 * @return The new definition object.
 * @note If a format ends with `.b`, the payload is automatically base64-encoded.
 * @note If a format ends with `.z`, the payload is automatically zlib+base64-compressed.
 */

nyx_dict_t *nyx_blob_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    __NULLABLE__ STR_t format,
    __NULLABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sets the new value of the provided definition object.
 * @param def The definition object.
 * @param size Size of the new payload content.
 * @param buff Pointer to the new payload content.
 * @return \c true if the value was modified, \c false otherwise.
 * @note If a format ends with `.b`, the payload is automatically base64-encoded.
 * @note If a format ends with `.z`, the payload is automatically zlib+base64-compressed.
 */

bool nyx_blob_def_set(
    nyx_dict_t *def,
    __NULLABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Gets the current value of the provided definition object.
 * @param def The definition object.
 * @param size Size of the current payload content.
 * @param buff Pointer to current new payload content.
 * @warning If `buff` is not `NULL`, it must be freed.
 * @note If a format ends with `.b`, the payload is automatically base64-decoded.
 * @note If a format ends with `.z`, the payload is automatically zlib+base64-uncompresses.
 */

void nyx_blob_def_get(
    const nyx_dict_t *def,
    size_t *size,
    buff_t *buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx BLOB vector.
 * @param device Device name.
 * @param name Vector name.
 * @param state Vector state.
 * @param perm Vector permissions.
 * @param defs Array of definitions with `NULL` sentinel.
 * @param opts Options (group, label, hints, timeout, message).
 * @return The new vector object.
 */

nyx_dict_t *nyx_blob_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    __NULLABLE__ const nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_dict_t *nyx_blob_set_vector_new(
    const nyx_dict_t *vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_STREAM Nyx Stream
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new Nyx Stream.
 * @param name
 * @param label
 * @return
 */

/**
 * \brief Allocates a new Nyx Stream.
 * @param name Definition name.
 * @param label Definition label.
 * @return The new definition object.
 * @note If the name ends with `.b`, the payload is automatically base64-encoded, see @ref nyx_stream_pub.
 * @note If the name ends with `.z`, the payload is automatically zlib+base64-compressed, see @ref nyx_stream_pub.
 */

nyx_dict_t *nyx_stream_def_new(
    STR_t name,
    __NULLABLE__ STR_t label
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new Nyx stream vector.
 * @param device Device name.
 * @param name Vector name.
 * @param state Vector state.
 * @param defs Array of definitions with `NULL` sentinel.
 * @param opts Options (group, label, hints, timeout, message).
 * @return The new vector object.
 */

nyx_dict_t *nyx_stream_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_dict_t *defs[],
    __NULLABLE__ const nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief If Redis is enabled, publishes an entry to a stream, see https://redis.io/commands/xadd/.
 *
 * @param vector The Nyx node.
 * @param max_len Maximum number of entries to keep in the Redis stream.
 * @param n_fields Number of field triplets (name, length, buffer).
 * @param field_names Array of field names.
 * @param field_sizes Array of field lengths.
 * @param field_buffs Array of field buffers.
 * @note If a field name ends with `.b`, the payload is automatically base64-encoded.
 * @note If a field name ends with `.z`, the payload is automatically zlib+base64-compressed.
 */

bool nyx_stream_pub(
    nyx_dict_t *vector,
    size_t max_len,
    __ZEROABLE__ size_t n_fields,
    const str_t field_names[],
    const size_t field_sizes[],
    const buff_t field_buffs[]
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

nyx_dict_t *nyx_stream_set_vector_new(
    const nyx_dict_t *vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_MESSAGE Nyx Message
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx human message object.
 *
 * @param device Device name.
 * @param message Human message.
 * @return The new human message object.
 */

nyx_dict_t *nyx_message_new(
    STR_t device,
    STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_PROPERTY Nyx Property
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new INDI / Nyx `delete-property` message object.
 *
 * @param device Device name.
 * @param name Optional vector name (`NULL` = whole device).
 * @param message Optional human message.
 * @return The new `delete-property` message object.
 */

nyx_dict_t *nyx_del_property_new(
    STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NODE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NODE Nyx node
  * Nyx node.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @struct nyx_node_t
 * \brief Opaque struct describing a Nyx node.
 */

typedef struct nyx_node_s nyx_node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Nyx TCP or MQTT event type.
 */

typedef enum
{
    NYX_EVENT_OPEN = 1100,                                                                      //!< A connection is opened.
    NYX_EVENT_MSG = 1101,                                                                       //!< A message is received.

} nyx_event_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief MQTT event handler.
 *
 * @param node Nyx node.
 * @param event Event type.
 * @param topic_size MQTT topic length.
 * @param topic_buff MQTT topic buffer.
 * @param message_size MQTT message length.
 * @param message_buff MQTT message buffer.
 */

typedef void (* nyx_mqtt_handler_t)(
    nyx_node_t *node,
    nyx_event_t event,
    size_t topic_size,
    BUFF_t topic_buff,
    size_t message_size,
    BUFF_t message_buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Initializes a new Nyx node.
 *
 * @param node_id Unique node identifier.
 * @param vectors Array of vectors with `ǸULL` sentinel.
 * @param indi_url Optional INDI URL (e.g. tcp://0.0.0.0:7625).
 * @param mqtt_url Optional MQTT URL (e.g. mqtt://localhost:1883).
 * @param mqtt_username Optional MQTT username.
 * @param mqtt_password Optional MQTT password.
 * @param mqtt_handler Optional MQTT handler.
 * @param redis_url Optional Redis URL (e.g. tcp://localhost:6379).
 * @param redis_username Optional Redis username.
 * @param redis_password Optional Redis password.
 * @param retry_ms Connect retry time [milliseconds].
 * @param enable_xml Enables the XML messages.
 * @return The new Nyx node.
 */

__NULLABLE__ nyx_node_t *nyx_node_initialize(
    STR_t node_id,
    nyx_dict_t *vectors[],
    /**/
    __NULLABLE__ STR_t indi_url,
    /**/
    __NULLABLE__ STR_t mqtt_url,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    /**/
    __NULLABLE__ nyx_mqtt_handler_t mqtt_handler,
    /**/
    __NULLABLE__ STR_t redis_url,
    __NULLABLE__ STR_t redis_username,
    __NULLABLE__ STR_t redis_password,
    /**/
    int retry_ms,
    bool enable_xml
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Finalizes a Nyx node.
 *
 * @param node The Nyx node.
 * @param free_vectors If `true`, the definition vectors are released.
 */

void nyx_node_finalize(
    nyx_node_t *node,
    bool free_vectors
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Performs a single poll iteration.
 *
 * @param node The Nyx node.
 * @param timeout_ms Timeout [milliseconds].
 */

void nyx_node_poll(
    nyx_node_t *node,
    int timeout_ms
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Enables a device or a vector and notifies clients.
 *
 * @param node Nyx node.
 * @param device Device name.
 * @param name Optional vector name (`NULL` = whole device).
 * @param message Optional human message.
 */

void nyx_node_enable(
    nyx_node_t *node,
    /*--------*/ STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Disables a device or a vector and notifies clients.
 *
 * @param node Nyx node.
 * @param device Device name.
 * @param name Optional vector name (`NULL` = whole device).
 * @param message Optional human message.
 */

void nyx_node_disable(
    nyx_node_t *node,
    /*--------*/ STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Sends a human message to the clients.
 *
 * @param node Nyx node.
 * @param device Device name.
 * @param message Human message.
 */

void nyx_node_send_message(
    nyx_node_t *node,
    STR_t device,
    STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Sends a `del-property` message to the clients.
 *
 * @param node Nyx node.
 * @param device Device name.
 * @param name Optional vector name (`NULL` = whole device).
 * @param message Optional human message.
 */

void nyx_node_send_del_property(
    nyx_node_t *node,
    STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Subscribes to an MQTT topic (if MQTT enabled).
 *
 * @param node Nyx node.
 * @param topic MQTT topic.
 * @note `mqtt_handler` has to be defined in @ref nyx_node_initialize.
 */

void nyx_mqtt_sub(
    nyx_node_t *node,
    STR_t topic
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_node_t
 * \brief Publishes an MQTT message (if MQTT enabled).
 *
 * @param node Nyx node.
 * @param topic MQTT topic.
 * @param message_size MQTT message length.
 * @param message_buff MQTT message buffer.
 */

void nyx_mqtt_pub(
    nyx_node_t *node,
    STR_t topic,
    __ZEROABLE__ size_t message_size,
    __NULLABLE__ BUFF_t message_buff
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @} */
/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef ARDUINO
#  pragma clang diagnostic pop
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_NODE_H */

/*--------------------------------------------------------------------------------------------------------------------*/

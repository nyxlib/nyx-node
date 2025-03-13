/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnreachableCallsOfFunction"

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

double nan(const char *tag);

/*--------------------------------------------------------------------------------------------------------------------*/

#define __NULLABLE__ \
            /* do nothing */

#define __ZEROABLE__ \
            /* do nothing */

#define __UNUSED__ \
            __attribute__((unused))

#define __INLINE__ \
            __attribute__((always_inline)) static inline

/*--------------------------------------------------------------------------------------------------------------------*/

typedef /*-*/ void *buff_t;
typedef const void *BUFF_t;

typedef /*-*/ char *str_t;
typedef const char *STR_t;

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @defgroup MEMORY Memory primitives
  * Memory primitives with leak detection.
  * @{
  */
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
 * \brief Similar to libc free.
 */

__ZEROABLE__ size_t nyx_memory_free(
    __NULLABLE__ buff_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Similar to libc malloc.
 */

__NULLABLE__ buff_t nyx_memory_alloc(
    __ZEROABLE__ size_t size
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Similar to libc realloc.
 */

__NULLABLE__ buff_t nyx_memory_realloc(
    __NULLABLE__ buff_t buff,
    __ZEROABLE__ size_t size
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* BASE64                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup BASE64 Base64 encoding / decoding
  * Fast Base64 encoding / decoding.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Encodes a buffer to a Base64-encoded string.
 *
 * \param result_len Optional pointer to store the length of the encoded string.
 * \param size Size of the buffer to encode.
 * \param buff Input buffer to encode.
 * \return The encoded string.
 */

__NULLABLE__ str_t nyx_base64_encode(
    __NULLABLE__ size_t *result_len,
    __ZEROABLE__ size_t size,
    __NULLABLE__ BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Decodes a Base64-encoded string to a buffer.
 *
 * \param result_size Optional pointer to store the size of the decoded buffer.
 * \param len Length of the string to decode.
 * \param str Input string to decode.
 * \return The decoded buffer.
 */

__NULLABLE__ buff_t nyx_base64_decode(
    __NULLABLE__ size_t *result_size,
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

#define NYX_OBJECT_MAGIC 0x6565656565656565                                                     //!< Magic number for identifying JSON objects.

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_TYPE_NULL = 100,                                                                        //!< Null object.
    NYX_TYPE_BOOLEAN = 101,                                                                     //!< Boolean object.
    NYX_TYPE_NUMBER = 102,                                                                      //!< Number object.
    NYX_TYPE_STRING = 103,                                                                      //!< String object.
    NYX_TYPE_DICT = 104,                                                                        //!< Dict object.
    NYX_TYPE_LIST = 105,                                                                        //!< List object.

} nyx_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_FLAGS_XXXX_DISABLED     ((uint32_t) (1 << 0))                                       //!< Flag specifying that the object is disabled.
#define NYX_FLAGS_BLOB_DISABLED     ((uint32_t) (1 << 1))                                       //!< Flag specifying that the blob is disabled.

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_object_s
{
    uint64_t magic;                                                                             //!< Magic number, must always be @ref NYX_OBJECT_MAGIC.
    uint32_t flags;                                                                             //!< Mask of flags, see NYX_FLAGS_?_DISABLED.

    nyx_type_t type;                                                                            //!< Type of object, see @ref nyx_type_t.

    __NULLABLE__ struct nyx_node_s *node;                                                       //!< Pointer to the associated Nyx node.

    __NULLABLE__ struct nyx_object_s *parent;                                                   //!< Pointer to the parent object.

    __NULLABLE__ void (* in_callback)(
        struct nyx_object_s *object,                                                            //!< This object.
        bool modified                                                                           //!< Indicate weather the value has been modified.
    );                                                                                          //!< Callback triggered when a client modifies this object.
    __NULLABLE__ void (* out_callback)(
        struct nyx_object_s *object,                                                            //!< This object.
        bool modified                                                                           //!< Indicate weather the value has been modified.
    );                                                                                          //!< Callback triggered when the server modifies this object.

} nyx_object_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Parses a JSON object from a string buffer.
 *
 * \param buff The string pointer.
 * \param size The string size.
 * \return The new JSON object.
 */

nyx_object_t *nyx_object_parse_buff(
    __NULLABLE__ BUFF_t buff,
    __ZEROABLE__ size_t size
);

/**
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
 * \brief Frees memory for the provided JSON object.
 *
 * @param object The provided JSON document.
 */

void nyx_object_free(
    __NULLABLE__ /*-*/ nyx_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
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

typedef struct
{
    nyx_object_t base;                                                                          //!< ???

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
 * \brief Frees memory for the provided JSON null object.
 *
 * @param object The provided JSON null object.
 */

void nyx_null_free(
    /*-*/ nyx_null_t *object
);

/**
 * @memberof nyx_null_t
 * \brief Returns a string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
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

typedef struct
{
    nyx_object_t base;                                                                          //!< ???

    double value;                                                                               //!< ???

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
 * \brief Frees memory for the provided JSON number object.
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

bool nyx_number_set2(
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
 * @return
 */

__INLINE__ bool nyx_number_set(nyx_number_t *object, double value)
{
    return nyx_number_set2(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_number_t
 * \brief Returns a string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
 */

str_t nyx_number_to_string(
    const nyx_number_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_number_t
 * \brief Returns a JSON number object holding the value of the argument passed.
 *
 * @param value
 * @return A new JSON number object.
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

typedef struct
{
    nyx_object_t base;                                                                          //!< ???

    bool value;                                                                                 //!< ???

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
 * \brief Frees memory for the provided JSON boolean object.
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

bool nyx_boolean_set2(
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
 * @return
 */

__INLINE__ bool nyx_boolean_set(nyx_boolean_t *object, bool value)
{
    return nyx_boolean_set2(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_boolean_t
 * \brief Returns a string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
 */

str_t nyx_boolean_to_string(
    const nyx_boolean_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_boolean_t
 * \brief Returns a JSON boolean object holding the value of the argument passed.
 *
 * @param value
 * @return A new JSON boolean object.
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

typedef struct
{
    nyx_object_t base;                                                                          //!< ???

    size_t length;                                                                              //!< ???
    str_t value;                                                                                //!< ???

    bool dyn;                                                                                   //!< ???

} nyx_string_t;

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Allocates a new JSON string object.
 *
 * @return The new JSON string.
 */

nyx_string_t *nyx_string_new();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Frees memory for the provided JSON string object.
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

bool nyx_string_dynamic_set2(
    /*-*/ nyx_string_t *object,
    STR_t value,
    bool notify
);

/**
 * @memberof nyx_string_t
 * \brief Set the value of the provided JSON string object.
 *
 * @param object The provided JSON string object.
 * @param value The value for the provided JSON string object.
 * @return
 */

__INLINE__ bool nyx_string_dynamic_set(nyx_string_t *object, STR_t value)
{
    return nyx_string_dynamic_set2(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_string_static_set2(
    /*-*/ nyx_string_t *object,
    STR_t value,
    bool notify
);

/**
 * @memberof nyx_string_t
 * \brief Set the value of the provided JSON string object.
 *
 * @param object The provided JSON string object.
 * @param value The value for the provided JSON string object.
 * @return
 */

__INLINE__ bool nyx_string_static_set(nyx_string_t *object, STR_t value)
{
    return nyx_string_static_set2(object, value, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @private
 */

bool nyx_string_buff_set2(
    /*-*/ nyx_string_t *object,
    size_t size,
    BUFF_t buff,
    bool notify
);

/**
 * @memberof nyx_string_t
 * \brief Set the value of the provided JSON string object.
 *
 * @param object The provided JSON string object.
 * @param size The value size for the provided JSON string object.
 * @param buff The value buffer for the provided JSON string object.
 * @return
 */

__INLINE__ bool nyx_string_buff_set(nyx_string_t *object, size_t size, BUFF_t buff)
{
    return nyx_string_buff_set2(object, size, buff, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief ???
 *
 * @param object
 * @return
 */

size_t nyx_string_length(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
 */

str_t nyx_string_to_string(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a C/C++ string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
 */

str_t nyx_string_to_cstring(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a JSON string object holding the value of the argument passed (dynamic allocation).
 *
 * @param value
 * @return A new JSON string object.
 */

__INLINE__ nyx_string_t *nyx_string_dynamic_from(STR_t value)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_dynamic_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a JSON number object holding the value of the argument passed (static allocation).
 *
 * @param value
 * @return A new JSON string object.
 */

__INLINE__ nyx_string_t *nyx_string_static_from(STR_t value)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_static_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @memberof nyx_string_t
 * \brief Returns a JSON number object holding the value of the buffer passed (base64 encoded).
 *
 * @param size
 * @param buff
 * @return A new JSON string object.
 */

__INLINE__ nyx_string_t *nyx_string_buff_from(size_t size, BUFF_t buff)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_buff_set(result, size, buff);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @copydoc nyx_string_dynamic_set
 */

#define nyx_string_set nyx_string_dynamic_set

/**
 * @copydoc nyx_string_dynamic_from
 */

#define nyx_string_from nyx_string_dynamic_from

/*--------------------------------------------------------------------------------------------------------------------*/
/* DICT                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup DICT_OBJECT JSON Dict Object
  * @ingroup OBJECT
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    nyx_object_t base;                                                                          //!< ???

    struct nyx_dict_node_s *head;                                                               //!< ???
    struct nyx_dict_node_s *tail;                                                               //!< ???

} nyx_dict_t;

/*--------------------------------------------------------------------------------------------------------------------*/

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
 * \brief Frees memory for the provided JSON dict object.
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

bool nyx_dict_set2(
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
    return nyx_dict_set2(object, key, value, true);
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
 * \brief Returns a string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
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

typedef struct
{
    nyx_object_t base;                                                                          //!< ???

    struct nyx_list_node_s *head;                                                               //!< ???
    struct nyx_list_node_s *tail;                                                               //!< ???

} nyx_list_t;

/*--------------------------------------------------------------------------------------------------------------------*/

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
 * \brief Frees memory for the provided JSON list object.
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

nyx_list_t *nyx_list_set2(
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
    return nyx_list_set2(object, -1, value, true);
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
 * \brief Returns a string representing the provided object.
 *
 * @param object The provided object.
 * @return The string representing the provided object.
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

typedef enum
{
    NYX_XML_ELEM,
    NYX_XML_ATTR,
    NYX_XML_COMMENT,
    NYX_XML_CDATA,
    NYX_XML_TEXT,

} nyx_xml_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

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
 * \brief Parses an XML document from a string buffer.
 *
 * \param buff The string pointer.
 * \param size The string size.
 * \return The new XML document.
 */

nyx_xmldoc_t *nyx_xmldoc_parse_buff(
    __NULLABLE__ BUFF_t buff,
    __ZEROABLE__ size_t size
);

/**
 * \brief Parses an XML document from a string.
 *
 * \param text The string.
 * \return The new XML document.
 */

nyx_xmldoc_t *nyx_xmldoc_parse(
    __NULLABLE__ STR_t text
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Frees memory for the provided XML document.
 *
 * @param xmldoc The provided XML document.
 */

void nyx_xmldoc_free(
    __NULLABLE__ /*-*/ nyx_xmldoc_t *xmldoc
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Returns a string representing the provided XML document.
 *
 * @param xmldoc The provided XML document.
 * @return The string representing the provided XML document.
 */

str_t nyx_xmldoc_to_string(
    const nyx_xmldoc_t *xmldoc
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* VALIDATION                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup VALIDATION XML validation
  * XML validation for Nyx / INDI commands.
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Initialize the XML validation subsystem.
 */

bool nyx_validation_initialize();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Initialize the XML validation subsystem.
 */

bool nyx_validation_finalize();

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Validate the provided XML command using the INDI + Nyx additions XSD schema.
 *
 * @param xmldoc The provided XML document.
 * @return `True` if the document is valid, and `false` if not.
 *
 * @include ./src/schema/nyx.xsd
 */

bool nyx_validation_check(
    __NULLABLE__ const nyx_xmldoc_t *xmldoc
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
 * @param validate If `true`, the input XML command is validated (see @ref nyx_validation_check).
 * @return The generated JSON Nyx / INDI command.
 */

__NULLABLE__ nyx_object_t *nyx_xmldoc_to_object(
    __NULLABLE__ const nyx_xmldoc_t *xmldoc,
    bool validate
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Convert an JSON Nyx / INDI command to the XML one.
 *
 * @param object The provided JSON Nyx / INDI command.
 * @param validate If `true`, the output XML command is validated (see @ref nyx_validation_check).
 * @return The generated XML Nyx / INDI command.
 */

__NULLABLE__ nyx_xmldoc_t *nyx_object_to_xmldoc(
    __NULLABLE__ const nyx_object_t *object,
    bool validate
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

#define NYX_INDI_VERSION "1.7"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_STATE_IDLE = 200,
    NYX_STATE_OK = 201,
    NYX_STATE_BUSY = 202,
    NYX_STATE_ALERT = 203,

} nyx_state_t;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_state_to_str(
    nyx_state_t state
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_state_t nyx_str_to_state(
    STR_t state
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_PERM_RO = 300,
    NYX_PERM_WO = 301,
    NYX_PERM_RW = 302,

} nyx_perm_t;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_perm_to_str(
    nyx_perm_t perm
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_perm_t nyx_str_to_perm(
    STR_t perm
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_RULE_ONE_OF_MANY = 400,
    NYX_RULE_AT_MOST_ONE = 401,
    NYX_RULE_ANY_OF_MANY = 402,

} nyx_rule_t;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_rule_to_str(
    nyx_rule_t rule
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_rule_t nyx_str_to_rule(
    STR_t rule
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_ONOFF_ON = 500,
    NYX_ONOFF_OFF = 501,

} nyx_onoff_t;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_onoff_to_str(
    nyx_onoff_t onoff
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_onoff_t nyx_str_to_onoff(
    STR_t onoff
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_BLOB_NEVER = 600,
    NYX_BLOB_ALSO = 601,
    NYX_BLOB_ONLY = 602,

} nyx_blob_t;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_blob_to_str(
    nyx_blob_t blob
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_blob_t nyx_str_to_blob(
    STR_t blob
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    __NULLABLE__ STR_t label;
    __NULLABLE__ STR_t group;
    __NULLABLE__ double timeout;
    __NULLABLE__ STR_t message;

} nyx_opts_t;

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_NUMBER Nyx Number
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t format,
    double min,
    double max,
    double step,
    double value
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ bool nyx_number_def_set(nyx_dict_t *def, double value)
{
    return nyx_dict_set(def, "$", nyx_number_from(value));
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ double nyx_number_def_get(nyx_dict_t *def)
{
    return ((nyx_number_t *) nyx_dict_get(def, "$"))->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_TEXT Nyx Text
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_text_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ bool nyx_text_def_set(nyx_dict_t *def, STR_t value)
{
    return  nyx_dict_set(def, "$", nyx_string_from(value));
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ STR_t nyx_text_def_get(nyx_dict_t *def)
{
    return ((nyx_string_t *) nyx_dict_get(def, "$"))->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_text_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_text_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_LIGHT Nyx Light
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_light_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    nyx_state_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ bool nyx_light_def_set(nyx_dict_t *def, nyx_state_t value)
{
    return nyx_dict_set(def, "$", nyx_string_from(nyx_state_to_str(value)));
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ nyx_state_t nyx_light_def_get(nyx_dict_t *def)
{
    return nyx_str_to_state(((nyx_string_t *) nyx_dict_get(def, "$"))->value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_light_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_light_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_SWITCH Nyx Switch
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    nyx_onoff_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ bool nyx_switch_def_set(nyx_dict_t *def, nyx_onoff_t value)
{
    return nyx_dict_set(def, "$", nyx_string_from(nyx_onoff_to_str(value)));
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ nyx_onoff_t nyx_switch_def_get(nyx_dict_t *def)
{
    return nyx_str_to_onoff(((nyx_string_t *) nyx_dict_get(def, "$"))->value);
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_rule_t rule,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_BLOB Nyx BLOB
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_blob_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t value
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ bool nyx_blob_def_set(nyx_dict_t *def, STR_t value)
{
    return nyx_dict_set(def, "$", nyx_string_from(value));
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ STR_t nyx_blob_def_get(nyx_dict_t *def)
{
    return ((nyx_string_t *) nyx_dict_get(def, "$"))->value;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_blob_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_blob_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @}
  * @defgroup NYX_MESSAGE Nyx Message
  * @ingroup NYX
  * @{
  */
/*--------------------------------------------------------------------------------------------------------------------*/

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
 * \brief Opaque struct describing a Nyx node.
 */

typedef struct nyx_node_s nyx_node_t;

/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief W5500 chip select pin, set this value to enable the controller. Raspberry Pi Pico and Arduino modes only.
 */

extern int nyx_w5500_spi_cs_pin;

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Initializes the Nyx node.
 *
 * @param tcp_url Optional TCP URL (e.g. tcp://0.0.0.0:7625).
 * @param mqtt_url Optional MQTT URL (e.g. mqtt://localhost:1883).
 * @param mqtt_username Optional MQTT username.
 * @param mqtt_password Optional MQTT password.
 * @param node_id Unique node identifier.
 * @param def_vectors Array of definition vectors.
 * @param retry_ms Connect retry time [milliseconds].
 * @param enable_xml Enables the XML compatibility layer.
 * @param validate_xml Enables the XML message validation.
 * @return
 */

nyx_node_t *nyx_node_initialize(
    __NULLABLE__ STR_t tcp_url,
    __NULLABLE__ STR_t mqtt_url,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    /**/
    STR_t node_id,
    nyx_dict_t *def_vectors[],
    /**/
    int retry_ms,
    bool enable_xml,
    bool validate_xml
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Finalizes the Nyx node.
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
 * \brief Enables a whole device or a definition vector.
 *
 * @param node The Nyx node.
 * @param device The Nyx device name.
 * @param name Optional Nyx vector name.
 * @param message Optional message content.
 */

void nyx_node_enable(
    nyx_node_t *node,
    /*--------*/ STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Disables a whole device or a definition vector.
 *
 * @param node The Nyx node.
 * @param device The Nyx device name.
 * @param name Optional Nyx vector name.
 * @param message Optional message content.
 */

void nyx_node_disable(
    nyx_node_t *node,
    /*--------*/ STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sends a message to the clients.
 *
 * @param node The Nyx node.
 * @param device The Nyx device name.
 * @param message The message content.
 */

void nyx_node_send_message(
    nyx_node_t *node,
    STR_t device,
    STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/
/** @} */
/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_NODE_H */

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic pop

/*--------------------------------------------------------------------------------------------------------------------*/

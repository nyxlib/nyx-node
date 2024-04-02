########################################################################################################################

cmake_minimum_required(VERSION 3.5)

########################################################################################################################

project(indi_node C)

set(CMAKE_C_STANDARD 99)

add_definitions(-DMG_ENABLE_SSI=0)

########################################################################################################################

find_package(LibXml2 REQUIRED)

########################################################################################################################

include(CheckFunctionExists)

check_function_exists(malloc_size HAVE_MALLOC_SIZE)

check_function_exists(malloc_usable_size HAVE_MALLOC_USABLE_SIZE)

########################################################################################################################

set(SOURCE_FILES
    src/indi_node.h
    src/indi_node_schema.h
    src/indi_node_internal.h
    #
    src/mongoose/mongoose.c
    #
    src/json/json_boolean.c
    src/json/json_dict.c
    src/json/json_list.c
    src/json/json_null.c
    src/json/json_number.c
    src/json/json_string.c
    #
    src/indi/helpers.c
    src/indi/indi_blob.c
    src/indi/indi_light.c
    src/indi/indi_number.c
    src/indi/indi_switch.c
    src/indi/indi_text.c
    #
    src/alloc.c
    src/string_builder.c
    src/json.c
    src/xml.c
    src/validation.c
    src/transform_json_to_xml.c
    src/transform_xml_to_json.c
    src/stream.c
    src/node.c
)

########################################################################################################################

add_library(indi-node-static STATIC ${SOURCE_FILES})

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(indi-node-static PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(indi-node-static PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

target_include_directories(indi-node-static PRIVATE ${LIBXML2_INCLUDE_DIR})

######_link_libraries(indi-node-static ${LIBXML2_LIBRARIES})

set_target_properties(indi-node-static PROPERTIES
    OUTPUT_NAME "indi-node"
)

########################################################################################################################

add_library(indi-node-shared SHARED ${SOURCE_FILES}
        src/stream.c)

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(indi-node-shared PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(indi-node-shared PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

target_include_directories(indi-node-shared PRIVATE ${LIBXML2_INCLUDE_DIR})

target_link_libraries(indi-node-shared ${LIBXML2_LIBRARIES})

set_target_properties(indi-node-shared PROPERTIES
    OUTPUT_NAME "indi-node"
)

########################################################################################################################

add_executable(test1 test/test1.c)

target_link_libraries(test1 ${LIBXML2_LIBRARIES} indi-node-static)

########################################################################################################################

add_executable(test2 test/test2.c)

target_link_libraries(test2 ${LIBXML2_LIBRARIES} indi-node-static)

########################################################################################################################

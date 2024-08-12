########################################################################################################################

cmake_minimum_required(VERSION 3.5)

########################################################################################################################

project(nyx_node C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-D_GNU_SOURCE -DMG_ENABLE_SSI=0 -Wall -Wno-unknown-pragmas -Wno-format-truncation -O3)

########################################################################################################################

find_package(LibXml2 REQUIRED)

########################################################################################################################

include(CheckFunctionExists)

check_function_exists(malloc_size HAVE_MALLOC_SIZE)

check_function_exists(malloc_usable_size HAVE_MALLOC_USABLE_SIZE)

########################################################################################################################

set(SOURCE_FILES
    src/nyx_node.h
    src/nyx_node_schema.h
    src/nyx_node_internal.h
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
    src/indi/indi_del_property.c
    src/indi/indi_light.c
    src/indi/indi_message.c
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

add_library(nyx-node-static STATIC ${SOURCE_FILES})

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(nyx-node-static PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(nyx-node-static PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

target_include_directories(nyx-node-static PRIVATE ${LIBXML2_INCLUDE_DIR})

######_link_libraries(nyx-node-static ${LIBXML2_LIBRARIES})

set_target_properties(nyx-node-static PROPERTIES
    OUTPUT_NAME "nyx-node"
)

########################################################################################################################

add_library(nyx-node-shared SHARED ${SOURCE_FILES})

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(nyx-node-shared PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(nyx-node-shared PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

target_include_directories(nyx-node-shared PRIVATE ${LIBXML2_INCLUDE_DIR})

target_link_libraries(nyx-node-shared ${LIBXML2_LIBRARIES})

set_target_properties(nyx-node-shared PROPERTIES
    OUTPUT_NAME "nyx-node"
)

########################################################################################################################

add_executable(check_json test/check_json.c)

target_link_libraries(check_json nyx-node-static ${LIBXML2_LIBRARIES})

########################################################################################################################

add_executable(test1 test/test1.c)

target_link_libraries(test1 nyx-node-static ${LIBXML2_LIBRARIES})

########################################################################################################################

add_executable(test2 test/test2.c)

target_link_libraries(test2 nyx-node-static ${LIBXML2_LIBRARIES})

########################################################################################################################

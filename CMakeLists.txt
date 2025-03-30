########################################################################################################################

cmake_minimum_required(VERSION 3.5)

########################################################################################################################

project(nyx_node LANGUAGES C VERSION 1.0.0)

set(CMAKE_C_STANDARD 99)

add_compile_options(-D_GNU_SOURCE -DMG_ENABLE_LOG=0 -DMG_ENABLE_SSI=0 -Wall -Wno-unknown-pragmas -Wno-unused-function -O3)

########################################################################################################################

find_package(Doxygen)

########################################################################################################################
# LIBS                                                                                                                 #
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
    src/stack/generated/mongoose.c
    src/stack/mongoose.c
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
    src/utils/addr.c
    src/utils/base64.c
    src/utils/hash32.c
    src/utils/utf8.c
    #
    src/parsers/json.c
    src/parsers/xml.c
    #
    src/log.c
    src/string_builder.c
    src/object.c
    src/dom.c
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

set_target_properties(nyx-node-shared PROPERTIES
    OUTPUT_NAME "nyx-node"
)

########################################################################################################################
# INSTALLATION                                                                                                         #
########################################################################################################################

install(TARGETS nyx-node-static EXPORT LibNyxNodeTargets ARCHIVE DESTINATION lib)
install(TARGETS nyx-node-shared EXPORT LibNyxNodeTargets LIBRARY DESTINATION lib)

install(FILES src/nyx_node.h DESTINATION include)

########################################################################################################################

install(EXPORT LibNyxNodeTargets FILE LibNyxNodeTargets.cmake NAMESPACE LibNyxNode:: DESTINATION lib/cmake/LibNyxNode)

########################################################################################################################

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/LibNyxNodeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LibNyxNodeConfig.cmake"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LibNyxNodeConfig.cmake" DESTINATION lib/cmake/LibNyxNode)

########################################################################################################################
# TESTS                                                                                                                #
########################################################################################################################

add_executable(check_json test/check_json.c)
target_link_libraries(check_json nyx-node-static)

add_executable(check_xml test/check_xml.c)
target_link_libraries(check_xml nyx-node-static)

add_executable(test1 test/test1.c)
target_link_libraries(test1 nyx-node-static)

add_executable(test2 test/test2.c)
target_link_libraries(test2 nyx-node-static)

########################################################################################################################
# DOCS                                                                                                                 #
########################################################################################################################

if(DOXYGEN_FOUND)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation"
        VERBATIM
    )

    add_custom_command(TARGET docs PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/docs/)

else()
    message("Doxygen needs to be installed")
endif()

########################################################################################################################

########################################################################################################################

cmake_minimum_required(VERSION 3.5)

########################################################################################################################

project(nyx_node LANGUAGES C VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

add_compile_options(-D_GNU_SOURCE -DMG_ENABLE_LOG=0 -DMG_ENABLE_SSI=0 -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-function -Wno-implicit-fallthrough)

########################################################################################################################

find_package(Doxygen)

########################################################################################################################
# LIBS                                                                                                                 #
########################################################################################################################

include(CheckFunctionExists)

check_function_exists(malloc_size HAVE_MALLOC_SIZE)

check_function_exists(malloc_usable_size HAVE_MALLOC_USABLE_SIZE)

########################################################################################################################

find_package(ZLIB QUIET)

if(ZLIB_FOUND)
    set(HAVE_ZLIB 1)
    message(STATUS "Zlib found: ${ZLIB_INCLUDE_DIRS} ${ZLIB_LIBRARIES}")
else()
    set(HAVE_ZLIB 0)
    message(STATUS "Zlib not found -> building with minimal compression support")
endif()

########################################################################################################################

set(SOURCE_FILES
    src/nyx_node.h
    src/nyx_node.hpp
    src/nyx_node_internal.h
    #
    src/utils/addr.c
    src/utils/base64.c
    src/utils/zlib.c
    src/utils/hash32.c
    src/utils/utf8.c
    #
    src/log.c
    src/string_builder.c
    src/object.c
    src/dom.c
    #
    src/parsers/json.c
    src/parsers/xml.c
    #
    src/json/json_boolean.c
    src/json/json_dict.c
    src/json/json_list.c
    src/json/json_null.c
    src/json/json_number.c
    src/json/json_string.c
    #
    src/indi/helpers.c
    src/indi/format.c
    src/indi/indi_blob.c
    src/indi/indi_del_property.c
    src/indi/indi_light.c
    src/indi/indi_message.c
    src/indi/indi_number.c
    src/indi/indi_switch.c
    src/indi/indi_stream.c
    src/indi/indi_text.c
    #
    src/stacks/generated/mongoose.c
    src/stacks/mongoose.c
    #
    src/xml_stream.c
    src/transform_json_to_xml.c
    src/transform_xml_to_json.c
    src/node.c
    #
    src/mqtt.c
    src/redis.c
)

########################################################################################################################

add_library(nyx-node-static STATIC ${SOURCE_FILES})

target_include_directories(nyx-node-static
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
    $<INSTALL_INTERFACE:include>
)

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(nyx-node-static PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(nyx-node-static PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

if(HAVE_ZLIB)
    target_compile_definitions(nyx-node-static PRIVATE HAVE_ZLIB)
    target_link_libraries(nyx-node-static PUBLIC ZLIB::ZLIB m)
else()
    target_link_libraries(nyx-node-static PUBLIC m)
endif()

set_target_properties(nyx-node-static PROPERTIES
    OUTPUT_NAME "nyx-node"
)

########################################################################################################################

add_library(nyx-node-shared SHARED ${SOURCE_FILES})

target_include_directories(nyx-node-shared
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
    $<INSTALL_INTERFACE:include>
)

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(nyx-node-shared PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(nyx-node-shared PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

if(HAVE_ZLIB)
    target_compile_definitions(nyx-node-shared PRIVATE HAVE_ZLIB)
    target_link_libraries(nyx-node-shared PUBLIC ZLIB::ZLIB m)
else()
    target_link_libraries(nyx-node-shared PUBLIC m)
endif()

set_target_properties(nyx-node-shared PROPERTIES
    OUTPUT_NAME "nyx-node"
)

########################################################################################################################
# INSTALLATION                                                                                                         #
########################################################################################################################

install(TARGETS nyx-node-static EXPORT NyxNodeTargets ARCHIVE DESTINATION lib)
install(TARGETS nyx-node-shared EXPORT NyxNodeTargets LIBRARY DESTINATION lib)

install(FILES src/nyx_node.h src/nyx_node.hpp DESTINATION include)

########################################################################################################################

install(EXPORT NyxNodeTargets FILE NyxNodeTargets.cmake NAMESPACE NyxNode:: DESTINATION lib/cmake/NyxNode)

########################################################################################################################

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/NyxNodeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/NyxNodeConfig.cmake"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/NyxNodeConfig.cmake" DESTINATION lib/cmake/NyxNode)

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
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation"
        VERBATIM
    )

    add_custom_command(TARGET docs POST_BUILD
        COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/docs -type f -name "*.html" -exec sed -i "s/Topics/C API/g" {} +
        VERBATIM
    )

    add_custom_command(TARGET docs POST_BUILD
        COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/docs -type f -name "*.js" -exec sed -i "s/Topics/C API/g" {} +
        VERBATIM
    )

else()
    message("Doxygen not installed")
endif()

########################################################################################################################

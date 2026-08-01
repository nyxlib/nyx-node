########################################################################################################################

cmake_minimum_required(VERSION 3.10)

########################################################################################################################

project(nyx_node
    VERSION 1.0.0
    LANGUAGES C CXX
)

########################################################################################################################

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -fno-omit-frame-pointer -fsanitize=address,undefined")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

add_compile_options(-D_GNU_SOURCE -DMG_ENABLE_LOG=0 -DMG_ENABLE_DIRLIST=0 -DMG_ENABLE_POLL=0 -DMG_ENABLE_EPOLL=1 -DMG_ENABLE_SSI=0 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unknown-pragmas -Wno-unused-function)

########################################################################################################################

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

if(ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

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

find_package(Doxygen)

########################################################################################################################

set(CPP_HEADER_FILES
    src/nyx_node.hpp
    src/cpp/__init__.hpp
    src/cpp/common.hpp
    src/cpp/logger.hpp
    src/cpp/node.hpp
    src/cpp/obj.hpp
    src/cpp/runtime.hpp
    src/cpp/utils.hpp
    src/cpp/xml.hpp
    src/cpp/indi/__init__.hpp
    src/cpp/indi/enums.hpp
    src/cpp/indi/indi_blob.hpp
    src/cpp/indi/indi_del_property.hpp
    src/cpp/indi/indi_light.hpp
    src/cpp/indi/indi_message.hpp
    src/cpp/indi/indi_number.hpp
    src/cpp/indi/indi_stream.hpp
    src/cpp/indi/indi_switch.hpp
    src/cpp/indi/indi_text.hpp
    src/cpp/indi/vector.hpp
    src/cpp/json/__init__.hpp
    src/cpp/json/json_boolean.hpp
    src/cpp/json/json_dict.hpp
    src/cpp/json/json_list.hpp
    src/cpp/json/json_null.hpp
    src/cpp/json/json_number.hpp
    src/cpp/json/json_string.hpp
)

set(SOURCE_FILES
    src/nyx_node.h
    ${CPP_HEADER_FILES}
    src/nyx_node_internal.h
    #
    src/utils/addr.c
    src/utils/utf8.c
    src/utils/base64.c
    src/utils/hash.c
    src/utils/zlib.c
    #
    src/logger.c
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
    src/indi/utils.c
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
    src/stacks/external/mongoose.c
    src/stacks/mongoose.c
    #
    src/xml_stream.c
    src/transform_json_to_xml.c
    src/transform_xml_to_json.c
    #
    src/mqtt.c
    src/nss.c
    #
    src/node.c
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
install(DIRECTORY src/cpp DESTINATION include FILES_MATCHING PATTERN "*.hpp")

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

add_executable(demo test/demo.c)
target_link_libraries(demo nyx-node-static)

add_executable(demo_cpp test/demo.cpp)
target_link_libraries(demo_cpp nyx-node-static)

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

else()
    message("Doxygen not installed")
endif()

########################################################################################################################

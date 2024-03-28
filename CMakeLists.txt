########################################################################################################################

cmake_minimum_required(VERSION 3.5)

########################################################################################################################

project(indi_base C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-result -O3 -fPIC -flto -fomit-frame-pointer")

########################################################################################################################

find_package(LibXml2 REQUIRED)

########################################################################################################################

include(CheckFunctionExists)

check_function_exists(malloc_size HAVE_MALLOC_SIZE)

check_function_exists(malloc_usable_size HAVE_MALLOC_USABLE_SIZE)

########################################################################################################################

set(SOURCE_FILES
    src/indi_base.h
    src/indi_base_schema.h
    src/indi_base_internal.h
    #
    src/structs/indi_boolean.c
    src/structs/indi_dict.c
    src/structs/indi_list.c
    src/structs/indi_null.c
    src/structs/indi_number.c
    src/structs/indi_string.c
    #
    src/alloc.c
    src/string_builder.c
    src/json.c
    src/xml.c
    src/indi.c
    src/validation.c
    src/transform_json_to_xml.c
    src/transform_xml_to_json.c
    src/server.c
    #
    src/mongoose/mongoose.c
)

########################################################################################################################

add_library(indi-base-static STATIC ${SOURCE_FILES})

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(indi-base-static PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(indi-base-static PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

target_include_directories(indi-base-static PRIVATE ${LIBXML2_INCLUDE_DIR})

######_link_libraries(indi-base-static ${LIBXML2_LIBRARIES})

set_target_properties(indi-base-static PROPERTIES
    OUTPUT_NAME "indi-base"
)

########################################################################################################################

add_library(indi-base-shared SHARED ${SOURCE_FILES}
        src/indi.c
        src/string_builder.c)

if(HAVE_MALLOC_SIZE)
    target_compile_definitions(indi-base-shared PRIVATE HAVE_MALLOC_SIZE)
endif()

if(HAVE_MALLOC_USABLE_SIZE)
    target_compile_definitions(indi-base-shared PRIVATE HAVE_MALLOC_USABLE_SIZE)
endif()

target_include_directories(indi-base-shared PRIVATE ${LIBXML2_INCLUDE_DIR})

target_link_libraries(indi-base-shared ${LIBXML2_LIBRARIES})

set_target_properties(indi-base-shared PROPERTIES
    OUTPUT_NAME "indi-base"
)

########################################################################################################################

add_executable(test1 test/test1.c)

target_link_libraries(test1 ${LIBXML2_LIBRARIES} indi-base-static)

########################################################################################################################

add_executable(test2 test/test2.c)

target_link_libraries(test2 ${LIBXML2_LIBRARIES} indi-base-static)

########################################################################################################################

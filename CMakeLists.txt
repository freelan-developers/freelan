cmake_minimum_required (VERSION 2.8)

cmake_policy(SET CMP0022 NEW)

project(FreeLAN)

# On Windows third-parties are manually provided.
if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCHITECTURE "x64")
    else()
        set(ARCHITECTURE "x86")
    endif()

    set(THIRD_PARTY_ROOT ${PROJECT_SOURCE_DIR}/third-party/install/${ARCHITECTURE})

    if (NOT BOOST_ROOT)
        set(BOOST_ROOT "${THIRD_PARTY_ROOT}")
    endif()

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)

    # Disable Boost's auto-link feature
    add_definitions(/DBOOST_ALL_NO_LIB)

    # Fixes some behavioral issues with stdint.h
    add_definitions(/D__STDC_LIMIT_MACROS)

    # Let's be cautious instead.
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)

    # TAP adapter identifier.
    set(TAP_ID "tap0901" CACHE STRING "TAP adapter identifier")
    add_definitions(/DTAP_ID=\"${TAP_ID}\")
endif()

find_package(Threads)
find_package(Boost COMPONENTS system date_time regex thread REQUIRED)

if (WIN32 AND MSVC_VERSION EQUAL 1900 AND Boost_VERSION EQUAL 105900)
    # Boost 1.59.0 on VS 2015 has annoying but non-fatal warnings that we can
    # get rid of easily.
    add_definitions(/wd4005)
endif()

# Default install path
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE PATH "Default installation path" FORCE)
endif()

# Version number.
set(FreeLAN_VERSION_MAJOR 3)
set(FreeLAN_VERSION_MINOR 0)
set(FreeLAN_VERSION_PATCH 0)

# Global include directories.
include_directories("${Boost_INCLUDE_DIRS}")

if (WIN32)
    include_directories("${PROJECT_SOURCE_DIR}/extra/msstdint/include")
    add_definitions("-D_WIN32_WINNT=0x0600")
endif()

# Specific compiler flags.
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-uninitialized")
endif()

# On OSX, boost 1.58.0 causes these warnings.
if (APPLE AND ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang")) AND Boost_VERSION EQUAL 105800)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef")
endif()

# The internal library.
set (INTERNAL_VERSION_HEADER "${PROJECT_BINARY_DIR}/src/internal/version.hpp")
file(RELATIVE_PATH INTERNAL_VERSION_HEADER ${PROJECT_SOURCE_DIR} ${INTERNAL_VERSION_HEADER})

configure_file(
    "src/internal/version.hpp.in"
    "${INTERNAL_VERSION_HEADER}"
)

set (INTERNAL_GENERIC_SRC_FILES ${INTERNAL_GENERIC_SRC_FILES}
    src/internal/common.hpp
    src/internal/error.hpp
    src/internal/fscp/constants.hpp
    src/internal/fscp/message.cpp
    src/internal/fscp/message.hpp
    src/internal/fscp/socket.cpp
    src/internal/fscp/socket.hpp
    src/internal/log.cpp
    src/internal/log.hpp
    src/internal/memory.cpp
    src/internal/memory.hpp
    src/internal/perf_counter.cpp
    src/internal/perf_counter.hpp
    src/internal/platform.hpp
    src/internal/stdint.hpp
    src/internal/stream_parsers.cpp
    src/internal/stream_parsers.hpp
    src/internal/tap_adapter/generic_tap_adapter.hpp
    src/internal/tap_adapter/tap_adapter.hpp
    src/internal/traits.hpp
    src/internal/types/ethernet_address.cpp
    src/internal/types/ethernet_address.hpp
    src/internal/types/generic_ip_address.hpp
    src/internal/types/generic_ip_endpoint.hpp
    src/internal/types/generic_ip_prefix_length.hpp
    src/internal/types/generic_ip_route.hpp
    src/internal/types/generic_value_type.hpp
    src/internal/types/host.hpp
    src/internal/types/host_endpoint.hpp
    src/internal/types/hostname.hpp
    src/internal/types/hostname_endpoint.hpp
    src/internal/types/ip_address.hpp
    src/internal/types/ip_endpoint.hpp
    src/internal/types/ip_route.hpp
    src/internal/types/ipv4_address.hpp
    src/internal/types/ipv4_endpoint.cpp
    src/internal/types/ipv4_endpoint.hpp
    src/internal/types/ipv4_prefix_length.hpp
    src/internal/types/ipv4_route.hpp
    src/internal/types/ipv6_address.hpp
    src/internal/types/ipv6_endpoint.cpp
    src/internal/types/ipv6_endpoint.hpp
    src/internal/types/ipv6_prefix_length.hpp
    src/internal/types/ipv6_route.hpp
    src/internal/types/port_number.hpp
    src/internal/types/tap_adapter_layer.hpp
)

if (WIN32)
    set (INTERNAL_WINDOWS_SRC_FILES ${INTERNAL_WINDOWS_SRC_FILES}
        src/internal/tap_adapter/windows/extra/tap-windows.h
        src/internal/tap_adapter/windows/tap_adapter.cpp
        src/internal/tap_adapter/windows/tap_adapter.hpp
        src/internal/windows/registry.hpp
    )
else()
    set (INTERNAL_UNIX_SRC_FILES ${INTERNAL_UNIX_SRC_FILES}
        src/internal/tap_adapter/unix/tap_adapter.cpp
        src/internal/tap_adapter/unix/tap_adapter.hpp
    )
endif()

set (INTERNAL_SRC_FILES ${INTERNAL_SRC_FILES}
    ${INTERNAL_GENERIC_SRC_FILES}
    "${INTERNAL_VERSION_HEADER}"
)

if (WIN32)
    set (INTERNAL_SRC_FILES ${INTERNAL_SRC_FILES}
        ${INTERNAL_WINDOWS_SRC_FILES}
    )
else()
    set (INTERNAL_SRC_FILES ${INTERNAL_SRC_FILES}
        ${INTERNAL_UNIX_SRC_FILES}
    )
endif()

foreach (SRC_FILE ${INTERNAL_SRC_FILES})
    get_filename_component(SRC_FILE_DIR ${SRC_FILE} DIRECTORY)
    file(TO_NATIVE_PATH ${SRC_FILE_DIR} SRC_FILE_DIR)
    source_group(${SRC_FILE_DIR} FILES ${SRC_FILE})
endforeach(SRC_FILE)

add_library(FreeLANInternalLibrary ${INTERNAL_SRC_FILES})
target_include_directories(FreeLANInternalLibrary PUBLIC "${PROJECT_SOURCE_DIR}/src/internal")
target_include_directories(FreeLANInternalLibrary PUBLIC "${PROJECT_BINARY_DIR}/src/internal")
set_target_properties(FreeLANInternalLibrary PROPERTIES PROJECT_LABEL "FreeLAN internal library")
set_target_properties(FreeLANInternalLibrary PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(FreeLANInternalLibrary PROPERTIES OUTPUT_NAME "freelan_internal")

if (WIN32)
    target_link_libraries(FreeLANInternalLibrary PRIVATE "iphlpapi")
endif()

# The main library.
set (API_HEADER_FILES ${API_HEADER_FILES}
    include/freelan.h
    include/freelan/common.h
    include/freelan/debug.h
    include/freelan/error.h
    include/freelan/io.h
    include/freelan/log.h
    include/freelan/memory.h
    include/freelan/types.h
    include/freelan/version.h
)

set (API_SRC_FILES ${API_SRC_FILES}
    src/freelan/debug.cpp
    src/freelan/error.cpp
    src/freelan/io.cpp
    src/freelan/log.cpp
    src/freelan/memory.cpp
    src/freelan/types.cpp
    src/freelan/version.cpp
)

foreach (SRC_FILE ${API_SRC_FILES})
    get_filename_component(SRC_FILE_DIR ${SRC_FILE} DIRECTORY)
    file(TO_NATIVE_PATH ${SRC_FILE_DIR} SRC_FILE_DIR)
    source_group(${SRC_FILE_DIR} FILES ${SRC_FILE})
endforeach(SRC_FILE)

add_library(FreeLANLibrary SHARED ${API_SRC_FILES} ${API_HEADER_FILES})
target_include_directories(FreeLANLibrary INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(FreeLANLibrary PRIVATE "${PROJECT_SOURCE_DIR}/include/freelan")
target_link_libraries(FreeLANLibrary PRIVATE FreeLANInternalLibrary)
target_link_libraries(FreeLANLibrary PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(FreeLANLibrary PUBLIC ${Boost_LIBRARIES})
set_target_properties(FreeLANLibrary PROPERTIES PROJECT_LABEL "FreeLAN library")
set_target_properties(FreeLANLibrary PROPERTIES OUTPUT_NAME "freelan")
set_target_properties(FreeLANLibrary PROPERTIES MACOSX_RPATH OFF)
target_compile_definitions(FreeLANLibrary PRIVATE "FREELAN_API_EXPORTS")

install(TARGETS FreeLANLibrary RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

foreach (HEADER_FILE ${API_HEADER_FILES})
    get_filename_component(HEADER_FILE_DIR ${HEADER_FILE} DIRECTORY)
    install(FILES ${HEADER_FILE} DESTINATION ${HEADER_FILE_DIR})
    file(TO_NATIVE_PATH ${HEADER_FILE_DIR} HEADER_FILE_DIR)
    source_group(${HEADER_FILE_DIR} FILES ${HEADER_FILE})
endforeach(HEADER_FILE)

# The tests
set (GTEST_VERSION "1.7.0")
set (GTEST_ROOT "extra/gtest-${GTEST_VERSION}")

set (TESTS_SRC_FILES ${TESTS_SRC_FILES}
    src/tests/common.hpp
    src/tests/fscp/message.cpp
    src/tests/fscp/socket.cpp
    src/tests/log.cpp
    src/tests/perf_counter.cpp
    src/tests/stream_parsers.cpp
    src/tests/types/ethernet_address.cpp
    src/tests/types/host.cpp
    src/tests/types/host_endpoint.cpp
    src/tests/types/hostname.cpp
    src/tests/types/hostname_endpoint.cpp
    src/tests/types/ip_address.cpp
    src/tests/types/ip_endpoint.cpp
    src/tests/types/ip_route.cpp
    src/tests/types/ipv4_address.cpp
    src/tests/types/ipv4_endpoint.cpp
    src/tests/types/ipv4_prefix_length.cpp
    src/tests/types/ipv4_route.cpp
    src/tests/types/ipv6_address.cpp
    src/tests/types/ipv6_endpoint.cpp
    src/tests/types/ipv6_prefix_length.cpp
    src/tests/types/ipv6_route.cpp
    src/tests/types/port_number.cpp
    src/tests/types/tap_adapter_layer.cpp
)

if (WIN32)
    set (TESTS_SRC_FILES ${TESTS_SRC_FILES}
        src/tests/tap_adapter/windows/tap_adapter.cpp
    )
else()
    set (TESTS_SRC_FILES ${TESTS_SRC_FILES}
        src/tests/tap_adapter/unix/tap_adapter.cpp
    )
endif()

set (GTEST_SRC_FILES ${GTEST_SRC_FILES}
    ${GTEST_ROOT}/src/gtest-all.cc
    ${GTEST_ROOT}/src/gtest_main.cc
)

foreach (SRC_FILE ${TESTS_SRC_FILES} ${GTEST_SRC_FILES})
    get_filename_component(SRC_FILE_DIR ${SRC_FILE} DIRECTORY)
    file(TO_NATIVE_PATH ${SRC_FILE_DIR} SRC_FILE_DIR)
    source_group(${SRC_FILE_DIR} FILES ${SRC_FILE})
endforeach(SRC_FILE)

if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set_property(SOURCE ${GTEST_SRC_FILES} APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-missing-field-initializers")
endif()

add_executable(FreeLANTests ${TESTS_SRC_FILES} ${GTEST_SRC_FILES})
target_include_directories(FreeLANTests PRIVATE "${GTEST_ROOT}")
target_include_directories(FreeLANTests PRIVATE "${GTEST_ROOT}/include")
target_link_libraries(FreeLANTests PRIVATE FreeLANInternalLibrary FreeLANLibrary)
set_target_properties(FreeLANTests PROPERTIES PROJECT_LABEL "FreeLAN tests")
set_target_properties(FreeLANTests PROPERTIES OUTPUT_NAME "freelan_tests")

if (WIN32)
    target_compile_options(FreeLANTests PRIVATE "/wd4805")
endif()

install(TARGETS FreeLANTests DESTINATION bin)

if (WIN32)
    set(PYTHON_TEST_FILES ${PYTHON_TEST_FILES}
        tests/__init__.py
        tests/test_api_error.py
        tests/test_api_io.py
        tests/test_api_log.py
        tests/test_api_types.py
        tests/test_api_version.py
        tests/test_gtest_binary.py
        tests/test_native_calls.py
    )

    foreach (PYTHON_TEST_FILE ${PYTHON_TEST_FILES})
        get_filename_component(PYTHON_TEST_FILE_DIR ${PYTHON_TEST_FILE} DIRECTORY)
        file(TO_NATIVE_PATH ${PYTHON_TEST_FILE_DIR} PYTHON_TEST_FILE_DIR)
        source_group(${PYTHON_TEST_FILE_DIR} FILES ${PYTHON_TEST_FILE})
    endforeach(PYTHON_TEST_FILE)

    add_custom_target(
        PythonTests
        COMMAND ${CMAKE_COMMAND} -E env
        "PATH=${CMAKE_INSTALL_PREFIX}/bin;$ENV{PATH}"
        "INCLUDE=${CMAKE_INSTALL_PREFIX}/include"
        "LIB=${CMAKE_INSTALL_PREFIX}/lib"
        tox -e py35
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        SOURCES ${PYTHON_TEST_FILES}
    )
endif()

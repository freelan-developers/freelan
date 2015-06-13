cmake_minimum_required (VERSION 2.6)

project(FreeLAN)

find_package(Threads)

# Version number.
set(FreeLAN_VERSION_MAJOR 3)
set(FreeLAN_VERSION_MINOR 0)
set(FreeLAN_VERSION_PATCH 0)

# Third-party include paths.
if (WIN32)
    include_directories("${PROJECT_SOURCE_DIR}/extra/msstdint/include")

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCHITECTURE "x64")
    else()
        set(ARCHITECTURE "x86")
    endif()

    set(THIRD_PARTY_ROOT ${PROJECT_SOURCE_DIR}/third-party/install/${ARCHITECTURE})
    string(REGEX REPLACE "/" "\\\\" THIRD_PARTY_ROOT ${THIRD_PARTY_ROOT})
    set(BOOST_SUFFIX "1_57")

    # Debug
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /I${THIRD_PARTY_ROOT}\\Debug\\include")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /I${THIRD_PARTY_ROOT}\\Debug\\include\\boost-${BOOST_SUFFIX}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /LIBPATH:${THIRD_PARTY_ROOT}\\Debug\\lib")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /LIBPATH:${THIRD_PARTY_ROOT}\\Debug\\lib")

    # Release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /I${THIRD_PARTY_ROOT}\\Release\\include")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /I${THIRD_PARTY_ROOT}\\Release\\include\\boost-${BOOST_SUFFIX}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LIBPATH:${THIRD_PARTY_ROOT}\\Release\\lib")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${THIRD_PARTY_ROOT}\\Release\\lib")

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
endif()

# The internal library.
set (INTERNAL_VERSION_HEADER "${PROJECT_BINARY_DIR}/src/internal/version.hpp")

configure_file(
    "src/internal/version.hpp.in"
    "${INTERNAL_VERSION_HEADER}"
)

set (INTERNAL_SRC_FILES ${INTERNAL_SRC_FILES}
    src/internal/common.hpp
    src/internal/error.hpp
    src/internal/generic_ip_address.hpp
    src/internal/generic_ip_endpoint.hpp
    src/internal/generic_ip_prefix_length.hpp
    src/internal/generic_value_type.hpp
    src/internal/hostname.hpp
    src/internal/ipv4_address.hpp
    src/internal/ipv4_endpoint.cpp
    src/internal/ipv4_endpoint.hpp
    src/internal/ipv4_prefix_length.hpp
    src/internal/ipv6_address.hpp
    src/internal/ipv6_endpoint.cpp
    src/internal/ipv6_endpoint.hpp
    src/internal/ipv6_prefix_length.hpp
    src/internal/log.cpp
    src/internal/log.hpp
    src/internal/memory.cpp
    src/internal/memory.hpp
    src/internal/port_number.hpp
    src/internal/stdint.hpp
    src/internal/stream_parsers.cpp
    src/internal/stream_parsers.hpp
    src/internal/traits.hpp
    "${INTERNAL_VERSION_HEADER}"
)
source_group(internal FILES ${INTERNAL_SRC_FILES})

add_library(FreeLANInternalLibrary ${INTERNAL_SRC_FILES})
target_include_directories(FreeLANInternalLibrary PUBLIC "${PROJECT_SOURCE_DIR}/src/internal")
target_include_directories(FreeLANInternalLibrary PUBLIC "${PROJECT_BINARY_DIR}/src/internal")
set_target_properties(FreeLANInternalLibrary PROPERTIES PROJECT_LABEL "FreeLAN internal library")
set_target_properties(FreeLANInternalLibrary PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(FreeLANInternalLibrary PROPERTIES OUTPUT_NAME "freelan_internal")

# The main library.
set (API_HEADER_FILES ${API_HEADER_FILES}
    include/freelan/common.h
    include/freelan/error.h
    include/freelan/log.h
    include/freelan/memory.h
    include/freelan/types.h
    include/freelan/version.h
)
set (API_SRC_FILES ${API_SRC_FILES}
    src/freelan/error.cpp
    src/freelan/log.cpp
    src/freelan/memory.cpp
    src/freelan/types.cpp
    src/freelan/version.cpp
)
source_group(api FILES ${API_HEADER_FILES} ${API_SRC_FILES})

add_library(FreeLANLibrary SHARED ${API_SRC_FILES})
target_include_directories(FreeLANLibrary INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(FreeLANLibrary PRIVATE "${PROJECT_SOURCE_DIR}/include/freelan")
target_link_libraries(FreeLANLibrary PRIVATE FreeLANInternalLibrary)
target_link_libraries(FreeLANLibrary PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(FreeLANLibrary PUBLIC "boost_system")
set_target_properties(FreeLANLibrary PROPERTIES PROJECT_LABEL "FreeLAN library")
set_target_properties(FreeLANLibrary PROPERTIES OUTPUT_NAME "freelan")
target_compile_definitions(FreeLANLibrary PRIVATE "FREELAN_API_EXPORTS")

install(TARGETS FreeLANLibrary DESTINATION lib)
install(FILES ${API_HEADER_FILES} DESTINATION include/freelan)

# The tests
set (GTEST_VERSION "1.7.0")
set (GTEST_ROOT "extra/gtest-${GTEST_VERSION}")

set (TESTS_SRC_FILES ${TESTS_SRC_FILES}
    src/tests/hostname.cpp
    src/tests/ipv4_address.cpp
    src/tests/ipv4_endpoint.cpp
    src/tests/ipv4_prefix_length.cpp
    src/tests/ipv6_address.cpp
    src/tests/ipv6_endpoint.cpp
    src/tests/ipv6_prefix_length.cpp
    src/tests/log.cpp
    src/tests/port_number.cpp
    src/tests/stream_parsers.cpp
)

set (GTEST_SRC_FILES ${GTEST_SRC_FILES}
    ${GTEST_ROOT}/src/gtest-all.cc
    ${GTEST_ROOT}/src/gtest_main.cc
)
source_group(gtest FILES ${GTEST_SRC_FILES})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
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

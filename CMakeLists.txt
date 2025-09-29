cmake_minimum_required(VERSION 3.23)

project(tcb.pointer
    DESCRIPTION "Safer, better pointers for C++20"
    LANGUAGES CXX
    VERSION 0.1.0
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
enable_testing()

option(TCB_POINTER_BUILD_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})
option(TCB_POINTER_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})
option(TCB_POINTER_BUILD_MODULE "Build C++20 module" Off)

add_library(tcb.pointer INTERFACE)
add_library(tcb::pointer ALIAS tcb.pointer)

target_sources(tcb.pointer PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
    FILES include/tcb/pointer.hpp)
target_compile_features(tcb.pointer INTERFACE cxx_std_20)
set_target_properties(tcb.pointer PROPERTIES EXPORT_NAME pointer)

if(TCB_POINTER_BUILD_MODULE)
    add_library(tcb.pointer.module)
    target_sources(tcb.pointer.module PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS module
        FILES module/pointer.cpp
    )
    target_compile_features(tcb.pointer.module PUBLIC cxx_std_20)
    target_link_libraries(tcb.pointer.module PRIVATE tcb::pointer)
    add_library(tcb::pointer::module ALIAS tcb.pointer.module)
endif()

if (TCB_POINTER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (TCB_POINTER_BUILD_TESTS)
    add_subdirectory(tests)
endif()

set(TCB_POINTER_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/tcb.pointer)

install(
    TARGETS tcb.pointer
    EXPORT tcb.pointer-targets
    FILE_SET HEADERS
)

install(
    EXPORT tcb.pointer-targets
    NAMESPACE tcb::
    DESTINATION ${TCB_POINTER_INSTALL_CMAKE_DIR}
)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/tcb.pointer-version.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

if (NOT EXISTS "${PROJECT_BINARY_DIR}/tcb.pointer-config.cmake.in")
  file(WRITE ${PROJECT_BINARY_DIR}/tcb.pointer-config.cmake.in [[
    @PACKAGE_INIT@
    include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake")
  ]])
endif()

configure_package_config_file(
    "${PROJECT_BINARY_DIR}/tcb.pointer-config.cmake.in"
    "${PROJECT_BINARY_DIR}/tcb.pointer-config.cmake"
    INSTALL_DESTINATION ${TCB_POINTER_INSTALL_CMAKE_DIR}
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/tcb.pointer-config.cmake"
        "${PROJECT_BINARY_DIR}/tcb.pointer-version.cmake"
    DESTINATION ${TCB_POINTER_INSTALL_CMAKE_DIR}
)

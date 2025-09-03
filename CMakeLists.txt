cmake_minimum_required(VERSION 3.23)

project(tcb_pointer CXX)

option(TCB_POINTER_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})
option(TCB_POINTER_BUILD_MODULE "Build C++20 module" Off)

if(TCB_POINTER_BUILD_MODULE)
    add_library(tcb_pointer)
else()
    add_library(tcb_pointer INTERFACE)
endif()
add_library(tcb::pointer ALIAS tcb_pointer)

target_sources(tcb_pointer PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
    FILES include/tcb/pointer.hpp)
target_compile_features(tcb_pointer INTERFACE cxx_std_20)

if(TCB_POINTER_BUILD_MODULE)
    target_sources(tcb_pointer PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS module
        FILES module/pointer.cpp
    )
    target_compile_features(tcb_pointer PUBLIC cxx_std_20)
endif()


if (TCB_POINTER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

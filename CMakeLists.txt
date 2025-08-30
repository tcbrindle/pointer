cmake_minimum_required(VERSION 3.23)

project(tcb_pointer CXX)

add_library(tcb_pointer INTERFACE)
target_sources(tcb_pointer INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES pointer.hpp)
target_compile_features(tcb_pointer INTERFACE cxx_std_20)

add_library(tcb::pointer ALIAS tcb_pointer)

option(TCB_POINTER_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})

if (TCB_POINTER_BUILD_TESTS)
    add_subdirectory(test)
endif()

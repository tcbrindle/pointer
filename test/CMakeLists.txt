
add_library(tcb.pointer.test-config INTERFACE)
target_sources(tcb.pointer.test-config INTERFACE
    FILE_SET HEADERS
    BASE_DIRS .
    FILES pointer.config.hpp
)
target_compile_definitions(tcb.pointer.test-config INTERFACE TCB_PTR_CONFIG_HEADER="pointer.config.hpp")
target_link_libraries(tcb.pointer.test-config INTERFACE tcb::pointer)
target_compile_options(tcb.pointer.test-config INTERFACE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU,Intel>:
        -Wall -Wextra -Wconversion -Wsign-conversion -pedantic
        -fno-omit-frame-pointer
    >

    $<$<CXX_COMPILER_ID:MSVC>:
        # Various options for closer standard conformance
        /utf-8 /Zc:__cplusplus /Zc:throwingNew /Zc:inline /Zc:externConstexpr
        /Zc:templateScope /Zc:checkGwOdr /Zc:enumTypes
        /W4
    >
)

add_executable(tcb.pointer.test pointer.test.cpp)
target_link_libraries(tcb.pointer.test PRIVATE tcb.pointer.test-config)
set_target_properties(tcb.pointer.test PROPERTIES CXX_EXTENSIONS Off)
add_test(NAME "Test tcb::pointer" COMMAND tcb.pointer.test)

if(TCB_POINTER_BUILD_MODULE)
    # First, make sure we can import the module target defined in our parent CML
    add_executable(tcb.pointer.test.module_import pointer.module_import.test.cpp)
    target_link_libraries(tcb.pointer.test.module_import PRIVATE tcb::pointer::module)
    set_target_properties(tcb.pointer.test.module_import PROPERTIES CXX_SCAN_FOR_MODULES On)
    add_test(NAME "Test tcb.pointer module import" COMMAND tcb.pointer.test.module_import)

    # Now build the tests again, this time using import rather than include,
    # with our own private version of the module using the test config
    add_executable(tcb.pointer.test.module pointer.test.cpp)
    target_sources(tcb.pointer.test.module PRIVATE
        FILE_SET CXX_MODULES
        BASE_DIRS ../module
        FILES ../module/pointer.cpp
    )
    target_compile_definitions(tcb.pointer.test.module PRIVATE MODULE_BUILD)
    target_link_libraries(tcb.pointer.test.module PRIVATE tcb.pointer.test-config)
    set_target_properties(tcb.pointer.test.module PROPERTIES
        CXX_SCAN_FOR_MODULES On
        CXX_EXTENSIONS Off
    )
    add_test(NAME "Test tcb::pointer with module" COMMAND tcb.pointer.test.module)
endif()
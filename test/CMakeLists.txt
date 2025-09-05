
add_executable(tcb.pointer.test pointer.test.cpp)
target_link_libraries(tcb.pointer.test PRIVATE tcb::pointer)
target_compile_options(tcb.pointer.test PRIVATE
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
add_test(NAME "Test tcb::pointer" COMMAND tcb.pointer.test)

if(TCB_POINTER_BUILD_MODULE)
    add_executable(tcb.pointer.test.module_import pointer.module_import.test.cpp)
    target_link_libraries(tcb.pointer.test.module_import PRIVATE tcb::pointer::module)
    set_target_properties(tcb.pointer.test.module_import PROPERTIES CXX_SCAN_FOR_MODULES On)
    add_test(NAME "Test tcb.pointer module import" COMMAND tcb.pointer.test.module_import)
endif()
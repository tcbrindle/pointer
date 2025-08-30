
add_executable(test_tcb_pointer test_pointer.cpp)
target_link_libraries(test_tcb_pointer PUBLIC tcb_pointer)

target_compile_options(test_tcb_pointer PUBLIC
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

add_test(NAME "Test tcb::pointer" COMMAND test_tcb_pointer)
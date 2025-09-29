
function(add_example NAME SOURCE)
    add_executable(${NAME} ${SOURCE})

    if (TCB_POINTER_BUILD_MODULE)
        set_target_properties(${NAME} PROPERTIES CXX_SCAN_FOR_MODULES On)
        target_link_libraries(${NAME} PRIVATE tcb::pointer::module)
        target_compile_definitions(${NAME} PRIVATE IMPORT_MODULE)
    else()
        target_link_libraries(${NAME} PRIVATE tcb::pointer)
    endif()

    add_test(NAME ${NAME} COMMAND ${NAME})

endfunction()

add_example(tcb.pointer.example_01 01_construction.cpp)
add_example(tcb.pointer.example_02 02_usage.cpp)
add_example(tcb.pointer.example_03 03_conversions.cpp)
add_example(tcb.pointer.example_04 04_void_pointers.cpp)
add_example(tcb.pointer.example_05 05_array_pointer_construction.cpp)
add_example(tcb.pointer.example_06 06_slices.cpp)

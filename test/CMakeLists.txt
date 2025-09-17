
add_executable(tcb.pointer.test)
target_sources(tcb.pointer.test
    PRIVATE
        pointer.test.cpp

    PRIVATE
        FILE_SET HEADERS
        FILES pointer.config.hpp
)
target_link_libraries(tcb.pointer.test PRIVATE tcb::pointer)
target_compile_definitions(tcb.pointer.test PRIVATE TCB_PTR_CONFIG_HEADER="pointer.config.hpp")
add_test(NAME "Test tcb::pointer" COMMAND tcb.pointer.test)

if(TCB_POINTER_BUILD_MODULE)
    # First, make sure we can import the module target defined in our parent CML
    add_executable(tcb.pointer.test.module_import pointer.module_import.test.cpp)
    target_link_libraries(tcb.pointer.test.module_import PRIVATE tcb::pointer::module)
    set_target_properties(tcb.pointer.test.module_import PROPERTIES CXX_SCAN_FOR_MODULES On)
    add_test(NAME "Test tcb.pointer module import" COMMAND tcb.pointer.test.module_import)

    # Now build the tests again, this time using import rather than include,
    # with our own private version of the module using the test config header
    add_executable(tcb.pointer.test.module)
    target_sources(tcb.pointer.test.module
        PRIVATE
            pointer.test.cpp

        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS ../module
            FILES ../module/pointer.cpp

        PRIVATE
            FILE_SET HEADERS
            FILES pointer.config.hpp
    )
    target_compile_definitions(tcb.pointer.test.module PRIVATE
        MODULE_BUILD
        TCB_PTR_CONFIG_HEADER="pointer.config.hpp"
    )
    target_link_libraries(tcb.pointer.test.module PRIVATE tcb::pointer)
    set_target_properties(tcb.pointer.test.module PROPERTIES CXX_SCAN_FOR_MODULES On)
    add_test(NAME "Test tcb::pointer with module" COMMAND tcb.pointer.test.module)
endif()
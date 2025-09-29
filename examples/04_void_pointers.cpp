
#include <typeinfo> // Required for GCC module build 🤷🏻‍♂️

#ifdef IMPORT_MODULE
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

void void_pointers()
{
    int i = 0;

    // Any pointer-to-object can be implicitly converted to a pointer-to-void
    tcb::pointer<void> pv = tcb::pointer_to_mut(i);

    // Note that const-ness must be preserved: conversion from
    // pointer<const T> to pointer</*non-const*/ void> is an error
    // (try changing this to tcb::pointer<void>)
    tcb::pointer<void const> pcv = tcb::pointer_to(i);

    // In order to do anything useful with a void pointer, you need to
    // explicitly convert it back into an object pointer:
    auto p_int = static_cast<tcb::pointer<int>>(pv);

    // You can also explicitly convert directly to a raw pointer:
    int* raw_ptr = static_cast<int*>(pv);

    // tcb::pointer<void> performs a runtime check to ensure you are
    // casting back to the correct type, in the same way as std::any.
    // Try uncommenting this line and running the program to see the error:
    // [[maybe_unused]] auto error = tcb::pointer<float>(pv);

    // (Avoid compiler warnings by "using" variables)
    [](auto&...) { }(pv, pcv, p_int, raw_ptr);
}

int main() { void_pointers(); }
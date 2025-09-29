// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <memory>

#ifdef IMPORT_MODULE
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

void pointer_construction()
{
    int i = 0;

    // To create a tcb::pointer to an object, we can use the pointer_to() static method
    tcb::pointer<int> p1 = tcb::pointer<int>::pointer_to(i);

    // Of course, we might want to create a pointer-to-const instead
    // (and use CTAD with the return type)
    tcb::pointer p2 = tcb::pointer<int const>::pointer_to(i);

    // A static class method is a lot of typing, so there's a free function
    // tcb::pointer_to(obj) that we can use instead. Note that this function
    // will return a pointer-to-const:
    auto p3 = tcb::pointer_to(i);
    static_assert(std::same_as<decltype(p2), tcb::pointer<int const>>);

    // To create a pointer-to-mutable, we can use tcb::pointer_to_mut():
    auto p4 = tcb::pointer_to_mut(i);

    // Calling pointer_to_mut() on a const object is a compile error
    // (try uncommenting these lines)
    // int const c = 1;
    // auto error = tcb::pointer_to_mut(c);

    // If you're not a fan of typing, there are some shortened aliases:
    tcb::ptr<int const> p5 = tcb::ptr_to(i);
    tcb::ptr<int> p6 = tcb::ptr_to_mut(i);

    // Alternatively you can use std::pointer_traits, if you're not
    // into the whole brevity thing
    auto p7 = std::pointer_traits<tcb::pointer<int>>::pointer_to(i);

    // You can convert a raw pointer to a tcb::pointer using from_address():
    int* r = std::addressof(i);
    auto p8 = tcb::pointer<int>::from_address(r);

    // Because tcb::pointers have no null state, from_address() will
    // perform a runtime check to make sure it has not been passed NULL.
    // Try uncommenting the following lines and running the program to
    // see the error:
    // r = nullptr;
    // [[maybe_unused]] auto p9 = tcb::pointer<int>::from_address(r);

    // Going in the other direction, we can convert a tcb::pointer<T> to a T*
    // using various forms of to_address():
    int* r1 = p1.to_address();
    int* r2 = tcb::to_address(p1);
    int* r3 = std::to_address(p1);

    // We can alternatively use an explicit conversion to go from
    // pointer<T> to T*:
    auto r4 = static_cast<int*>(p1);
    auto r5 = (int*)(p1);
    int* r6(p1);

    // (Avoid compiler warnings by "using" variables)
    [](auto&...) { }(p1, p2, p3, p4, p5, p6, p7, p8, r1, r2, r3, r4, r5, r6);
}

int main() { pointer_construction(); }
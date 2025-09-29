// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <compare>
#include <vector>

#ifdef IMPORT_MODULE
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

void pointer_usage()
{
    int i = 0;
    int j = 1;

    // Copying and assignment work just like you'd expect
    auto p1 = tcb::ptr_to(i);
    auto p2 = p1;
    p1 = tcb::ptr_to(j);

    // Pointers of the same type can be compared for equality
    bool eq = p1 == p2 ? true : false;

    // They also have a total ordering (even if not pointing into the same array)
    bool lt = p1 < p2 ? true : false;
    std::strong_ordering ord = p1 <=> p2;

    // Of course, we can use operator* and operator-> for dereferencing
    auto vec = std::vector{1, 2, 3};
    auto p_vec = tcb::ptr_to_mut(vec);

    (*p_vec).at(0) = 99;
    p_vec->at(1) = 100;

    // However, tcb::pointer **DOES NOT** provide any arithmetic operations!
    // The following will not work:
    // ++p1;
    // p1++;
    // auto p3 = p1 + 1;
    // auto& ref = p1[0];

    // (Avoid compiler warnings by "using" variables)
    [](auto&...) { }(eq, lt, ord);
}

int main() { pointer_usage(); }
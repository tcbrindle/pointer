// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <optional>

import tcb.pointer;

int main()
{
    // Ensure we can use basic pointer functions
    int i = 0;
    auto p1 = tcb::pointer_to(i);
    if (*p1 != 0) {
        return -1;
    }

    // Ensure we can cast to void ptr and back
    auto p2 = tcb::pointer<void const>(p1);
    auto p3 = tcb::pointer<int const>(p2);
    if (p3 != p1) {
        return -1;
    }

    // Ensure our optional specialisation is being picked up
    std::optional opt = p1;
    static_assert(sizeof(opt) == sizeof(int*));
    if (**opt != 0) {
        return 1;
    }
}
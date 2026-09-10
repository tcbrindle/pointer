// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <cassert>
#include <numeric>
#include <ranges>
#include <vector>

#ifdef IMPORT_MODULE
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

void unchecked_slices()
{
    std::vector vec{1, 2, 3, 4, 5};
    tcb::ptr<int[]> ptr = tcb::ptr_to_mut(vec);

    auto& slice = *ptr;

    // As we saw in the last example, operations on slices are
    // *bounds checked*, so for example asking for element 100
    // of a slice of 5 elements is a runtime error. Try
    // uncommenting this line:
    // [[maybe_unused]] auto crash = slice[100];

    // Bounds checking can add some overhead, so we might want
    // to avoid the checks in specific situations where we're
    // sure we won't be going out of bounds. For this purpose,
    // tcb::slice<T> has a public member named `unchecked`.
    // This provides the same operations as `slice`, but as
    // the name suggests, omits bounds checks.
    //
    // For example, we can use it to get the first element of a
    // slice without first checking whether the slice is empty:
    auto front = slice.unchecked.front();

    // We can use the subscript operator of an unchecked slice
    // like so:
    auto with_check = slice[1];
    auto without_check = slice.unchecked[1];

    // Slice iterators are bounds checked, but if we want
    // we can explicitly use unchecked iterators instead:
    auto sum = std::accumulate(slice.unchecked.begin(), slice.unchecked.end(), 0);

    // We can call range algorithms on the unchecked slice as well
    std::ranges::for_each(slice.unchecked, [](int) {
        // Look ma, no bounds checks
    });

    // The default, bounds-checked slice should be what you use most of
    // the time. But having `unchecked` available means you can explicitly
    // omit bounds checks in specific places where they might cause unacceptable
    // overhead, without sacrificing checks elsewhere in the program.

    [](auto&...) { }(vec, ptr, slice, front, with_check, without_check, sum);
}

int main() { unchecked_slices(); }
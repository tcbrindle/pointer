// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <vector>

#ifdef IMPORT_MODULE
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

void array_pointer_construction()
{
    int array1[] = {1, 2, 3, 4, 5};

    // The type tcb::pointer<T[]> has a special meaning: it behaves like
    // a pointer to an array of T with *runtime* size.
    // The type tcb::array_pointer<T> is an alias for tcb::pointer<T[]>.

    // We can create one using the pointer<T[]>::pointer_to() static function,
    // and passing a reference to a contiguous range:
    auto p1 = tcb::pointer<int[]>::pointer_to(array1);

    // Alternatively, we can use the free functions pointer_to_array() or
    // pointer_to_mut_array()
    auto p2 = tcb::pointer_to_array(array1);
    auto p3 = tcb::pointer_to_mut_array(array1);

    // A tcb::pointer<R>, where R is a contiguous range whose elements are of
    // type E, can be converted to a tcb::pointer<E[]>
    std::vector<float> vec{100.0f, 200.0f, 300.0f};
    tcb::ptr<float[]> p4 = tcb::ptr_to_mut(vec);

    // We can also construct a tcb::pointer<T[]> using a raw pointer to
    // the first element of an array and the number of elements, using
    // the from_address_with_size() static function.
    // Important: this is an unsafe function, because it is not possible to
    // check whether the size argument is valid, and it is undefined behaviour
    // if the array does not contains at least the given number of elements
    // Be careful!
    auto p5 = tcb::pointer<int[]>::from_address_with_size(array1, 3);

    // (Avoid compiler warnings by "using" variables)
    [](auto&...) { }(p1, p2, p3, p4, p5);
}

int main() { array_pointer_construction(); }
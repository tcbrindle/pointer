
#include <algorithm>
#include <cassert>
#include <ranges>
#include <vector>

#ifdef IMPORT_MODULE
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

void slices()
{
    std::vector vec{1, 2, 3, 4, 5};
    tcb::ptr<int[]> ptr = tcb::ptr_to_mut(vec);

    // Dereferencing an array pointer yields a *reference* to a tcb::slice<T>
    auto& slice = *ptr;

    // A slice is a sized, contiguous range with all the usual member
    // functions that you'd expect.
    //
    // For example, you can use it with a range-for loop
    for (int i : slice) {
        // Do something with i
        (void)i;
    }

    // Or use reverse iterators with a stdlib algorithm
    std::sort(slice.rbegin(), slice.rend());

    // Or create a view pipeline
    auto view = *ptr | std::views::filter([](int) { return true; })
        | std::views::transform(std::identity{});

    // Slices are *bounds checked*.
    // Using operator[] to try to access an out-of-bounds element
    // (or trying to use front() or back() on an empty slice) will
    // raise a runtime error.
    // Uncomment these lines and run the code to see the error:
    // auto& oob = *ptr;
    // oob[1'000] = 0;

    // Slice *iterators* are bounds checked by default as well.
    // This means that trying to use an iterator which
    // would point to an invalid location will be a runtime error:
    // [[maybe_unused]] auto error1 = *slice.end();
    // [[maybe_unused]] auto error2 = *(slice.begin() - 1'000);

    // With slices, constness is "deep" -- meaning it works
    // exactly the way you'd want it to.
    // If you have a non-const slice then you can mutate its
    // elements. If you have a const slice then you cannot mutate
    // its elements. Simple.
    tcb::slice<int>& mut_slice = *ptr;
    mut_slice[0] = 100; // Okay
    tcb::slice<int> const& const_slice = *ptr;
    // const_slice[0] = 100; // Error!

    // slices are *non-copyable* and *non-movable*. This means
    // that you cannot use them by value, only by reference:
    auto& okay = *ptr;
    // auto error = *ptr;

    // If you want to copy the elements of a slice, you can
    // use C++23's ranges::to(), or vector's range constructor
    // if you aren't using C++23 yet.
    auto copy = std::vector(slice.begin(), slice.end());

    // Slices are equality comparable when their element types are
    // Comparison is *element-wise* (like std::vector) -- that is,
    // two slices are equal if they have the same number of elements,
    // and each pair of elements compares equal
    std::vector vec1{1, 2, 3, 4, 5};
    std::vector vec2{1, 2, 3, 4, 5};
    tcb::ptr<int const[]> ptr1 = tcb::ptr_to(vec1);
    tcb::ptr<int const[]> ptr2 = tcb::ptr_to(vec2);

    assert(ptr1 != ptr2); // point to different vectors
    assert(*ptr1 == *ptr2); // elements are equal

    // Slices are ordered when their element types are ordered
    // Comparison is lexicographical, i.e. each pair of elements is
    // compared in turn. If elements are equal but one slice is shorter
    // than the other, it is considered "less than" the longer slice
    {
        float shorter[] = {1.0f, 2.0f, 3.0f};
        float longer[] = {1.0f, 2.0f, 3.0f, 4.0f};

        tcb::ptr<float const[]> p_shorter = tcb::ptr_to(shorter);
        tcb::ptr<float const[]> p_longer = tcb::ptr_to(longer);

        assert(*p_shorter < *p_longer);
        assert(*p_shorter <=> *p_longer == std::partial_ordering::less);
    }

    [](auto&...) { }(view, const_slice, okay, copy, ptr1, ptr2);
}

int main() { slices(); }
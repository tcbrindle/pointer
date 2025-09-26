
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef MODULE_BUILD
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

/*
 * MARK: Test machinery
 */

// Clang 19 and earlier with libstdc++ seems to have a bug when
// using spaceship with pointers in constexpr
#if defined(__clang_major__) && defined(__GLIBCXX__)
constexpr bool clang19_with_libstdcxx = (__clang_major__ < 20);
#else
constexpr bool clang19_with_libstdcxx = false;
#endif

struct test_failure : std::runtime_error {
    using std::runtime_error::runtime_error;
};

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define REQUIRE(...)        \
    if (!(__VA_ARGS__))     \
        throw test_failure( \
            __FILE__ ":" STRINGIFY(__LINE__) ": Test \"" STRINGIFY(__VA_ARGS__) "\" failed");

#define REQUIRE_THROWS_AS(type, ...)                                                   \
    do {                                                                               \
        bool caught = false;                                                           \
        try {                                                                          \
            (void)__VA_ARGS__;                                                         \
        } catch (type const&) {                                                        \
            caught = true;                                                             \
        } catch (...) {                                                                \
            throw test_failure(__FILE__ ":" STRINGIFY(__LINE__) ": Test \"" STRINGIFY( \
                __VA_ARGS__) "\" threw an exception of unexpected type");              \
        }                                                                              \
        if (!caught) {                                                                 \
            throw test_failure(__FILE__ ":" STRINGIFY(__LINE__) ": Test \"" STRINGIFY( \
                __VA_ARGS__) "\" did not throw an exception when one was expected");   \
        }                                                                              \
    } while (0)

#define REQUIRE_ERROR(...) REQUIRE_THROWS_AS(std::runtime_error, __VA_ARGS__)

/*
 * MARK: Test types
 */
struct IncompleteClass;
struct BaseClass {
    virtual char fn() const { return 'B'; };
    virtual ~BaseClass() = default;
};
struct DerivedClass : BaseClass {
    char fn() const override { return 'D'; }
};
union Union {
};
union IncompleteUnion;
enum Enum { };
enum class EnumClass { };
enum IncompleteEnum : int;
enum class IncompleteEnumClass;

struct ConvertibleToIntPtr {
    operator int*() const;
};

// Multiple inheritance types
namespace MI {

struct A {
    virtual ~A() = default;
};

struct B : A { };
struct C : A { };
struct D : B, C { };

} // namespace MI

/*
 * MARK: Static tests
 */

template <typename T>
constexpr bool test_pointer_static_properties()
{
    using P = tcb::pointer<T>;

    // I know arrays of unknown bound are technically objects, but...
    constexpr bool is_object = std::is_object_v<T> && !std::is_unbounded_array_v<T>;

    // pointer to object is the same size as T*
    if constexpr (is_object) {
        static_assert(sizeof(P) == sizeof(T*));
    }

    static_assert(not std::is_default_constructible_v<P>);
    static_assert(not std::default_initializable<P>);

    // pointer<T> is copyable, movable, etc (type traits)
    static_assert(std::is_copy_constructible_v<P>);
    static_assert(std::is_move_constructible_v<P>);
    static_assert(std::is_copy_assignable_v<P>);
    static_assert(std::is_move_assignable_v<P>);
    static_assert(std::is_destructible_v<P>);
    static_assert(std::is_swappable_v<P>);

    // pointer<T> is copyable, movable, etc (concepts)
    static_assert(std::copyable<P>);
    static_assert(std::movable<P>);
    static_assert(std::destructible<P>);
    static_assert(std::swappable<P>);

    // pointer<T> special members are all trivial
    // (except array pointers, annoyingly)
    if constexpr (not std::is_unbounded_array_v<T>) {
        static_assert(std::is_trivially_copyable_v<P>);
        static_assert(std::is_trivially_copy_constructible_v<P>);
        static_assert(std::is_trivially_move_constructible_v<P>);
        static_assert(std::is_trivially_copy_assignable_v<P>);
        static_assert(std::is_trivially_move_assignable_v<P>);
    }
    static_assert(std::is_trivially_destructible_v<P>);

    // pointer<T> special members are all noexcept
    static_assert(std::is_nothrow_copy_constructible_v<P>);
    static_assert(std::is_nothrow_move_constructible_v<P>);
    static_assert(std::is_nothrow_copy_assignable_v<P>);
    static_assert(std::is_nothrow_move_assignable_v<P>);
    static_assert(std::is_nothrow_destructible_v<P>);

    // pointer<T> is equality comparable and strongly totally ordered
    static_assert(std::equality_comparable<P>);
    static_assert(std::totally_ordered<P>);
    static_assert(std::three_way_comparable<P, std::strong_ordering>);

    // pointer<T> is explicitly (but not implicitly) convertible to bool
    static_assert(not std::is_convertible_v<P, bool>);
    static_assert(requires(P& p) {
        { static_cast<bool>(p) };
        { p ? 1 : 0 };
    });

    // pointer_to object is explicitly but not implicitly convertible to T*
    static_assert(not std::is_convertible_v<P, T*>);
    if constexpr (is_object) {
        static_assert(requires(P& p) {
            { static_cast<T*>(p) } -> std::same_as<T*>;
        });
    }

    // P::to_address() returns the correct type for non-arrays
    if constexpr (!std::is_unbounded_array_v<T>) {
        static_assert(std::same_as<decltype(std::declval<P&>().to_address()), T*>);
    }

    // pointer to object dereferences to the correct type
    if constexpr (is_object) {
        static_assert(std::same_as<decltype(*std::declval<P&>()), T&>);
        static_assert(std::same_as<decltype(std::declval<P&>().operator->()), T*>);
    }

    // std::pointer_traits tests
    {
        using Traits = std::pointer_traits<P>;
        static_assert(std::same_as<typename Traits::pointer, P>);
        if constexpr (!std::is_unbounded_array_v<T>) {
            static_assert(std::same_as<typename Traits::element_type, T>);
        }
        static_assert(std::same_as<typename Traits::difference_type, std::ptrdiff_t>);
        static_assert(std::same_as<typename Traits::template rebind<int>, tcb::pointer<int>>);
    }

    return true;
}
static_assert(test_pointer_static_properties<int>());
static_assert(test_pointer_static_properties<int const>());
static_assert(test_pointer_static_properties<std::vector<int>>()); // non-trivial type
static_assert(test_pointer_static_properties<std::vector<int> const>());
static_assert(test_pointer_static_properties<BaseClass>()); // polymorphic type
static_assert(test_pointer_static_properties<BaseClass const>());
static_assert(test_pointer_static_properties<DerivedClass>());
static_assert(test_pointer_static_properties<DerivedClass const>());
static_assert(test_pointer_static_properties<IncompleteClass>()); // incomplete type
static_assert(test_pointer_static_properties<IncompleteClass const>());
static_assert(test_pointer_static_properties<Union>());
static_assert(test_pointer_static_properties<Union const>());
static_assert(test_pointer_static_properties<IncompleteUnion>());
static_assert(test_pointer_static_properties<IncompleteUnion const>());
static_assert(test_pointer_static_properties<Enum>());
static_assert(test_pointer_static_properties<Enum const>());
static_assert(test_pointer_static_properties<EnumClass>());
static_assert(test_pointer_static_properties<IncompleteEnum>());
static_assert(test_pointer_static_properties<IncompleteEnum const>());
static_assert(test_pointer_static_properties<IncompleteEnumClass>());
static_assert(test_pointer_static_properties<IncompleteEnumClass const>());
static_assert(test_pointer_static_properties<int[5]>()); // array of known bound
static_assert(test_pointer_static_properties<int const[5]>());
static_assert(test_pointer_static_properties<int[2][3]>()); // array of array of known bound
static_assert(test_pointer_static_properties<int const[2][3]>());
static_assert(test_pointer_static_properties<int*>()); // pointer to raw pointer
static_assert(test_pointer_static_properties<int const*>()); // pointer to raw pointer to const
static_assert(test_pointer_static_properties<int* const>()); // pointer to const raw pointer
static_assert(
    test_pointer_static_properties<int const* const>()); // pointer to const raw pointer to const
static_assert(test_pointer_static_properties<tcb::ptr<int>>()); // pointer to tcb::pointer
static_assert(
    test_pointer_static_properties<tcb::ptr<int const>>()); // pointer to tcb::pointer to const
static_assert(
    test_pointer_static_properties<tcb::ptr<int> const>()); // pointer to const tcb::pointer
static_assert(test_pointer_static_properties<tcb::ptr<int const> const>()); // pointer to const
                                                                            // tcb::pointer to const

static_assert(test_pointer_static_properties<void>());
static_assert(test_pointer_static_properties<void const>());
static_assert(test_pointer_static_properties<void volatile>()); // why not
static_assert(test_pointer_static_properties<void const volatile>());

static_assert(test_pointer_static_properties<int[]>()); // array of unknown bound
static_assert(test_pointer_static_properties<int const[]>());
static_assert(test_pointer_static_properties<int[][5]>());
static_assert(test_pointer_static_properties<int const[][5]>());

/*
 * MARK: pointer_to() tests
 */

constexpr bool test_pointer_to()
{
    using namespace tcb;

    // pointer_to() is not callable with rvalues, returns a pointer-to-const
    {
        using F = decltype((pointer_to));
        static_assert(std::invocable<F, int&>);
        static_assert(std::invocable<F, int const&>);
        static_assert(not std::invocable<F, int>);
        static_assert(not std::invocable<F, int&&>);
        static_assert(not std::invocable<F, int const&&>);

        int i = 0;
        const int c = 0;

        static_assert(std::is_same_v<decltype(pointer_to(i)), pointer<int const>>);
        static_assert(std::is_same_v<decltype(pointer_to(c)), pointer<int const>>);
    }

    // pointer_to pointer works as expected, with all combinations of const and mutable
    {
        int i = 0;

        auto mut = pointer_to_mut(i);
        auto const_ = pointer_to(i);

        auto mut_to_mut = pointer_to_mut(mut);
        auto const_to_mut = pointer_to(mut);
        auto mut_to_const = pointer_to_mut(const_);
        auto const_to_const = pointer_to(const_);

        static_assert(std::same_as<decltype(mut_to_mut), ptr<ptr<int>>>);
        static_assert(std::same_as<decltype(const_to_mut), ptr<ptr<int> const>>);
        static_assert(std::same_as<decltype(mut_to_const), ptr<ptr<int const>>>);
        static_assert(std::same_as<decltype(const_to_const), ptr<ptr<int const> const>>);

        **mut_to_mut = 99;
        REQUIRE(i == 99);
        **const_to_mut = 100;
        REQUIRE(i == 100);
        // These should not compile
        // **mut_to_const = 101;
        // **const_to_const = 102;
    }

    // pointer_to_mut() is not callable with rvalues or const lvalues, returns a
    // pointer-to-non-const
    {
        using F = decltype((pointer_to_mut));
        static_assert(std::invocable<F, int&>);
        static_assert(not std::invocable<F, int const&>);
        static_assert(not std::invocable<F, int>);
        static_assert(not std::invocable<F, int&&>);
        static_assert(not std::invocable<F, int const&&>);

        int i = 0;
        static_assert(std::is_same_v<decltype(pointer_to_mut(i)), pointer<int>>);
    }

    return true;
}
static_assert(test_pointer_to());

/*
 * MARK: Object ptr tests
 */

constexpr bool test_pointer_to_object()
{
    using namespace tcb;

    // Basic pointer<T>
    {
        int i = 0;

        auto p = pointer_to_mut(i);

        // member to_address() returns the right address
        REQUIRE(p.to_address() == std::addressof(i));

        // so does free to_address()
        REQUIRE(to_address(p) == std::addressof(i));

        // so does std::to_address()
        REQUIRE(std::to_address(p) == std::addressof(i));

        // explicit cast to int* works correctly
        REQUIRE(static_cast<int*>(p) == std::addressof(i));

        // bool contextual conversion works as expected
        REQUIRE(p);

        // dereferencing works correctly
        REQUIRE(*p == 0);
        *p = 1;
        REQUIRE(i == 1);

        REQUIRE(p.operator->() == std::addressof(i));
    }

    // Derived-to-base works
    if (!std::is_constant_evaluated()) {
        DerivedClass d;
        ptr<BaseClass> pb = pointer_to_mut(d);

        // Virtual call works, not somehow slicing
        REQUIRE(pb->fn() == 'D');
    }

    // from_address()
    {
        // The following should not compile (try uncommenting them)
        // ptr<int>::from_address(nullptr);
        // ptr<int>::from_address(0);
        // ptr<int>::from_address(NULL);
        // ptr<int>::from_address((float*)nullptr);
        // ptr<int>::from_address((void*)nullptr);
        // ptr<int>::from_address((int const*)nullptr);
        // ptr<int>::from_address(ConvertibleToIntPtr{});

        auto make_ptr = [](auto p) -> decltype(ptr<int>::from_address(p)) {
            return ptr<int>::from_address(p);
        };
        using F = decltype(make_ptr);
        static_assert(std::invocable<F, int*>);
        static_assert(not std::invocable<F, std::nullptr_t>);
        static_assert(not std::invocable<F, int>);
        static_assert(not std::invocable<F, decltype(NULL)>);
        static_assert(not std::invocable<F, float*>);
        static_assert(not std::invocable<F, void*>);
        static_assert(not std::invocable<F, int const*>);
        static_assert(not std::invocable<F, ConvertibleToIntPtr>);

        int i = 0;

        int* r = &i;
        auto p1 = ptr<int>::from_address(r);
        REQUIRE(p1.to_address() == r);

        auto p2 = ptr<int const>::from_address(r);
        REQUIRE(p2.to_address() == r);

        if (!std::is_constant_evaluated()) {
            REQUIRE_ERROR(ptr<int>::from_address((int*)nullptr));
        }

        DerivedClass d;
        auto pb = ptr<BaseClass>::from_address(&d);
        REQUIRE(pb.to_address() == &d);

        {
            MI::D obj;
            // Should NOT compile, A base is ambiguous
            // auto p = tcb::ptr<MI::A>::from_address(&obj);
            // Okay:
            [[maybe_unused]] auto p3 = tcb::ptr<MI::A>::from_address(&static_cast<MI::B&>(obj));
            [[maybe_unused]] auto p4 = tcb::ptr<MI::A>::from_address(static_cast<MI::C*>(&obj));
        }
    }

    // Comparisons
    if constexpr (!clang19_with_libstdcxx) {
        std::array arr{1, 2, 3, 4, 5};

        auto p0 = tcb::pointer_to(arr[0]);
        auto p4 = tcb::pointer_to(arr[4]);

        REQUIRE(p0 == p0);
        REQUIRE(p0 != p4);

        REQUIRE(p0 <=> p0 == std::strong_ordering::equal);
        REQUIRE(p0 <=> p4 == std::strong_ordering::less);
        REQUIRE(p4 <=> p0 == std::strong_ordering::greater);

        // Everything else is generated from <=>
        REQUIRE(p0 < p4);
        REQUIRE(p4 > p0);
    }

    // std::pointer_traits (weirdly, not constexpr)
    if (!std::is_constant_evaluated()) {
        // non-const
        {
            using Traits = std::pointer_traits<ptr<int>>;

            int i = 0;

            auto p = Traits::pointer_to(i);
            static_assert(std::same_as<decltype(p), ptr<int>>);

            auto p3 = std::to_address(p);
            static_assert(std::same_as<decltype(p3), int*>);
            REQUIRE(std::to_address(p) == std::addressof(i));
        }

        // const
        {
            using Traits = std::pointer_traits<ptr<int const>>;

            int i = 0;

            auto p = Traits::pointer_to(i);
            static_assert(std::same_as<decltype(p), ptr<int const>>);

            auto p3 = std::to_address(p);
            static_assert(std::same_as<decltype(p3), int const*>);
            REQUIRE(std::to_address(p) == std::addressof(i));
        }
    }

    return true;
}
static_assert(test_pointer_to_object());

/*
 * MARK: void ptr tests
 */

static bool test_pointer_to_void()
{
    using namespace tcb;

    // pointer<void>
    {
        int i = 0;

        auto p = tcb::pointer_to_mut(i);

        // Can convert lvalue pointer<int> to pointer<void>
        static_assert(std::convertible_to<ptr<int>, ptr<void>>);
        pointer<void> v = p;
        REQUIRE(v.to_address() == std::addressof(i));

        // Can convert rvalue pointer<int> to pointer<void>
        pointer<void> v2 = tcb::pointer_to_mut(i);
        REQUIRE(v2.to_address() == std::addressof(i));

        // *Cannot* convert pointer<int const> to pointer<void>
        static_assert(not std::convertible_to<ptr<int const>, ptr<void>>);
        // ptr<void> v3 = tcb::addr(i);

        // ptr<void> can be explicitly converted back to ptr<original-type>
        ptr<int> p2 = static_cast<ptr<int>>(v);
        REQUIRE(p2.to_address() == std::addressof(i));

        // ptr<void> can be explicitly converted back to original-type*
        int* p3 = static_cast<int*>(v.to_address());
        REQUIRE(p3 == std::addressof(i));

        // Converting ptr<void> to pointer of the wrong type is a runtime error
        REQUIRE_ERROR(static_cast<ptr<float>>(v));

        // Can increase const-ness in conversion from ptr<void>
        ptr<int const> p4 = static_cast<ptr<int const>>(v);
        REQUIRE(p4.to_address() == std::addressof(i));
    }

    // pointer<void const>
    {
        int const i = 0;

        auto p = tcb::pointer_to(i);

        // Can convert lvalue pointer<int> to pointer<void>
        static_assert(std::convertible_to<ptr<int const>, ptr<void const>>);
        pointer<void const> v = p;
        REQUIRE(v.to_address() == std::addressof(i));

        // Can convert rvalue pointer<int> to pointer<void>
        pointer<void const> v2 = tcb::pointer_to(i);
        REQUIRE(v2.to_address() == std::addressof(i));

        // ptr<void const> can be explicitly converted back to ptr<original-type>
        ptr<int const> p2 = static_cast<ptr<int const>>(v);
        REQUIRE(p2.to_address() == std::addressof(i));

        // ptr<void> can be explicitly converted back to original-type*
        int const* p3 = static_cast<int const*>(v.to_address());
        REQUIRE(p3 == std::addressof(i));

        // Converting ptr<void> to pointer of the wrong type is a runtime error
        REQUIRE_ERROR(static_cast<ptr<float const>>(v));

        // Cannot remove const in conversion from ptr<void const>
        static_assert(not std::convertible_to<ptr<void const>, ptr<int>>);
        // ptr<int> p4 = static_cast<ptr<int>>(v);
    }

    // std::to_address
    {
        // non-const
        {
            int i = 0;
            auto p = ptr<void>::pointer_to(i);

            auto p2 = std::to_address(p);
            static_assert(std::same_as<decltype(p2), void*>);
            REQUIRE(std::to_address(p) == std::addressof(i));
        }

        // const
        {
            int i = 0;
            auto p = ptr<void const>::pointer_to(i);

            auto p2 = std::to_address(p);
            static_assert(std::same_as<decltype(p2), void const*>);
            REQUIRE(std::to_address(p) == std::addressof(i));
        }
    }

    return true;
}

/*
 * MARK: Checked iter tests
 */

// checked_iterator is not exported from the module
template <typename T>
using checked_iterator_t = decltype(std::declval<tcb::pointer<T[]>&>()->begin());

constexpr bool test_checked_iterator()
{
    using Iter = checked_iterator_t<int>;
    using CIter = checked_iterator_t<int const>;

    static_assert(std::contiguous_iterator<Iter>);
    static_assert(std::same_as<std::iter_value_t<Iter>, int>);
    static_assert(std::same_as<std::iter_reference_t<Iter>, int&>);
    static_assert(std::same_as<std::iter_rvalue_reference_t<Iter>, int&&>);
    static_assert(std::same_as<std::iter_difference_t<Iter>, std::ptrdiff_t>);

    static_assert(std::contiguous_iterator<CIter>);
    static_assert(std::same_as<std::iter_value_t<CIter>, int>);
    static_assert(std::same_as<std::iter_reference_t<CIter>, int const&>);
    static_assert(std::same_as<std::iter_rvalue_reference_t<CIter>, int const&&>);
    static_assert(std::same_as<std::iter_difference_t<CIter>, std::ptrdiff_t>);

    // Basic iteration
    {
        std::array arr{1, 2, 3, 4, 5};

        auto start = Iter(arr.data(), 0, arr.size());
        auto end = Iter(arr.data(), arr.size(), arr.size());

        REQUIRE(std::ranges::equal(arr, std::ranges::subrange(start, end)));
        REQUIRE(std::ranges::equal(arr | std::views::reverse,
                                   std::ranges::subrange(start, end) | std::views::reverse));
    }

    // Comparisons
    {
        std::array arr{1, 2, 3, 4, 5};

        auto start = Iter(arr.data(), 0, arr.size());
        auto next = std::next(start);

        REQUIRE(start == start);
        REQUIRE(start != next);
        REQUIRE(start < next);
        REQUIRE(next > start);
        REQUIRE(start <=> start == std::strong_ordering::equal);
        REQUIRE(start <=> next == std::strong_ordering::less);
    }

    // Random-access jumps
    {
        std::array arr{1, 2, 3, 4, 5};

        auto start = Iter(arr.data(), 0, arr.size());
        auto end = Iter(arr.data(), arr.size(), arr.size());

        REQUIRE(start + 5 == end);
        REQUIRE(end - 5 == start);
        REQUIRE(start[1] == 2);
    }

    // Other bits
    {
        std::array arr{1, 2, 3, 4, 5};

        Iter start = Iter(arr.data(), 0, arr.size());
        ++start;

        CIter copy = start;
        REQUIRE(*copy == 2);

        REQUIRE(std::to_address(start) == arr.data() + 1);
    }

    return true;
}
static_assert(test_checked_iterator());

bool test_checked_iterator_bounds_checking()
{
    using Iter = checked_iterator_t<int>;

    std::array arr{1, 2, 3, 4, 5};

    auto start = Iter(arr.data(), 0, arr.size());
    auto end = Iter(arr.data(), arr.size(), arr.size());

    // Cannot deref end iterator
    REQUIRE_ERROR(*end);

    // Cannot advance end iterator
    REQUIRE_ERROR(++Iter(end));
    REQUIRE_ERROR(Iter(end)++);

    // Cannot decrement start iterator
    REQUIRE_ERROR(--Iter(start));
    REQUIRE_ERROR(Iter(start)--);

    // Cannot perform out-of-bounds RA jumps
    REQUIRE_ERROR((start + -1));
    REQUIRE_ERROR((start - 1));
    REQUIRE_ERROR((start + std::ssize(arr) + 1));
    REQUIRE_ERROR((end + 1));
    REQUIRE_ERROR((end - std::ssize(arr) - 1));

    REQUIRE_ERROR(start[-1]);
    REQUIRE_ERROR(start[std::ssize(arr)]);
    REQUIRE_ERROR(start[std::ssize(arr) + 1]);
    REQUIRE_ERROR(end[0]);
    REQUIRE_ERROR(end[-std::ssize(arr) - 1]);

    // Integer overflow checks
    REQUIRE_ERROR((start + PTRDIFF_MAX));
    REQUIRE_ERROR((start + PTRDIFF_MIN));
    REQUIRE_ERROR((start - PTRDIFF_MAX));
    REQUIRE_ERROR((start - PTRDIFF_MIN));
    REQUIRE_ERROR((end + PTRDIFF_MAX));
    REQUIRE_ERROR((end + PTRDIFF_MIN));
    REQUIRE_ERROR((end - PTRDIFF_MAX));
    REQUIRE_ERROR((end - PTRDIFF_MIN));
    REQUIRE_ERROR(start[PTRDIFF_MAX]);
    REQUIRE_ERROR(start[PTRDIFF_MIN]);
    REQUIRE_ERROR(end[PTRDIFF_MAX]);
    REQUIRE_ERROR(end[PTRDIFF_MIN]);

    return true;
}

/*
 * MARK: Slice tests
 */

constexpr bool test_slice_traits()
{
    using S = tcb::slice<int>;

    // Slices are not default constructible, copyable or movable
    static_assert(not std::is_default_constructible_v<S>);
    static_assert(not std::is_copy_constructible_v<S>);
    static_assert(not std::is_copy_assignable_v<S>);
    static_assert(not std::is_move_constructible_v<S>);
    static_assert(not std::is_move_assignable_v<S>);
    static_assert(std::is_trivially_destructible_v<S>);

    static_assert(not std::default_initializable<S>);
    static_assert(not std::copyable<S>);
    static_assert(not std::movable<S>);
    static_assert(std::destructible<S>);

    // Slices are contiguous, sized, and common ranges
    static_assert(std::ranges::contiguous_range<S>);
    static_assert(std::ranges::sized_range<S>);
    static_assert(std::ranges::common_range<S>);

    static_assert(std::ranges::contiguous_range<S const>);
    static_assert(std::ranges::sized_range<S const>);
    static_assert(std::ranges::common_range<S const>);

    // Slices are not views, but they are (weirdly) borrowed ranges,
    // even though you can't ever get one as an rvalue
    static_assert(not std::ranges::view<S>); // not movable
    static_assert(std::ranges::borrowed_range<S>);

    static_assert(not std::ranges::view<S const>);
    static_assert(std::ranges::borrowed_range<S const>);

    // Associated range types are as expected
    static_assert(std::same_as<std::ranges::range_reference_t<S>, int&>);
    static_assert(std::same_as<std::ranges::range_value_t<S>, int>);
    static_assert(std::same_as<std::ranges::range_rvalue_reference_t<S>, int&&>);

    static_assert(std::same_as<std::ranges::range_reference_t<S const>, int const&>);
    static_assert(std::same_as<std::ranges::range_value_t<S const>, int>);
    static_assert(std::same_as<std::ranges::range_rvalue_reference_t<S const>, int const&&>);

    return true;
}
static_assert(test_slice_traits());

struct no_spaceship {
    int i;

    constexpr bool operator==(no_spaceship other) const { return i == other.i; }
    constexpr bool operator<(no_spaceship other) const { return i < other.i; }
    constexpr bool operator>(no_spaceship other) const { return other < *this; }
    constexpr bool operator<=(no_spaceship other) const { return !(*this > other); }
    constexpr bool operator>=(no_spaceship other) const { return !(*this < other); }
};

constexpr bool test_slice()
{
    // Basic slice functionality
    {
        std::array arr{0, 1, 2, 3, 4};

        auto ptr = tcb::ptr<int[]>::pointer_to(arr);
        auto& slice = *ptr;

        REQUIRE(&slice[0] == &arr[0]);
        REQUIRE(&slice.at(1) == &arr[1]);
        REQUIRE(&slice.front() == &arr.front());
        REQUIRE(&slice.back() == &arr.back());

        REQUIRE(slice.size() == arr.size());
        REQUIRE(slice.empty() == arr.empty());
        REQUIRE(slice.data() == arr.data());

        REQUIRE(std::ranges::equal(slice, arr));
        REQUIRE(std::ranges::equal(slice.cbegin(), slice.cend(), arr.cbegin(), arr.cend()));
        REQUIRE(std::ranges::equal(slice | std::views::reverse, arr | std::views::reverse));
        REQUIRE(std::ranges::equal(slice.crbegin(), slice.crend(), arr.crbegin(), arr.crend()));
    }

    // Same again, but const this time
    {
        std::array const arr{0, 1, 2, 3, 4};

        auto ptr = tcb::ptr<int const[]>::pointer_to(arr);
        auto& slice = *ptr;

        REQUIRE(&slice[0] == &arr[0]);
        REQUIRE(&slice.at(1) == &arr[1]);
        REQUIRE(&slice.front() == &arr.front());
        REQUIRE(&slice.back() == &arr.back());

        REQUIRE(slice.size() == arr.size());
        REQUIRE(slice.empty() == arr.empty());
        REQUIRE(slice.data() == arr.data());

        REQUIRE(std::ranges::equal(slice, arr));
        REQUIRE(std::ranges::equal(slice.cbegin(), slice.cend(), arr.cbegin(), arr.cend()));
        REQUIRE(std::ranges::equal(slice | std::views::reverse, arr | std::views::reverse));
        REQUIRE(std::ranges::equal(slice.crbegin(), slice.crend(), arr.crbegin(), arr.crend()));
    }

    // Empty ranges are handled correctly
    {
        std::array<int, 0> arr{};
        auto ptr = tcb::ptr<int[]>::pointer_to(arr);
        auto& slice = *ptr;

        REQUIRE(slice.size() == 0);
        REQUIRE(slice.empty());
        REQUIRE(slice.data() == arr.data());

        REQUIRE(std::ranges::equal(slice, arr));
    }

    // Slice comparisons work as expected
    {
        auto array = std::array{1, 2, 3, 4, 5};
        auto same_array = array;
        auto shorter_array = std::array{1, 2, 3, 4};
        auto different_array = std::array{1, 2, 99, 4, 5};

        auto p_array = tcb::ptr<int[]>::pointer_to(array);
        auto p_same_array = tcb::ptr<int[]>::pointer_to(same_array);
        auto p_shorter_array = tcb::ptr<int[]>::pointer_to(shorter_array);
        auto p_different_array = tcb::ptr<int[]>::pointer_to(different_array);

        REQUIRE(*p_array == *p_same_array);
        REQUIRE(*p_array != *p_shorter_array);
        REQUIRE(*p_array != *p_different_array);

        REQUIRE(*p_array <=> *p_same_array == std::strong_ordering::equal);
        REQUIRE(*p_array <=> *p_shorter_array == std::strong_ordering::greater);
        REQUIRE(*p_shorter_array <=> *p_array == std::strong_ordering::less);

        // Float comparison should be partially ordered, and handle nans
        {
            float nan = std::numeric_limits<float>::quiet_NaN();
            float floats[] = {1.0f, nan, 3.0f};
            auto p_floats = tcb::ptr<float const[]>::pointer_to(floats);
            auto float_cmp = *p_floats <=> *p_floats;
            static_assert(std::same_as<decltype(float_cmp), std::partial_ordering>);
            REQUIRE(float_cmp == std::partial_ordering::unordered);
        }

        // We can compare types without a spaceship operator
        {
            no_spaceship ns[] = {{1}, {2}, {3}};
            auto ptr = tcb::ptr_to_array(ns);
            auto cmp = *ptr <=> *ptr;
            static_assert(std::same_as<decltype(cmp), std::weak_ordering>);
            REQUIRE(cmp == std::weak_ordering::equivalent);
        }
    }

    // Bounds checking works correctly
    if (!std::is_constant_evaluated()) {
        std::array array{1, 2, 3, 4, 5};

        auto p_array = tcb::ptr<int[]>::pointer_to(array);
        auto p_const_array = tcb::ptr<int const[]>::pointer_to(array);

        // op[]
        REQUIRE_ERROR((*p_array)[5]);
        REQUIRE_ERROR((*p_const_array)[5]);

        // at()
        REQUIRE_THROWS_AS(std::out_of_range, p_array->at(5));
        REQUIRE_THROWS_AS(std::out_of_range, p_const_array->at(5));

        auto empty_arr = std::array<int, 0>{};
        tcb::ptr<int[]> p_empty_array = tcb::pointer_to_mut(empty_arr);
        tcb::ptr<int const[]> p_const_empty_array = tcb::pointer_to(empty_arr);

        // front()
        REQUIRE_ERROR(p_empty_array->front());
        REQUIRE_ERROR(p_const_empty_array->front());

        // back()
        REQUIRE_ERROR(p_empty_array->back());
        REQUIRE_ERROR(p_const_empty_array->back());
    }

    return true;
}
static_assert(test_slice());

/*
 * MARK: array ptr tests
 */
constexpr bool test_array_pointer()
{
    using namespace tcb;

    // pointer<T[]>::pointer_to
    {
        std::array arr{1, 2, 3, 4, 5};

        auto make_ptr
            = [](auto&& a) -> decltype(pointer<int[]>::pointer_to(static_cast<decltype(a)>(a))) {
            return pointer<int[]>::pointer_to(static_cast<decltype(a)>(a));
        };

        // Can call with lvalue contiguous array
        auto ptr = make_ptr(arr);
        REQUIRE(ptr->data() == arr.data() && ptr->size() == arr.size());

        // Can call with rvalue borrowed range
        auto ptr2 = make_ptr(std::span(arr));
        REQUIRE(ptr2->data() == arr.data() && ptr2->size() == arr.size());

        // Cannot call with non-borrowed rvalue
        static_assert(not std::invocable<decltype(make_ptr), std::array<int, 5>&&>);

        // Cannot call with non-contiguous range
        auto transformed = arr | std::views::transform(std::identity{});
        static_assert(not std::invocable<decltype(make_ptr), decltype(transformed)&>);

        // Cannot call with const array
        static_assert(not std::invocable<decltype(make_ptr), std::array<int, 5> const&>);

        // Cannot perform dangerous derived-to-base array conversion
        std::array<DerivedClass, 1> derived_array{};
        auto make_base_ptr = [](auto& a) -> decltype(pointer<BaseClass[]>::pointer_to(a)) {
            return pointer<BaseClass[]>::pointer_to(a);
        };
        static_assert(not std::invocable<decltype(make_base_ptr), decltype(derived_array)>);
    }

    // pointer_to<T const[]>::pointer_to
    {
        std::array arr{1, 2, 3, 4, 5};

        auto make_ptr = [](auto&& a) -> decltype(pointer<int const[]>::pointer_to(
                                         static_cast<decltype(a)>(a))) {
            return pointer<int const[]>::pointer_to(static_cast<decltype(a)>(a));
        };

        // Can call with lvalue contiguous array of non-const
        auto ptr = make_ptr(arr);
        REQUIRE(ptr->data() == arr.data() && ptr->size() == arr.size());

        // Can call with lvalue contiguous array of const
        auto ptr2 = make_ptr(std::as_const(arr));
        REQUIRE(ptr2->data() == arr.data() && ptr2->size() == arr.size());

        // Can call with rvalue borrowed range
        auto ptr3 = make_ptr(std::span(arr));
        REQUIRE(ptr3->data() == arr.data() && ptr3->size() == arr.size());

        // Cannot call with non-borrowed rvalue
        static_assert(not std::invocable<decltype(make_ptr), std::array<int, 5>&&>);

        // Cannot call with non-contiguous range
        auto transformed = arr | std::views::transform(std::identity{});
        static_assert(not std::invocable<decltype(make_ptr), decltype(transformed)&>);

        // Cannot perform dangerous derived-to-base array conversion
        std::array<DerivedClass, 1> derived_array{};
        auto make_base_ptr = [](auto& a) -> decltype(pointer<BaseClass const[]>::pointer_to(a)) {
            return pointer<BaseClass const[]>::pointer_to(a);
        };
        static_assert(not std::invocable<decltype(make_base_ptr), decltype(derived_array)>);
    }

    // pointer_to_array()
    {
        std::array arr{1, 2, 3, 4, 5};

        auto make_ptr = [](auto&& a) -> decltype(pointer_to_array(static_cast<decltype(a)>(a))) {
            return pointer_to_array(static_cast<decltype(a)>(a));
        };

        // Can call with lvalue contiguous array
        auto ptr = make_ptr(arr);
        REQUIRE(ptr->data() == arr.data() && ptr->size() == arr.size());

        // Can call with lvalue contiguous array of const
        auto ptr2 = make_ptr(std::as_const(arr));
        REQUIRE(ptr2->data() == arr.data() && ptr2->size() == arr.size());

        // Can call with rvalue borrowed range
        auto ptr3 = make_ptr(std::span(arr));
        REQUIRE(ptr3->data() == arr.data() && ptr3->size() == arr.size());

        // Cannot call with non-borrowed rvalue
        static_assert(not std::invocable<decltype(make_ptr), std::array<int, 5>&&>);

        // Cannot call with non-contiguous range
        auto transformed = arr | std::views::transform(std::identity{});
        static_assert(not std::invocable<decltype(make_ptr), decltype(transformed)&>);
    }

    // pointer_to_mut_array()
    {
        std::array arr{1, 2, 3, 4, 5};

        auto make_ptr
            = [](auto&& a) -> decltype(pointer_to_mut_array(static_cast<decltype(a)>(a))) {
            return pointer_to_mut_array(static_cast<decltype(a)>(a));
        };

        // Can call with lvalue contiguous array
        auto ptr = make_ptr(arr);
        REQUIRE(ptr->data() == arr.data() && ptr->size() == arr.size());

        // Can call with rvalue borrowed range
        auto ptr2 = make_ptr(std::span(arr));
        REQUIRE(ptr2->data() == arr.data() && ptr2->size() == arr.size());

        // Cannot call with non-borrowed rvalue
        static_assert(not std::invocable<decltype(make_ptr), std::array<int, 5>&&>);

        // Cannot call with non-contiguous range
        auto transformed = arr | std::views::transform(std::identity{});
        static_assert(not std::invocable<decltype(make_ptr), decltype(transformed)&>);

        // Cannot call with const array
        static_assert(not std::invocable<decltype(make_ptr), std::array<int, 5> const&>);
    }

    // pointer<T[]>::from_address_with_size
    {
        int array[] = {1, 2, 3, 4, 5};

        auto ptr = pointer<int[]>::from_address_with_size(array, std::size(array));
        REQUIRE(ptr->data() == array && ptr->size() == std::size(array));

        // Can treat a pointer-to-object as an array if we try
        int val = 99;
        auto ptr2 = pointer<int[]>::from_address_with_size(&val, 1);
        REQUIRE(ptr2->data() == &val && ptr2->size() == 1);
        REQUIRE(ptr2->at(0) == 99);

        // Can create an array of size zero
        auto ptr3 = pointer<int[]>::from_address_with_size(array, 0);
        REQUIRE(ptr3->data() == array && ptr3->size() == 0);

        // Passing a null pointer is a runtime error
        if (!std::is_constant_evaluated()) {
            REQUIRE_ERROR(pointer<int[]>::from_address_with_size((int*)nullptr, 1));
        }
    }

    // pointer<T const[]>::from_address_with_size
    {
        int array[] = {1, 2, 3, 4, 5};

        auto ptr = pointer<int const[]>::from_address_with_size(array, std::size(array));
        REQUIRE(ptr->data() == array && ptr->size() == std::size(array));

        // Can treat a pointer-to-object as an array if we try
        int val = 99;
        auto ptr2 = pointer<int const[]>::from_address_with_size(&val, 1);
        REQUIRE(ptr2->data() == &val && ptr2->size() == 1);
        REQUIRE(ptr2->at(0) == 99);

        // Can create an array of size zero
        auto ptr3 = pointer<int const[]>::from_address_with_size(array, 0);
        REQUIRE(ptr3->data() == array && ptr3->size() == 0);

        // Passing a null pointer is a runtime error
        if (!std::is_constant_evaluated()) {
            REQUIRE_ERROR(pointer<int const[]>::from_address_with_size((int const*)nullptr, 1));
        }
    }

    // constructors etc
    {
        std::array arr1{1, 2, 3, 4, 5};
        std::array arr2{6, 7, 8, 9, 10};

        auto p1 = ptr_to_mut_array(arr1);
        auto p2 = p1; // copy-construct
        REQUIRE(p2->data() == arr1.data() && p2->size() == arr1.size());

        p2 = ptr_to_mut_array(arr2); // copy-assign
        REQUIRE(p2->data() == arr2.data() && p2->size() == arr2.size());
        REQUIRE(p1->data() == arr1.data() && p1->size() == arr1.size());

        // Can convert pointer<T[]> to pointer<T const[]>
        static_assert(std::convertible_to<pointer<int[]>, pointer<int const[]>>);
        ptr<int const[]> p3 = p1;
        REQUIRE(p3->data() == arr1.data() && p3->size() == arr1.size());

        // Cannot convert pointer<T const[]> to pointer<T[]>
        static_assert(not std::convertible_to<pointer<int const[]>, pointer<int[]>>);

        // Cannot convert from pointer<Derived[]> to pointer<Base[]>
        static_assert(not std::convertible_to<pointer<DerivedClass[]>, pointer<BaseClass[]>>);
        static_assert(
            not std::convertible_to<pointer<DerivedClass const[]>, pointer<BaseClass const[]>>);

        // Can convert from pointer<R> to pointer<T[]> when R is a contiguous range of T
        static_assert(std::convertible_to<pointer<std::array<int, 5>>, pointer<int[]>>);
        static_assert(std::convertible_to<pointer<std::array<int, 5>>, pointer<int const[]>>);
        pointer<int[]> p4 = ptr_to_mut(arr1);
        REQUIRE(p4->data() == arr1.data() && p4->size() == arr1.size());
        pointer<int const[]> p5 = ptr_to(arr2);
        REQUIRE(p5->data() == arr2.data() && p5->size() == arr2.size());
    }

    // pointer<T[]> equality
    {
        std::array array{1, 2, 3, 4, 5};
        std::array same_values{1, 2, 3, 4, 5};
        std::array different_values{6, 7, 8, 9, 10};

        auto p_array = pointer_to_array(array);
        auto p_array2 = pointer_to_array(array);
        auto p_same = pointer_to_array(same_values);
        auto p_different = pointer_to_array(different_values);

        REQUIRE(p_array == p_array2);
        REQUIRE(*p_array == *p_array2);

        REQUIRE(p_array != p_same);
        REQUIRE(*p_array == *p_same);

        REQUIRE(p_array != p_different);
        REQUIRE(*p_array != *p_different);

        auto p_short_array = pointer_to_array(std::span(array).first(3));
        REQUIRE(p_array != p_short_array);
        REQUIRE(*p_array != *p_short_array);
    }

    // pointer<T[]> ordering
    if constexpr (!clang19_with_libstdcxx) {
        std::array<std::array<int, 3>, 2> arrays{std::array<int, 3>{1, 2, 3},
                                                 std::array<int, 3>{4, 5, 6}};

        auto p1 = pointer_to_array(arrays[0]);
        auto p2 = pointer_to_array(arrays[1]);

        REQUIRE(p1 <=> p1 == std::strong_ordering::equal);
        REQUIRE(p1 <=> p2 == std::strong_ordering::less);
        REQUIRE(p2 <=> p1 == std::strong_ordering::greater);

        auto p_short = pointer_to_array(std::span(arrays[0]).first(2));
        REQUIRE(p1 <=> p_short == std::strong_ordering::greater);
        REQUIRE(p_short <=> p1 == std::strong_ordering::less);
    }

    return true;
}
static_assert(test_array_pointer());

/*
 * MARK: cast tests
 */
bool test_pointer_casts()
{
    // static cast from void to object pointer works
    // (can also do this as an explicit conversion)
    {
        int i = 0;

        auto void_ptr = tcb::ptr<void>::pointer_to(i);

        auto int_ptr = tcb::static_pointer_cast<int>(void_ptr);

        REQUIRE(std::to_address(int_ptr) == std::addressof(i));
    }

    // unsafe static cast from base to derived works
    {
        DerivedClass d;

        auto base_ptr = tcb::ptr<BaseClass>::pointer_to(d);
        REQUIRE(std::to_address(base_ptr) == std::addressof(d));

        auto derived_ptr = tcb::static_pointer_cast<DerivedClass>(base_ptr);
        REQUIRE(std::to_address(derived_ptr) == std::addressof(d));
    }

    // const cast from const to non-const works
    {
        // for objects...
        {
            int i = 0;
            auto cptr = tcb::pointer_to(i);

            auto mptr = tcb::const_pointer_cast<int>(cptr);

            *mptr = 3;
            REQUIRE(i == 3);
        }

        // for void...
        {
            int i = 0;

            auto cptr = tcb::ptr<void const>::pointer_to(i);

            auto mptr = tcb::const_pointer_cast<void>(cptr);

            auto iptr = tcb::static_pointer_cast<int>(mptr);

            *iptr = 3;
            REQUIRE(i == 3);
        }

        // for arrays...
        {
            std::array arr{1, 2, 3, 4, 5};

            auto cptr = tcb::ptr_to_array(arr);

            auto mptr = tcb::const_pointer_cast<int[]>(cptr);

            mptr->at(0) = 3;

            REQUIRE(arr[0] == 3);
        }
    }

    // Dynamic casts work as expected
    {
        // Successful
        {
            DerivedClass d;

            auto bptr = tcb::ptr<BaseClass>::pointer_to(d);

            auto opt = tcb::dynamic_pointer_cast<DerivedClass>(bptr);

            REQUIRE(opt.has_value());
            REQUIRE(std::to_address(*opt) == std::addressof(d));
        }

        // Unsuccessful
        {
            BaseClass b;

            auto bptr = tcb::ptr<BaseClass>::pointer_to(b);

            auto opt = tcb::dynamic_pointer_cast<DerivedClass>(bptr);

            REQUIRE(not opt.has_value());
        }
    }

    return true;
}

/*
 * MARK: std::hash tests
 */
bool test_std_hash_specialisation()
{
    using namespace tcb;

    // pointer-to-object hash
    {
        int i = 0;
        auto p1 = pointer_to_mut(i);
        REQUIRE(std::hash<ptr<int>>{}(p1) == std::hash<int*>{}(&i));
    }

    // void pointer hash
    {
        int i = 0;
        ptr<void> p1 = ptr_to_mut(i);
        REQUIRE(std::hash<ptr<void>>{}(p1) == std::hash<void*>{}((void*)&i));
    }

    // array pointer hash
    // need to make sure that hashes with the same base pointer and different lengths are different
    {
        int array[] = {1, 2, 3, 4, 5};

        auto p1 = pointer<int[]>::from_address_with_size(array, 5);
        auto p2 = pointer<int[]>::from_address_with_size(array, 3);

        auto hasher = std::hash<ptr<int[]>>{};
        REQUIRE(hasher(p1) != hasher(p2));
    }

    // Make sure we can construct unordered_sets of pointers
    {
        [[maybe_unused]] std::unordered_set<pointer<int>> set1{};
        [[maybe_unused]] std::unordered_set<pointer<int const>> set2{};
        [[maybe_unused]] std::unordered_set<pointer<void>> set3{};
        [[maybe_unused]] std::unordered_set<pointer<void const>> set4{};
        [[maybe_unused]] std::unordered_set<pointer<int[]>> set5{};
        [[maybe_unused]] std::unordered_set<pointer<int const[]>> set6{};
    }

    return true;
}

struct ConvertibleToPointer {
    int* ptr;

    constexpr operator tcb::pointer<int>() const { return tcb::pointer<int>::from_address(ptr); }
};

/*
 * MARK: std::optional tests
 */
constexpr bool test_std_optional_specialisation()
{
    using namespace tcb;

    // Constructor traits and concepts
    {
        using Opt = std::optional<pointer<int>>;

        // Type traits tests
        static_assert(std::is_default_constructible_v<Opt>);
        static_assert(std::is_trivially_copy_constructible_v<Opt>);
        static_assert(std::is_trivially_move_constructible_v<Opt>);
        static_assert(std::is_trivially_copy_assignable_v<Opt>);
        static_assert(std::is_trivially_move_assignable_v<Opt>);
        static_assert(std::is_trivially_destructible_v<Opt>);

        static_assert(std::is_nothrow_default_constructible_v<Opt>);
        static_assert(std::is_nothrow_copy_constructible_v<Opt>);
        static_assert(std::is_nothrow_move_constructible_v<Opt>);
        static_assert(std::is_nothrow_copy_assignable_v<Opt>);
        static_assert(std::is_nothrow_move_assignable_v<Opt>);
        static_assert(std::is_nothrow_destructible_v<Opt>);

        // Concepts tests
        static_assert(std::default_initializable<Opt>);
        static_assert(std::copy_constructible<Opt>);
        static_assert(std::move_constructible<Opt>);
        // static_assert(std::copyable<Opt>);
        static_assert(std::movable<Opt>);
        static_assert(std::destructible<Opt>);
    }

    // can default construct an optional<pointer>
    {
        using Opt = std::optional<pointer<int>>;

        Opt o1;
        REQUIRE(!o1.has_value());

        Opt o2{};
        REQUIRE(!o2.has_value());

        auto o3 = Opt();
        REQUIRE(!o3.has_value());
    }

    // Can implicitly construct using nullopt constructor
    {
        using Opt = std::optional<pointer<int>>;
        static_assert(std::is_nothrow_constructible_v<Opt, std::nullopt_t>);
        static_assert(std::constructible_from<Opt, std::nullopt_t>);

        Opt o1 = std::nullopt;
        REQUIRE(!o1.has_value());

        Opt o2{std::nullopt};
        REQUIRE(!o2.has_value());

        auto o3 = Opt(std::nullopt);
        REQUIRE(!o3.has_value());
    }

    // Converting copy constructor
    {
        using Opt1 = std::optional<pointer<int>>;
        using Opt2 = std::optional<pointer<int const>>;
        using Opt3 = std::optional<ConvertibleToPointer>;

        // Can convert optional<pointer<int>> to optional<pointer<int const>>
        static_assert(std::is_constructible_v<Opt2, Opt1 const&>);
        static_assert(std::is_convertible_v<Opt1 const&, Opt2>);
        static_assert(std::is_nothrow_constructible_v<Opt2, Opt1 const&>);
        static_assert(std::constructible_from<Opt2, Opt1 const&>);

        // ...but not the other way round
        static_assert(!std::is_constructible_v<Opt1, Opt2 const&>);
        static_assert(!std::is_convertible_v<Opt2 const&, Opt1>);
        static_assert(!std::is_nothrow_constructible_v<Opt1, Opt2 const&>);
        static_assert(!std::constructible_from<Opt1, Opt2 const&>);

        // Can convert optional<ConvertibleToPointer> to optional<pointer>
        static_assert(std::is_constructible_v<Opt1, Opt3 const&>);
        static_assert(std::is_convertible_v<Opt3 const&, Opt1>);
        static_assert(not std::is_nothrow_constructible_v<Opt1, Opt3 const&>);
        static_assert(std::constructible_from<Opt1, Opt3 const&>);

        Opt1 o1{};

        Opt2 o2{o1};
        REQUIRE(not o2.has_value());

        int i = 0;
        o1 = pointer_to_mut(i);

        Opt2 o2b = o1;
        REQUIRE(o2b.has_value());
        REQUIRE(**o2b == 0);

        Opt3 o3 = ConvertibleToPointer{.ptr = &i};
        [[maybe_unused]] Opt1 o1b = o3;
    }

    // Converting move constructor
    {
        using Opt1 = std::optional<pointer<int>>;
        using Opt2 = std::optional<pointer<int const>>;

        // Can convert optional<pointer<int>> to optional<pointer<int const>>
        static_assert(std::is_constructible_v<Opt2, Opt1>);
        static_assert(std::is_convertible_v<Opt1, Opt2>);
        static_assert(std::is_nothrow_constructible_v<Opt2, Opt1>);
        static_assert(std::constructible_from<Opt2, Opt1>);

        // ...but not the other way round
        static_assert(!std::is_constructible_v<Opt1, Opt2>);
        static_assert(!std::is_convertible_v<Opt2, Opt1>);
        static_assert(!std::is_nothrow_constructible_v<Opt1, Opt2>);
        static_assert(!std::constructible_from<Opt1, Opt2>);

        Opt1 o1{};

        Opt2 o2{std::move(o1)};
        REQUIRE(not o2.has_value());

        int i = 0;
        o1 = pointer_to_mut(i);

        Opt2 o3 = std::move(o1);
        REQUIRE(o3.has_value());
        REQUIRE(**o3 == 0);
    }

    // in_place_t constructor
    {
        using Opt = std::optional<pointer<int>>;

        int i = 99;
        Opt o1{std::in_place, tcb::pointer_to_mut(i)};
        REQUIRE(o1.has_value());
        REQUIRE(**o1 == 99);
    }

    // value constructor
    {
        using Opt = std::optional<pointer<int const>>;

        int i = 99;
        Opt o1{tcb::pointer_to_mut(i)};
        REQUIRE(o1.has_value());
        REQUIRE(**o1 == 99);

        // Can implicitly construct from pointer to different (but convertible) type
        Opt o2 = tcb::pointer_to_mut(i);
        REQUIRE(o2.has_value());
        REQUIRE(**o2 == 99);
    }

    // assignment from nullopt
    {
        using Opt = std::optional<pointer<int>>;

        int i = 0;
        Opt o1 = tcb::pointer_to_mut(i);
        REQUIRE(o1.has_value());

        o1 = std::nullopt;
        REQUIRE(!o1.has_value());

        o1 = {};
        REQUIRE(!o1.has_value());
    }

    // converting copy-assignment operator
    {
        using Opt1 = std::optional<tcb::pointer<int>>;
        using Opt2 = std::optional<ConvertibleToPointer>;

        static_assert(std::assignable_from<Opt1&, Opt2 const&>);

        // nullopt = nullopt
        {
            Opt1 o1 = std::nullopt;
            Opt2 o2 = std::nullopt;

            o1 = o2;
            REQUIRE(not o1.has_value());
        }
        // nullopt = value
        {
            int i = 0;
            Opt1 o1 = std::nullopt;
            Opt2 o2 = ConvertibleToPointer{.ptr = &i};

            o1 = o2;
            REQUIRE(o1.has_value());
            REQUIRE(o1->to_address() == &i);
        }
        // value = nullopt
        {
            int i = 0;
            Opt1 o1 = tcb::pointer_to_mut(i);
            Opt2 o2 = std::nullopt;

            o1 = o2;
            REQUIRE(not o1.has_value());
        }
        // value = value
        {
            int i = 0;
            int j = 99;

            Opt1 o1 = tcb::pointer_to_mut(i);
            Opt2 o2 = ConvertibleToPointer{.ptr = &j};

            o1 = o2;
            REQUIRE(o1.has_value());
            REQUIRE(o1->to_address() == &j);
        }
    }

    // converting move-assignment operator
    {
        using Opt1 = std::optional<tcb::pointer<int>>;
        using Opt2 = std::optional<ConvertibleToPointer>;

        static_assert(std::assignable_from<Opt1&, Opt2>);

        // nullopt = nullopt
        {
            Opt1 o1 = std::nullopt;

            o1 = Opt2{};
            REQUIRE(not o1.has_value());
        }
        // nullopt = value
        {
            int i = 0;
            Opt1 o1 = std::nullopt;

            o1 = Opt2(ConvertibleToPointer{.ptr = &i});
            REQUIRE(o1.has_value());
            REQUIRE(o1->to_address() == &i);
        }
        // value = nullopt
        {
            int i = 0;
            Opt1 o1 = tcb::pointer_to_mut(i);

            o1 = Opt2{};
            REQUIRE(not o1.has_value());
        }
        // value = value
        {
            int i = 0;
            int j = 99;

            Opt1 o1 = tcb::pointer_to_mut(i);

            o1 = Opt2(ConvertibleToPointer{.ptr = &j});
            REQUIRE(o1.has_value());
            REQUIRE(o1->to_address() == &j);
        }
    }

    // Value assignment operator
    {
        using Opt = std::optional<tcb::pointer<int const>>;

        static_assert(std::assignable_from<Opt&, tcb::pointer<int const> const&>);
        static_assert(std::assignable_from<Opt&, tcb::pointer<int const>>);
        static_assert(std::assignable_from<Opt&, tcb::pointer<int> const&>);
        static_assert(std::assignable_from<Opt&, tcb::pointer<int>>);

        const int i = 0;
        int j = 99;

        Opt o1;

        o1 = tcb::pointer_to(i);
        REQUIRE(o1.has_value());
        REQUIRE(o1->to_address() == &i);

        o1 = tcb::pointer_to_mut(j);
        REQUIRE(o1.has_value());
        REQUIRE(o1->to_address() == &j);
    }

    // Iterator support
    {
        using Opt = std::optional<tcb::pointer<int>>;

        static_assert(std::ranges::contiguous_range<Opt>);
        static_assert(std::ranges::contiguous_range<Opt const>);
        static_assert(std::same_as<std::ranges::range_value_t<Opt>, tcb::pointer<int>>);
        static_assert(std::same_as<std::ranges::range_reference_t<Opt>, tcb::pointer<int>&>);
        static_assert(
            std::same_as<std::ranges::range_reference_t<Opt const>, tcb::pointer<int> const&>);

        Opt o1;
        bool called = false;
        for (auto& p [[maybe_unused]] : o1) {
            called = true;
        }
        REQUIRE(not called);

        int i = 0;
        o1 = tcb::pointer_to_mut(i);
        for (auto& p : o1) {
            *p = 99;
            called = true;
        }
        REQUIRE(i == 99);
        REQUIRE(called == true);

        // We can form a pointer<pointer<int>[]> from an optional...
        auto p = tcb::pointer_to_mut_array(o1);
        std::ranges::fill(*p | std::views::transform([](auto p) -> int& { return *p; }), 1000);
        REQUIRE(i == 1000);
    }

    // optional<pointer<T[]>> works correctly
    {
        using Opt = std::optional<tcb::pointer<int[]>>;
        static_assert(sizeof(Opt) == sizeof(tcb::pointer<int[]>));

        Opt opt{};
        REQUIRE(not opt.has_value());

        std::array arr{1, 2, 3, 4, 5};

        opt = tcb::pointer_to_mut_array(arr);

        REQUIRE(opt.has_value());
        REQUIRE(&(*opt)->at(0) == &arr[0]);

        std::ranges::fill(**opt, 99);

        REQUIRE(std::ranges::all_of(arr, [](int i) { return i == 99; }));
    }

    return true;
}
static_assert(test_std_optional_specialisation());

/*
 * MARK: main()
 */

int main()
{
    bool b = true;

    // addr tests
    b = test_pointer_to();
    REQUIRE(b);

    // object pointer tests
    b = test_pointer_to_object();
    REQUIRE(b);

    // void pointer tests
    b = test_pointer_to_void();
    REQUIRE(b);

    // checked iterator tests
    b = test_checked_iterator();
    REQUIRE(b);
    b = test_checked_iterator_bounds_checking();
    REQUIRE(b);

    // slice tests
    b = test_slice();
    REQUIRE(b);

    // array pointer tests
    b = test_array_pointer();
    REQUIRE(b);

    b = test_pointer_casts();
    REQUIRE(b);

    // std::hash tests
    b = test_std_hash_specialisation();
    REQUIRE(b);

    // std::optional tests
    b = test_std_optional_specialisation();
    REQUIRE(b);
}


#include <cassert>

#ifdef IMPORT_MODULE
import tcb.pointer;
#else
#    include <tcb/pointer.hpp>
#endif

struct Base {
    virtual ~Base() = default;
};
struct Derived : Base { };
struct OtherDerived : Base { };

void pointer_conversions()
{
    // Just like with raw pointers, we can implicitly convert a
    // pointer-to-non-const into a pointer-to-const:
    int i = 0;
    tcb::pointer<int> p1 = tcb::ptr_to_mut(i);
    tcb::pointer<int const> p2 = p1;

    // Going in the other direction (const to mutable) is a compile error:
    // [[maybe_unused]] tcb::pointer<int> error = p2;

    // If we really need to, const-to-mutable pointer conversion can be
    // accomplished using tcb::const_pointer_cast().
    // Of course, attempting to modify an object that was "born const"
    // is undefined behaviour, so be careful!
    tcb::pointer<int> p3 = tcb::const_pointer_cast<int>(p2);

    // Similarly, we can implicitly convert from a pointer-to-derived
    // to a pointer-to-base, as we'd expect:
    Derived d{};
    tcb::pointer<Base> p_base = tcb::ptr_to_mut(d);

    // The reverse conversion (base to derived) is a compile error:
    // [[maybe_derived]] tcb::pointer<Derived> error = p_base;

    // The function tcb::static_pointer_cast() can be used to perform
    // a dangerous base-to-derived conversion.
    // Note that this function does not perform any compile-time or run-time
    // checks, and is undefined behaviour if p_base does not
    // point to a Base subobject of a Derived:
    tcb::pointer<Derived> p_derived = tcb::static_pointer_cast<Derived>(p_base);

    // A safer alternative is to use tcb::dynamic_pointer_cast().
    // This returns a `std::optional` which contains a derived pointer if the
    // cast was valid, or otherwise is disengaged.
    auto opt1 = tcb::dynamic_pointer_cast<Derived>(p_base);
    assert(opt1.has_value()); // conversion was okay

    auto opt2 = tcb::dynamic_pointer_cast<OtherDerived>(p_base);
    assert(not opt2.has_value()); // conversion failed

    // (Avoid compiler warnings by "using" variables)
    [](auto&...) { }(p1, p2, p3, p_base, p_derived, opt1, opt2);
}

int main() { pointer_conversions(); }
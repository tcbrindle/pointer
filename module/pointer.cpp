module;

#include <tcb/pointer.hpp>

export module tcb.pointer;

export namespace tcb {

// Public classes
using tcb::pointer;
using tcb::slice;

// Public type aliases
using tcb::array_pointer;
using tcb::ptr;

// Public function objects
using tcb::const_pointer_cast_t;
using tcb::dynamic_pointer_cast_t;
using tcb::pointer_to_array_t;
using tcb::pointer_to_mut_array_t;
using tcb::pointer_to_mut_t;
using tcb::pointer_to_t;
using tcb::static_pointer_cast_t;
using tcb::to_address_t;

// Public functions
using tcb::const_pointer_cast;
using tcb::dynamic_pointer_cast;
using tcb::pointer_to;
using tcb::pointer_to_array;
using tcb::pointer_to_mut;
using tcb::pointer_to_mut_array;
using tcb::ptr_to;
using tcb::ptr_to_array;
using tcb::ptr_to_mut;
using tcb::ptr_to_mut_array;
using tcb::static_pointer_cast;
using tcb::to_address;

} // namespace tcb
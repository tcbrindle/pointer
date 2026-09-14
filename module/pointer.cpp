// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

module;

#include <tcb/pointer.hpp>

export module tcb.pointer;

export namespace tcb {

// Types
using tcb::pointer;
using tcb::slice;
using tcb::unchecked_slice;

using tcb::array_pointer;

// Functions
using tcb::pointer_to;
using tcb::pointer_to_array;
using tcb::pointer_to_mut;
using tcb::pointer_to_mut_array;
using tcb::to_address;

using tcb::const_pointer_cast;
using tcb::dynamic_pointer_cast;
using tcb::static_pointer_cast;

// Aliases
using tcb::array_ptr;
using tcb::ptr;
using tcb::ptr_to;
using tcb::ptr_to_array;
using tcb::ptr_to_mut;
using tcb::ptr_to_mut_array;

} // namespace tcb

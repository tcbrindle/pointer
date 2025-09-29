// Copyright (c) 2025 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <stdexcept>

struct ptr_runtime_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};
#define TCB_PTR_RUNTIME_ERROR(msg) throw ptr_runtime_error(msg)

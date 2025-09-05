
#pragma once

#include <stdexcept>

struct ptr_runtime_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};
#define TCB_PTR_RUNTIME_ERROR(msg) throw ptr_runtime_error(msg)

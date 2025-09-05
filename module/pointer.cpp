module;

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <typeinfo>
#include <type_traits>

#ifndef NDEBUG
#    include <cstdio>
#    include <exception>
#endif

#ifdef _MSC_VER
#    include <intrin.h> // for __fastfail
#endif

export module tcb.pointer;

#define TCB_PTR_BUILDING_MODULE

extern "C++" {
#include "tcb/pointer.hpp"
}
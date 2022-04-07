#pragma once
#include "../../pch.h"

#include <cstddef>

namespace SkEd {

template <typename T>
struct Span {
    T* data;
    std::size_t size;
};

using TextSpan = Span<const char>;

}

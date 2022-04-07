#pragma once
#include "../../pch.h"

#include <cstddef>
#include <vector>

// TODO: Decide if this functionality should be moved into SkShaper as an extra utility.
std::vector<bool> GetUtf8WordBoundaries(const char* begin, std::size_t byteLen, const char* locale);

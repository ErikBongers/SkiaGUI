// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "TextBuffer.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

using namespace SkEd;

void TextBuffer::FreeWrapper::operator()(void* t) { std::free(t); }

TextBuffer::TextBuffer(TextBuffer&& that) noexcept
    : fPtr(std::move(that.fPtr))
    , fLength(that.fLength)
    , fCapacity(that.fCapacity)
{
    that.fLength = 0;
    that.fCapacity = 0;
}

TextBuffer& TextBuffer::operator=(TextBuffer&& that) noexcept
{
    if (this != &that) {
        this->~TextBuffer();
        new (this)TextBuffer(std::move(that));
    }
    return *this;
}

TextBuffer& TextBuffer::operator=(const TextBuffer& that) {
    if (this != &that) {
        fLength = 0;
        if (that.size() > 0) {
            this->insert(0, that.begin(), that.size());
        }
    }
    return *this;
}

void TextBuffer::insert(std::size_t offset, const char* text, std::size_t length) {
    if (length) {
        offset = std::min(fLength, offset);
        this->reserve(fLength + length);
        char* s = fPtr.get();
        assert(s);
        if (offset != fLength) {
            std::memmove(s + offset + length, s + offset, fLength - offset);
        }
        if (text) {
            std::memcpy(s + offset, text, length);
        } else {
            std::memset(s + offset, 0, length);
        }
        fLength += length;
    }
}

void TextBuffer::remove(std::size_t offset, std::size_t length) {
    if (length && offset < fLength) {
        length = std::min(length, fLength - offset);
        assert(length > 0);
        assert(length + offset <= fLength);
        if (length + offset < fLength) {
            char* s = fPtr.get();
            assert(s);
            std::memmove(s + offset, s + offset + length, fLength - (length + offset));
        }
        fLength -= length;
    }
}

void TextBuffer::realloc(std::size_t size) {
    // round up to multiple of (1 << kBits) bytes
    static constexpr unsigned kBits = 4;
    fCapacity = size ? (((size - 1) >> kBits) + 1) << kBits : 0;
    assert(fCapacity % (1u << kBits) == 0);
    assert(fCapacity >= size);
    fPtr.reset((char*)std::realloc(fPtr.release(), fCapacity));
    assert(fCapacity >= fLength);
}

size_t TextBuffer::count_char(char value) 
    {
    size_t count = 0;
    for (char c : *this) { if (c == value) { ++count; } }
    return count;
    }

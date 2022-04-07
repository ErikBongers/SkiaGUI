#pragma once
#include "TextBuffer.h"

inline bool is_utf8_continuation(char v) {
    return ((unsigned char)v & 0b11000000) ==
        0b10000000;
    }

const char* align_utf8(const char* p, const char* begin);

inline const char* begin(const SkEd::TextBuffer& s) { return s.begin(); }
inline const char* end(const SkEd::TextBuffer& s) { return s.end(); }

size_t align_column(const SkEd::TextBuffer& str, size_t p);

const char* next_utf8(const char* p, const char* end);

const char* prev_utf8(const char* p, const char* begin);
#include "utf8_helpers.h"

const char* align_utf8(const char* p, const char* begin) {
    while (p > begin && is_utf8_continuation(*p)) {
        --p;
        }
    return p;
    }

size_t align_column(const SkEd::TextBuffer& str, size_t p) {
    if (p >= str.size()) {
        return str.size();
        }
    return align_utf8(begin(str) + p, begin(str)) - begin(str);
    }

const char* next_utf8(const char* p, const char* end) {
    if (p < end) {
        do {
            ++p;
            } while (p < end && is_utf8_continuation(*p));
        }
    return p;
    }

const char* prev_utf8(const char* p, const char* begin) {
    return p > begin ? align_utf8(p - 1, begin) : begin;
    }


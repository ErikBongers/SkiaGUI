#pragma once
#include "../../pch.h"

#include "TextSpan.h"

#include <memory>
#include <cstddef>
#include <algorithm>
#include "src/utils/SkUTF.h"

namespace SkEd {
// A lightweight modifiable string class.
class TextBuffer {
public:
    TextBuffer() = default;
    TextBuffer(const char* s, std::size_t l) { this->insert(0, s, l); }
    TextBuffer(TextBuffer&&) noexcept;
    TextBuffer(const TextBuffer& that) : TextBuffer(that.begin(), that.size()) {}
    ~TextBuffer() = default;
    TextBuffer& operator=(TextBuffer&&) noexcept;
    TextBuffer& operator=(const TextBuffer&);

    // access:
    // Does not have a c_str method; is *not* NUL-terminated.
    const char* begin() const { return fPtr.get(); }
    const char* end() const { return fPtr ? fPtr.get() + fLength : nullptr; }
    std::size_t size() const { return fLength; }
    SkEd::TextSpan view() const { return {fPtr.get(), fLength}; }
    size_t count_char(char value);
    size_t count_utf8() { return SkUTF::CountUTF8(fPtr.get(), fLength); }

    // mutation:
    void insert(std::size_t offset, const char* text, std::size_t length);
    void remove(std::size_t offset, std::size_t length);

    // modify capacity only:
    void reserve(std::size_t size) { if (size > fCapacity) { this->realloc(size); } }
    void shrink() { this->realloc(fLength); }
    void trim(std::size_t len) { fLength = std::min(len, fLength); }
private:
    struct FreeWrapper { void operator()(void*); };
    std::unique_ptr<char[], FreeWrapper> fPtr;
    std::size_t fLength = 0;
    std::size_t fCapacity = 0;
    void realloc(std::size_t);
};
}  // namespace SkPlainTextEditor;

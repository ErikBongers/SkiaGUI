#include "../../pch.h"
#include "include/core/SkFont.h"
#include "include/core/SkTextBlob.h"

#include <cstddef>
#include <vector>

namespace SkEd {

struct ShapeResult {
    sk_sp<SkTextBlob> blob;
    std::vector<std::size_t> lineBreakOffsets;
    std::vector<SkRect> glyphBounds;
    std::vector<bool> wordBreaks;
    int verticalAdvance;
};

ShapeResult shapeTextToWidth(const char* ut8text,
                  size_t textByteLen,
                  const SkFont& font,
                  const char* locale,
                  float width);

}

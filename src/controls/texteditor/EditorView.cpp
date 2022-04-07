#include "EditorView.h"
#include "shape.h"
#include "utf8_helpers.h"

using namespace SkEd;

EditorView::ParagraphFormat& EditorView::ParaData::operator[](std::size_t i) 
    { 
    if (i >= formats.size())
        {
        formats.resize(i + 1);
        formats[i] = std::make_shared<ParagraphFormat>();
        }
    return *formats[i]; 
    }

void EditorView::attachDoc(EditorDoc* doc)
    {    
    this->doc = doc;
    doc->addListener(this);
    if (!doc->getUserData())
        doc->setUserData(new EditorViewData());
    EditorViewData* userData = (EditorViewData*)doc->getUserData();
    userDataIndex = userData->getNewId();
    }

void EditorView::cursorMoved(EditorDoc& doc)
    {
    if(docCursorMoved)
       docCursorMoved(); 
    resetCursorBlink();
    }

void EditorView::paragraphChanged(EditorDoc& doc, EditorDoc::Paragraph& para)
    {
    if (!para.data)
        para.data = std::make_shared<ParaData>();
    auto& pf = formats(para)[userDataIndex];
    pf.fBlob = nullptr;
    pf.fShaped = false;
    pf.fWordBoundaries = std::vector<bool>();
    fNeedsReshape = true;
    if (docChanged)
        docChanged();
    }

void EditorView::setFont(SkFont font) {
    if (font != fFont) {
        fFont = std::move(font);
        fNeedsReshape = true;
        if(doc)
            for (auto& para : doc->fParas) { getFormat(para).fShaped = false; }
        }
    }
    
static constexpr SkRect kUnsetRect{ -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
static SkPoint to_point(SkIPoint p) { return { (float)p.x(), (float)p.y() }; }

TextPosition EditorView::getPosition(SkPoint xy) {
    TextPosition approximatePosition{ 0, 0 };
    this->reshapeAll();
    for (size_t iPara = 0; iPara < doc->fParas.size(); ++iPara)
        {
        EditorDoc::Paragraph& para = doc->fParas[iPara];
        auto& pf = getFormat(para);
        SkIRect lineRect = { 0,
                            pf.fOrigin.y(),
                            (int)width,
                            iPara + 1 < doc->fParas.size() ? getFormat(doc->fParas[iPara + 1]).fOrigin.y() : INT_MAX };
        if (const SkTextBlob* b = pf.fBlob.get()) {
            SkIRect r = b->bounds().roundOut();
            r.offset(pf.fOrigin);
            lineRect.join(r);
            }
        if (!lineRect.contains((int)xy.x(), (int)xy.y())) {
            continue;
            }
        SkPoint pt = xy - to_point(pf.fOrigin);
        const std::vector<SkRect>& pos = pf.fCursorPos;
        for (size_t iByte = 0; iByte < pos.size(); ++iByte) {
            if (pos[iByte] != kUnsetRect && pos[iByte].contains(pt.x(), pt.y())) {
                return TextPosition{ iPara, iByte };
                }
            }
        approximatePosition.Para = iPara;
        approximatePosition.Byte = xy.x() <= pf.fOrigin.x() ? 0 : para.fText.size();
        }
    return approximatePosition;
    }

void EditorView::reshapeAll() {
    if (!fNeedsReshape)
        return;
    //shape each paragraph, for a given width
    for (EditorDoc::Paragraph& para : doc->fParas) 
        {
        auto& pf = getFormat(para);
        if (!pf.fShaped) {
            ShapeResult result = shapeTextToWidth(para.fText.begin(), para.fText.size(), fFont, fLocale, (float)width);
            pf.fBlob = std::move(result.blob);
            pf.fLineEndOffsets = std::move(result.lineBreakOffsets);
            pf.fCursorPos = std::move(result.glyphBounds);
            pf.fWordBoundaries = std::move(result.wordBreaks);
            pf.fHeight = result.verticalAdvance;
            pf.fShaped = true;
            }
        }
    //position paragraphs below each other.
    int y = 0;
    for (EditorDoc::Paragraph& para : doc->fParas) 
        {
        auto& pf = getFormat(para);
        pf.fOrigin = { 0, y };
        y += pf.fHeight;
        }
    fullTextHeight = (SkScalar)y;
    fNeedsReshape = false;
    }

static inline SkRect offset(SkRect r, SkIPoint p) { return r.makeOffset((float)p.x(), (float)p.y()); }

SkRect EditorView::getTextLocation(TextPosition cursor) {
    reshapeAll();
    cursor = doc->refitPosition(cursor);
    if (doc->fParas.size() > 0) {
        EditorDoc::Paragraph& para = doc->fParas[cursor.Para];
        auto& pf = getFormat(para);
        SkRect pos = { 0, 0, 0, 0 };
        if (cursor.Byte < pf.fCursorPos.size()) {
            pos = pf.fCursorPos[cursor.Byte];
            }
        pos.fRight = pos.fLeft + 0.5f;
        pos.fLeft -= 1;
        return offset(pos, pf.fOrigin);
        }
    return SkRect{ 0, 0, 0, 0 };
    }

void EditorView::paint(SkCanvas& canvas)
    {
    reshapeAll();

    //paint selection
    if (doc->hasSelection()) 
        {
        TextPosition fSelectionBegin;
        TextPosition fSelectionEnd;
        fSelectionBegin = doc->getSelectionPos();
        fSelectionEnd = doc->getCursorPos();
        SkColor4f selColor = fSelectionColor;
        if(!this->uiElement->hasFocus())
            selColor.fA /= 4;
        SkPaint selection = SkPaint(selColor);
        for (TextPosition pos = std::min(fSelectionBegin, fSelectionEnd),
                end = std::max(fSelectionBegin, fSelectionEnd);
                pos < end;
                pos = getPositionMoved(Movement::kRight, pos))
            {
            EditorDoc::Paragraph& para = doc->fParas[pos.Para];
            auto& pf = getFormat(para);

            canvas.drawRect(offset(pf.fCursorPos[pos.Byte], pf.fOrigin), selection);
            }
        }
        
    //paint cursor
    if (cursorBlinkOn && showCursor && this->uiElement->hasFocus())
        {
        SkPaint cursorPaint(fCursorColor);
        cursorPaint.setAntiAlias(false);
        canvas.drawRect(getTextLocation(doc->getCursorPos()), cursorPaint);
        }
        
    //paint text
    SkPaint foreground = SkPaint(fForegroundColor);
    for (EditorDoc::Paragraph& para : doc->fParas) {
        auto& pf = getFormat(para);
        if (pf.fBlob) {
            canvas.drawTextBlob(pf.fBlob.get(), (SkScalar)pf.fOrigin.fX, (SkScalar)pf.fOrigin.fY, foreground);
            }
        }
    }

void EditorView::resetCursorBlink()
    {
    startCursorTime = std::chrono::steady_clock::now();
    cursorBlinkOn = true;
    }

bool EditorView::onIdle()
    {
    auto ellapsedTime = std::chrono::steady_clock::now() - startCursorTime;
    if (ellapsedTime > std::chrono::milliseconds(500)) //TODO: make user defined
        {
        cursorBlinkOn = !cursorBlinkOn;
        startCursorTime = std::chrono::steady_clock::now();
        return this->showCursor; //only update window if cursor is actually shown.
        }
    return false;
    }

template <typename T>
static size_t find_first_larger(const std::vector<T>& list, T value) {
    return (size_t)(std::upper_bound(list.begin(), list.end(), value) - list.begin());
    }

static size_t find_closest_x(const std::vector<SkRect>& bounds, float x, size_t b, size_t e) {
    if (b >= e) {
        return b;
        }
    SkASSERT(e <= bounds.size());
    size_t best_index = b;
    float best_diff = ::fabsf(bounds[best_index].x() - x);
    for (size_t i = b + 1; i < e; ++i) {
        float d = ::fabsf(bounds[i].x() - x);
        if (d < best_diff) {
            best_diff = d;
            best_index = i;
            }
        }
    return best_index;
    }

TextPosition EditorView::getPositionMoved(Movement m, TextPosition pos)
    {
    if (doc->fParas.empty()) {
        return { 0, 0 };
        }
    pos = doc->refitPosition(pos);
    switch (m) {
        case Movement::kNowhere:
            break;
        case Movement::kLeft:
            pos = doc->getPositionRelative(pos, false);
            break;
        case Movement::kRight:
            pos = doc->getPositionRelative(pos, true);
            break;
        case Movement::kHome:
            {
            const std::vector<size_t>& list = getFormat(doc->fParas[pos.Para]).fLineEndOffsets;
            size_t f = find_first_larger(list, pos.Byte);
            pos.Byte = f > 0 ? list[f - 1] : 0;
            }
            break;
        case Movement::kEnd:
            {
            const std::vector<size_t>& list = getFormat(doc->fParas[pos.Para]).fLineEndOffsets;
            size_t f = find_first_larger(list, pos.Byte);
            if (f < list.size()) {
                pos.Byte = list[f] > 0 ? list[f] - 1 : 0;
                }
            else {
                pos.Byte = doc->fParas[pos.Para].fText.size();
                }
            }
            break;
        case Movement::kUp:
            {
            SkASSERT(pos.Byte < getFormat(doc->fParas[pos.Para]).fCursorPos.size());
            float x = getFormat(doc->fParas[pos.Para]).fCursorPos[pos.Byte].left();
            const std::vector<size_t>& list = getFormat(doc->fParas[pos.Para]).fLineEndOffsets;
            size_t f = find_first_larger(list, pos.Byte);
            // list[f] > value.  value > list[f-1]
            if (f > 0) {
                // not the first line in paragraph.
                pos.Byte = find_closest_x(getFormat(doc->fParas[pos.Para]).fCursorPos, x,
                                                    (f == 1) ? 0 : list[f - 2],
                                                    list[f - 1]);
                }
            else if (pos.Para > 0) 
                {
                --pos.Para;
                auto& newPara = doc->fParas[pos.Para];
                auto& pf = getFormat(newPara);
                size_t r = pf.fLineEndOffsets.size();
                if (r > 0) 
                    {
                    pos.Byte = find_closest_x(pf.fCursorPos, x, pf.fLineEndOffsets[r - 1], pf.fCursorPos.size());
                    }
                else 
                    {
                    pos.Byte = find_closest_x(pf.fCursorPos, x, 0, pf.fCursorPos.size());
                    }
                }
            pos.Byte =
                align_column(doc->fParas[pos.Para].fText, pos.Byte);
            }
            break;
        case Movement::kDown:
            {
            const std::vector<size_t>& list = getFormat(doc->fParas[pos.Para]).fLineEndOffsets;
            float x = getFormat(doc->fParas[pos.Para]).fCursorPos[pos.Byte].left();

            size_t f = find_first_larger(list, pos.Byte);
            if (f < list.size()) 
                {
                const auto& bounds = getFormat(doc->fParas[pos.Para]).fCursorPos;
                pos.Byte = find_closest_x(bounds, x, list[f], f + 1 < list.size() ? list[f + 1] : bounds.size());
                }
            else if (pos.Para + 1 < doc->fParas.size()) 
                {
                ++pos.Para;
                const auto& bounds = getFormat(doc->fParas[pos.Para]).fCursorPos;
                const std::vector<size_t>& l2 = getFormat(doc->fParas[pos.Para]).fLineEndOffsets;
                pos.Byte = find_closest_x(bounds, x, 0, l2.size() > 0 ? l2[0] : bounds.size());
                }
            pos.Byte =
                align_column(doc->fParas[pos.Para].fText, pos.Byte);
            }
            break;
        case Movement::kWordLeft:
            {
            if (pos.Byte == 0) {
                pos = getPositionMoved(Movement::kLeft, pos);
                break;
                }
            const std::vector<bool>& words = getFormat(doc->fParas[pos.Para]).fWordBoundaries;
            SkASSERT(words.size() == doc->fParas[pos.Para].fText.size());
            do {
                --pos.Byte;
                } while (pos.Byte > 0 && !words[pos.Byte]);
            }
            break;
        case Movement::kWordRight:
            {
            const TextBuffer& text = doc->fParas[pos.Para].fText;
            if (pos.Byte == text.size()) {
                pos = getPositionMoved(Movement::kRight, pos);
                break;
                }
            const std::vector<bool>& words = getFormat(doc->fParas[pos.Para]).fWordBoundaries;
            SkASSERT(words.size() == text.size());
            do {
                ++pos.Byte;
                } while (pos.Byte < text.size() && !words[pos.Byte]);
            }
            break;

        }
    return pos;
    }

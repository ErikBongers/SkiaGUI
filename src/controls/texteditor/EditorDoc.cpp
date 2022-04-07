#include "EditorDoc.h"
#include "utf8_helpers.h"

using namespace SkEd;

void EditorDoc::insert(const char* utf8Text, size_t byteLen)
    {
    if (!canInsert(utf8Text, byteLen))
        return;
    CmdInsert* cmdInsert = new CmdInsert(*this);
    cmdInsert->str.append(utf8Text, byteLen);
    cmdInsert->saveSelBefore();
    undoRedo.execute(cmdInsert);
    }

void SkEd::CmdInsert::execute()
    {
    setSelBefore();
    if (doc.hasSelection())
        {
        strBefore = doc.selectionToString();
        doc._remove();
        }
    doc._insert(str.c_str(), str.size());
    cursorPosAfter = doc.getCursorPos();
    }

void SkEd::CmdInsert::undo()
    {
    doc.setCursor(std::min(selectPosBefore, cursorPosBefore));
    doc.setCursor(cursorPosAfter, true);
    doc._remove();
    if (!strBefore.empty())
        doc._insert(strBefore.c_str(), strBefore.length());
    setSelBefore();
    }

void SkEd::EditorDoc::remove(bool backSpace)
    {
    CmdRemove* cmdRemove = new CmdRemove(*this);
    cmdRemove->backspace = backSpace;
    cmdRemove->saveSelBefore();
    undoRedo.execute(cmdRemove);
    }

void SkEd::CmdRemove::execute()
    {
    setSelBefore();
    if (!doc.hasSelection())
        doc.moveCursor(!backspace, true);
    strBefore = doc.selectionToString();
    doc._remove();
    cursorPosAfter = doc.getCursorPos();
    }

void SkEd::CmdRemove::undo()
    {
    doc.setCursor(cursorPosAfter);
    doc._insert(strBefore.c_str(), strBefore.length());
    setSelBefore();
    }

bool EditorDoc::canInsert(const char* utf8Text, size_t byteLen)
    {
    if (!valid_utf8(utf8Text, byteLen) || 0 == byteLen)
        return false;
    if (maxLength && SkUTF::CountUTF8(utf8Text, byteLen) + sizeUtf8() > maxLength)
        return false;
    return true;
    }

void EditorDoc::_insert(const char* utf8Text, size_t byteLen) {
    if (!canInsert(utf8Text, byteLen))
        return;
    TextBuffer txt(utf8Text, byteLen);
    LineLooper looper(utf8Text, byteLen);
    for(auto it : looper)
        {
        if (it.newLine)
            {
            Paragraph newPara;
            if (maxParagraphs && fParas.size() >= maxParagraphs)
                break;
            //split current line at current cursorLoc.
            const char* strAfterSplitPart = fParas[fCursorPos.Para].fText.begin() + fCursorPos.Byte;
            newPara.fText.insert(0, strAfterSplitPart, fParas[fCursorPos.Para].fText.end() - strAfterSplitPart);
            fParas.insert(this->fParas.begin() + fCursorPos.Para + 1, newPara);
            fParas[fCursorPos.Para].fText.trim(fCursorPos.Byte);
            fireParagraphChanged(fParas[fCursorPos.Para]);
            fCursorPos.Para++;
            fCursorPos.Byte = 0;
            }
        fParas[fCursorPos.Para].fText.insert(fCursorPos.Byte, it.startOfLine, it.len);
        fireParagraphChanged(this->fParas[fCursorPos.Para]);
        fCursorPos.Byte += it.len;
        }
    selectionPos = fCursorPos;
    return;
    }

void EditorDoc::_remove()
    {
    remove(fCursorPos, selectionPos);
    }

SkEd::EditorDoc::EditorDoc() 
    { 
    fParas.push_back(EditorDoc::Paragraph());
    fCursorPos = { 0, 0 };
    selectionPos = fCursorPos;
    }

EditorDoc& SkEd::EditorDoc::operator=(const std::string str)
    {
    fParas.clear();
    fParas.push_back(EditorDoc::Paragraph()); // a doc has always at least 1 paragraph.
    _insert(str.c_str(), str.length());
    undoRedo.clear();
    return *this;
    }

void SkEd::EditorDoc::fireParagraphChanged(Paragraph& para) { for (auto l : listeners) l->paragraphChanged(*this, para); }

void SkEd::EditorDoc::fireCursorMoved() { for (auto l : listeners) l->cursorMoved(*this); }

void EditorDoc::remove(TextPosition pos1, TextPosition pos2) {
    TextPosition start = std::min(pos1, pos2);
    TextPosition end = std::max(pos1, pos2);
    
    if (start == end || start.Para == fParas.size()) 
        return;
    
    if (start.Para == end.Para) 
        {
        fParas[start.Para].fText.remove(
            start.Byte, end.Byte - start.Byte);
        fireParagraphChanged(fParas[start.Para]);
        }
    else 
        {
        auto& line = fParas[start.Para];
        line.fText.remove(start.Byte,
                          line.fText.size() - start.Byte);
        line.fText.insert(start.Byte,
                          fParas[end.Para].fText.begin() + end.Byte,
                          fParas[end.Para].fText.size() - end.Byte);
        fireParagraphChanged(line);
        fParas.erase(fParas.begin() + start.Para + 1,
                     fParas.begin() + end.Para + 1);
        }
    fCursorPos = selectionPos = start;
    fireCursorMoved();
    }

bool EditorDoc::setCursor(TextPosition pos, bool expandSelection)
    {
    if (pos == fCursorPos)
        return false;

    if (expandSelection)
        fCursorPos = pos;
    else
        selectionPos = fCursorPos = pos;
    fireCursorMoved();
    return true;
    }

size_t SkEd::EditorDoc::sizeUtf8()
    {
    size_t size = 0;
    for (auto para : fParas)
        size += para.fText.count_utf8();
    return size;
    }


std::string EditorDoc::selectionToString() const
    {
    return toString(fCursorPos, selectionPos);
    }

std::string EditorDoc::toString() const
    {
    return toString({ 0, 0 }, { fParas.size() - 1, fParas[fParas.size() - 1].fText.size() });
    }

std::string EditorDoc::toString(TextPosition startPos, TextPosition endPos) const
    {
    std::string str;
    TextPosition start = std::min(startPos, endPos);
    TextPosition end = std::max(startPos, endPos);
    if (start == end)
        return "";
    if (start.Para == end.Para)
        {
        auto& fText = fParas[start.Para].fText;
        str.append(fText.begin() + start.Byte, end.Byte - start.Byte);
        return str;
        }
    const std::vector<Paragraph>::const_iterator firstP = fParas.begin() + start.Para;
    const std::vector<Paragraph>::const_iterator lastP = fParas.begin() + end.Para;
    const auto& first = firstP->fText;
    const auto& last = lastP->fText;

    str.append(first.begin() + start.Byte, first.size() - start.Byte);
    for (auto para = firstP + 1; para < lastP; ++para) {
        str += "\n";
        str.append(para->fText.begin(), para->fText.size());
        }
    str += '\n';
    str.append(last.begin(), end.Byte);
    return str;
    }

void SkEd::EditorDoc::refitSelection()
    {
    fCursorPos = refitPosition(fCursorPos);
    selectionPos = refitPosition(selectionPos);
    }

TextPosition SkEd::EditorDoc::refitPosition(TextPosition pos)
    {
    if (pos.Para >= fParas.size()) {
        pos.Para = fParas.size() - 1;
        pos.Byte = fParas[pos.Para].fText.size();
        }
    else {
        pos.Byte = align_column(fParas[pos.Para].fText, pos.Byte);
        }
    return pos;
    }

void SkEd::EditorDoc::moveCursor(bool right, bool expandSelection)
    {
    if (expandSelection)
        if (fCursorPos == selectionPos)
            selectionPos = fCursorPos;
    fCursorPos = getPositionRelative(fCursorPos, right);
    }

TextPosition SkEd::EditorDoc::getPositionRelative(TextPosition pos, bool right)
    {
    if (right)
        {
        if (fParas[pos.Para].fText.size() == pos.Byte) {
            if (pos.Para + 1 < fParas.size()) {
                ++pos.Para;
                pos.Byte = 0;
                }
            }
        else {
            const auto& str = fParas[pos.Para].fText;
            pos.Byte =
                next_utf8(begin(str) + pos.Byte, end(str)) - begin(str);
            }
        }
    else
        {
        if (0 == pos.Byte) {
            if (pos.Para > 0) {
                --pos.Para;
                pos.Byte = fParas[pos.Para].fText.size();
                }
            }
        else {
            const auto& str = fParas[pos.Para].fText;
            pos.Byte =
                prev_utf8(begin(str) + pos.Byte, begin(str)) - begin(str);
            }
        }
    return pos;
    }

void SkEd::DocCmd::saveSelBefore() {
    cursorPosBefore = doc.getCursorPos();
    selectPosBefore = doc.getSelectionPos();
    }

void SkEd::DocCmd::setSelBefore()
    {
    doc.setCursor(selectPosBefore);
    doc.setCursor(cursorPosBefore, true);
    }

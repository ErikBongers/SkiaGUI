#pragma once
#include "../../pch.h"
#include "TextBuffer.h"
#include "TextSpan.h"
#include "UndoRedo.h"

namespace SkEd
{
class LineLooper
    {
    private:
        const char* data;
        const char* pEnd;
        const char* findNewLine(const char* ptr)
            {
            while (ptr < pEnd)
                {
                if (*ptr == '\n')
                    return ptr;
                ptr++;
                }
            return ptr;
            }
    public:
        class LineSegment {
            private:
                LineLooper* looper;
                LineSegment(LineLooper* looper) : looper(looper) {}
            public:
                const char* startOfLine;
                size_t len;
                bool newLine = false;
                bool operator==(const LineSegment& segment) const { return segment.startOfLine == startOfLine && segment.newLine == newLine; }
                bool operator!=(const LineSegment& segment) const { return !(segment==(*this)); }
                LineSegment& operator++() 
                    {
                    startOfLine = startOfLine + len;
                    if (startOfLine == looper->pEnd)
                        {
                        //if our buffer ends with a newline, a segment is sent that starts at pEnd, len=0.
                        //Therefor, end() is not just defined by pEnd, but also by newLine = false.
                        newLine = false; //make *this = end()
                        return *this;
                        }
                    newLine = true;
                    startOfLine++; //skip newline char
                    len = looper->findNewLine(startOfLine) - startOfLine;
                    return *this;
                    }
                LineSegment operator++(int)
                    {
                    LineSegment tmp(*this);
                    operator++();
                    return tmp;
                    }
                LineSegment& operator*() 
                    {
                    return *this;
                    }
                friend class LineLooper;
            };
        LineLooper(const char* data, size_t size) : data(data), pEnd(data+size), _begin(this), _end(this)
            {
            _begin.startOfLine = this->data;
            _begin.len = findNewLine(this->data) - this->data;
            _begin.newLine = false;
            _end.startOfLine = pEnd;
            _end.newLine = false; //see comment above...
            }
        LineSegment& begin() { return _begin; }
        LineSegment& end() { return _end; }
    private:
        LineSegment _begin;
        LineSegment _end;
    };

static bool valid_utf8(const char* ptr, size_t size) { return SkUTF::CountUTF8(ptr, size) >= 0; }

struct TextPosition {
    size_t Para = SIZE_MAX;  // logical line, based on hard newline characters.
    size_t Byte = SIZE_MAX;   // index into UTF-8 representation of line.
    operator bool() const { return Byte != SIZE_MAX && Para != SIZE_MAX; }
    bool operator==(const TextPosition& v) const { return Para == v.Para && Byte == v.Byte; }
    bool operator!=(const TextPosition& v) const { return !this->operator==(v); }
    bool operator<(const SkEd::TextPosition& v) const {
        bool test1 = Para < v.Para; //todo: get rid of this
        return test1 || (Para == v.Para && Byte < v.Byte); 
        }
    };

enum class Movement { kNowhere, kLeft, kUp, kRight, kDown, kHome, kEnd, kWordLeft, kWordRight, };

class EditorDoc;

class DocCmd : public Cmd
    {
    public:
        TextPosition cursorPosBefore;
        TextPosition selectPosBefore;
        TextPosition cursorPosAfter;
        EditorDoc& doc;
        DocCmd(EditorDoc& doc) : doc(doc) {}
        void saveSelBefore();
        void setSelBefore();
    };

class CmdInsert : public DocCmd
    {
    public:
        CmdInsert(EditorDoc& doc) : DocCmd(doc) {}
        std::string str;
        std::string strBefore;

        virtual void execute() override;
        virtual void undo() override;
    };

class CmdRemove : public DocCmd
    {
    public:
        CmdRemove(EditorDoc& doc) : DocCmd(doc), backspace(false) {}
        std::string strBefore;
        bool backspace = false;

        virtual void execute() override;
        virtual void undo() override;
    };


class EditorDocData
    {
    public:
        virtual ~EditorDocData() {}
    };

class EditorDocListener;

class EditorDoc
    {
    public:
        struct Paragraph {
            TextBuffer fText;
            Paragraph() {}
            std::shared_ptr<void> data = nullptr;
            };

        std::vector<Paragraph> fParas;
        std::vector<EditorDocListener*> listeners;

        void insert(const char* utf8Text, size_t byteLen);
        void insert(const std::string& str) { insert(str.c_str(), str.size()); }
        void remove(bool backSpace = false);
        bool setCursor(TextPosition pos, bool expandSelection = false);

        bool canInsert(const char* utf8Text, size_t byteLen);
        size_t sizeUtf8();
        size_t lineCount() const { return fParas.size(); }
        TextSpan line(size_t i) const { return i < fParas.size() ? fParas[i].fText.view() : TextSpan{ nullptr, 0 }; }
        std::string toString() const;
        std::string toString(TextPosition startPos, TextPosition endPos) const;
        std::string selectionToString() const;
        bool hasSelection() const { return fCursorPos != selectionPos; }
        void refitSelection();
        TextPosition refitPosition(TextPosition pos);
        void moveCursor(bool right, bool expandSelection = false);
        TextPosition getPositionRelative(TextPosition pos, bool right);
        TextPosition getCursorPos() { return fCursorPos; }
        TextPosition getSelectionPos() { return selectionPos; }
        void undo() { undoRedo.undo(); }
        void redo() { undoRedo.redo(); }
        EditorDoc();
        unsigned int maxParagraphs = 0;
        unsigned int maxWidth = 0;
        unsigned int maxLength = 0;
        EditorDoc& operator=(const std::string str);
        void addListener(EditorDocListener* listener) { listeners.push_back(listener); }
        //setUserData: EditorDoc will be owner of data and delete it on destruction!
        void setUserData(EditorDocData* data) { if (userData) delete userData; userData = data; }
        EditorDocData* getUserData() { return userData; }
        ~EditorDoc() { if (userData) delete userData; }
    private:
        EditorDocData* userData = nullptr;
        TextPosition fCursorPos{ 0, 0 };
        TextPosition selectionPos{ 0, 0 };
        UndoRedo undoRedo;

        void fireParagraphChanged(Paragraph& para);
        void fireCursorMoved();
        void remove(TextPosition, TextPosition);
        void _insert(const char* utf8Text, size_t byteLen);
        void _remove();
        friend class CmdInsert;
        friend class CmdRemove;
    };

class EditorDocListener
    {
    public:
        virtual void paragraphChanged(EditorDoc& doc, EditorDoc::Paragraph& para) {}
        virtual void cursorMoved(EditorDoc& doc) {}
    };

}


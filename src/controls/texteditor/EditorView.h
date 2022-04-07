#pragma once
#include "../../pch.h"
#include <climits>
#include <utility>
#include <cstddef>
#include "EditorDoc.h"
#include "core/UIElement.h"

namespace SkEd {

typedef std::function<void()> PDocChanged;
typedef std::function<void()> PCursorMoved;

class EditorView;

class EditorViewData : public EditorDocData
    {
    private:
        int viewerCount = 0;
    public:
        int getNewId() { return viewerCount++; }
    };

class EditorView : public EditorDocListener
    {
    private:
        bool fNeedsReshape = true;
        SkScalar width = 0;
        SkScalar fullTextHeight = 0;
        void resetCursorBlink();
        unsigned int userDataIndex = -1;
        UIElement* uiElement = nullptr;
    public:
        virtual void paragraphChanged(EditorDoc& doc, EditorDoc::Paragraph& para);
        virtual void cursorMoved(EditorDoc& doc);

        struct ParagraphFormat {
            sk_sp<const SkTextBlob> fBlob;
            std::vector<SkRect> fCursorPos;
            std::vector<size_t> fLineEndOffsets;
            std::vector<bool> fWordBoundaries;
            SkIPoint fOrigin = { 0, 0 };
            int fHeight = 0;
            bool fShaped = false;
            };

        class ParaData
            {
            private:
                std::vector<std::shared_ptr<ParagraphFormat>> formats;
            public:
                ParagraphFormat& operator[](std::size_t i);
            };

        EditorDoc* doc;
        SkFont fFont;
        const char* fLocale = "en"; //TODO: should this not be in Doc?
        bool showCursor = false;
        std::chrono::time_point<std::chrono::steady_clock> startCursorTime; //TODO: add cursor blinktime user defined.
        bool cursorBlinkOn = false;
        SkColor4f fForegroundColor = { 1, 1, 1, 1 };
        SkColor4f fSelectionColor = { 0.729f, 0.827f, 0.988f, 1 };
        SkColor4f fCursorColor = { 1, 0, 0, 1 };
        
        void attachDoc(EditorDoc* doc);
        ParaData& formats(EditorDoc::Paragraph& para) 
            { 
            if (!para.data)
                para.data = std::make_shared<ParaData>();
            return *std::static_pointer_cast<ParaData>(para.data); 
            }
        ParagraphFormat& getFormat(EditorDoc::Paragraph& para) 
            { 
            return formats(para)[userDataIndex]; 
            }
        void setFont(SkFont font);
        void setWidth(SkScalar w) {
            if (width != w) {
                width = w;
                fNeedsReshape = true;
                if(doc)
                    for (auto& para : doc->fParas) { getFormat(para).fShaped = false; }
                }
            }
        SkScalar getFullTextHeight() { return fullTextHeight; }
        TextPosition getPosition(SkPoint);
        void reshapeAll();
        SkRect getTextLocation(TextPosition);
        void paint(SkCanvas& canvas);
        bool onIdle();
        TextPosition getPositionMoved(Movement m, TextPosition pos);
        PDocChanged docChanged = nullptr;
        PCursorMoved docCursorMoved = nullptr;
        friend class Editor;
    };
}


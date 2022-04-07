#pragma once
#include "pch.h"
#include "core/UIElement.h"

class InfiniteCanvasListener;

struct DragDropInfo
    {
    SkPoint startPosMouse;
    SkPoint startPos;
    SkPoint delta;
    SkPoint pos;
    SkPoint posMouse;
    };

class InfiniteCanvas : public UIElement
    {
    protected:
        void redraw() override;
    private:
        SkPoint dragStartMousePos;
        SkPoint dragStartPos;
        bool draggingSelf = false;
        bool isDragging = false;
        bool dragMoved = false; //only set when after mouseDown, an actual mouseMove occurred.

        SkMatrix baseTransform;
    
    public:
        virtual ~InfiniteCanvas() {}
        InfiniteCanvasListener* dragListener = nullptr;
        void _mouseMove(SDL_MouseMotionEvent& event) override;
        bool _mouseDown(SDL_MouseButtonEvent& event) override;
        bool _mouseUp(SDL_MouseButtonEvent& event) override;
        void _mouseWheel(SDL_MouseWheelEvent_EX& event) override;
        DragDropInfo buildDragDropInfo(Sint32 x, Sint32 y);
        SkScalar xTranslate = 0, yTranslate = 0;
        SkScalar scaleFactor = 1;
        SkScalar scaleSpeed = 0.1f;
        SkRect getVisibleArea();
    };

class InfiniteCanvasListener
    {
    public:
        virtual bool requestDrag(InfiniteCanvas& canvas, DragDropInfo info) { return true; }
        virtual void dragging(InfiniteCanvas& canvas, DragDropInfo info) { }
        virtual void dropped(InfiniteCanvas& canvas, DragDropInfo info) { }
        virtual void mouseUpNoDrag(InfiniteCanvas& canvas, SDL_MouseButtonEvent& event) {}
        virtual void canvasTransformed(InfiniteCanvas& canvas) {}
    };


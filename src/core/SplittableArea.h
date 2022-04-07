#pragma once
#include "../pch.h"
#include "UIArea.h"


class SplittableArea : public UIArea
    {
    private:
        SplittableArea* myCreatedView = nullptr;
        SkRect oldSize;
        Sint32 startDragMousePos;
        SkScalar startSplitPos;
        bool dragging = false;
        void resizeViews();
    protected:
        UIArea* area = nullptr; //always fills the view.
        //OR
        SplittableArea* view1 = nullptr; // can provide a min and max size.
        SplittableArea* view2 = nullptr;

        enum class Direction { LEFT_RIGHT, ABOVE_BELOW};
        Direction splitDirection = Direction::LEFT_RIGHT;
        SkScalar splitPoint = -1;
        void _resizeContent() override;
        SkRect rectGap;
    public:
        SkScalar mindTheGap = 4;
        enum class PrefSize { PROPORTIONAL = 0, DONTCARE = -1, FIXED = 1 };
        struct MinMax {
            SkScalar min = 0;
            SkScalar max = SK_ScalarMax/2; // half, to avoid overflow.
            PrefSize prefSize = PrefSize::PROPORTIONAL;
            };
        struct SizePrefs {
            static const int WIDHT_IDX = 0;
            static const int HEIGHT_IDX = 1;
            MinMax widthPrefs;
            MinMax heightPrefs;
            MinMax& operator [](int i) { return i ? heightPrefs : widthPrefs; }
            };
        SizePrefs sizePrefs;
        SizePrefs getSizePrefs();
        SizePrefs defaultSizePrefs;
        void redraw() override;
        virtual void trickleResizeEvent() override;
        virtual bool trickleMouseMoveEvent(SDL_MouseMotionEvent& event) override;
        virtual bool trickleMouseUpEvent(SDL_MouseButtonEvent& event) override;
        virtual bool trickleMouseDownEvent(SDL_MouseButtonEvent& event) override;
        virtual bool trickleMouseWheelEvent(SDL_MouseWheelEvent_EX& event) override;
        virtual void trickleIdle() override;
        virtual void trickleKeyDown(SDL_KeyboardEvent& event) override;
        virtual void trickleTextEvent(SDL_TextInputEvent& event) override;

        bool _mouseDown(SDL_MouseButtonEvent& event) override;
        void _mouseMove(SDL_MouseMotionEvent& event) override;
        bool _mouseUp(SDL_MouseButtonEvent& event) override;

        void setContent(UIArea* area);
        UIArea& getContent() { return *area; }
        enum class Location { BELOW, ABOVE, LEFT, RIGHT };
        void splitView(SplittableArea* secondView, Location loc);
        friend class SDLSkiaWindow;
        friend class WindowClient;
        virtual ~SplittableArea() { if (myCreatedView) delete myCreatedView; }
    };
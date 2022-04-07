#pragma once
#include "UIArea.h"

class UIElement : public UIArea
    {
    public:
        virtual ~UIElement() {}
        SkColor backgroundColor = SK_ColorTRANSPARENT;
        std::vector<UIArea*> children;
        UIElement& operator+=(UIArea* child);
        UIElement& operator+=(UIArea& child) { return *this += &child; }
        
        virtual void trickleResizeEvent() override;
        virtual bool trickleMouseMoveEvent(SDL_MouseMotionEvent& event) override;
        virtual bool trickleMouseUpEvent(SDL_MouseButtonEvent& event) override;
        virtual bool trickleMouseDownEvent(SDL_MouseButtonEvent& event) override;
        virtual bool trickleMouseWheelEvent(SDL_MouseWheelEvent_EX& event) override;
        virtual void trickleIdle() override;
        virtual void trickleKeyDown(SDL_KeyboardEvent& event) override;
        virtual void trickleTextEvent(SDL_TextInputEvent& event) override;
        //Called after painting background and before drawing children.
        virtual void drawMe() {} 
        UIArea* getChildAt(SkScalar x, SkScalar y);

    protected:
        void drawBackground();
        void drawChildren();
        virtual void redraw() override;
    };
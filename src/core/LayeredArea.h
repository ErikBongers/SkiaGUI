#pragma once
#include "../pch.h"
#include "UIArea.h"


class LayeredArea : public UIArea
    {
    protected:
        UIArea& areaContent;
        UIArea& areaOverlay;

    public:
        void redraw() override;
        virtual void trickleResizeEvent() override;
        virtual bool trickleMouseMoveEvent(SDL_MouseMotionEvent& event) override;
        virtual bool trickleMouseUpEvent(SDL_MouseButtonEvent& event) override;
        virtual bool trickleMouseDownEvent(SDL_MouseButtonEvent& event) override;
        virtual bool trickleMouseWheelEvent(SDL_MouseWheelEvent_EX& event) override;
        virtual void trickleIdle() override;
        virtual void trickleKeyDown(SDL_KeyboardEvent& event) override;
        virtual void trickleTextEvent(SDL_TextInputEvent& event) override;

        UIArea& content() { return areaContent; }
        
        friend class SDLSkiaWindow;
        friend class WindowClient;
        LayeredArea(UIArea& content, UIArea& overlay);
        virtual ~LayeredArea() {}
    };
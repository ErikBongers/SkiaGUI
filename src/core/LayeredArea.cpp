#include "LayeredArea.h"

void LayeredArea::redraw()
    {
        areaContent.transformAndDraw();
        areaOverlay.transformAndDraw();
    }

void LayeredArea::trickleResizeEvent()
    {
    _resizeContent();
    areaContent.rect = rect;
    areaOverlay.rect = rect;
    areaContent.trickleResizeEvent();
    areaOverlay.trickleResizeEvent();
    }

bool LayeredArea::trickleMouseMoveEvent(SDL_MouseMotionEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;

    _mouseMove(event);
    bool res = false;
    res |= areaContent.trickleMouseMoveEvent(event);
    res |= areaOverlay.trickleMouseMoveEvent(event);
    return res;
    }

bool LayeredArea::trickleMouseUpEvent(SDL_MouseButtonEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    _mouseUp(event);
    bool res = false;
    if (areaOverlay.trickleMouseUpEvent(event))
        return true;
    return areaContent.trickleMouseUpEvent(event);
    }
bool LayeredArea::trickleMouseDownEvent(SDL_MouseButtonEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    _mouseDown(event);
    bool res = false;
    if (areaOverlay.trickleMouseDownEvent(event))
        return true;
    return areaContent.trickleMouseDownEvent(event);
    }
bool LayeredArea::trickleMouseWheelEvent(SDL_MouseWheelEvent_EX& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    bool res = false;
    if (areaOverlay.trickleMouseWheelEvent(event))
        return true;
    return areaContent.trickleMouseWheelEvent(event);
    }
void LayeredArea::trickleIdle()
    {
    areaContent.trickleIdle();
    areaOverlay.trickleIdle();
    }
void LayeredArea::trickleKeyDown(SDL_KeyboardEvent& event)
    {
    areaContent.trickleKeyDown(event);
    areaOverlay.trickleKeyDown(event);
    }
void LayeredArea::trickleTextEvent(SDL_TextInputEvent& event)
    {
    areaContent.trickleTextEvent(event);
    areaOverlay.trickleTextEvent(event);
    }

LayeredArea::LayeredArea(UIArea& content, UIArea& overlay)
    : areaContent(content), areaOverlay(overlay)
    {
    content.parent = this;
    overlay.parent = this;
    }

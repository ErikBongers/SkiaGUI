#include "UIElement.h"
#include "UIArea.h"

void UIElement::redraw()
    {
    drawBackground();
    drawMe();
    drawChildren();
    }

void UIElement::drawChildren()
    {
    for (auto el : children)
        el->transformAndDraw();
    }

UIElement& UIElement::operator+=(UIArea* child)
    {
    children.push_back(child);
    child->parent = this;
    return *this;
    }


void UIElement::trickleIdle() 
    { 
    onIdle();  
    for (auto c : children) 
        c->trickleIdle(); 
    }

void UIElement::trickleKeyDown(SDL_KeyboardEvent& event) 
    { 
    keyDown(event);  
    for (auto c : children) 
        c->trickleKeyDown(event); 
    }

void UIElement::trickleTextEvent(SDL_TextInputEvent& event) 
    { 
    textInput(event);
    for (auto c : children) 
        c->trickleTextEvent(event); 
    }

UIArea* UIElement::getChildAt(SkScalar x, SkScalar y)
    {
    for (auto it = children.rbegin(); it != children.rend(); ++it) //reverse iterator!
        {
        auto element = *it;
        if (element->hitTest(x, y))
            return element;
        }
    return nullptr;
    }

void UIElement::trickleResizeEvent() 
    { 
    UIArea::trickleResizeEvent(); 
    for (auto el : children) 
        el->trickleResizeEvent(); 
    }

bool UIElement::trickleMouseMoveEvent(SDL_MouseMotionEvent& event)
    {
    if (!UIArea::trickleMouseMoveEvent(event))
        return false;
    for (auto el : children)
        if (el->trickleMouseMoveEvent(event))
            return true;
    return false;
    }

bool UIElement::trickleMouseWheelEvent(SDL_MouseWheelEvent_EX& event)
    {
    if (!UIArea::trickleMouseWheelEvent(event))
        return false;
    for (auto el : children)
        if (el->trickleMouseWheelEvent(event))
            return true;
    return false;
    }

bool UIElement::trickleMouseUpEvent(SDL_MouseButtonEvent& event)
    { 
    if (!hitTest(event.x, event.y))
        return false;

    if(UIArea::trickleMouseUpEvent(event))
      return true;
    for (auto el : children)
        if (el->trickleMouseUpEvent(event))
            return true; 
    return false;
    }

bool UIElement::trickleMouseDownEvent(SDL_MouseButtonEvent& event) 
    { 
    if (!hitTest(event.x, event.y))
        return false;

    if(UIArea::trickleMouseDownEvent(event))
        return true;
    for (auto el : children)
        if (el->trickleMouseDownEvent(event))
            return true;
    return false; 
    }

void UIElement::drawBackground()
    {
    if (backgroundColor != SK_ColorTRANSPARENT)
        {
        SkPaint paint;
        paint.setColor(backgroundColor);
        paint.setStyle(SkPaint::Style::kFill_Style);
        getWindow()->Canvas().drawRect(SkRect::MakeWH(rect.width(), rect.height()), paint);
        }
    }

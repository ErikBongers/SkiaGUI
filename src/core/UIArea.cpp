#include "UIArea.h"

SDLSkiaWindow* UIArea::getWindow()
    {
    if (!window && parent)
        window = parent->getWindow();
    return window;
    }

SplittableArea* UIArea::getView()
    {
    if (!view && parent)
        view = parent->getView();
    return view;
    }

void UIArea::_mouseMove(SDL_MouseMotionEvent& event)
    {
    if(!toolTipText.empty())
        getWindow()->setToolTip(this);
    }

void UIArea::setPos(SkScalar x, SkScalar y)
    {
    rect = SkRect::MakeXYWH(x, y, rect.width(), rect.height());
    if(window)
        window->setInvalid();
    }

void UIArea::transformAndDraw()
    {
    getWindow()->Canvas().save();
    getWindow()->Canvas().translate(rect.fLeft, rect.fTop);
    totalTransform = getWindow()->Canvas().getTotalMatrix();
    redraw();
    getWindow()->Canvas().restore();
    }

void UIArea::trickleResizeEvent()
    {
    if (resize)
        resize(*this);
    _resizeContent();
    }

bool UIArea::trickleMouseMoveEvent(SDL_MouseMotionEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    _mouseMove(event);
    if (mouseMove)
        mouseMove(*this, event);
    return true;
    }

bool UIArea::trickleMouseWheelEvent(SDL_MouseWheelEvent_EX& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    _mouseWheel(event);
    if (mouseWheel)
        mouseWheel(*this, event);
    return true;
    }

bool UIArea::trickleMouseUpEvent(SDL_MouseButtonEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    if (!getWindow()->isMouseCaptured(*this))
        {
        if (_mouseUp(event))
            return true;
        if (mouseUp)
            return mouseUp(*this, event);
        }
    return false;
    }

bool UIArea::trickleMouseDownEvent(SDL_MouseButtonEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    if (_mouseDown(event))
        return true;
    if (mouseDown)
        return mouseDown(*this, event);
    return false;
    }

void UIArea::mapPixelsToPoints(SkPoint* points, int count)
    {
    SkMatrix invertedMatrix;
    invertedMatrix.reset();
    totalTransform.invert(&invertedMatrix);
    invertedMatrix.mapPoints(points, count);
    }

void UIArea::mapPointsToPixels(SkPoint* dst, SkPoint* src, int count)
    {
    totalTransform.mapPoints(dst, src, count);
    }

bool UIArea::hitTest(const SkRect& rect, SkScalar x, SkScalar y)
    {
    SkPoint pos = SkPoint::Make(x, y);
    if (parent)
        parent->mapPixelsToPoints(&pos, 1);
    return rect.contains(pos.fX, pos.fY);
    }

SkRect UIArea::absoluteRect()
    {
    SkScalar xOffset, yOffset;
    xOffset = 0;
    yOffset = 0;
    UIArea* p = parent;
    while (p)
        {
        xOffset += p->rect.left();
        yOffset += p->rect.top();
        p = p->parent;
        }
    SkRect rectAbs = rect;
    rectAbs.offset(xOffset, yOffset);
    return rectAbs;
    }


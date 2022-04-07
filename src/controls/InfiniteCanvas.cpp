#include "pch.h"
#include "InfiniteCanvas.h"

void InfiniteCanvas::redraw()
    {
    baseTransform = getWindow()->Canvas().getTotalMatrix();
    drawBackground();
    drawMe();
    getWindow()->Canvas().clipRect(SkRect::MakeWH(rect.width(), rect.height()));
    getWindow()->Canvas().translate(xTranslate, yTranslate);
    getWindow()->Canvas().scale(scaleFactor, scaleFactor); //pre scaling!
    totalTransform = getWindow()->Canvas().getTotalMatrix();
    drawChildren();
    }

SkRect InfiniteCanvas::getVisibleArea()
    {
    SkRect area = absoluteRect();
    mapPixelsToPoints((SkPoint*)&area, 2);
    return area;
    }

bool InfiniteCanvas::_mouseDown(SDL_MouseButtonEvent& event)
    {
    if (isDragging)
        return true;
    dragMoved = false;
    draggingSelf = false;

    if (event.state == SDL_PRESSED && event.button == SDL_BUTTON_MIDDLE)
        {
        dragStartMousePos.set((SkScalar)event.x, (SkScalar)event.y);
        dragStartPos.set(xTranslate, yTranslate);
        isDragging = true;
        draggingSelf = true;
        return true;
        }
    if (getChildAt((SkScalar)event.x, (SkScalar)event.y))
        {
        //start drag of object
        dragStartMousePos.set((SkScalar)event.x, (SkScalar)event.y);

        DragDropInfo info;
        info.pos = info.startPosMouse = info.startPos = info.posMouse = dragStartMousePos;
        mapPixelsToPoints(&info.pos, 1);
        mapPixelsToPoints(&info.startPos, 1);
        info.startPos = info.pos;
        if (dragListener)
            isDragging = dragListener->requestDrag(*this, info);
        return false; //don't consume
        }
    return true;
    }

DragDropInfo InfiniteCanvas::buildDragDropInfo(Sint32 x, Sint32 y)
    {
    DragDropInfo info;
    info.startPosMouse = info.startPos = dragStartMousePos;
    mapPixelsToPoints(&info.startPos, 1);
    info.posMouse.set((SkScalar)x, (SkScalar)y);
    info.pos = info.posMouse;
    mapPixelsToPoints(&info.pos, 1);
    info.delta.fX = info.pos.fX - info.startPos.fX;
    info.delta.fY = info.pos.fY - info.startPos.fY;
    return info;
    }

void InfiniteCanvas::_mouseMove(SDL_MouseMotionEvent& event)
    {
    if (!isDragging)
        return;
    dragMoved = true;//actually moving!
    if (draggingSelf)
        {
        xTranslate = dragStartPos.fX + event.x - dragStartMousePos.fX;
        yTranslate = dragStartPos.fY + event.y - dragStartMousePos.fY;
        }
    else
        {
        if (dragListener)
            dragListener->dragging(*this, buildDragDropInfo(event.x, event.y));
        }
    getWindow()->setInvalid();
    }

bool InfiniteCanvas::_mouseUp(SDL_MouseButtonEvent& event)
    {
    if (draggingSelf)
        {
        if (dragListener)
            dragListener->canvasTransformed(*this);
        draggingSelf = false;
        isDragging = false;
        dragMoved = false;
        return true;
        }
    if (!isDragging)
        {
        if (dragListener)
            dragListener->mouseUpNoDrag(*this, event);
        return false;
        }
    isDragging = false;
    if (!dragMoved)
        {
        if (dragListener)
            dragListener->mouseUpNoDrag(*this, event);
        return true;
        }
    if (dragListener)
        dragListener->dropped(*this, buildDragDropInfo(event.x, event.y));
    getWindow()->setInvalid();
    return true;
    }

void InfiniteCanvas::_mouseWheel(SDL_MouseWheelEvent_EX& event)
    {
    auto state = SDL_GetMouseState(nullptr, nullptr);
    if (SDL_BUTTON(SDL_BUTTON_MIDDLE) & state)
        return; //accidentally scrolled while pressing the middle (wheel) button

    SkPoint mouseLoc;
    SkPoint canvasLoc;
    mouseLoc.set((SkScalar)event.x, (SkScalar)event.y);
    canvasLoc = mouseLoc;
    mapPixelsToPoints(&canvasLoc, 1);

    if (event.mouseWheelEvent.y > 0)
        scaleFactor *= 1 + scaleSpeed;
    else
        scaleFactor *= 1 - scaleSpeed;

    SkPoint newMouseLoc;
    totalTransform = baseTransform;
    totalTransform.preTranslate(xTranslate, yTranslate)
                  .preScale(scaleFactor, scaleFactor);
    mapPointsToPixels(&newMouseLoc, &canvasLoc, 1);
    xTranslate += mouseLoc.fX - newMouseLoc.fX;
    yTranslate += mouseLoc.fY - newMouseLoc.fY;
    //set totalTransform again to the new translation:
    totalTransform = baseTransform;
    totalTransform.preTranslate(xTranslate, yTranslate)
                  .preScale(scaleFactor, scaleFactor);
    if (dragListener)
        dragListener->canvasTransformed(*this);
    getWindow()->setInvalid();
    }

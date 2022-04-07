#include "SplittableArea.h"

SplittableArea::SizePrefs SplittableArea::getSizePrefs()
    {
    if(!view1 && !view2)
        return defaultSizePrefs;
    auto getPrefMinMax = [this](int idxDir) ->MinMax {
        MinMax prefs1 = view1->getSizePrefs()[idxDir];
        MinMax prefs2 = view2->getSizePrefs()[idxDir];
        MinMax buildMinMax;
        buildMinMax.min = prefs1.min + prefs2.min;
        buildMinMax.max = prefs1.max + prefs2.max;
        buildMinMax.prefSize = std::min(prefs1.prefSize, prefs2.prefSize);

        return buildMinMax;
        };
    SizePrefs buildPrefs;
    buildPrefs.widthPrefs = getPrefMinMax(SizePrefs::WIDHT_IDX);
    buildPrefs.heightPrefs = getPrefMinMax(SizePrefs::HEIGHT_IDX);
    return buildPrefs;
    }

void SplittableArea::redraw()
    {
    if (view1)
        {
        view1->transformAndDraw();
        view2->transformAndDraw();
        }
    else if (area)
        area->transformAndDraw();
    }

void SplittableArea::trickleResizeEvent()
    {
    _resizeContent();
    if (view1)
        view1->trickleResizeEvent();
    if (view2)
        view2->trickleResizeEvent();
    else if (area)
        area->trickleResizeEvent();
    }

bool SplittableArea::trickleMouseMoveEvent(SDL_MouseMotionEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    if (!area && hitTest(rectGap, event.x, event.y))
        {
        window->setCursor(splitDirection == Direction::LEFT_RIGHT ? SDL_SYSTEM_CURSOR_SIZEWE : SDL_SYSTEM_CURSOR_SIZENS);
        }
    else
        window->setCursor();

    _mouseMove(event);
    bool res = false;
    if (view1)
        res |= view1->trickleMouseMoveEvent(event);
    if (view2)
        res |= view2->trickleMouseMoveEvent(event);
    else if (area)
        res |= area->trickleMouseMoveEvent(event);
    return res;
    }
bool SplittableArea::trickleMouseUpEvent(SDL_MouseButtonEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    _mouseUp(event);
    bool res = false;
    if (view1)
        res |= view1->trickleMouseUpEvent(event);
    if (view2)
        res |= view2->trickleMouseUpEvent(event);
    else if (area)
        res |= area->trickleMouseUpEvent(event);
    return res;
    }
bool SplittableArea::trickleMouseDownEvent(SDL_MouseButtonEvent& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    _mouseDown(event);
    bool res = false;
    if (view1)
        res |= view1->trickleMouseDownEvent(event);
    if (view2)
        res |= view2->trickleMouseDownEvent(event);
    else if (area)
        res |= area->trickleMouseDownEvent(event);
    return res;
    }
bool SplittableArea::trickleMouseWheelEvent(SDL_MouseWheelEvent_EX& event)
    {
    if (!hitTest(event.x, event.y))
        return false;
    bool res = false;
    if (view1)
        res |= view1->trickleMouseWheelEvent(event);
    if (view2)
        res |= view2->trickleMouseWheelEvent(event);
    else if (area)
        res |= area->trickleMouseWheelEvent(event);
    return res;
    }
void SplittableArea::trickleIdle()
    {
    if (view1)
        view1->trickleIdle();
    if (view2)
        view2->trickleIdle();
    else if (area)
        area->trickleIdle();
    }
void SplittableArea::trickleKeyDown(SDL_KeyboardEvent& event)
    {
    if (view1)
        view1->trickleKeyDown(event);
    if (view2)
        view2->trickleKeyDown(event);
    else if (area)
        area->trickleKeyDown(event);

    }
void SplittableArea::trickleTextEvent(SDL_TextInputEvent& event)
    {
    if (view1)
        view1->trickleTextEvent(event);
    if (view2)
        view2->trickleTextEvent(event);
    else if (area)
        area->trickleTextEvent(event);
     }

 void SplittableArea::setContent(UIArea* area) 
    { 
    if (myCreatedView)
        delete myCreatedView;
    view1 = view2 = myCreatedView = nullptr;
    this->area = area;
    area->parent = this;
    _resizeContent();
    }

void SplittableArea::splitView(SplittableArea* secondView, Location loc)
    {
    SplittableArea* clone = new SplittableArea(*this);
    clone->parent = this;
    clone->view = this; //TODO: is both parent and view needed?
    if (clone->area)
        {
        clone->area->parent = clone;
        this->area = nullptr;;
        }
    myCreatedView = clone;
    if (loc == Location::BELOW || loc == Location::RIGHT)
        {
        view1 = clone;
        view2 = secondView;
        view2->parent = view2->view = this;
        }
    else
        {
        view2 = clone;
        view1 = secondView;
        view1->parent = view1->view = this;
        }
    if (loc == Location::ABOVE || loc == Location::BELOW)
        splitDirection = Direction::ABOVE_BELOW;
    else
        splitDirection = Direction::LEFT_RIGHT;

    _resizeContent();
    }

void SplittableArea::_resizeContent()
    {
    if (area)
        {
        area->rect = rect.makeOffset(-rect.fLeft, -rect.fTop);
        area->trickleResizeEvent();
        if (getWindow())
            window->setInvalid(); //TODO: needed? resize always invalidates window?
        return;
        }
    if (view1 && view2)
        {
        if (splitPoint == -1)
            {
            if (splitDirection == Direction::LEFT_RIGHT)
                splitPoint = rect.width() / 2;
            else
                splitPoint = rect.height() / 2;
            }

        //recalculate splitPoint
        SizePrefs prefs1 = view1->getSizePrefs();
        SizePrefs prefs2 = view2->getSizePrefs();
        SkScalar mySize = splitDirection == Direction::LEFT_RIGHT ? rect.width() : rect.height();
        auto idxDir = (splitDirection == Direction::LEFT_RIGHT ? SizePrefs::WIDHT_IDX : SizePrefs::HEIGHT_IDX);
        if ((prefs1[idxDir].prefSize == PrefSize::PROPORTIONAL && prefs2[idxDir].prefSize == PrefSize::PROPORTIONAL)
            || (prefs1[idxDir].prefSize == PrefSize::DONTCARE && prefs2[idxDir].prefSize == PrefSize::DONTCARE))
            {
            if (!oldSize.isEmpty())
                {
                if(splitDirection == Direction::LEFT_RIGHT)
                    splitPoint = splitPoint * mySize / oldSize.width();
                else
                    splitPoint = splitPoint * mySize / oldSize.height();
                }
            }
        else if (prefs1[idxDir].prefSize == PrefSize::FIXED)
            splitPoint = prefs1[idxDir].min;
        else if (prefs2[idxDir].prefSize == PrefSize::FIXED)
            splitPoint = (splitDirection == Direction::LEFT_RIGHT ? rect.width() : rect.height()) - prefs2[idxDir].min;
        else if (prefs1[idxDir].prefSize == PrefSize::PROPORTIONAL)
            splitPoint = prefs1[idxDir].min;
        else if (prefs1[idxDir].prefSize == PrefSize::PROPORTIONAL)
            splitPoint = (splitDirection == Direction::LEFT_RIGHT ? rect.width() : rect.height()) - prefs2[idxDir].min;
        splitPoint = std::clamp(splitPoint, prefs1[idxDir].min, prefs1[idxDir].max);
        splitPoint = std::clamp(splitPoint, mySize - prefs2[idxDir].max, mySize - prefs2[idxDir].min);
        resizeViews();
        }
    oldSize = rect;

    if(getWindow())
        window->setInvalid(); //TODO: needed? resize always invalidates window?
    }

void SplittableArea::resizeViews()
    {
    SkScalar halfGap = mindTheGap / 2;
    if (splitDirection == Direction::LEFT_RIGHT)
        {
        view1->rect = SkRect::MakeXYWH(0, 0, splitPoint - halfGap, rect.height());
        view2->rect = SkRect::MakeXYWH(splitPoint + halfGap, 0, rect.width() - splitPoint - halfGap, rect.height());
        }
    else
        {
        view1->rect = SkRect::MakeXYWH(0, 0, rect.width(), splitPoint - halfGap);
        view2->rect = SkRect::MakeXYWH(0, splitPoint + halfGap, rect.width(), rect.height() - splitPoint - halfGap);
        }
    view1->_resizeContent();
    view2->_resizeContent();
    if (splitDirection == Direction::LEFT_RIGHT)
        rectGap = SkRect::MakeXYWH(splitPoint - halfGap, 0, mindTheGap, rect.height());
    else
        rectGap = SkRect::MakeXYWH(0, splitPoint - halfGap, rect.width(), mindTheGap);
    }


bool SplittableArea::_mouseDown(SDL_MouseButtonEvent& event)
    {
    if (!mindTheGap)
        return false;
    if (!area && hitTest(rectGap, event.x, event.y))
        {
            startSplitPos = splitPoint;
            dragging = true;
        if (splitDirection == Direction::LEFT_RIGHT)
            startDragMousePos = event.x;
        else
            startDragMousePos = event.y;
        return true;
        }
    return false;
    }
void SplittableArea::_mouseMove(SDL_MouseMotionEvent& event)
    {
    if (!dragging)
        return;
    if (splitDirection == Direction::LEFT_RIGHT)
        splitPoint = startSplitPos + event.x - startDragMousePos;
    else
        splitPoint = startSplitPos + event.y - startDragMousePos;

    resizeViews();
    window->setInvalid();
    }
bool SplittableArea::_mouseUp(SDL_MouseButtonEvent& event)
    {
    dragging = false;
    return false;
    }

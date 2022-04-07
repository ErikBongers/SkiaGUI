#include "Button.h"

Button::Button()
    {
    mouseMove = [](UIArea& e, SDL_MouseMotionEvent& event) {
        Button& button = (Button&)e;
        button.highlight(button.hitTest((SkScalar)event.x, (SkScalar)event.y));
        };
    }

void Button::highlight(bool high)
    {
    if (high)
        {
        if (!mouseOver)
            {
            mouseOver = true;
            getWindow()->addMouseCapture(*this);
            originalBkcColor = backgroundColor;
            if (highlightColor != SK_ColorTRANSPARENT)
                backgroundColor = highlightColor;
            getWindow()->setInvalid();
            }
        }
    else
        {
        if (mouseOver)
            {
            mouseOver = false;
            backgroundColor = originalBkcColor;
            getWindow()->setInvalid();
            }
        getWindow()->removeMouseCapture(*this);
        }
    }

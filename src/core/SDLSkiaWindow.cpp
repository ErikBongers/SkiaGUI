#include "SDLSkiaWindow.h"
#include "Functions.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <thread>
#include <chrono>
#endif
#include "UIElement.h"
#include "SplittableArea.h"
#include "include/core/SkFontMetrics.h"

//int SDLCALL SDLSkiaWindow::onEventsReceived(void* userdata, SDL_Event* event)
//    {
//    ((SDLSkiaWindow*)userdata)->loopOnce();
//    return 0;//ignored
//    }

SDLSkiaWindow::SDLSkiaWindow(WindowClient& client) 
    : client(client) 
    { 
    client.setWindow(this); 
    rootView = client.getMainView(); 
    rootView->window = this; 
    }

void WindowClient::resize()
    {
    auto view = window->getRootView();
    view->rect = SkRect::MakeXYWH(0, 0, (SkScalar)getWindow()->getWidth(), (SkScalar)getWindow()->getHeight());
    view->trickleResizeEvent();
    }

void WindowClient::draw() 
    { 
    window->getRootView()->transformAndDraw(); 
    }

void WindowClient::mouseMoved(SDL_MouseMotionEvent& event) 
    { 
    getMainView()->trickleMouseMoveEvent(event); 
    }

void WindowClient::mouseDown(SDL_MouseButtonEvent& event) 
    { 
    getMainView()->trickleMouseDownEvent(event); 
    }

void WindowClient::mouseUp(SDL_MouseButtonEvent& event) 
    { 
    getMainView()->trickleMouseUpEvent(event); 
    }   

void WindowClient::mouseWheel(SDL_MouseWheelEvent_EX& event) 
    { 
    getMainView()->trickleMouseWheelEvent(event); 
    }   

void WindowClient::keyDown(SDL_KeyboardEvent& event) 
    { 
    getMainView()->trickleKeyDown(event); 
    }   

void WindowClient::textInput(SDL_TextInputEvent& event) 
    { 
    getMainView()->trickleTextEvent(event); 
    }   

SDL_Window*  SDLSkiaWindow::createWindow(const char* title, int width, int height, int stencilBits, int msaaSampleCount, SkColor bkgColor)
    {
    this->bkgColor = bkgColor;
    this->stencilBits = stencilBits;
    this->msaaSampleCount = msaaSampleCount;
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window) {
        handle_error();
        return nullptr;
        }

    // To go fullscreen
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // try and setup a GL context
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        handle_error();
        return nullptr;
        }

    int success = SDL_GL_MakeCurrent(window, glContext);
    if (success != 0) {
        handle_error();
        return nullptr;
        }

    windowFormat = SDL_GetWindowPixelFormat(window);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &contextType);


    resizeViewportToWindow(window);
    glClearColor(SkColorGetR(bkgColor) / 255.0f, SkColorGetG(bkgColor) / 255.0f, SkColorGetB(bkgColor) / 255.0f, SkColorGetA(bkgColor) / 255.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //SKIA SETUP starts here...

    // setup GrContext
    interfac = GrGLMakeNativeInterface();

    // setup contexts
    grContext = GrContext::MakeGL(interfac);
    SkASSERT(grContext);

    canvas = createSurfaceAndCanvas(interfac, windowFormat, contextType, grContext);
    getRootView()->window = this;
    client.resize(); //force a size for rootView.
    client.initialize();
    auto prefs = getRootView()->getSizePrefs();
    SDL_SetWindowMinimumSize(window, std::max((int)prefs.widthPrefs.min, 1), std::max((int)prefs.heightPrefs.min, 1));
    client.resize();
    return window;
    }

void SDLSkiaWindow::destroyWindow()
    {
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
        }

    SDL_DestroyWindow(window);
    }

void SDLSkiaWindow::addMouseCapture(UIArea& e)
    {
    mouseCaptures.insert(&e);
    }

void SDLSkiaWindow::removeMouseCapture(UIArea& e)
    {
    mouseCaptures.extract(&e);
    }

void SDLSkiaWindow::setCursor(SDL_SystemCursor id)
    {
    if (currentCursorId == id)
        return;
    if(currentCursor)
        SDL_FreeCursor(currentCursor);
    currentCursor = SDL_CreateSystemCursor(id);
    SDL_SetCursor(currentCursor);
    currentCursorId = id;
    }


SkCanvas* SDLSkiaWindow::createSurfaceAndCanvas(sk_sp<const GrGLInterface> interfac, uint32_t windowFormat, int contextType, sk_sp<GrContext> grContext)
    {
    // Wrap the frame buffer object attached to the screen in a Skia render target so Skia can render to it
    GR_GL_GetIntegerv(interfac.get(), GR_GL_FRAMEBUFFER_BINDING, &buffer);
    info.fFBOID = (GrGLuint)buffer;

    //SkDebugf("%s", SDL_GetPixelFormatName(windowFormat));
    // TODO: the windowFormat is never any of these?
    if (SDL_PIXELFORMAT_RGBA8888 == windowFormat) {
        info.fFormat = GR_GL_RGBA8;
        colorType = kRGBA_8888_SkColorType;
        }
    else {
        colorType = kBGRA_8888_SkColorType;
        if (SDL_GL_CONTEXT_PROFILE_ES == contextType) {
            info.fFormat = GR_GL_BGRA8;
            }
        else {
            // We assume the internal format is RGBA8 on desktop GL
            info.fFormat = GR_GL_RGBA8;
            }
        }

    // setup SkSurface
    // To use distance field text, use commented out SkSurfaceProps instead
    // SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag, SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    GrBackendRenderTarget target(dw, dh, msaaSampleCount, stencilBits, info);
    surface = SkSurface::MakeFromBackendRenderTarget(grContext.get(), target, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, &props);

    return surface->getCanvas();
    }

void SDLSkiaWindow::resizeViewportToWindow(SDL_Window* window)
    {
    SDL_GL_GetDrawableSize(window, &dw, &dh);
    glViewport(0, 0, dw, dh);
    }

void SDLSkiaWindow::handleEvent(SDL_Event event)
    {
    switch (event.type) {
        case SDL_MOUSEMOTION:
            {
            std::set<UIArea*> cloneMouseCaptures(mouseCaptures);
            for (auto el = cloneMouseCaptures.begin(); el != cloneMouseCaptures.end(); ++el)
                {
                (*el)->_mouseMove(event.motion);
                if ((*el)->mouseMove)
                    (*el)->mouseMove(**el, event.motion);
                }
            if (toolTipRequester)
                {
                if (!toolTipRequester->hitTest(event.motion.x, event.motion.y))
                    {
                    toolTipRequester = nullptr;
                    if(showToolTip)
                        toolTipLeaveTime = std::chrono::steady_clock::now();
                    showToolTip = false;
                    }
                }
            client.mouseMoved(event.motion);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            client.mouseDown(event.button);
            break;
        case SDL_MOUSEBUTTONUP:
            for (auto el : mouseCaptures)
                {
                if (el->_mouseUp(event.button))
                    return; //handled, so, consume.
                if (el->mouseUp)
                    el->mouseUp(*el, event.button);
                }
            client.mouseUp(event.button);
            break;
        case SDL_MOUSEWHEEL:
            {
            SDL_MouseWheelEvent_EX event2;
            event2.mouseWheelEvent = event.wheel;
            SDL_GetMouseState(&event2.x, &event2.y);
            client.mouseWheel(event2);
            }
            break;
        case SDL_KEYDOWN:
            client.keyDown(event.key);
            break;
        case SDL_TEXTINPUT:
            client.textInput(event.text);
        case SDL_WINDOWEVENT:
            switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED: //called after SIZE_CHANGED user or system changes window size.
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED: //always called
                    resizeViewportToWindow(window);
                    client.resize();
                    canvas = nullptr;
                    break;
                }
            break;
        default:
            break;
        }
    }

void SDLSkiaWindow::draw()
    {
    loopCounter++;
    if (canvas == nullptr)
        canvas = createSurfaceAndCanvas(interfac, windowFormat, contextType, grContext);

    if (invalid)
        {
        fps.beginFrame();
        canvas->restoreToCount(0);
        canvas->resetMatrix();
        canvas->clear(bkgColor);

        client.draw();

        std::string strFps = std::to_string(fps.getFps());
        std::string strMs = std::to_string((int)fps.getMsPerFrame()) + "ms";
        std::string strLoopCnt = std::to_string(loopCounter) + "loop iterations";
        paint.setColor(SK_ColorWHITE); // move to init function?
        canvas->drawString(strFps.c_str(), 100.0f, 160.0f, font, paint);
        canvas->drawString(strMs.c_str(), 100.0f, 180.0f, font, paint);
        canvas->drawString(strLoopCnt.c_str(), 100.0f, 200.0f, font, paint);

        if (toolTipRequester && showToolTip)
            {
            SkRect bounds;
            SkFont font;
            SkFontMetrics metrix;
            font.getMetrics(&metrix);
            font.measureText(toolTipRequester->toolTipText.c_str(), toolTipRequester->toolTipText.size(), SkTextEncoding::kUTF8, &bounds);
            bounds.setWH(bounds.width() + 20, -metrix.fAscent + metrix.fDescent + 10);
            SkRect absoluteRect = toolTipRequester->absoluteRect();
            bounds.offset(absoluteRect.fLeft, absoluteRect.fBottom + 5);
            paint.setColor(SkColorSetRGB(30, 30, 30));
            canvas->drawRoundRect(bounds, 5, 5, paint);
            paint.setColor(SkColorSetRGB(255,255,200));
            canvas->drawString(toolTipRequester->toolTipText.c_str(), bounds.fLeft + 10, bounds.fTop + 5 - metrix.fAscent, font, paint);
            }

        fps.endFrame();

        canvas->flush();
        SDL_GL_SwapWindow(window);
        invalid = false;
        }
    }

using namespace std::chrono_literals;

void SDLSkiaWindow::onIdle()
    { 
    if (toolTipRequester)
        {
        if(!showToolTip)
            {
            auto ellapsedTime = std::chrono::steady_clock::now() - toolTipRequestTime;
            if (ellapsedTime > 1s)
                {
                showToolTip = true;
                setInvalid();
                }
            }
        }
    client.getMainView()->trickleIdle(); 
    }

bool SDLSkiaWindow::onCloseWindowRequest() { return client.closeWindowRequest(); }

void SDLSkiaWindow::setToolTip(UIArea* area)
    {
    if (area == toolTipRequester)
        return; //already set.
    toolTipRequester = area;
    toolTipRequestTime = std::chrono::steady_clock::now();
    if (toolTipRequestTime - toolTipLeaveTime < 1s)
        showToolTip = true;
    }


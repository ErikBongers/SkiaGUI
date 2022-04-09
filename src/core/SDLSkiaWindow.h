#pragma once
#include "pch.h"
#include "Fps.h"
#include <set>

class SDLSkiaWindow;
class UIArea;
class UIElement;
class SplittableArea;

struct SDL_MouseWheelEvent_EX
    {
    int x;
    int y;
    SDL_MouseWheelEvent mouseWheelEvent;
    };

class WindowClient
    {
    private:
        void setWindow(SDLSkiaWindow* window) { this->window = window; }
    protected:
        SDLSkiaWindow* window;
        UIArea* focusElement = nullptr;
        void draw();
    public:
        SDLSkiaWindow* getWindow() { return window; }
        virtual void onIdle() {}
        virtual void initialize() {}
        void mouseMoved(SDL_MouseMotionEvent& event);
        void mouseDown(SDL_MouseButtonEvent& event);
        void mouseUp(SDL_MouseButtonEvent& event);
        void mouseWheel(SDL_MouseWheelEvent_EX& event);
        void keyDown(SDL_KeyboardEvent& event);
        void textInput(SDL_TextInputEvent& event);
        void resize();
        virtual SplittableArea* getMainView() = 0;
        void takeFocus(UIArea* el) { this->focusElement = el; }
        bool hasFocus(UIArea* el) { return this->focusElement == el; }
        virtual bool closeWindowRequest() { return true; }
        friend class SDLSkiaWindow;
    };

class SDLSkiaWindow
    {
    private:
        WindowClient& client;
        SDL_Window* window = nullptr;
        int dw = 0, dh = 0;
        GrGLint buffer = 0;
        GrGLFramebufferInfo info;
        SkColorType colorType = SkColorType::kUnknown_SkColorType;
        sk_sp<SkSurface> surface;
        SkCanvas* canvas = nullptr;
        int rotation = 0;
        SkFont font;
        graffel::Fps fps;
        SkPaint paint;
        SDL_GLContext glContext = nullptr;
        uint32_t windowFormat = 0;
        sk_sp<const GrGLInterface> interfac;
        int contextType = 0;
        int stencilBits = 0;
        int msaaSampleCount = 0;
        sk_sp<GrContext> grContext;
        bool invalid = true;
        int loopCounter = 0;
        SDL_SystemCursor currentCursorId = SDL_SYSTEM_CURSOR_ARROW;
        SDL_Cursor* currentCursor = nullptr;
        SkColor bkgColor;

        SkCanvas* createSurfaceAndCanvas(sk_sp<const GrGLInterface> interfac, uint32_t windowFormat, int contextType, sk_sp<GrContext> grContext);
        void resizeViewportToWindow(SDL_Window* window);
        //static int onEventsReceived(void* userdata, SDL_Event* event);
        std::set<UIArea*> mouseCaptures;
        SplittableArea* rootView = nullptr;
        bool fpsDraw = true;
        SkPoint fpsAt = {0, 0};
        SkColor fpsColor = SK_ColorBLACK;
    public:
        SDLSkiaWindow(WindowClient& client);
        SDL_Window* createWindow(const char* title, int width, int height, int stencilBits, int msaaSampleCount, SkColor bkgColor);
        void destroyWindow();
        int getWidth() { return dw; }
        int getHeight() { return dh; }
        SkCanvas& Canvas() { return *canvas; }
        void setInvalid() { invalid = true; }
        void addMouseCapture(UIArea& e);
        void removeMouseCapture(UIArea& e);
        bool isMouseCaptured(UIArea& e) { return mouseCaptures.find(&e) != mouseCaptures.end(); }
        WindowClient& getClient() { return client; }
        SplittableArea* getRootView() { return rootView; }
        void setCursor(SDL_SystemCursor id = SDL_SYSTEM_CURSOR_ARROW);
        void handleEvent(SDL_Event event);
        void draw();
        void onIdle();
        bool onCloseWindowRequest();
        void setToolTip(UIArea* area);
        UIArea* toolTipRequester = nullptr;
        std::chrono::time_point<std::chrono::steady_clock> toolTipRequestTime;
        std::chrono::time_point<std::chrono::steady_clock> toolTipLeaveTime;
        bool showToolTip = false;
        void drawFps(bool draw) { fpsDraw = draw; }
        void drawFps(SkPoint at, SkColor color) { fpsDraw = true; fpsAt = at; fpsColor = color; }
    };


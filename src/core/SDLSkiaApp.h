#pragma once
#include "../pch.h"
#include "core/SDLSkiaWindow.h"

class SDLSkiaApp
    {
    public:
        SDLSkiaApp();
        void initSDL_GL();
        SDL_DisplayMode dm;
        static const int kStencilBits = 8;  // Skia needs 8 stencil bits
        int kMsaaSampleCount = 0; //or 4 or so for multisample;
        std::unordered_map<int, SDLSkiaWindow*> skiaWindows;
        bool createWindow(const char* title, int w, int h, WindowClient& client, SkColor bkgColor);
        void startEventLoop();
        bool handleEvents();
        void loopOnce();
        bool quit = false;
        bool closeRequested = false;
        bool closeRequestResult = false;
        ~SDLSkiaApp() { SDL_Quit(); } //Quit SDL subsystems
    };

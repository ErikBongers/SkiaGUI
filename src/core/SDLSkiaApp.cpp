#include "SDLSkiaApp.h"
#include "core/Functions.h"

SDLSkiaApp::SDLSkiaApp()
    {
    initSDL_GL();
    //In a real application you might want to initialize more subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        handle_error();
        return;
        }

    // Setup window
    // This code will create a window with the same resolution as the user's desktop.
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        handle_error();
        return;
        }
    SDL_StartTextInput();
    }

void SDLSkiaApp::initSDL_GL()
    {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // For all other clients we use the core profile and operate in a window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, kStencilBits);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // If you want multisampling, uncomment the below lines and set a sample count
    // kMsaaSampleCount = 4;
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, kMsaaSampleCount);
    }

bool SDLSkiaApp::createWindow(const char* title, int w, int h, WindowClient& client, SkColor bkgColor)
    {
    SDLSkiaWindow* wnd = new SDLSkiaWindow(client);
    auto sdlWnd = wnd->createWindow(title, w, h, kStencilBits, kMsaaSampleCount, bkgColor);
    skiaWindows.emplace(SDL_GetWindowID(sdlWnd), wnd);
    return true;
    }

void SDLSkiaApp::startEventLoop()
    {
    //SDL_AddEventWatch(onEventsReceived, (void*)this);

    while (!quit) { // Our application loop
        loopOnce();
#ifdef _WIN32
        Sleep(10);
#else
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
#endif
        }
    }

void SDLSkiaApp::loopOnce()
    {
    if (!handleEvents())
        {
        for (auto wnd : skiaWindows)
            wnd.second->onIdle();
        }
    for(auto wnd : skiaWindows)
        wnd.second->draw();
    }

bool SDLSkiaApp::handleEvents()
    {
    bool eventsHappened = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        eventsHappened = true;
        switch (event.type) {
            case SDL_MOUSEMOTION:
                skiaWindows.at(event.motion.windowID)->handleEvent(event);
                break;
            case SDL_MOUSEBUTTONDOWN:
                skiaWindows.at(event.button.windowID)->handleEvent(event);
                break;
            case SDL_MOUSEBUTTONUP:
                skiaWindows.at(event.button.windowID)->handleEvent(event);
                break;
            case SDL_MOUSEWHEEL:
                skiaWindows.at(event.wheel.windowID)->handleEvent(event);
                break;
            case SDL_KEYDOWN:
                skiaWindows.at(event.key.windowID)->handleEvent(event);
                break;
            case SDL_TEXTINPUT:
                skiaWindows.at(event.text.windowID)->handleEvent(event);
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                    {
                    case SDL_WINDOWEVENT_RESIZED: //called after SIZE_CHANGED user or system changes window size.
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED: //always called
                        skiaWindows.at(event.window.windowID)->handleEvent(event);
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        skiaWindows.at(event.window.windowID)->handleEvent(event);
                        closeRequested = true;
                        closeRequestResult = skiaWindows.at(event.window.windowID)->onCloseWindowRequest();
                        break;
                    }
                break;
            case SDL_QUIT:
                if (closeRequested)
                    quit = closeRequestResult;
                else
                    quit = true;
                break;
            default:
                break;
            }
        }
    return eventsHappened;
    }


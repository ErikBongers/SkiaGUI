# SkiaGUI
A basic GUI lib for Skia based projects.
The only base layer for this GUI is currently SDL.

# Basic structure
A SKiaGui program creates an SDLSkiaApp instance. This app object handles the event loop.
Create a SDLSkiaWindow and hand it a WindowClient, which handles the window's events.
A typical `main.cpp` looks like this

```
int main(int argc, char** argv) {
    SDLSkiaApp app;
    auto wndClient = GraffelWindowClient();

    if (app.createWindow("My App", (int)(app.dm.w * 0.7), (int)(app.dm.h * 0.7), wndClient, 0))
        app.startEventLoop();

    return 0;
    }
```

# Fill the window
A WindowClient is used to interact with the actual window.
A basic WindowClient would simply add visual components to the window.
```
class MyWindowClient : public WindowClient
    {
    private:
        SplittableArea mainView;
        UIElement toolbar;
        UIElement *button1;
        ...
    public:
        void initialize() override; //create the mainView and other visual components and setup the hierarchy.
        SplittableArea* getMainView() override { return &mainView; } //only required override. The Window must know the top of the visual's hierarchy.
        on...someEvent(); //note that these are NOT virtual functions
        on...otherEvent)();
        ...
    };
```

# Layout and visual components
TODO

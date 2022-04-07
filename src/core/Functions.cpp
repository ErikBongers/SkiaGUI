#include "Functions.h"

void handle_error() {
    const char* error = SDL_GetError();
    SkDebugf("SDL Error: %s\n", error);
    SDL_ClearError();
    }

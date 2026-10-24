#include <SDL3/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

enum {
    TOUCH_LEFT,
    TOUCH_DOWN,
    TOUCH_RIGHT,
    TOUCH_ROTATE,
    TOUCH_HARD_DROP,
    TOUCH_PAUSE
};

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void PushTouchKey(int action, int pressed)
{
    SDL_Keycode key;

    switch (action) {
        case TOUCH_LEFT:
            key = SDLK_LEFT;
            break;
        case TOUCH_DOWN:
            key = SDLK_DOWN;
            break;
        case TOUCH_RIGHT:
            key = SDLK_RIGHT;
            break;
        case TOUCH_ROTATE:
            key = SDLK_UP;
            break;
        case TOUCH_HARD_DROP:
            key = SDLK_SPACE;
            break;
        case TOUCH_PAUSE:
            key = SDLK_ESCAPE;
            break;
        default:
            return;
    }

    SDL_Event event = {0};

    event.type = pressed
        ? SDL_EVENT_KEY_DOWN
        : SDL_EVENT_KEY_UP;

    event.key.key = key;
    event.key.repeat = false;

    SDL_PushEvent(&event);
}
#endif

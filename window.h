#ifndef WINDOW_H
#define WINDOW_H

// For SDL
#include <SDL.h>
#include <SDL_ttf.h>

#include "global.h"
#include "video.h"

typedef void (*FilterFunc)(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);
typedef struct {
    FilterFunc f;
    uint8_t src;
} Filter;

class MainWin
{
    public:
        MainWin(uint32_t width, uint32_t height, uint32_t depth, uint8_t windows = 1);
        ~MainWin(void);
        void MainLoop(void);
        void AddFilter(FilterFunc f, uint8_t idx, uint8_t src = 0);
        void DrawText(const char *text, SDL_Rect loc, SDL_Color fg, SDL_Color bg);
        void ScreenShot(SDL_Surface *s);
    private:
        uint32_t width, height, depth;
        SDL_Surface *screen;
        VideoDevice *v;
        byte **framebuf;
        uint8_t windows, winside;
        Filter *funcs;
        TTF_Font *font;
};

#endif

#ifndef WINDOW_H
#define WINDOW_H

#include <vector>

// For SDL
#include <SDL.h>
#include <SDL_ttf.h>

#include "global.h"
#include "video/videodevice.h"
using std::vector;
using std::string;

typedef void (*FilterFunc)(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);
struct Filter {
    Filter(FilterFunc f, SDL_Surface* frame, string name, uint32_t src): f(f), frame(frame), name(name), src(src) {};
    FilterFunc f;
    SDL_Surface *frame;
    string name;
    uint32_t src;
};

class MainWin
{
    public:
        MainWin(VideoDevice &v, uint32_t windows);
        ~MainWin(void);
        void MainLoop(void);
        void AddFilter(const char *name, FilterFunc f, uint32_t idx, uint32_t src = 0);
        void DrawText(const char *text, SDL_Rect loc, SDL_Color fg, SDL_Color bg);
        void ScreenShot(SDL_Surface *s);
    private:
        SDL_Surface *screen;
        VideoDevice &v;
        uint32_t windows, winside;
        vector<Filter> funcs;
        TTF_Font *font;
};

#endif

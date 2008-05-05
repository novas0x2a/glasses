#include <iostream>
#include <string>
#include <cmath>
#include <cerrno>
#include <vector>

// For open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

// For SDL
#include <SDL.h>
#include <SDL_ttf.h>

#include "global.h"
#include "window.h"
#include "utils/average.h"

using namespace std;
using namespace novas0x2a;

inline SDL_Surface* makeFrame(VideoDevice &v)
{
    Context c("When making framebuffer");
    void *px = new byte[v.getWidth() * v.getHeight() * (v.getDepth()>>3)];
    return SDL_CreateRGBSurfaceFrom(px, v.getWidth(), v.getHeight(), v.getDepth(), v.getWidth()*(v.getDepth()>>3), 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
}

Window::Window(VideoDevice &_v, uint32_t _windows) : v(_v), windows(_windows+1)
{
    Context c("When constructing Main Window");
    if (v.getDepth() != 32) // TODO: Not pixel-format generic
        throw ArgumentError("Only 3-byte color with 4-byte pixels is supported");

    winside = ceil(sqrt(windows));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw GeneralError(DEBUG_HERE, "Could not init SDL");

    SDL_WM_SetCaption(PROGRAM "-" VERSION, PROGRAM);

    if (!(screen = SDL_SetVideoMode(v.getWidth()*winside, v.getHeight()*winside, v.getDepth(), SDL_HWSURFACE)))
        throw SDLError("Unable to set video mode");

    if (TTF_Init() == -1)
        throw TTFError("Could not init TTF");

    if (!(font = TTF_OpenFont(FONT, 20)))
        throw TTFError("Could not load font");

    for (uint16_t i = 0; i < windows; ++i)
        funcs.push_back(Filter(0, NULL, string(i == 0 ? "source" : "None"), -1));
    funcs[0].frame = makeFrame(v);
}

Window::~Window(void)
{
    Context c("When Destructing Main Window");
    vector<Filter>::iterator i;
    for (i = funcs.begin(); i != funcs.end(); ++i)
    {
        if (i->frame)
        {
            delete [] static_cast<byte*>(i->frame->pixels);
            SDL_FreeSurface(i->frame);
        }
    }
    SDL_FreeSurface(screen);
    SDL_Quit();
}

void Window::DrawText(const char *text, SDL_Rect loc, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface *txt = TTF_RenderText_Shaded(font, text, fg, bg);
    if (SDL_BlitSurface(txt, NULL, screen, &loc) != 0)
        throw SDLError("Text Blit failed: ");
    SDL_FreeSurface(txt);
}

void Window::MainLoop(void)
{
    Context c("When running main loop");
    SDL_Event event;
    struct timeval t1, t2 = {0,0};
    RunningAverage<uint32_t> avg(10);

    while (1)
    {
        gettimeofday(&t1, NULL);
        while(unlikely(SDL_PollEvent(&event)))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case 'q':
                        case SDLK_ESCAPE:
                            return;
                        case 's':
                            this->ScreenShot(screen);
                            break;
                        case 'p':
                            // TODO: HACK. VideoDevice needs a debugString method
                            //cerr << *dynamic_cast<V4LDevice*>(this->v) << endl;
                            break;

                        case 'b': this->v.setBrightness(this->v.getBrightness() - 1); break;
                        case 'B': this->v.setBrightness(this->v.getBrightness() + 1); break;
                        case 'h': this->v.setHue(this->v.getHue() - 1);               break;
                        case 'H': this->v.setHue(this->v.getHue() + 1);               break;
                        case 'c': this->v.setColour(this->v.getColour() - 1);         break;
                        case 'C': this->v.setColour(this->v.getColour() + 1);         break;
                        case 'n': this->v.setContrast(this->v.getContrast() - 1);     break;
                        case 'N': this->v.setContrast(this->v.getContrast() + 1);     break;
                        case 'w': this->v.setWhiteness(this->v.getWhiteness() - 1);   break;
                        case 'W': this->v.setWhiteness(this->v.getWhiteness() + 1);   break;

                        default:
                            break;
                    }
                    break;
                case SDL_QUIT:
                    return;
                default:
                    break;
            }
        }

        v.getFrame(static_cast<byte*>(funcs[0].frame->pixels));

        {
            Context c("Running filters");
            SDL_Rect r_tmp = {0,0,0,0};
            SDL_Rect *r = NULL;
            size_t idx;
            vector<Filter>::const_iterator i;
            for (idx = 0, i = funcs.begin(); i != funcs.end(); ++idx, ++i)
            {
                if (likely(idx != 0))
                {
                    if (i->f)
                        i->f(static_cast<Pixel*>(funcs[i->src].frame->pixels), static_cast<Pixel*>(i->frame->pixels), v.getWidth(), v.getHeight());
                    r_tmp = (SDL_Rect){idx % winside, idx / winside, 0, 0};
                    r = &r_tmp;
                    r_tmp.x *= v.getWidth();
                    r_tmp.y *= v.getHeight();
                }
                if (likely(i->frame != NULL))
                {
                    // r == NULL the first time through, which is what i want for funcs[0], the source image
                    if (unlikely(SDL_BlitSurface(i->frame, NULL, screen, r) != 0))
                        throw SDLError("Blit failed");
                }
                else
                {
                    if (unlikely(SDL_FillRect(screen, r, 0) != 0))
                        throw SDLError("FillRect failed");
                }
            }
        }

        avg.add(1/((double)(t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec)/1000000.0));

        this->DrawText(stringify(avg.get()).c_str(), (SDL_Rect){0,0,0,0}, (SDL_Color){0xff,0xff,0xff,0}, (SDL_Color){0,0,0,0});

        SDL_Flip(screen);

        t2.tv_sec  = t1.tv_sec;
        t2.tv_usec = t1.tv_usec;

        if (unlikely(avg.get() < 1))
            SDL_Delay(33);
        else if (1000/avg.get() < 33)
            SDL_Delay(33 - 1000/avg.get());
    }
}

void Window::ScreenShot(SDL_Surface *s)
{
    Context c("When taking a screenshot");
    for (uint32_t i = 0; i < 50; ++i)
    {
        // use open and then fdopen so I can use O_EXCL and avoid the race condition
        int fd = open(string("shot" + stringify(i) + ".ppm").c_str(), O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        if (fd < 0)
        {
            if(errno == EEXIST)
                continue;
            else
                throw GeneralError(DEBUG_HERE, string("Could not open screenshot file: ") + strerror(errno));
        }
        FILE *f = fdopen(fd, "w");
        if (!f)
            throw GeneralError(DEBUG_HERE, string("Could not get FILE pointer to screenshot file: ") + strerror(errno));

        fprintf(f, "P6\n%u %u\n255\n", s->w, s->h);

        Pixel *p = (Pixel*)s->pixels;

        for (int32_t y = 0; y < s->h; ++y)
            for (int32_t x = 0; x < s->w; ++x)
            {
                uint32_t off = y*s->w + x;
                fprintf(f, "%c%c%c", R(p[off]), G(p[off]), B(p[off]));
            }
        fclose(f);
        return;
    }
    throw GeneralError(DEBUG_HERE, "Too many screenshots exist already.");
}

void Window::AddFilter(const char* name, FilterFunc f, uint32_t idx, uint32_t src)
{
    Context c(string("When adding a filter named \"") + name + "\" at index " + stringify(uint32_t(idx)) + " with source " + stringify(uint32_t(src)));
    if (idx == 0 || idx >= windows)
        throw ArgumentError("Illegal filter index (range is 1:" + stringify(windows-1) + " inclusive)");
    if (src >= windows)
        throw ArgumentError("Illegal source index (max index is " + stringify(windows-1) + ")");
    if (!funcs[src].frame)
        throw ArgumentError("Create the source before you try to use it");

    funcs[idx] = Filter(f,makeFrame(v),string(name),src);
}
/*}}}*/


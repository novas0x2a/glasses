#include <iostream>
#include <string>
#include <cassert>
#include <cmath>
#include <cerrno>

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
#include "video/v4l.h"
#include "window.h"

using namespace std;

MainWin::MainWin(uint32_t w, uint32_t h, uint32_t d, uint8_t win) : width(w), height(h), depth(d)
{
    assert(depth % 8 == 0 && depth >= 24);
    assert(windows > 0);

    v = new V4LDevice("/dev/video0");

    v->setParams(width, height,
            depth == 32 ? VIDEO_PALETTE_RGB32 :
            depth == 24 ? VIDEO_PALETTE_RGB24 : VIDEO_PALETTE_RGB565, depth);

    assert(v->getWidth()  == width);
    assert(v->getHeight() == height);
    assert(v->getDepth()  == depth);

    winside = (uint8_t)ceil(sqrt(win));
    windows = winside * winside;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw string("Could not init SDL");

    SDL_WM_SetCaption(PROGRAM "-" VERSION, PROGRAM);

    if (!(screen = SDL_SetVideoMode(width*winside, height*winside, depth, SDL_HWSURFACE)))
        throw string("Unable to set video mode: ") + SDL_GetError();

    if (TTF_Init() == -1)
        throw string("Could not init TTF: ") + TTF_GetError();

    if (!(font = TTF_OpenFont("/usr/share/fonts/ttf-bitstream-vera/Vera.ttf", 20)))
        throw string("Could not load font: ") + TTF_GetError();

    framebuf    = (byte**)malloc(windows * sizeof(char*));
    framebuf[0] = (byte*)malloc(windows * v->getWidth() * v->getHeight() * (v->getDepth()>>3));
    for (uint16_t i = 1; i < windows; ++i)
        framebuf[i] = framebuf[0] + (i * v->getWidth() * v->getHeight() * (v->getDepth()>>3));

    funcs = new Filter[windows];
    memset(funcs, 0, sizeof(Filter) * windows);
}

MainWin::~MainWin(void)
{
    delete[] funcs;
    free(framebuf[0]);
    free(framebuf);
    SDL_FreeSurface(screen);
    SDL_Quit();
}

void MainWin::DrawText(const char *text, SDL_Rect loc, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface *txt = TTF_RenderText_Shaded(font, text, fg, bg);
    if (SDL_BlitSurface(txt, NULL, screen, &loc) != 0)
        throw string("Text Blit failed: ") + SDL_GetError();
    SDL_FreeSurface(txt);
}

void MainWin::MainLoop(void)
{
    SDL_Event event;
    SDL_Surface *frame[windows];
    struct timeval t1, t2 = {0,0};
    static const uint16_t AVG_SAMP = 10;
    uint32_t fps[AVG_SAMP] = {0}, fps_avg = 0, fps_i = 0;

    for (uint16_t i = 0; i < windows; ++i)
        if (!(frame[i] = SDL_CreateRGBSurfaceFrom(framebuf[i], v->getWidth(), v->getHeight(), v->getDepth(), v->getWidth()*(depth>>3), 0x00ff0000, 0x0000ff00, 0x000000ff, 0)))
            throw string("CreateSurface failed") + SDL_GetError();

    while (1)
    {
        gettimeofday(&t1, NULL);
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case 'q':
                            return;
                        case 's':
                            this->ScreenShot(screen);
                            break;
                        case 'p':
                            cerr << *dynamic_cast<V4LDevice*>(this->v) << endl;
                            break;

                        case 'b': this->v->setBrightness(this->v->getBrightness() - 1); break;
                        case 'B': this->v->setBrightness(this->v->getBrightness() + 1); break;
                        case 'h': this->v->setHue(this->v->getHue() - 1); break;
                        case 'H': this->v->setHue(this->v->getHue() + 1); break;
                        case 'c': this->v->setColour(this->v->getColour() - 1); break;
                        case 'C': this->v->setColour(this->v->getColour() + 1); break;
                        case 'n': this->v->setContrast(this->v->getContrast() - 1); break;
                        case 'N': this->v->setContrast(this->v->getContrast() + 1); break;
                        case 'w': this->v->setWhiteness(this->v->getWhiteness() - 1); break;
                        case 'W': this->v->setWhiteness(this->v->getWhiteness() + 1); break;

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

        v->getFrame(framebuf[0]);

        for (uint16_t i = 1; i < windows; ++i)
            if (likely(funcs[i].f != NULL))
            {
                funcs[i].f((Pixel*)framebuf[funcs[i].src], (Pixel*)framebuf[i], v->getWidth(), v->getHeight());
                SDL_Rect r = {i % winside, i / winside, 0, 0};
                r.x *= width;
                r.y *= height;
                if (unlikely(SDL_BlitSurface(frame[i], NULL, screen, &r) != 0))
                    throw string("Blit failed") + SDL_GetError();
            }
            else
                continue;

        if (unlikely(SDL_BlitSurface(frame[0], NULL, screen, NULL) != 0))
            throw string("Blit failed") + SDL_GetError();

        fps[fps_i++] = (uint16_t)(1/((double)(t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec)/1000000.0));
        if (fps_i == AVG_SAMP)
            fps_i = 0;
        for (uint16_t i = 0; i < AVG_SAMP; ++i)
            fps_avg += fps[i];
        fps_avg /= AVG_SAMP;

        this->DrawText(stringify(fps_avg).c_str(), (SDL_Rect){0,0,0,0}, (SDL_Color){0xff,0xff,0xff,0}, (SDL_Color){0,0,0,0});

        SDL_Flip(screen);

        t2.tv_sec  = t1.tv_sec;
        t2.tv_usec = t1.tv_usec;
    }

    for (uint16_t i = 0; i < windows; ++i)
        SDL_FreeSurface(frame[i]);
}

void MainWin::ScreenShot(SDL_Surface *s)
{
    for (uint32_t i = 0; true; ++i)
    {
        int fd = open(string("shot" + stringify(i) + ".ppm").c_str(), O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        if (fd < 0)
        {
            if(errno == EEXIST)
                continue;
            else
                throw string("Could not open screenshot file: ") + strerror(errno);
        }
        FILE *f = fdopen(fd, "w");
        if (!f)
            throw string("Could not get FILE pointer to screenshot file: ") + strerror(errno);

        fprintf(f, "P6\n%i %i 255\n", s->w, s->h);

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
}

void MainWin::AddFilter(FilterFunc f, uint8_t idx, uint8_t src)
{
    if (idx > windows-1 || src > windows-1)
        throw string("Illegal filter func index");
    if (src >= idx)
        throw string("Illegal filter source (must be <= index)");

    funcs[idx] = (Filter){f, src};
}
/*}}}*/


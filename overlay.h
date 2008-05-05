#ifndef OVERLAY_H
#define OVERLAY_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "global.h"

using novas0x2a::SDLError;
using novas0x2a::TTFError;
using std::string;

class Overlay
{
    public:
        Overlay(Pixel *data, const uint32_t width, const uint32_t height);
        virtual ~Overlay();
    protected:
        uint32_t width, height;
        SDL_Surface *s;

};

Overlay::Overlay(Pixel *data, const uint32_t width, const uint32_t height) : width(width), height(height)
{
    s = SDL_CreateRGBSurfaceFrom((char*)data, width, height, 32, width*4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    if (!s)
        throw SDLError("Could not create overlay surface");
}

Overlay::~Overlay()
{
    SDL_FreeSurface(s);
}

class Text : public Overlay
{
    public:
        Text(Pixel *data, const uint32_t width, const uint32_t height, const char *font, const uint32_t size);
        ~Text();
        void draw(const char* str, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) const;
    private:
        TTF_Font *font;
};

Text::Text(Pixel *data, const uint32_t width, const uint32_t height, const char *font, const uint32_t size) : Overlay(data, width, height)
{
    this->font = TTF_OpenFont(font, size);
    if (!this->font)
        throw TTFError("Could not open font");
}

Text::~Text()
{
    TTF_CloseFont(font);
}

void Text::draw(const char* str, uint8_t r, uint8_t g, uint8_t b) const
{
    SDL_Surface *txt = TTF_RenderText_Solid(font, str, (SDL_Color){r,g,b,0});
    if (unlikely(!txt))
        throw TTFError(string("Text Render failed [") + str + "]");
    if (unlikely(SDL_BlitSurface(txt, NULL, s, NULL)) != 0)
        throw SDLError("Text Blit failed");
}

template <typename T>
class Histogram : public Overlay
{
    public:
        Histogram(Pixel *data, const uint32_t width, const uint32_t height, const uint32_t count);
        ~Histogram();
        void draw(T peak = 0) const;
        //TODO: void setAnchor
        void clear();
        const T& operator[] (unsigned i) const;
        T& operator[] (unsigned i);
    private:
        uint32_t count;
        T* bins;
        T  peak;
};

template <typename T>
Histogram<T>::Histogram(Pixel *data, const uint32_t width, const uint32_t height, const uint32_t count) : Overlay(data, width, height), count(count)
{
    bins = new T[count];
}

template <typename T>
Histogram<T>::~Histogram()
{
    delete [] bins;
}

template <typename T>
const T& Histogram<T>::operator[] (unsigned i) const
{
    return bins[i];
}

template <typename T>
T& Histogram<T>::operator[] (unsigned i)
{
    return bins[i];
}

std::ostream& operator<< (std::ostream& os, const SDL_Rect& a)
{
    os << "SDL_Rect[x[" << a.x << "] y[" << a.y << "] w[" << a.w << "] h[" << a.h << "]]";
    return os;
}

template <typename T>
void Histogram<T>::draw(T peak) const
{
    static const uint32_t sep = 2;
    static T last_peak = 0;
    if (peak == 0)
        for (uint32_t i = 0; i < count; ++i)
            peak = max(peak, bins[i]);
    if (last_peak > peak)
        peak = last_peak;
    else
        last_peak = peak;

    uint16_t bwidth = (width-(count-1*sep))/count;

    for (uint32_t i = 0; i < count; ++i)
    {
        uint16_t bheight = height*bins[i]/peak;
        SDL_Rect tgt = {(bwidth+sep)*i, height-bheight, bwidth, bheight};
        SDL_FillRect(s, &tgt, SDL_MapRGB(s->format, 0,255,0));
    }
}

template <typename T>
void Histogram<T>::clear()
{
    for (uint32_t i = 0; i < count; ++i)
        bins[i] = 0;
}

#endif

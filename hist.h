#ifndef HIST_H
#define HIST_H

#include <SDL.h>
#include "global.h"

template <typename T>
class Histogram
{
    public:
        Histogram(Pixel *data, const uint32_t width, const uint32_t height, const uint8_t count);
        ~Histogram(void);
        void draw(T peak = 0) const;
        //TODO: void setAnchor
        void clear(void);
        const T& operator[] (unsigned i) const;
        T& operator[] (unsigned i);
    private:
        uint32_t width, height, count;
        SDL_Surface *s;
        T *bins;
};

template <typename T>
Histogram<T>::Histogram(Pixel *data, const uint32_t width, const uint32_t height, const uint8_t count) : width(width), height(height), count(count)
{
    s = SDL_CreateRGBSurfaceFrom((char*)data, width, height, 32, width*4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    bins = (T*)malloc(count * sizeof(T));
}

template <typename T>
Histogram<T>::~Histogram(void)
{
    free(bins);
    SDL_FreeSurface(s);
}

template <typename T>
const T& Histogram<T>::operator[] (unsigned i) const
{
    if (i > count-1)
        throw "Illegal Index";
    return bins[i];
}

template <typename T>
T& Histogram<T>::operator[] (unsigned i)
{
    if (i > count-1)
        throw "Illegal Index";
    return bins[i];
}

template <typename T>
void Histogram<T>::draw(T peak) const
{
    if (peak == 0)
        for (uint8_t i = 0; i < count; ++i)
            peak = max(peak, bins[i]);

    uint16_t bwidth = width/count;

    for (uint8_t i = 0; i < count; ++i)
    {
        uint16_t height = bins[i]/width;
        SDL_Rect tgt = {bwidth*i, 240-height, bwidth, height};
        SDL_FillRect(s, &tgt, SDL_MapRGB(s->format, 0,255,0));
    }
}

template <typename T>
void Histogram<T>::clear(void)
{
    for (uint8_t i = 0; i < count; ++i)
        bins[i] = 0;
}

#endif

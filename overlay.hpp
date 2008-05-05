#ifndef OVERLAY_HPP
#define OVERLAY_HPP

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

void Text::draw(const char* str, Pixel color) const
{
    SDL_Surface *txt = TTF_RenderText_Solid(font, str, (SDL_Color){R(color),G(color),B(color),A(color)});
    if (unlikely(!txt))
        throw TTFError(string("Text Render failed [") + str + "]");
    if (unlikely(SDL_BlitSurface(txt, NULL, s, NULL)) != 0)
        throw SDLError("Text Blit failed");
    SDL_FreeSurface(txt);
}


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

template <typename T>
void Histogram<T>::draw(T peak) const
{
    static const uint32_t sep = 2; // separator between bins in px
    static T last_peak = 0;
    if (peak == 0)
        for (uint32_t i = 0; i < count; ++i)
            peak = max(peak, bins[i]);
    if (last_peak > peak)
        peak = last_peak;
    else
        last_peak = peak;

    uint16_t bwidth = (width-((count+1)*sep))/count;

    for (uint32_t i = 0; i < count; ++i)
    {
        uint16_t bheight = height*bins[i]/peak;
        SDL_Rect tgt = {(bwidth+sep)*i + sep, height-bheight, bwidth, bheight};
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

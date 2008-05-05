#ifndef OVERLAY_H
#define OVERLAY_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "global.h"

using novas0x2a::SDLError;
using novas0x2a::TTFError;
using std::string;

// Base class for overlays; handles creating and destroying the sdl surface
class Overlay
{
    public:
        Overlay(Pixel *data, const uint32_t width, const uint32_t height);
        virtual ~Overlay();
    protected:
        uint32_t width, height;
        SDL_Surface *s;
    private:
        explicit Overlay(const Overlay&);
        Overlay& operator=(const Overlay& original);
};

// Text overlay. Puts a string in the upper left
// TODO: needs a setLocation
class Text : public Overlay
{
    public:
        /**
         * Text overlay
         * @param data      pixel array to write to
         * @param width     width of a row of pixels (in pixels)
         * @param height    height of a row of pixels (in pixels)
         * @param font      path to a TTF font
         * @param size      size of font in points
         */
        Text(Pixel *data, const uint32_t width, const uint32_t height, const char *font, const uint32_t size);
        ~Text();
        /**
         * Overlay a string
         * @param str       The string
         * @param color     The text color
         */
        void draw(const char* str, Pixel color) const;
    private:
        TTF_Font *font;
};

// Histogram. Use template parameter to choose type for precision or speed
template <typename T>
class Histogram : public Overlay
{
    public:
        /**
         * Histogram overlay
         * @param data      pixel array to write to
         * @param width     width of a row of pixels (in pixels)
         * @param height    height of a row of pixels (in pixels)
         * @param count     number of bins
         */
        Histogram(Pixel *data, const uint32_t width, const uint32_t height, const uint32_t count);
        ~Histogram();

        /**
         * Draws the overlay onto the surface
         * @param peak  Defines the 100% point. 0 (default) means remember the max and use that
         */
        void draw(T peak = 0) const;

        /**
         * Empties the bins
         */
        void clear();

        /**
         * Access the bins by index. THERE IS NO BOUNDS-CHECKING.
         * @param i bin index
         */
        const T& operator[] (unsigned i) const;
        T& operator[] (unsigned i);
    private:
        uint32_t count;
        T* bins;
        T  peak;
};

#include "overlay.hpp"

#endif

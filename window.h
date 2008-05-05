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

// Characterizes a filter
typedef void (*FilterFunc)(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);
struct Filter {
    Filter(FilterFunc f, SDL_Surface* frame, string name, uint32_t src): f(f), frame(frame), name(name), src(src) {};
    // Processing function
    FilterFunc f;
    // Buffer to draw in (persists)
    SDL_Surface *frame;
    // Name of filter (will be used later for config file filter chains)
    string name;
    // filter to use as the source
    uint32_t src;
};

class Window
{
    public:
        /**
         * Create the main window.
         * @param v         The VideoDevice to use as the primary source
         * @param windows   The number of empty frames to create.
         */
        Window(VideoDevice &v, uint32_t windows);
        ~Window(void);

        /** Run the main loop */
        void MainLoop(void);

        /**
         * Add a filter
         * @param name      Human-readable name for the filter operation
         * @param f         Function to use as the filter
         * @param idx       Filter ID. This should go away, and the name
         *                  should be used instead
         * @param src       Source ID. Sources are the inputs for the filters.
         */
        void AddFilter(const char *name, FilterFunc f, uint32_t idx, uint32_t src = 0);

        /**
         * Helper function to draw arbitrary text
         * @param text  Text string to draw
         * @param loc   Location and size of text
         * @param fg    Foreground color
         * @param bg    Background color
         * @depreciated in favor of the text overlay. Eventually.
         */
        void DrawText(const char *text, SDL_Rect loc, SDL_Color fg, SDL_Color bg);

        /**
         * Take a screenshot. Writes a PPM to a file shot%i.ppm
         * @param s Surface to take a screenshot of
         */
        void ScreenShot(SDL_Surface *s);
    private:
        SDL_Surface *screen;
        VideoDevice &v;
        // The number of total windows, and the number of windows on a side
        uint32_t windows, winside;
        vector<Filter> funcs;
        TTF_Font *font;
};

#endif

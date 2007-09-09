#include <iostream>
#include <string>
#include <cassert>

#include "global.h"
#include "window.h"

using namespace std;

void copy(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    memcpy(out, in, width * height * sizeof(Pixel));
}

void red(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = (in[y] & (Pixel){0,0,0xff,0});
}

void green(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = (in[y] & (Pixel){0,0xff,0,0});
}

void blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
    {
        B(out[y]) = (R(in[y]) + G(in[y]))/2;
        R(out[y]) = R(in[y]);
        G(out[y]) = G(in[y]);
    }
}

int main(int argc, char *argv[])
{
    assert(sizeof(Pixel) == 4);

    try {
        MainWin win(320, 240, 32, 4);
        win.AddFilter(red,   1);
        win.AddFilter(green, 2);
        win.AddFilter(blue,  3);
        win.MainLoop();
    } catch (string p) {
        cerr << "Error: " << p << endl;
    }
    return 0;
}

/* vim: set fdm=marker : */

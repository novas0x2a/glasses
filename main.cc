#include <iostream>
#include <string>

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
        out[y] = in[y] & RGB(0xff, 0, 0);
}

void green(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = in[y] & RGB(0,0xff,0);
}

void blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = in[y] & RGB(0,0,0xff);
}

void replace_blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
    {
        B(out[y]) = (R(in[y]) + G(in[y]))/2;
        R(out[y]) = R(in[y]);
        G(out[y]) = G(in[y]);
    }
}

void invert(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = RGB(0xff,0xff,0xff,0xff) - in[y];
}

void cyan(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    red(in, out, width, height);
    invert(out, out, width, height);
}

void magenta(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    green(in, out, width, height);
    invert(out, out, width, height);
}

void yellow(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    blue(in, out, width, height);
    invert(out, out, width, height);
}

int main(int argc, char *argv[])
{
    try {
        MainWin win(320, 240, 32, 5);

        win.AddFilter(invert,  1);
        win.AddFilter(replace_blue, 2);
        win.AddFilter(red,     3);
        win.AddFilter(green,   4);
        win.AddFilter(blue,    5);
        win.AddFilter(cyan,    6);
        win.AddFilter(magenta, 7);
        win.AddFilter(yellow,  8);

        win.MainLoop();
    } catch (string p) {
        cerr << "Error: " << p << endl;
    }
    return 0;
}

/* vim: set fdm=marker : */

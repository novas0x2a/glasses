#include <iostream>
#include <string>
#include <cmath>

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

void text(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    static SDL_Surface *f = SDL_CreateRGBSurfaceFrom((char*)out, width, height, 32, width*4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    static TTF_Font *font = TTF_OpenFont("/usr/share/fonts/ttf-bitstream-vera/Vera.ttf", 20);
    static uint32_t i = 0;

    SDL_FillRect(f, NULL, SDL_MapRGB(f->format, 0,0,0));

    SDL_Surface *txt = TTF_RenderText_Solid(font, stringify(i++).c_str(), (SDL_Color){ 0xff, 0, 0, 0});
    if (SDL_BlitSurface(txt, NULL, f, NULL) != 0)
        throw string("Text Blit failed: ") + SDL_GetError();
    SDL_FreeSurface(txt);
}

#define V(x) sqrt(pow((double)R((x)),2) + pow((double)G((x)),2) + pow((double)B((x)),2))
void corr(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    double sum_sq_x = 0, sum_sq_y = 0, sum_coproduct = 0;
    double mean_x, mean_y, sweep, delta_x, delta_y, cov_x_y;
    double pop_sd_x, pop_sd_y;

    const Pixel *x = out;
    const Pixel *y = in;

    mean_x = V(x[0]);
    mean_y = V(y[0]);

    for (uint32_t i = 1; i < width*height; ++i)
    {
        sweep = (i - 1.0) / (double)i;
        delta_x = V(x[i]) - mean_x;
        delta_y = V(y[i]) - mean_y;

        sum_sq_x      += delta_x * delta_x * sweep;
        sum_sq_y      += delta_y * delta_y * sweep;
        sum_coproduct += delta_x * delta_y * sweep;

        mean_x += delta_x / (double)i;
        mean_y += delta_y / (double)i;
        out[i] = in[i];
    }
    pop_sd_x = sqrt( sum_sq_x / (width*height) );
    pop_sd_y = sqrt( sum_sq_y / (width*height) );
    cov_x_y = sum_coproduct   / (width*height);
    cerr << cov_x_y / (pop_sd_x * pop_sd_y) << endl;
}

int main(int argc, char *argv[])
{
    try {
        //MainWin win(320, 240, 32, 2);
        //win.AddFilter(corr,  1);

        MainWin win(320, 240, 32, 5);

        win.AddFilter(invert,  1);
        win.AddFilter(replace_blue, 2);
        win.AddFilter(red,     3);
        win.AddFilter(green,   4);
        win.AddFilter(blue,    5);
        win.AddFilter(cyan,    6);
        win.AddFilter(magenta, 7);
        win.AddFilter(corr,  8);


        win.MainLoop();
    } catch (string p) {
        cerr << "Error: " << p << endl;
    }
    return 0;
}

/* vim: set fdm=marker : */

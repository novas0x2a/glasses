#include <iostream>
#include <string>
#include <limits>
#include <algorithm>

#include "global.h"
#include "window.h"
#include "hist.h"

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

void fixbright(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    uint8_t Min = numeric_limits<uint8_t>::max(), Max = 0;

    for (uint32_t y = 0; y < height*width; ++y)
    {
        Min = min(min(Min, R(in[y])), min(G(in[y]), B(in[y])));
        Max = max(max(Max, R(in[y])), max(G(in[y]), B(in[y])));
    }

    double scale = 255/(Max-Min);

    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = (in[y] - RGB(Min, Min, Min)) * RGB(scale, scale, scale);
}

void rgb(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    static Histogram<double> bin(out, width, height, 3);

    bin.clear();

    for (uint32_t y = 0; y < height*width; ++y)
    {
        bin[0] += R(out[y]) / V(out[y]);
        bin[1] += G(out[y]) / V(out[y]);
        bin[2] += B(out[y]) / V(out[y]);
        out[y] = in[y];
    }

    bin.draw();
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
        MainWin win(320, 240, 32, 5);

        win.AddFilter(rgb,       1, 0);
        win.AddFilter(invert,    2, 0);
        win.AddFilter(red,       3, 0);
        win.AddFilter(green,     4, 0);
        win.AddFilter(blue,      5, 0);
        win.AddFilter(invert,    6, 3);  // cyan
        win.AddFilter(invert,    7, 4);  // magenta
        win.AddFilter(invert,    8, 5);  // yellow

        win.MainLoop();
    } catch (string p) {
        cerr << "Error: " << p << endl;
    }
    return 0;
}

/* vim: set fdm=marker : */

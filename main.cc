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
    static Text txt(out, width, height, "/usr/share/fonts/ttf-bitstream-vera/Vera.ttf", 20);
    static uint32_t i = 0;
    memcpy(out, in, width * height * sizeof(Pixel));
    txt.draw(stringify(i++).c_str(), 0xff, 0xff, 0);
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

void gray(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t i = 0; i < width*height; ++i)
        out[i] = RGB(V(in[i]), V(in[i]), V(in[i]));

}

void edge(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    double val;
    static Text txt(out, width, height, "/usr/share/fonts/ttf-bitstream-vera/Vera.ttf", 20);
    static uint32_t j = 0;
    for (uint32_t i = 1; i < width*height-1; ++i)
    {
        //val = abs(V(in[i-1]) - 2*V(in[i]) + V(in[i+1]));
        val = abs(-V(in[i-1]) + V(in[i+1]))/2;
        out[i] = val > j ? RGB(0xff,0xff,0xff) : RGB(0,0,0);
    }
    txt.draw(stringify(j).c_str(), 0xff, 0, 0);
    if (j > 50)
        j = 0;
    else
        j++;
}

int main(int argc, char *argv[])
{
    try {
        MainWin win(176, 144, 32, 10);

        win.AddFilter(rgb,       1, 0);
        win.AddFilter(invert,    2, 0);
        win.AddFilter(text,      3, 0);
        win.AddFilter(gray,      4, 0);
        win.AddFilter(edge,      5, 4);

        win.AddFilter(red,       8, 0);
        win.AddFilter(green,     9, 0);
        win.AddFilter(blue,     10, 0);
        win.AddFilter(invert,   12, 8);   // cyan
        win.AddFilter(invert,   13, 9);   // magenta
        win.AddFilter(invert,   14, 10);  // yellow

        win.MainLoop();
    } catch (string p) {
        cerr << "Error: " << p << endl;
    }
    return 0;
}

/* vim: set fdm=marker : */

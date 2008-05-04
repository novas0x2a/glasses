#include <iostream>
#include <string>
#include <limits>
#include <algorithm>

#include "global.h"
#include "window.h"
#include "overlay.h"

#include "video/v4l.h"

using namespace std;
using namespace novas0x2a;

// Helpers for some of the filters
inline Pixel& get(Pixel *array, const uint32_t x, const uint32_t y, const uint32_t width) {return array[y*width + x];}
inline bool has_red(const Pixel *array, const uint32_t x, const uint32_t y, const uint32_t width) {return bool(R(get(const_cast<Pixel*>(array), x, y, width)));}

// Identity
void copy(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    memcpy(out, in, width * height * sizeof(Pixel));
}

// Red Channel
void red(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = in[y] & RGB(0xff, 0, 0);
}

// Green Channel
void green(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = in[y] & RGB(0,0xff,0);
}

// Blue Channel
void blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = in[y] & RGB(0,0,0xff);
}

// 3-Pixel Radius Blur
void blur(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 1; y < height*width-1; ++y)
        out[y] = (in[y-1] + in[y] + in[y+1])/RGB(3,3,3);
}

// Replace the blue channel with the average of the red and green.
// This makes the blue channel noise less obvious
void replace_blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
    {
        B(out[y]) = (R(in[y]) + G(in[y]))/2;
        R(out[y]) = R(in[y]);
        G(out[y]) = G(in[y]);
    }
}

// invert the image
void invert(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = RGB(0xff,0xff,0xff,0xff) - in[y];
}

// Do a dumb rescale of the contrast
void linear_contrast(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    uint8_t Min = numeric_limits<uint8_t>::max(), Max = 0;

    for (uint32_t y = 0; y < height*width; ++y)
    {
        Min = min(min(Min, R(in[y])), min(G(in[y]), B(in[y])));
        Max = max(max(Max, R(in[y])), max(G(in[y]), B(in[y])));
    }

    // TODO: This dumb function is made even worse by the lack of a floating point vector
    uint8_t scale = 255.0/(Max-Min);

    for (uint32_t y = 0; y < height*width; ++y)
        out[y] = (in[y] - RGB(Min, Min, Min)) * RGB(scale, scale, scale);
}

// Histogram of the rgb pixels
void rgb_hist(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    static Histogram<double> bin(out, width, height, 3);

    bin.clear();

    for (uint32_t y = 0; y < height*width; ++y)
    {
        bin[0] += R(in[y]) / Vd(in[y]);
        bin[1] += G(in[y]) / Vd(in[y]);
        bin[2] += B(in[y]) / Vd(in[y]);
    }
    memset(out, 0, width*height*sizeof(Pixel));

    bin.draw();
}

// Draw a simple counter for the number of frames seen
void frame_counter(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    static Text txt(out, width, height, FONT, 20);
    static uint32_t i = 0;
    memcpy(out, in, width * height * sizeof(Pixel));
    txt.draw(stringify(i++).c_str(), 0xff, 0xff, 0);
}

// Greyscale (NTSC)
void gray(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t i = 0; i < width*height; ++i)
        out[i] = RGB(Vd(in[i]), Vd(in[i]), Vd(in[i]));
}

// Edge-detection
void edge(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    double val;
    for (uint32_t i = 1; i < width*height-1; ++i)
    {
        val = abs(-Vd(in[i-1]) + Vd(in[i+1]))/2;
        out[i] = val > 15 ? RGB(0xff,0xff,0xff) : RGB(0,0,0);
    }
}

// Crazy color effects
void colorize(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    static Pixel color[5];
    int32_t idx = 0;
    static bool done = 0;
    if (!done++)
    {
        srand(time(NULL));
        for (uint32_t i = 0; i < 5; ++i)
            color[i] = RGB(rand() % 255, rand() % 255, rand() % 255);
    }
    for (uint32_t y = 0; y < height; ++y)
    {
        idx = 0;
        for (uint32_t x = 0; x < width; ++x)
        {
            static bool chg,last;
            chg = Vd(get(const_cast<Pixel*>(in), x, y, width));
            if (!last && chg)
                idx = (idx + 1) % 5;
            get(out, x, y, width) = color[idx];
            last = chg;
        }
    }
}

void edge2(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    double val;
    static Text txt(out, width, height, FONT, 20);
    static uint32_t j = 0;
    for (uint32_t i = 1; i < width*height-1; ++i)
    {
        val = abs(Vd(in[i-1]) - 2*Vd(in[i]) + Vd(in[i+1]));
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
        Context c("When running " PROGRAM " " VERSION);
#if 1
        V4LDevice v("/dev/video0");
        //TODO: Tied to SDL pixel format definitions
        v.setParams(176, 144, 32, VIDEO_PALETTE_RGB32);

        MainWin win(v, 14);

        win.AddFilter("Brightness",      linear_contrast, 1, 0);
        win.AddFilter("RGB Histogram",   rgb_hist,      2, 1);
        win.AddFilter("Inverter",        invert,        3, 1);
        win.AddFilter("Counter",         frame_counter, 4, 1);
        win.AddFilter("Grayscale",       gray,          5, 1);
        win.AddFilter("Edge detect",     edge,          6, 5);
        win.AddFilter("Colorize",        colorize,      7, 6);

        win.AddFilter("Red Channel",     red,           8, 1);
        win.AddFilter("Green Channel",   green,         9, 1);
        win.AddFilter("Blue Channel",    blue,         10, 1);
        win.AddFilter("Cyan Channel",    invert,       12, 8);
        win.AddFilter("Magenta Channel", invert,       13, 9);
        win.AddFilter("Yellow Channel",  invert,       14, 10);
#else
        MainWin win(320, 240, 32, 3);
        win.AddFilter("Fixing Brightness", linear_contrast, 1, 0);
        win.AddFilter("Grayscale",         gray,      2, 1);
        win.AddFilter("Blurring",          blur,      3, 2);
#endif

        win.MainLoop();
    } catch (const Exception &e) {
        cerr << "Exception:" << endl
            << "  * "
            << e.backtrace("\n  * ")
            << e.message()
            << endl;
    } catch (const exception &e) {
        cerr << "Exception: " << e.what() << endl;
    } catch (...) {
        cerr << "Ack. Really unhandled exception." << endl;
    }

    return 0;
}

#if 0
void corr(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    double sum_sq_x = 0, sum_sq_y = 0, sum_coproduct = 0;
    double mean_x, mean_y, sweep, delta_x, delta_y, cov_x_y;
    double pop_sd_x, pop_sd_y;

    const Pixel *x = out;
    const Pixel *y = in;

    mean_x = Vd(x[0]);
    mean_y = Vd(y[0]);

    for (uint32_t i = 1; i < width*height; ++i)
    {
        sweep = (i - 1.0) / (double)i;
        delta_x = Vd(x[i]) - mean_x;
        delta_y = Vd(y[i]) - mean_y;

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
void erode(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height, const uint32_t size)
{
}

void opening(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    double val;
    for (uint32_t y = 0; y < height; ++y)
    {
        uint32_t idx = 0;
        for (uint32_t x = 0; x < width; ++x)
        {
            static bool chg,last;
            chg = Vd(get(const_cast<Pixel*>(in), x, y, width));
            if (!last && chg)
                idx = (idx + 1) % 5;
            get(out, x, y, width) = color[idx];
            last = chg;
        }
    }
}

void lines(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    uint32_t sum;
    static const uint32_t threas = 4;

    for (uint32_t y = threas; y < height-threas; ++y)
        for (uint32_t x = threas; x < width-threas; ++x)
        {
            sum = 0;
            for (uint32_t i = 0; i < threas+1; ++i)
                for (uint32_t j = 0; j < threas+1; ++j)
                {
                    sum += has_red(in,x-i,y-j,width);
                    sum += has_red(in,x+i,y+j,width);
                }
            sum += has_red(in,x,y,width);

            if (sum > threas)
                get(out,x,y,width) = RGB(0xff, 0xff, 0xff);
            else
                get(out,x,y,width) = RGB(0,0,0);
        }
}

#endif

/* vim: set fdm=marker : */

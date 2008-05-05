#ifndef FILTERS_H
#define FILTERS_H

#include <iostream>
#include <string>
#include <limits>
#include <algorithm>

#include "global.h"

// Identity
void copy(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Red Channel
void red(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Green Channel
void green(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Blue Channel
void blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// 3-Pixel Radius Blur
void blur(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Replace the blue channel with the average of the red and green.
// This makes the blue channel noise less obvious
void replace_blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// invert the image
void invert(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Do a dumb rescale of the contrast
void linear_contrast(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Histogram of the rgb pixels
void rgb_hist(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Draw a simple counter for the number of frames seen
void frame_counter(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Greyscale (NTSC)
void gray(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Edge-detection
void edge(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

// Crazy color effects
void colorize(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

#if 0
void edge2(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);
void corr(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);
void erode(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height, const uint32_t size)
void opening(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);
void lines(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);
#endif

#endif

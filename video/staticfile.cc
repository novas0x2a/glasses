#include <iostream>

// For open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../global.h"
#include "videodevice.h"
#include "staticfile.h"

using namespace std;
using namespace novas0x2a;

// TODO: Parsing with scanf is hackish. I should probably use a real image
// library so I can load stuff besides ppms.
StaticFile::StaticFile(const char *file) : image(NULL)
{
    Context c("While creating StaticFile");
    FILE *f = NULL;

    f = fopen(file, "r");
    if (!f)
        throw StaticFileError(string("Could not open file ") + file);

    try {

        if (fscanf(f, "P6\n%u %u 255\n", &width, &height) != 2)
            throw ArgumentError(string(file) + " doesn't look like a 24bpp ppm. See the README.");
        depth = 32;

        if (width * height > 1024*768)
            throw ArgumentError("StaticFiles are limited to 1024*768 pixels");

        image_width  = width;
        image_height = height;
        image_depth  = depth;

        image = new Pixel[width*height];

        byte r,g,b;

        for (uint32_t y = 0; y < height; ++y)
            for (uint32_t x = 0; x < width; ++x)
            {
                uint32_t off = y*width + x;
                if (fscanf(f, "%c%c%c", &r, &g, &b) != 3)
                    throw ArgumentError(string(file) + " the ppm header doesn't agree with the data");
                image[off] = RGB(r, g, b);
            }
    } catch (...) {
        if (image)
            delete [] image;
        fclose(f);
        throw;
    }
    fclose(f);
}

StaticFile::~StaticFile(void)
{
    Context c("While closing staticfile");
    delete [] image;
}

void StaticFile::setParams(uint32_t width, uint32_t height, uint16_t depth, uint16_t palette)
{
    Context c("While setting StaticFile params (" + stringify(width) + "," + stringify(height) + "@" + stringify(depth) + "bpp)");
    this->width  = width;
    this->height = height;
    this->depth  = depth;
}

uint16_t StaticFile::getBrightness(void) const
{
    throw UnimplementedError(FUNCTION_HERE);
}
uint16_t StaticFile::getHue(void) const
{
    throw UnimplementedError(FUNCTION_HERE);
}
uint16_t StaticFile::getColour(void) const
{
    throw UnimplementedError(FUNCTION_HERE);
}
uint16_t StaticFile::getContrast(void) const
{
    throw UnimplementedError(FUNCTION_HERE);
}
uint16_t StaticFile::getWhiteness(void) const
{
    throw UnimplementedError(FUNCTION_HERE);
}

void StaticFile::setBrightness(uint16_t x)
{
    throw UnimplementedError(FUNCTION_HERE);
}
void StaticFile::setHue(uint16_t x)
{
    throw UnimplementedError(FUNCTION_HERE);
}
void StaticFile::setColour(uint16_t x)
{
    throw UnimplementedError(FUNCTION_HERE);
}
void StaticFile::setContrast(uint16_t x)
{
    throw UnimplementedError(FUNCTION_HERE);
}
void StaticFile::setWhiteness(uint16_t x)
{
    throw UnimplementedError(FUNCTION_HERE);
}

void StaticFile::getFrame(byte *buf)
{
    if (image_width * image_height * image_depth >= width * height * depth)
        memcpy(buf, image, width*height*depth>>3);
    else
        throw UnimplementedError(
                string("image(")
                + stringify(image_width) + "x" + stringify(image_height) + "@" + stringify(image_depth) + ")"
                + " must be at least as big as requested("
                + stringify(width) + "x" + stringify(height) + "@" + stringify(depth) + ")" +
                FUNCTION_HERE);
}

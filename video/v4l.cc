#include <iostream>

// For open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// For V4l stuff
#include <linux/types.h>
#include <linux/videodev.h>

// For ioctl
#include <sys/ioctl.h>

#include "../global.h"
#include "videodevice.h"
#include "v4l.h"

using namespace std;
using novas0x2a::Context;
using novas0x2a::stringify;

ostream& operator<< (ostream& os, const VideoCapability& a) {
    os  << "Name["        << a.name
        << "] Type["      << a.type
        << "] Channels["  << a.channels
        << "] Audios["    << a.audios
        << "] MaxWidth["  << a.maxwidth
        << "] MaxHeight[" << a.maxheight
        << "] MinWidth["  << a.minwidth
        << "] MinHeight["  << a.minheight << "]";
    return os;
}

ostream& operator<< (ostream& os, const VideoWindow& a) {
    os  << "Pos[" << a.x << "," << a.y << "] Size[" << a.width << "," << a.height << "]";
    return os;
}

ostream& operator<< (ostream& os, const VideoPicture& a) {
    os  << "Brightness["    << a.brightness
        << "] Hue["         << a.hue
        << "] Color["       << a.colour
        << "] Contrast["    << a.contrast
        << "] Whiteness["   << a.whiteness
        << "] Depth["       << a.depth
        << "] Palette["     << a.palette << "]";
    return os;
}

ostream& operator<< (ostream &os, const V4LDevice& v)
{
    os << "Cap: " << v.getCap() << endl
       << "Win: " << v.getWin() << endl
       << "Pic: " << v.getPic();
    return os;
}

V4LDevice::V4LDevice(const char *device) : devname(device)
{
    Context c("While creating V4L device");
    if ((dev = open(device, O_RDONLY)) < 0)
        throw V4LError(string("Could not open video device ") + device);

    VideoWindow     win = this->getWin();
    VideoPicture    pic = this->getPic();

    width  = win.width;
    height = win.height;
    depth  = pic.depth;
}

V4LDevice::~V4LDevice(void)
{
    Context c("While closing V4L device");
    if (close(dev) < 0)
        throw V4LError("Could not close video device");
}

VideoCapability& V4LDevice::getCap(void) const
{
    Context c("While getting V4L capabilities");
    static VideoCapability cap;
    if (ioctl(dev, VIDIOCGCAP, &cap) < 0)
        throw V4LError("Couldn't get capabilities");
    return cap;
}

VideoWindow& V4LDevice::getWin(void) const
{
    Context c("While getting V4L window");
    static VideoWindow win;
    if (ioctl(dev, VIDIOCGWIN, &win) < 0)
        throw V4LError("Couldn't get window");
    return win;
}

VideoPicture& V4LDevice::getPic(void) const
{
    Context c("While getting V4L picture");
    static VideoPicture vpic;
    if (ioctl(dev, VIDIOCGPICT, &vpic) < 0)
        throw V4LError("Couldn't get picture");
    return vpic;
}

void V4LDevice::setWin(const VideoWindow& win)
{
    Context c("While setting V4L window: " + stringify(win));
    VideoWindow w;
    if(ioctl(dev, VIDIOCSWIN, &win) < 0)
        throw V4LError("Couldn't set window");
    if(ioctl(dev, VIDIOCGWIN, &w) < 0)
        throw V4LError("Couldn't get window");

    width  = w.width;
    height = w.height;
}

void V4LDevice::setPic(const VideoPicture& pic)
{
    Context c("While setting V4L picture: " + stringify(pic));
    VideoPicture p;
    if(ioctl(dev, VIDIOCSPICT, &pic) < 0)
        throw V4LError("Couldn't set picture");
    if(ioctl(dev, VIDIOCGPICT, &p) < 0)
        throw V4LError("Couldn't get picture");
    depth = p.depth;
}

void V4LDevice::setParams(uint32_t width, uint32_t height, uint16_t depth, uint16_t palette)
{
    Context c("While setting V4L params (" + stringify(width) + "," + stringify(height) + "@" + stringify(depth) + "bpp)");
    VideoWindow w = this->getWin();
    w.width  = width;
    w.height = height;
    this->setWin(w);

    if (depth != 0)
    {
        VideoPicture p = this->getPic();
        p.depth   = depth;
        p.palette = palette;
        this->setPic(p);
    }
}

uint16_t V4LDevice::getBrightness(void) const
{
    return this->getPic().brightness;
}
uint16_t V4LDevice::getHue(void) const
{
    return this->getPic().hue;
}
uint16_t V4LDevice::getColour(void) const
{
    return this->getPic().colour;
}
uint16_t V4LDevice::getContrast(void) const
{
    return this->getPic().contrast;
}
uint16_t V4LDevice::getWhiteness(void) const
{
    return this->getPic().whiteness;
}

void V4LDevice::setBrightness(uint16_t x)
{
    VideoPicture p = this->getPic();
    p.brightness = x;
    this->setPic(p);
}
void V4LDevice::setHue(uint16_t x)
{
    VideoPicture p = this->getPic();
    p.hue = x;
    this->setPic(p);
}
void V4LDevice::setColour(uint16_t x)
{
    VideoPicture p = this->getPic();
    p.colour = x;
    this->setPic(p);
}
void V4LDevice::setContrast(uint16_t x)
{
    VideoPicture p = this->getPic();
    p.contrast = x;
    this->setPic(p);
}
void V4LDevice::setWhiteness(uint16_t x)
{
    VideoPicture p = this->getPic();
    p.whiteness = x;
    this->setPic(p);
}

void V4LDevice::getFrame(byte *buf)
{
    if (read(dev, buf, width * height * depth>>3) < 0)
        throw V4LError("Unable to read from device");
}

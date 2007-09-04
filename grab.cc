#include <iostream>
#include <string>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cassert>

#include <linux/types.h>
#include <linux/videodev.h>

using namespace std;

/*{{{ VideoDevice */
typedef struct video_capability VideoCapability;
typedef struct video_window     VideoWindow;
typedef struct video_picture    VideoPicture;

ostream& operator<< (ostream& os, VideoCapability& a) {
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

ostream& operator<< (ostream& os, VideoWindow& a) {
    os  << "Pos[" << a.x << "," << a.y << "] Size[" << a.width << "," << a.height << "]";
    return os;
}

ostream& operator<< (ostream& os, VideoPicture& a) {
    os  << "Brightness["    << a.brightness
        << "] Hue["         << a.hue
        << "] Color["       << a.colour
        << "] Contrast["    << a.contrast
        << "] Whiteness["   << a.whiteness
        << "] Depth["       << a.depth
        << "] Palette["     << a.palette << "]";
    return os;
}

class VideoDevice
{
    public:

        VideoDevice(const char *);
        ~VideoDevice(void);

        VideoCapability& getCap(void);
        VideoWindow&     getWin(void);
        VideoPicture&    getPic(void);

        void setWin(const VideoWindow&);
        void setPic(const VideoPicture&);

        void setParams(const uint32_t width, const uint32_t height, const uint16_t palette = VIDEO_PALETTE_RGB24, const uint16_t depth = 0);
        pair<pair<uint32_t, uint32_t>, uint16_t> getParams(void);

        void getFrame(char *buf);

        uint32_t width, height;
        uint16_t depth;
    private:
        const string devname;
        int dev;
};

VideoDevice::VideoDevice(const char *device) : devname(device)
{
    if ((dev = open(devname.c_str(), O_RDONLY)) < 0)
        throw string("Could not open video device");

    VideoCapability cap = this->getCap();
    cerr << "Opened " << cap.name << endl;

    VideoWindow     win = this->getWin();
    VideoPicture    pic = this->getPic();

    width  = win.width;
    height = win.height;
    depth  = pic.depth;
}

VideoDevice::~VideoDevice(void)
{
    if (close(dev) < 0)
        throw string("Could not close video device");
}


VideoCapability& VideoDevice::getCap(void)
{
    static VideoCapability cap;
    if (ioctl(dev, VIDIOCGCAP, &cap) < 0)
        throw string("Couldn't get capabilities");
    return cap;
}

VideoWindow& VideoDevice::getWin(void)
{
    static VideoWindow win;
    if (ioctl(dev, VIDIOCGWIN, &win) < 0)
        throw string("Couldn't get window");
    return win;
}

VideoPicture& VideoDevice::getPic(void)
{
    static VideoPicture vpic;
    if (ioctl(dev, VIDIOCGPICT, &vpic) < 0)
        throw string("Couldn't get picture");
    return vpic;
}

void VideoDevice::setWin(const VideoWindow& win)
{
    VideoWindow w;
    if(ioctl(dev, VIDIOCSWIN, &win) < 0)
        throw string("Couldn't set window");
    if(ioctl(dev, VIDIOCGWIN, &w) < 0)
        throw string("Couldn't get window");

    width  = w.width;
    height = w.height;
}

void VideoDevice::setPic(const VideoPicture& pic)
{
    VideoPicture p;
    if(ioctl(dev, VIDIOCSPICT, &pic) < 0)
        throw string("Couldn't set picture");
    if(ioctl(dev, VIDIOCGPICT, &p) < 0)
        throw string("Couldn't set picture");
    depth = p.depth;
}

void VideoDevice::setParams(const uint32_t width, const uint32_t height, const uint16_t palette, const uint16_t depth)
{
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

pair<pair<uint32_t, uint32_t>, uint16_t> VideoDevice::getParams(void)
{
    static pair<pair<uint32_t, uint32_t>, uint16_t> data;

    data.first.first  = width;
    data.first.second = height;
    data.second       = depth;

    return data;
}

void VideoDevice::getFrame(char *buf)
{
    if (read(dev, buf, width * height * depth/3) < 0)
        throw string("Unable to read from device");
}
/*}}}*/

class MainWin {
    MainWin(uint32_t width, uint32_t height, uint32_t depth);
    ~MainWin(void);
    MainLoop(void);
}

void Run(void)
{
    VideoDevice v("/dev/video0");

    v.setParams(640,480,VIDEO_PALETTE_RGB24,24);
    cerr << v.getCap() << endl << v.getWin() << endl << v.getPic() << endl;

    sleep(1);

    assert(v.width  == 640);
    assert(v.height == 480);
    assert(v.depth  == 24);

    char *buf = new char[v.width*v.height*3];

    v.getFrame(buf);
    cout << "P6\n" << v.width << " " << v.height << " 255\n";

    for (uint32_t y = 0; y < v.height; ++y)
        for (uint32_t x = 0; x < v.width; ++x)
        {
            uint32_t off = (y*v.width + x) * 3;
            fputc(buf[off],   stdout);
            fputc(buf[off+1], stdout);
            fputc(buf[off+2], stdout);
        }

    delete[] buf;
}

int main(int argc, char *argv[])
{
    try {
        Run();
    } catch (string p) {
        cerr << "Error: " << p << endl;
    }
    return 0;
}

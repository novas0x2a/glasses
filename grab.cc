#include <iostream>
#include <string>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/types.h>
#include <linux/videodev.h>

using namespace std;

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

        void setParams(const uint32_t width, const uint32_t height, const uint16_t depth = 0);
        pair<pair<uint32_t, uint32_t>, uint16_t> getParams(void);

    private:
        const string devname;
        int dev;
};

VideoDevice::VideoDevice(const char *device) : devname(device)
{
    VideoCapability cap;

    if ((dev = open(devname.c_str(), O_RDONLY)) < 0)
        throw string("Could not open video device");

    if (ioctl(dev, VIDIOCGCAP, &cap) < 0)
        throw string("Couldn't get capabilities. Not a V4L device?");
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
    if(ioctl(dev, VIDIOCSWIN, &win) < 0)
        throw string("Couldn't set window");
}

void VideoDevice::setPic(const VideoPicture& pic)
{
    if(ioctl(dev, VIDIOCSPICT, &pic) < 0)
        throw string("Couldn't set picture");
}

void VideoDevice::setParams(const uint32_t width, const uint32_t height, const uint16_t depth)
{
    VideoWindow w = this->getWin();
    w.width  = width;
    w.height = height;
    this->setWin(w);

    if (depth != 0)
    {
        VideoPicture p = this->getPic();
        p.depth = depth;
        this->setPic(p);
    }
}

pair<pair<uint32_t, uint32_t>, uint16_t> VideoDevice::getParams(void)
{
    static pair<pair<uint32_t, uint32_t>, uint16_t> data;
    VideoWindow  w = this->getWin();
    VideoPicture p = this->getPic();

    data.first.first = w.width;
    data.first.second = w.height;
    data.second = p.depth;

    return data;
}

void Run(void)
{
    VideoDevice v("/dev/video0");

    v.setParams(640,480,24);
    cout << v.getCap() << endl << v.getWin() << endl << v.getPic() << endl;
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

#if 0

    buffer = malloc(win.width * win.height * vpic.depth);
    read(v, buffer, win.width * win.height * vpic.depth);

    fprintf(stdout, "P6\n%d %d 255\n", win.width, win.height);

    assert(vpic.palette == VIDEO_PALETTE_RGB24);

    src = buffer;
    for (i = 0; i < win.width * win.height; ++i)
    {
        fputc(src[0], stdout);
        fputc(src[1], stdout);
        fputc(src[2], stdout);
        src += 3;
    }

    free(buffer);

exit1:
    if (close(v) < 0)
        pgoto(exit0, "Close failed");
exit0:
#endif

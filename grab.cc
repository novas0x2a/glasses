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

#if 0
#include <assert.h>

#endif
#if 0
char _pdie[255] = {0};
char ret = 0;

#define pwarn(...) do                       \
{                                           \
    snprintf(_pdie, 254, __VA_ARGS__);      \
    perror(_pdie);                          \
} while (0)

#define pgoto(lbl, ...) do                  \
{                                           \
    pwarn(__VA_ARGS__);                     \
    ret = 1;                                \
    goto lbl;                               \
} while (0)

typedef struct {
    uint16_t depth;
    uint16_t palette;
} CapMode;

CapMode modes[] = {
    {24, VIDEO_PALETTE_RGB24},
    {16, VIDEO_PALETTE_RGB565},
    {15, VIDEO_PALETTE_RGB555}
};

const unsigned mode_count = sizeof(modes) / sizeof(modes[0]);

#endif
typedef struct video_capability VideoCapability;
typedef struct video_window     VideoWindow;
typedef struct video_picture    VideoPicture;

class VideoDevice
{
    public:

        VideoDevice(const char *);
        ~VideoDevice(void);

        VideoCapability getCap(void);
        VideoWindow     getWin(void);
        VideoPicture    getPic(void);

        void setWin(const VideoWindow&    );
        void setPic(const VideoPicture&   );

    private:
        const string devname;
        int dev;
};

VideoDevice::VideoDevice(const char *device) : devname(device)
{
    VideoCapability cap;

    if ((dev = open(device, O_RDONLY)) < 0)
        throw string("Could not open video device");

    if (ioctl(dev, VIDIOCGCAP, &cap) < 0)
        throw string("Couldn't get capabilities. Not a V4L device?");
}

VideoDevice::~VideoDevice(void)
{
    if (close(dev) < 0)
        throw string("Could not close video device");
}


VideoCapability VideoDevice::getCap(void)
{
}

VideoWindow VideoDevice::getWin(void)
{
}

VideoPicture VideoDevice::getPic(void)
{
}


void Run(void)
{
    VideoDevice v("/dev/video0");
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
    int v;
    unsigned i;
    unsigned char *buffer, *src;

    if ((v = open("/dev/video0", O_RDONLY)) < 0)
        pgoto(exit0, "Could not open video device");

    sleep(5);

    if (ioctl(v, VIDIOCGCAP, &cap) < 0)
        pgoto(exit1, "Couldn't get capabilities. Not a V4L device?\n");

    if (ioctl(v, VIDIOCGWIN, &win) < 0)
        pgoto(exit1, "Couldn't get window\n");

    if (ioctl(v, VIDIOCGPICT, &vpic) < 0)
        pgoto(exit1, "Couldn't get picture\n");

    for (i = 0; i < mode_count; ++i)
    {
        vpic.palette = modes[i].palette;
        vpic.depth   = modes[i].depth;
        if(ioctl(v, VIDIOCSPICT, &vpic) >= 0)
            goto got_one;
    }
    pgoto(exit1, "Couldn't pick a good video mode");

got_one:

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

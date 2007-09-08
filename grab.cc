#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <cmath>
#include <vector>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include <linux/types.h>
#include <linux/videodev.h>

using namespace std;

template<typename T>
inline std::string stringify(const T& x)
{
    ostringstream o;
    o << x;
    return o.str();
}

/*{{{ Print Overloads */
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
}/*}}}*/

class VideoDevice /*{{{*/
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

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} Pixel;
typedef void (*FilterFunc)(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height);

class MainWin /*{{{*/
{
    public:
        MainWin(uint32_t width, uint32_t height, uint32_t depth, uint8_t windows = 1);
        ~MainWin(void);
        void MainLoop(void);
        void WriteFrame(const string filename, bool newFrame);
        void AddFilter(FilterFunc f, uint8_t idx);
        void DrawText(char text[], SDL_Rect loc, SDL_Color fg, SDL_Color bg);
        void ScreenShot(SDL_Surface *s);
    private:
        uint32_t width, height, depth;
        SDL_Surface *screen;
        VideoDevice *v;
        char **framebuf;
        uint8_t windows, winside;
        FilterFunc *funcs;
        TTF_Font *font;
};

MainWin::MainWin(uint32_t w, uint32_t h, uint32_t d, uint8_t win) : width(w), height(h), depth(d)
{
    assert(depth == 24);
    assert(windows > 0);

    v = new VideoDevice("/dev/video0");

    v->setParams(width, height, VIDEO_PALETTE_RGB24, depth);

    assert(v->width  == width);
    assert(v->height == height);
    assert(v->depth  == depth);

    winside = (uint8_t)ceil(sqrt(win));
    windows = winside * winside;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw string("Could not init SDL");

    if (!(screen = SDL_SetVideoMode(width*winside, height*winside, depth, SDL_HWSURFACE)))
        throw string("Unable to set video mode: ") + SDL_GetError();

    if (TTF_Init() == -1)
        throw string("Could not init TTF: ") + TTF_GetError();

    if (!(font = TTF_OpenFont("/usr/share/fonts/ttf-bitstream-vera/Vera.ttf", 20)))
        throw string("Could not load font: ") + TTF_GetError();

    framebuf    = (char**)malloc(windows * sizeof(char*));
    framebuf[0] = (char*)malloc(windows * v->width * v->height * v->depth * 3 * sizeof(char));
    for (uint16_t i = 1; i < windows; ++i)
        framebuf[i] = framebuf[0] + (i * v->width * v->height * v->depth * 3 * sizeof(char));

    funcs = new FilterFunc[windows];
    memset(funcs, 0, sizeof(FilterFunc) * windows);
}

MainWin::~MainWin(void)
{
    delete[] funcs;
    free(framebuf[0]);
    free(framebuf);
    SDL_FreeSurface(screen);
    SDL_Quit();
}

void MainWin::DrawText(char text[], SDL_Rect loc, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface *txt = TTF_RenderText_Shaded(font, text, fg, bg);
    if (SDL_BlitSurface(txt, NULL, screen, &loc) != 0)
        throw string("Text Blit failed") + SDL_GetError();
    SDL_FreeSurface(txt);
}

void MainWin::MainLoop(void)
{
    SDL_Event event;
    SDL_Surface *frame[windows];
    int i = 0;
    struct timeval t1, t2 = {0,0};
    static uint16_t fps[5] = {0};
    uint16_t fps_i = 0;

    for (uint16_t i = 0; i < windows; ++i)
        if (!(frame[i] = SDL_CreateRGBSurfaceFrom(framebuf[i], v->width, v->height, v->depth, v->width*3, 0x00ff0000, 0x0000ff00, 0x000000ff, 0)))
            throw string("CreateSurface failed") + SDL_GetError();

    while (1)
    {
        gettimeofday(&t1, NULL);
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case 'q':
                            return;
                        case 'r':
                            this->WriteFrame(string("file") + stringify(i++) + ".ppm", false);
                            break;
                        case 's':
                            this->ScreenShot(screen);
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_QUIT:
                    return;
                default:
                    break;
            }
        }

        v->getFrame(framebuf[0]);

        for (uint16_t i = 1; i < windows; ++i)
            if (funcs[i])
            {
                funcs[i]((Pixel*)framebuf[0], (Pixel*)framebuf[i], v->width, v->height);
                SDL_Rect r = {i % winside, i / winside, 0, 0};
                r.x *= width;
                r.y *= height;
                if (SDL_BlitSurface(frame[i], NULL, screen, &r) != 0)
                    throw string("Blit failed") + SDL_GetError();
            }

        if (SDL_BlitSurface(frame[0], NULL, screen, NULL) != 0)
            throw string("Blit failed") + SDL_GetError();

        fps[fps_i++ % 5] = (uint16_t)(1/((double)(t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec)/1000000.0));

        this->DrawText((char*)stringify((fps[0] + fps[1] + fps[2] + fps[3] + fps[4]) / 5).c_str(), (SDL_Rect){0,0,0,0}, (SDL_Color){0xff,0xff,0xff,0}, (SDL_Color){0,0,0,0});

        SDL_Flip(screen);

        t2.tv_sec  = t1.tv_sec;
        t2.tv_usec = t1.tv_usec;
    }

    for (uint16_t i = 0; i < windows; ++i)
        SDL_FreeSurface(frame[i]);
}

void MainWin::WriteFrame(const string filename, bool newFrame)
{
    ofstream file(filename.c_str(), ios::out|ios::binary);

    if (newFrame)
        v->getFrame(framebuf[0]);

    file << "P6\n" << v->width << " " << v->height << " 255\n";

    for (uint32_t y = 0; y < v->height; ++y)
        for (uint32_t x = 0; x < v->width; ++x)
        {
            uint32_t off = (y*v->width + x) * 3;
            file << framebuf[0][off] << framebuf[0][off+1] << framebuf[0][off+2];
        }
    file.close();
}

void MainWin::ScreenShot(SDL_Surface *s)
{
    for (uint32_t i = 0; true; ++i)
    {
        int fd = open(string("shot" + stringify(i) + ".ppm").c_str(), O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        if (fd < 0)
        {
            if(errno == EEXIST)
                continue;
            else
                throw string("Could not open screenshot file: ") + strerror(errno);
        }
        FILE *f = fdopen(fd, "w");
        if (!f)
            throw string("Could not get FILE pointer to screenshot file: ") + strerror(errno);

        fprintf(f, "P6\n%i %i 255\n", s->w, s->h);

        Pixel *p = (Pixel*)s->pixels;

        for (int32_t y = 0; y < s->h; ++y)
            for (int32_t x = 0; x < s->w; ++x)
            {
                //uint32_t off = (y*s->w + x) * 3;
                uint32_t off = y*s->w + x;
                //Pixel *px = (Pixel*)&p[y*s->w + x];
                //fprintf(f, "%c%c%c", p[off+2], p[off+1], p[off]);
                fprintf(f, "%c%c%c", p[off].r, p[off].g, p[off].b);
            }
        fclose(f);
        return;
    }
}

void MainWin::AddFilter(FilterFunc f, uint8_t idx)
{
    if (idx > windows-1)
        throw string("Illegal filter func index");

    funcs[idx] = f;
}
/*}}}*/

void copy(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    memcpy(out, in, width * height * sizeof(Pixel));
}

void red(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            const uint32_t off = y*width + x;
            out[off].r = in[off].r;
            out[off].g = 0;
            out[off].b = 0;
        }
}

void green(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            const uint32_t off = y*width + x;
            out[off].r = 0;
            out[off].g = in[off].g;
            out[off].b = 0;
        }
}

void blue(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            const uint32_t off = y*width + x;
            out[off].r = 0;
            out[off].g = 0;
            out[off].b = in[off].b;
        }
}

void better(const Pixel *in, Pixel *out, const uint32_t width, const uint32_t height)
{
    for (uint32_t y = 0; y < height; ++y)
        for (uint32_t x = 0; x < width; ++x)
        {
            const uint32_t off = y*width + x;
            out[off].r = in[off].r;
            out[off].g = in[off].g;
            out[off].b = (uint8_t)(in[off].b*0.75);
        }
}

int main(int argc, char *argv[])
{
    try {
        MainWin win(320, 240, 24, 3);
        win.AddFilter(red,   1);
        win.AddFilter(green, 2);
        win.AddFilter(better,3);
        win.MainLoop();
    } catch (string p) {
        cerr << "Error: " << p << endl;
    }
    return 0;
}

/* vim: set fdm=marker : */

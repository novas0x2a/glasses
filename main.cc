#include <iostream>
#include <string>
#include <limits>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "global.h"
#include "window.h"
#include "overlay.h"

#include "video/v4l.h"
#include "video/staticfile.h"

using namespace std;
using namespace novas0x2a;

int main(int argc, char *argv[])
{
    VideoDevice *v = NULL;
    try {
        Context c("When running " PROGRAM " " VERSION);
        if (argc != 2)
            throw CommandLineError("Usage: glasses <v4l device or ppm file>");

        struct stat st;
        if (stat(argv[1], &st) < 0)
            throw CommandLineError(string("Couldn't stat file: ") + strerror(errno));

        if (S_ISREG(st.st_mode))
            v = new StaticFile(argv[1]);
        else if (S_ISCHR(st.st_mode))
            v = new V4LDevice(argv[1]);
        else
            throw CommandLineError("Usage: glasses <v4l device or ppm file>");

        //TODO: Tied to SDL pixel format definitions
        v->setParams(176, 144, 32, VIDEO_PALETTE_RGB32);

        MainWin win(*v, 14);

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

        win.MainLoop();

    } catch (const CommandLineError &e) {
        cerr << e.message() << endl;
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

    if (v)
        delete v;

    return 0;
}

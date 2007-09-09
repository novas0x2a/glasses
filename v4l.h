#ifndef V4L_H
#define V4L_H

#include <iostream>
#include <linux/types.h>
#include <linux/videodev.h>
#include "global.h"
#include "video.h"

typedef struct video_capability VideoCapability;
typedef struct video_window     VideoWindow;
typedef struct video_picture    VideoPicture;

std::ostream& operator<< (std::ostream& os, const VideoCapability& a);
std::ostream& operator<< (std::ostream& os, const VideoWindow& a);
std::ostream& operator<< (std::ostream& os, const VideoPicture& a);

class V4LDevice : public VideoDevice
{
    public:

        V4LDevice(const char *);
        ~V4LDevice(void);

        friend std::ostream& operator<< (std::ostream &os, const V4LDevice& v);

        void setParams(const uint32_t width, const uint32_t height, const uint16_t palette = VIDEO_PALETTE_RGB32, const uint16_t depth = 0);
        void getFrame(byte *buf);

        const uint16_t getBrightness(void) const;
        const uint16_t getHue(void) const;
        const uint16_t getColour(void) const;
        const uint16_t getContrast(void) const;
        const uint16_t getWhiteness(void) const;

        void setBrightness(const uint16_t);
        void setHue(const uint16_t);
        void setColour(const uint16_t);
        void setContrast(const uint16_t);
        void setWhiteness(const uint16_t);

    protected:
        VideoCapability& getCap(void) const;
        VideoWindow&     getWin(void) const;
        VideoPicture&    getPic(void) const;

        void setWin(const VideoWindow&);
        void setPic(const VideoPicture&);

    private:
        const std::string devname;
        int dev;
};

#endif

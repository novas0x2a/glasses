#ifndef V4L_H
#define V4L_H

#include <iostream>
#include <linux/types.h>
#include <linux/videodev.h>
#include "../global.h"
#include "videodevice.h"
#include <cerrno>

typedef struct video_capability VideoCapability;
typedef struct video_window     VideoWindow;
typedef struct video_picture    VideoPicture;

std::ostream& operator<< (std::ostream& os, const VideoCapability& a);
std::ostream& operator<< (std::ostream& os, const VideoWindow& a);
std::ostream& operator<< (std::ostream& os, const VideoPicture& a);

class V4LDevice : public VideoDevice
{
    public:

        /**
         * Create a V4L camera device
         * @param path  Path to the character device (ex: /dev/video0?)
         */
        explicit V4LDevice(const char *path);
        ~V4LDevice(void);

        friend std::ostream& operator<< (std::ostream &os, const V4LDevice& v);

        void setParams(uint32_t width, uint32_t height, uint16_t depth, uint16_t palette);
        void getFrame(byte *buf);

        uint16_t getBrightness(void) const;
        uint16_t getHue(void) const;
        uint16_t getColour(void) const;
        uint16_t getContrast(void) const;
        uint16_t getWhiteness(void) const;

        void setBrightness(uint16_t);
        void setHue(uint16_t);
        void setColour(uint16_t);
        void setContrast(uint16_t);
        void setWhiteness(uint16_t);

    protected:
        VideoCapability& getCap(void) const;
        VideoWindow&     getWin(void) const;
        VideoPicture&    getPic(void) const;

        void setWin(const VideoWindow&);
        void setPic(const VideoPicture&);

    private:
        explicit V4LDevice(const V4LDevice& original);
        V4LDevice& operator=(const V4LDevice& original);

        const std::string devname;
        int dev;
};

class V4LError : public VideoError
{
    public:
        V4LError(const std::string& our_message) throw ():
            VideoError(our_message + " (" + strerror(errno) + ")") {};
};

#endif

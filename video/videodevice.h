#ifndef VIDEO_H
#define VIDEO_H

#include "../global.h"

class VideoDevice
{
    public:

        virtual ~VideoDevice(void) {};

        virtual void setParams(const uint32_t width, const uint32_t height, const uint16_t palette, const uint16_t depth) = 0;
        virtual void getFrame(byte *buf) = 0;

        virtual const uint16_t getBrightness(void) const = 0;
        virtual const uint16_t getHue(void) const = 0;
        virtual const uint16_t getColour(void) const = 0;
        virtual const uint16_t getContrast(void) const = 0;
        virtual const uint16_t getWhiteness(void) const = 0;

        virtual void setBrightness(const uint16_t) = 0;
        virtual void setHue(const uint16_t) = 0;
        virtual void setColour(const uint16_t) = 0;
        virtual void setContrast(const uint16_t) = 0;
        virtual void setWhiteness(const uint16_t) = 0;

        inline uint32_t getWidth(void)  const {return width;};
        inline uint32_t getHeight(void) const {return height;};
        inline uint32_t getDepth(void)  const {return depth;};

    protected:
        uint32_t width, height;
        uint16_t depth;
};

#endif

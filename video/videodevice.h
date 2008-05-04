#ifndef VIDEODEVICE_H
#define VIDEODEVICE_H

#include "../utils/context.h"
#include "../global.h"

class VideoDevice
{
    public:

        virtual ~VideoDevice(void) {};

        virtual void setParams(uint32_t width, uint32_t height, uint16_t palette, uint16_t depth) = 0;
        virtual void getFrame(byte *buf) = 0;

        virtual uint16_t getBrightness(void) const = 0;
        virtual uint16_t getHue(void)        const = 0;
        virtual uint16_t getColour(void)     const = 0;
        virtual uint16_t getContrast(void)   const = 0;
        virtual uint16_t getWhiteness(void)  const = 0;

        virtual void setBrightness(uint16_t)  = 0;
        virtual void setHue(uint16_t)         = 0;
        virtual void setColour(uint16_t)      = 0;
        virtual void setContrast(uint16_t)    = 0;
        virtual void setWhiteness(uint16_t)   = 0;

        inline uint32_t getWidth(void)  const {return width;};
        inline uint32_t getHeight(void) const {return height;};
        inline uint32_t getDepth(void)  const {return depth;};

    protected:
        uint32_t width, height;
        uint32_t depth;
};

class VideoError : public novas0x2a::Exception
{
    public:
        VideoError(const std::string& our_message) throw ():
            Exception(our_message) {};
};



#endif

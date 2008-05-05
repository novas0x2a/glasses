#ifndef VIDEODEVICE_H
#define VIDEODEVICE_H

#include "../utils/context.h"
#include "../global.h"

class VideoDevice
{
    public:

        VideoDevice() {};
        virtual ~VideoDevice(void) {};

        /**
         * Configure the device
         * @param width     width in pixels
         * @param height    height in pixels
         * @param depth     depth in bpp
         * @param palette   pixel storage format (ie, RGB? BGR? etc)
         */
        virtual void setParams(uint32_t width, uint32_t height, uint16_t depth, uint16_t palette) = 0;

        /**
         * Get a frame from the device
         * @param buf   A buffer to copy the frame into. Needs to
         *              be width*height*depth>>3 bytes large
         */
        virtual void getFrame(byte *buf) = 0;

        // Getters and setters for various video parameters
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
        uint32_t width, height, depth;

    private:
        // Do not allow copying or assignment
        explicit VideoDevice(const VideoDevice& original);
        VideoDevice& operator=(const VideoDevice& original);
};

class VideoError : public novas0x2a::Exception
{
    public:
        VideoError(const std::string& our_message) throw ():
            Exception(our_message) {};
};



#endif

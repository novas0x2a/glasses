#ifndef STATICFILE_H
#define STATICFILE_H

#include <cerrno>
#include "videodevice.h"

class StaticFile : public VideoDevice
{
    public:

        /**
         * Create a StaticFile "device"
         * @param path  Path to the file. Must be a P6-format PPM sans comments.
         */
        explicit StaticFile(const char *path);
        ~StaticFile(void);

        void setParams(uint32_t width, uint32_t height, uint16_t depth, uint16_t palette);
        void getFrame(byte *buf);

        uint16_t getBrightness(void) const;
        uint16_t getHue(void)        const;
        uint16_t getColour(void)     const;
        uint16_t getContrast(void)   const;
        uint16_t getWhiteness(void)  const;

        void setBrightness(uint16_t);
        void setHue(uint16_t);
        void setColour(uint16_t);
        void setContrast(uint16_t);
        void setWhiteness(uint16_t);

    private:
        Pixel *image;
        // Parameters about the image, so we don't blow out of the buffer
        uint32_t image_width, image_height, image_depth;

        explicit StaticFile(const StaticFile& original);
        StaticFile& operator=(const StaticFile& original);
};

class StaticFileError : public VideoError
{
    public:
        StaticFileError(const std::string& our_message) throw ():
            VideoError(our_message + " (" + strerror(errno) + ")") {};
};

#endif

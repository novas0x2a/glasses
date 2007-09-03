#!/usr/bin/env python

from __future__ import with_statement

from ctypes import *
from fcntl import ioctl
from time import sleep
import sys
from sys import stderr
import pygame
from array import array

VIDIOCGCAP       = 0x803c7601
#VIDIOCGCHAN      = 0xc0307602
#VIDIOCSCHAN      = 0x40307603
#VIDIOCGTUNER     = 0xc0407604
#VIDIOCSTUNER     = 0x40407605
VIDIOCGPICT      = 0x800e7606
VIDIOCSPICT      = 0x400e7607
#VIDIOCCAPTURE    = 0x40047608
VIDIOCGWIN       = 0x80287609
VIDIOCSWIN       = 0x4028760a
#VIDIOCGFBUF      = 0x8018760b
#VIDIOCSFBUF      = 0x4018760c
#VIDIOCKEY        = 0x800c760d
#VIDIOCGFREQ      = 0x8008760e
#VIDIOCSFREQ      = 0x4008760f
#VIDIOCGAUDIO     = 0x80287610
#VIDIOCSAUDIO     = 0x40287611
#VIDIOCSYNC       = 0x40047612
#VIDIOCMCAPTURE   = 0x40107613
#VIDIOCGMBUF      = 0x80887614
#VIDIOCGUNIT      = 0x80147615
#VIDIOCGCAPTURE   = 0x80147616
#VIDIOCSCAPTURE   = 0x40147617
#VIDIOCSPLAYMODE  = 0x400c7618
#VIDIOCSWRITEMODE = 0x40047619
#VIDIOCGPLAYINFO  = 0x8118761a
#VIDIOCSMICROCODE = 0x4020761b
#VIDIOCGVBIFMT    = 0x8020761c
#VIDIOCSVBIFMT    = 0x4020761d

class VideoCapability(Structure):
    VID_TYPES = ['capture', 'tuner', 'teletext', 'overlay', 'chromakey', 'clipping', 'frameram', 'scales', 'monochrome', 'subcapture', 'mpeg_decoder', 'mpeg_encoder', 'mjpeg_decoder', 'mjpeg_encoder']
    _fields_ = [
        ('name',        c_char * 32),
        ('_type',       c_int),
        ('channels',    c_int),
        ('audios',      c_int),
        ('maxwidth',    c_int),
        ('maxheight',   c_int),
        ('minwidth',    c_int),
        ('minheight',   c_int),
    ]

    def __setattr__(self, name, value):
        raise AttributeError('Read-only attribute')

    def gettype(self):
        return [vid_type for n, vid_type in enumerate(self.VID_TYPES) if self._type & 2**n]
    type = property(gettype, None)

    def __str__(self):
        return '''Name[%s] Capabilities[%s] Channels[%i] Audios[%i] MaxWidth[%i] MaxHeight[%i] MinWidth[%i] MinHeight[%s]''' % \
                (self.name, ','.join(self.type), self.channels, self.audios, self.maxwidth, self.maxheight, self.minwidth, self.minheight)

class VideoPicture(Structure):
    PALETTE_TYPES = ['grey', 'hi240', 'rgb565', 'rgb24', 'rgb32', 'rgb555', 'yuv422', 'yuyv', 'uyvy', 'yuv420', 'yuv411', 'raw', 'yuv422p', 'yuv411p', 'yuv420p', 'yuv410p']
    _fields_ = [
        ('brightness' , c_ushort),
        ('hue'        , c_ushort),
        ('colour'     , c_ushort),
        ('contrast'   , c_ushort),
        ('whiteness'  , c_ushort),
        ('depth'      , c_ushort),
        ('_palette'   , c_ushort),
    ]

    def getpal(self):
        return self.PALETTE_TYPES[self._palette]
    def setpal(self, pal):
        if pal in self.PALETTE_TYPES:
            self._palette = self.PALETTE_TYPES.index(pal)
        else:
            raise AttributeError('%s is not a valid palette. Try one of %s' % (pal, self.PALETTE_TYPES))
    palette = property(getpal, setpal)

    @property
    def color(self):
        return self.colour

    def __str__(self):
        return '''Brightness[%u] Hue[%u] Color[%u] Contrast[%u] Whiteness[%u] Depth[%u] Palette[%s] ''' % \
                (self.brightness, self.hue, self.color, self.contrast, self.whiteness, self.depth, self.palette)

class Pixel(Structure):
    _fields_ = [
        ('r', c_byte),
        ('g', c_byte),
        ('b', c_byte)
    ]
    def __str__(self):
        return 'Pixel[%02x,%02x,%02x]' % (self.r, self.g, self.b)

class VideoWindow(Structure):
    _fields_ = [
        ('x',           c_uint),
        ('y',           c_uint),
        ('width',       c_uint),
        ('height',      c_uint),
        ('chromakey',   c_uint),
        ('flags',       c_uint),
        ('clips',       c_void_p),
        ('clipcount',   c_int),
    ]
    def __init__(self, r = 0, g = 0, b = 0):
        self.r = r
        self.g = g
        self.b = b
    def __str__(self):
        return '''Pos[%i,%i] Size[%i,%i]''' % (self.x, self.y, self.width, self.height)


class VideoDevice(object):
    def __init__(self, devname):
        self.name = devname
        self.f    = open(devname, 'rb')
        sleep(0.4)
    def __del__(self):
        self.f.close()

    def _iord(self, iid, obj):
        ioctl(self.f.fileno(), iid, obj, 1)
        return obj
    def _iowr(self, iid, obj):
        ioctl(self.f.fileno(), iid, obj, 0)

    def GetCaps(self):
        return self._iord(VIDIOCGCAP, VideoCapability())

    def GetWindow(self):
        return self._iord(VIDIOCGWIN, VideoWindow())
    def SetWindow(self, win):
        if type(win) != VideoWindow:
            raise TypeError('You must pass in a window')
        return self._iowr(VIDIOCSWIN, win)

    def GetPicture(self):
        return self._iord(VIDIOCGPICT, VideoPicture())
    def SetPicture(self, obj):
        if type(obj) != VideoPicture:
            raise TypeError('You must pass in a window')
        return self._iowr(VIDIOCSPICT, obj)

    def GetFrame(self):
        w = self.GetWindow()
        p = self.GetPicture()
        return (array('B', self.f.read(w.width * w.height * p.depth)), (w.width, w.height, p.depth))

def trace(*args):
    print args[0].f_code
#sys.settrace(trace)

class MainWin(object):
    def __init__(self, width=320, height=240):
        pygame.init()
        self.width  = width
        self.height = height
        self.screen = pygame.display.set_mode((self.width, self.height))
        self.v = VideoDevice('/dev/video0')
        p = self.v.GetPicture()
        p.palette = 'rgb24'
        p.depth   = 24
        self.v.SetPicture(p)
    def MainLoop(self):
        while 1:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    sys.exit()
            data,(width,height,depth) = self.v.GetFrame()
            self.screen.lock()
            for x in xrange(width):
                for y in xrange(height):
                    off = (y * height + x) * 3
                    self.screen.set_at((x,y), self.screen.map_rgb((data[off], data[off+1], data[off+2])))
            self.screen.unlock()
            pygame.display.flip()


if __name__ == "__main__":
    Win = MainWin()
    Win.MainLoop()

#import time
##
#v = VideoDevice('/dev/video0')
#time.sleep(1)
##
#p = v.GetPicture()
#p.palette = 'rgb24'
#p.depth   = 24
#v.SetPicture(p)
#
##
#print >>stderr, '%s\n%s\n%s' % (v.GetCaps(), v.GetWindow(), v.GetPicture())
##
#data,(width,height,depth) = v.GetFrame()
##print >>stderr, '%i,%i' % (len(data), width*height*depth)
##
#sys.stdout.write('P6\n%d %d 255\n' % (width, height))
#
#for i in xrange(width * height):
#    sys.stdout.write('%c%c%c' % (data[i*3], data[i*3+1], data[i*3+2]))

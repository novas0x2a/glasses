PROGRAM  := glasses
VERSION  := 0.02-pre

DISTFILES := doc/README doc/TODO shots/scary.jpg shots/subtle.jpg Makefile c.mk

CC           := g++
glasses_SRC  := main.cc video/v4l.cc window.cc utils/context.cc
HEADERS      := global.h video/v4l.h video/videodevice.h window.h hist.h
FLAGS        := `pkg-config --cflags-only-other sdl` $(subst -I,-isystem,$(shell pkg-config --cflags-only-I sdl))
LIBS         := `pkg-config --libs sdl` -lSDL_ttf
DEBUG        := y

PROGS    := $(PROGRAM)
include c.mk

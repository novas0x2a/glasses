PROGRAM  := glasses
VERSION  := 0.02-pre

DISTFILES := doc/README doc/TODO doc/scary.jpg doc/subtle.jpg Makefile c.mk

CC           := g++
glasses_SRC  := $(wildcard *.cc video/*.cc utils/*.cc)
HEADERS      := $(wildcard *.h video/*.h utils/*.h)
LIBS         := -lSDL_ttf
PKGS         := sdl
DEBUG        := y

PROGS    := $(PROGRAM)
include c.mk

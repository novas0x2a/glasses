PROGRAM  := glasses
VERSION  := 0.04

DISTFILES := doc/* Makefile c.mk Vera.ttf Makefile.old

CC           := g++
glasses_SRC  := $(wildcard *.cc video/*.cc utils/*.cc)
HEADERS      := $(wildcard *.h video/*.h utils/*.h) overlay.hpp
LIBS         := -lSDL_ttf
PKGS         := sdl
DEBUG        := y
PROFILE      := n

PROGS    := $(PROGRAM)
include c.mk

PROGRAM  := glasses
VERSION  := 0.01

SRC      := main.cc v4l.cc window.cc
HEADERS  := global.h v4l.h video.h window.h
CXXFLAGS := `pkg-config --cflags-only-other sdl` $(subst -I,-isystem,$(shell pkg-config --cflags-only-I sdl)) -Wall -Wextra -Wno-unused-parameter  -DVERSION="\"$(VERSION)\""
LDFLAGS  := `pkg-config --libs sdl` -lSDL_ttf

DBGFLAGS := -g3 -ggdb -DPROGRAM="\"$(PROGRAM)-debug\""
RELFLAGS := -O3 -DPROGRAM="\"$(PROGRAM)\""

all:   $(PROGRAM)
debug: $(PROGRAM)-debug

include .dep

$(PROGRAM): $(SRC:cc=o)
	g++ $(CXXFLAGS) $(RELFLAGS) $^ -o $@ $(LDFLAGS)

$(PROGRAM)-debug: $(SRC:.cc=-debug.o)
	g++ $(CXXFLAGS) $(DBGFLAGS) $^ -o $@ $(LDFLAGS)

%.o : %.cc
	g++ $(CXXFLAGS) $(RELFLAGS) $(filter %.cc,$^) -c -o $@

%-debug.o : %.cc
	g++ $(CXXFLAGS) $(DBGFLAGS)  $(filter %.cc,$^) -c -o $@

clean:
	rm -f $(PROGRAM) $(PROGRAM)-debug *.o

.dep: $(SRC)
	g++ -MM $(CXXFLAGS) $^ > $@

distclean: clean
	rm -f .dep $(PROGRAM)-$(VERSION).tar.gz

dist: $(SRC) $(HEADERS)
	tar czf $(PROGRAM)-$(VERSION).tar.gz --transform s,^,$(PROGRAM)-$(VERSION)/, $(DISTFILES) $(sort $(filter-out %.o:,$(shell g++ -MM $(CXXFLAGS) $(filter %.cc,$^))))

.PHONY: all clean dist distclean

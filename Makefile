PROG     := grab
SRC      := main.cc v4l.cc window.cc
CXXFLAGS := `pkg-config --cflags sdl` -Wall -Wextra -Wno-unused-parameter
LDFLAGS  := `pkg-config --libs sdl` -lSDL_ttf

DBGFLAGS := -g3 -ggdb
RELFLAGS := -O3

all: $(PROG)
debug: $(PROG)-debug

$(PROG): $(SRC:cc=o)
	g++ $(CXXFLAGS) $(RELFLAGS) $^ -o $@ $(LDFLAGS)

$(PROG)-debug: $(SRC:.cc=-debug.o)
	g++ $(CXXFLAGS) $(DBGFLAGS) $^ -o $@ $(LDFLAGS)

%.o : %.cc
	g++ $(CXXFLAGS) $(RELFLAGS) $^ -c -o $@

%-debug.o : %.cc
	g++ $(CXXFLAGS) $(DBGFLAGS)  $^ -c -o $@

clean:
	rm -f $(PROG) *.o

.PHONY: all clean

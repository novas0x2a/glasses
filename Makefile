
FLAGS := `pkg-config --cflags sdl` -Wall -Wextra -Wno-unused-parameter -g -ggdb
LIBS  := `pkg-config --libs sdl`

all:
	g++ $(FLAGS) grab.cc -o grab $(LIBS)

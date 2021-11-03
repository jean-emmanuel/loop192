CXX = g++
CXXFLAGS = -g -O0 $(shell pkg-config --cflags liblo jack alsa gtkmm-3.0) -Wall #-Wextra
LDFLAGS = $(shell pkg-config --libs liblo jack alsa gtkmm-3.0)
SOURCES = $(wildcard src/*.cpp)
OBJ = $(SOURCES:.cpp=.o)
PROG = midilooper

all: $(PROG)

$(PROG): $(OBJ)
	$(info Edition des liens)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(info Compilation de $^ vers $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	-rm $(OBJ)
	-rm $(PROG)

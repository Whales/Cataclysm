# comment these to toggle them as one sees fit.
# WARNINGS will spam hundreds of warnings, mostly safe, if turned on
# DEBUG is best turned on if you plan to debug in gdb -- please do!
# PROFILE is for use with gprof or a similar program -- don't bother generally
#WARNINGS = -Wall
#DEBUG = -g -ggdb
#PROFILE = -pg
OPTIMIZE = -O3 -Os

ODIR = obj
DDIR = .deps

TARGET = cataclysm

OS  = $(shell uname -s)
CXX = g++

CFLAGS = $(WARNINGS) $(DEBUG) $(PROFILE)

ifeq ($(OS), MINGW32_NT-5.1)
CFLAGS = $(WARNINGS) $(OPTIMIZE) $(DEBUG) $(PROFILE) -D__TILESET
LDFLAGS = -static -lpdcurses -lSDL -Wl,-stack,12000000,-subsystem,windows
else 
LDFLAGS = -lncurses -Wl,-stack,12000000,-subsystem,windows
endif

SOURCES = $(wildcard *.cpp)
_OBJS = $(SOURCES:.cpp=.o)
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

all: $(TARGET)
	@

$(TARGET): $(ODIR) $(DDIR) $(OBJS)
	$(CXX) -o $(TARGET) $(CFLAGS) $(OBJS) $(LDFLAGS) 

$(ODIR):
	mkdir $(ODIR)

$(DDIR):
	@mkdir $(DDIR)

$(ODIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(ODIR)/*.o

-include $(SOURCES:%.cpp=$(DEPDIR)/%.P)

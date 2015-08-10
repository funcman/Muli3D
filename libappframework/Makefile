# AppFramework Makefile for Linux/X11

SHELL    = /bin/sh
DEFINES  = -DLINUX_X11
CFLAGS   = -Wall $(DEFINES) -O3 -fomit-frame-pointer -funroll-all-loops -ffast-math -march=pentium-mmx
CPP      = g++
SH       = /bin/sh
ARADD    = ar rc
RANLIB   = ranlib
RM       = /bin/rm -f
INCLUDES = -I/usr/X11R6/include -I/usr/local/include -I/usr/include
CTARGETS = src/application.cpp src/camera.cpp src/fileio.cpp src/graphics.cpp src/input.cpp src/resmanager.cpp src/scene.cpp src/stateblock.cpp
OTARGETS = $(CTARGETS:.cpp=.o)
LIBRARY  = lib/libappframework.a

$(LIBRARY): $(OTARGETS)
	$(ARADD) $(LIBRARY) $(OTARGETS)
	$(RANLIB) $(LIBRARY)

%.o: %.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(LIBRARY) $(OTARGETS)

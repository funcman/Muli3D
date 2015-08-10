# Sample Makefile for Linux/X11

SHELL    = /bin/sh
DEFINES  = -DLINUX_X11
CFLAGS   = -Wall $(DEFINES) -O3 -fomit-frame-pointer -funroll-all-loops -ffast-math -march=pentium-mmx
CPP      = g++
SH       = /bin/sh
LDFLAGS	 = 
LD       = g++
STRIP    = strip
RM       = /bin/rm -f
INCLUDES = -I/usr/X11R6/include -I/usr/local/include -I/usr/include
LIBS     = -lm -lpng -L/usr/X11R6/lib -lX11 -L../libappframework/lib -lappframework -L../libmuli3d/lib -lmuli3d
CTARGETS = main.cpp mycamera.cpp app.cpp board.cpp
OTARGETS = $(CTARGETS:.cpp=.o)
EXECUTABLE  = checkerboard

$(EXECUTABLE): $(OTARGETS)
	$(LD) $(LDFLAGS) $(OTARGETS) $(LIBS) -o $(EXECUTABLE)
	$(STRIP) --strip-all $(EXECUTABLE)
	$(STRIP) --remove-section=".comment" $(EXECUTABLE)
	$(STRIP) --remove-section=".note" $(EXECUTABLE)

%.o: %.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OTARGETS)

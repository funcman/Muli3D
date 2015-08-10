# Muli3D Makefile for Linux/X11

SHELL    = /bin/sh
DEFINES  = -DLINUX_X11
CFLAGS   = -Wall $(DEFINES) -O3 -fomit-frame-pointer -funroll-all-loops -ffast-math -march=pentium-mmx
CPP      = g++
SH       = /bin/sh
ARADD    = ar rc
RANLIB   = ranlib
RM       = /bin/rm -f
INCLUDES = -I/usr/X11R6/include -I/usr/local/include -I/usr/include
CTARGETS = src/core/m3dcore.cpp src/core/m3dcore_baseshader.cpp src/core/m3dcore_basetexture.cpp src/core/m3dcore_cubetexture.cpp src/core/m3dcore_device.cpp src/core/m3dcore_indexbuffer.cpp src/core/m3dcore_presenttarget.cpp src/core/m3dcore_rendertarget.cpp src/core/m3dcore_shaders.cpp src/core/m3dcore_surface.cpp src/core/m3dcore_texture.cpp src/core/m3dcore_vertexbuffer.cpp src/core/m3dcore_vertexformat.cpp src/core/m3dcore_volume.cpp src/core/m3dcore_volumetexture.cpp src/math/m3dmath_matrix44.cpp src/math/m3dmath_vector4.cpp src/math/m3dmath_quaternion.cpp
OTARGETS = $(CTARGETS:.cpp=.o)
LIBRARY  = lib/libmuli3d.a

$(LIBRARY): $(OTARGETS)
	$(ARADD) $(LIBRARY) $(OTARGETS)
	$(RANLIB) $(LIBRARY)

%.o: %.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(LIBRARY) $(OTARGETS)

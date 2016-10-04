

bin_PROGRAMS = eacsmb

eacsmb_SOURCES = main.cpp \
	window.cpp

eacsmb_LDADD =  \
	-lm \
	-lX11 -lfreetype -lfontconfig -lpng \
	-lGL -lGLU -lGLEW
	 
AM_CFLAGS = "-I/usr/include/freetype2" -I../libs -g -DLINUX -Wno-unused-result
AM_CXXFLAGS = $(AM_CFLAGS)




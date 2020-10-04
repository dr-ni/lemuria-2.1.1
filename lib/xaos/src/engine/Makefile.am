CC     = @CC@
AM_CFLAGS = @CFLAGS1@
LIBS   = @LIBS@ -lm
LFLAGS = @LDFLAGS@
AR     = @AR@
RANLIB = @RANLIB@

noinst_LTLIBRARIES=libengine.la

libengine_la_SOURCES = formulas.c \
       fractal.c  \
       btrace.c \
       palettef.c \
       emboss.c \
       star.c \
       anti.c \
       dither.c \
       edge.c \
       edge2.c \
       rotate.c \
       zoom.c \
       blur.c \
       interlace.c \
       itersmall.c \
       stereogram.c \
       3d.c \
       subwindow.c \
       plane.c \
       julia.c \
       i386.c

EXTRA_DIST = docalc.c 3dd.c btraced.c edged.c edge2d.c paletted.c rotated.c \
stereod.c stard.c zoomd.c

noinst_HEADERS = calculate.h \
julia.h


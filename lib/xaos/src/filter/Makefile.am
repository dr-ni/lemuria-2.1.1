CC     = @CC@
AM_CFLAGS = @CFLAGS2@
LIBS   = @LIBS@ -lm
LFLAGS = @LDFLAGS@
AR     = @AR@
RANLIB = @RANLIB@

noinst_LTLIBRARIES = libfilter.la

libfilter_la_SOURCES = image.c \
       palette.c \
       random.c \
       grlib.c \
       font.c \
       filter.c

EXTRA_DIST = grlibd.c


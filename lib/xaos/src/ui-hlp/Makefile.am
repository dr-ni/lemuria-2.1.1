CC     = @CC@
AM_CFLAGS = @CFLAGS2@
LIBS   = @LIBS@ -lm
LFLAGS = @LDFLAGS@
AR     = @AR@
RANLIB = @RANLIB@

noinst_LTLIBRARIES = libui_hlp.la

libui_hlp_la_SOURCES = autopilot.c \
       ui_helper.c \
       menu.c \
       play.c \
       messg.c \
       render.c \
       playtext.c \
       save.c \
       wstack.c 

noinst_HEADERS = autopilot.h \
play.h

EXTRA_DIST = autod.c

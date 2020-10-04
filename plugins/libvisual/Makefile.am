libdir = @LV_PLUGIN_DIR@
AM_LDFLAGS = -avoid-version -module

AM_CFLAGS = @LV_CFLAGS@ -DLOCALE_DIR=\"$(localedir)\"

lib_LTLIBRARIES = \
libactor_lemuria.la

libactor_lemuria_la_SOURCES  = libactor_lemuria.c
libactor_lemuria_la_LIBADD  = @LV_LIBS@ $(top_srcdir)/lib/liblemuria.la

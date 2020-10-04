libdir=@GMERLIN_PLUGIN_DIR@
AM_LDFLAGS = @GMERLIN_PLUGIN_LDFLAGS@ -avoid-version -module

AM_CFLAGS = @GMERLIN_CFLAGS@ -DLOCALE_DIR=\"$(localedir)\"

lib_LTLIBRARIES = \
vis_lemuria.la

vis_lemuria_la_SOURCES  = vis_lemuria.c
vis_lemuria_la_LIBADD  = @GMERLIN_LIBS@ $(top_srcdir)/lib/liblemuria.la
